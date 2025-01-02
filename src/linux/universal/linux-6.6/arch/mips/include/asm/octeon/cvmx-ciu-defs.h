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
 * cvmx-ciu-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon ciu.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_CIU_DEFS_H__
#define __CVMX_CIU_DEFS_H__

#define CVMX_CIU_BIST (CVMX_ADD_IO_SEG(0x0001070000000730ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_BLOCK_INT CVMX_CIU_BLOCK_INT_FUNC()
static inline uint64_t CVMX_CIU_BLOCK_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_CIU_BLOCK_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010700000007C0ull);
}
#else
#define CVMX_CIU_BLOCK_INT (CVMX_ADD_IO_SEG(0x00010700000007C0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_CIB_L2C_ENX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0)))))
		cvmx_warn("CVMX_CIU_CIB_L2C_ENX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000E100ull);
}
#else
#define CVMX_CIU_CIB_L2C_ENX(offset) (CVMX_ADD_IO_SEG(0x000107000000E100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_CIB_L2C_RAWX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0)))))
		cvmx_warn("CVMX_CIU_CIB_L2C_RAWX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000E000ull);
}
#else
#define CVMX_CIU_CIB_L2C_RAWX(offset) (CVMX_ADD_IO_SEG(0x000107000000E000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_CIB_LMCX_ENX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_CIU_CIB_LMCX_ENX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000107000000E300ull);
}
#else
#define CVMX_CIU_CIB_LMCX_ENX(offset, block_id) (CVMX_ADD_IO_SEG(0x000107000000E300ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_CIB_LMCX_RAWX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_CIU_CIB_LMCX_RAWX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000107000000E200ull);
}
#else
#define CVMX_CIU_CIB_LMCX_RAWX(offset, block_id) (CVMX_ADD_IO_SEG(0x000107000000E200ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_CIB_OCLAX_ENX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_CIU_CIB_OCLAX_ENX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000107000000EE00ull);
}
#else
#define CVMX_CIU_CIB_OCLAX_ENX(offset, block_id) (CVMX_ADD_IO_SEG(0x000107000000EE00ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_CIB_OCLAX_RAWX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_CIU_CIB_OCLAX_RAWX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000107000000EC00ull);
}
#else
#define CVMX_CIU_CIB_OCLAX_RAWX(offset, block_id) (CVMX_ADD_IO_SEG(0x000107000000EC00ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_CIB_RST_ENX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0)))))
		cvmx_warn("CVMX_CIU_CIB_RST_ENX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000E500ull);
}
#else
#define CVMX_CIU_CIB_RST_ENX(offset) (CVMX_ADD_IO_SEG(0x000107000000E500ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_CIB_RST_RAWX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0)))))
		cvmx_warn("CVMX_CIU_CIB_RST_RAWX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000E400ull);
}
#else
#define CVMX_CIU_CIB_RST_RAWX(offset) (CVMX_ADD_IO_SEG(0x000107000000E400ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_CIB_SATA_ENX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0)))))
		cvmx_warn("CVMX_CIU_CIB_SATA_ENX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000E700ull);
}
#else
#define CVMX_CIU_CIB_SATA_ENX(offset) (CVMX_ADD_IO_SEG(0x000107000000E700ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_CIB_SATA_RAWX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0)))))
		cvmx_warn("CVMX_CIU_CIB_SATA_RAWX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000E600ull);
}
#else
#define CVMX_CIU_CIB_SATA_RAWX(offset) (CVMX_ADD_IO_SEG(0x000107000000E600ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_CIB_USBDRDX_ENX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_CIU_CIB_USBDRDX_ENX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000107000000EA00ull) + ((block_id) & 1) * 0x100ull;
}
#else
#define CVMX_CIU_CIB_USBDRDX_ENX(offset, block_id) (CVMX_ADD_IO_SEG(0x000107000000EA00ull) + ((block_id) & 1) * 0x100ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_CIB_USBDRDX_RAWX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_CIU_CIB_USBDRDX_RAWX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000107000000E800ull) + ((block_id) & 1) * 0x100ull;
}
#else
#define CVMX_CIU_CIB_USBDRDX_RAWX(offset, block_id) (CVMX_ADD_IO_SEG(0x000107000000E800ull) + ((block_id) & 1) * 0x100ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_DINT CVMX_CIU_DINT_FUNC()
static inline uint64_t CVMX_CIU_DINT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000720ull);
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return CVMX_ADD_IO_SEG(0x0001010000000180ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return CVMX_ADD_IO_SEG(0x0001010000000180ull);
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001010000000180ull);
			break;
	}
	cvmx_warn("CVMX_CIU_DINT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000180ull);
}
#else
#define CVMX_CIU_DINT CVMX_CIU_DINT_FUNC()
static inline uint64_t CVMX_CIU_DINT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000720ull);
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001010000000180ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001010000000180ull);
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001010000000180ull);
	}
	return CVMX_ADD_IO_SEG(0x0001010000000180ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_EN2_IOX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_CIU_EN2_IOX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000A600ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_CIU_EN2_IOX_INT(offset) (CVMX_ADD_IO_SEG(0x000107000000A600ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_EN2_IOX_INT_W1C(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_CIU_EN2_IOX_INT_W1C(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000CE00ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_CIU_EN2_IOX_INT_W1C(offset) (CVMX_ADD_IO_SEG(0x000107000000CE00ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_EN2_IOX_INT_W1S(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_CIU_EN2_IOX_INT_W1S(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000AE00ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_CIU_EN2_IOX_INT_W1S(offset) (CVMX_ADD_IO_SEG(0x000107000000AE00ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_EN2_PPX_IP2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_EN2_PPX_IP2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000A000ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_EN2_PPX_IP2(offset) (CVMX_ADD_IO_SEG(0x000107000000A000ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_EN2_PPX_IP2_W1C(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_EN2_PPX_IP2_W1C(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000C800ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_EN2_PPX_IP2_W1C(offset) (CVMX_ADD_IO_SEG(0x000107000000C800ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_EN2_PPX_IP2_W1S(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_EN2_PPX_IP2_W1S(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000A800ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_EN2_PPX_IP2_W1S(offset) (CVMX_ADD_IO_SEG(0x000107000000A800ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_EN2_PPX_IP3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_EN2_PPX_IP3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000A200ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_EN2_PPX_IP3(offset) (CVMX_ADD_IO_SEG(0x000107000000A200ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_EN2_PPX_IP3_W1C(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_EN2_PPX_IP3_W1C(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000CA00ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_EN2_PPX_IP3_W1C(offset) (CVMX_ADD_IO_SEG(0x000107000000CA00ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_EN2_PPX_IP3_W1S(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_EN2_PPX_IP3_W1S(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000AA00ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_EN2_PPX_IP3_W1S(offset) (CVMX_ADD_IO_SEG(0x000107000000AA00ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_EN2_PPX_IP4(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_EN2_PPX_IP4(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000A400ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_EN2_PPX_IP4(offset) (CVMX_ADD_IO_SEG(0x000107000000A400ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_EN2_PPX_IP4_W1C(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_EN2_PPX_IP4_W1C(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000CC00ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_EN2_PPX_IP4_W1C(offset) (CVMX_ADD_IO_SEG(0x000107000000CC00ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_EN2_PPX_IP4_W1S(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_EN2_PPX_IP4_W1S(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000107000000AC00ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_EN2_PPX_IP4_W1S(offset) (CVMX_ADD_IO_SEG(0x000107000000AC00ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_FUSE CVMX_CIU_FUSE_FUNC()
static inline uint64_t CVMX_CIU_FUSE_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000728ull);
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return CVMX_ADD_IO_SEG(0x00010100000001A0ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return CVMX_ADD_IO_SEG(0x00010100000001A0ull);
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00010100000001A0ull);
			break;
	}
	cvmx_warn("CVMX_CIU_FUSE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010100000001A0ull);
}
#else
#define CVMX_CIU_FUSE CVMX_CIU_FUSE_FUNC()
static inline uint64_t CVMX_CIU_FUSE_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000728ull);
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x00010100000001A0ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x00010100000001A0ull);
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00010100000001A0ull);
	}
	return CVMX_ADD_IO_SEG(0x00010100000001A0ull);
}
#endif
#define CVMX_CIU_GSTOP (CVMX_ADD_IO_SEG(0x0001070000000710ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_INT33_SUM0 CVMX_CIU_INT33_SUM0_FUNC()
static inline uint64_t CVMX_CIU_INT33_SUM0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_CIU_INT33_SUM0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000000110ull);
}
#else
#define CVMX_CIU_INT33_SUM0 (CVMX_ADD_IO_SEG(0x0001070000000110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_INTR_SLOWDOWN CVMX_CIU_INTR_SLOWDOWN_FUNC()
static inline uint64_t CVMX_CIU_INTR_SLOWDOWN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_CIU_INTR_SLOWDOWN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010700000007D0ull);
}
#else
#define CVMX_CIU_INTR_SLOWDOWN (CVMX_ADD_IO_SEG(0x00010700000007D0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_INTX_EN0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN30XX) && ((offset <= 1) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN31XX) && ((offset <= 3) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN38XX) && ((offset <= 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN50XX) && ((offset <= 3) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 7) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 23) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 11) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 19) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33))))))
		cvmx_warn("CVMX_CIU_INTX_EN0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000200ull) + ((offset) & 63) * 16;
}
#else
#define CVMX_CIU_INTX_EN0(offset) (CVMX_ADD_IO_SEG(0x0001070000000200ull) + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_INTX_EN0_W1C(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 7) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 23) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 11) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 19) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33))))))
		cvmx_warn("CVMX_CIU_INTX_EN0_W1C(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000002200ull) + ((offset) & 63) * 16;
}
#else
#define CVMX_CIU_INTX_EN0_W1C(offset) (CVMX_ADD_IO_SEG(0x0001070000002200ull) + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_INTX_EN0_W1S(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 7) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 23) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 11) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 19) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33))))))
		cvmx_warn("CVMX_CIU_INTX_EN0_W1S(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000006200ull) + ((offset) & 63) * 16;
}
#else
#define CVMX_CIU_INTX_EN0_W1S(offset) (CVMX_ADD_IO_SEG(0x0001070000006200ull) + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_INTX_EN1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN30XX) && ((offset <= 1) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN31XX) && ((offset <= 3) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN38XX) && ((offset <= 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN50XX) && ((offset <= 3) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 7) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 23) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 11) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 19) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33))))))
		cvmx_warn("CVMX_CIU_INTX_EN1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000208ull) + ((offset) & 63) * 16;
}
#else
#define CVMX_CIU_INTX_EN1(offset) (CVMX_ADD_IO_SEG(0x0001070000000208ull) + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_INTX_EN1_W1C(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 7) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 23) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 11) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 19) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33))))))
		cvmx_warn("CVMX_CIU_INTX_EN1_W1C(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000002208ull) + ((offset) & 63) * 16;
}
#else
#define CVMX_CIU_INTX_EN1_W1C(offset) (CVMX_ADD_IO_SEG(0x0001070000002208ull) + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_INTX_EN1_W1S(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 7) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 23) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 11) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 19) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7) || ((offset >= 32) && (offset <= 33))))))
		cvmx_warn("CVMX_CIU_INTX_EN1_W1S(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000006208ull) + ((offset) & 63) * 16;
}
#else
#define CVMX_CIU_INTX_EN1_W1S(offset) (CVMX_ADD_IO_SEG(0x0001070000006208ull) + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_INTX_EN4_0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN50XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 11))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_INTX_EN4_0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000C80ull) + ((offset) & 15) * 16;
}
#else
#define CVMX_CIU_INTX_EN4_0(offset) (CVMX_ADD_IO_SEG(0x0001070000000C80ull) + ((offset) & 15) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_INTX_EN4_0_W1C(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 11))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_INTX_EN4_0_W1C(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000002C80ull) + ((offset) & 15) * 16;
}
#else
#define CVMX_CIU_INTX_EN4_0_W1C(offset) (CVMX_ADD_IO_SEG(0x0001070000002C80ull) + ((offset) & 15) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_INTX_EN4_0_W1S(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 11))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_INTX_EN4_0_W1S(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000006C80ull) + ((offset) & 15) * 16;
}
#else
#define CVMX_CIU_INTX_EN4_0_W1S(offset) (CVMX_ADD_IO_SEG(0x0001070000006C80ull) + ((offset) & 15) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_INTX_EN4_1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN50XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 11))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_INTX_EN4_1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000C88ull) + ((offset) & 15) * 16;
}
#else
#define CVMX_CIU_INTX_EN4_1(offset) (CVMX_ADD_IO_SEG(0x0001070000000C88ull) + ((offset) & 15) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_INTX_EN4_1_W1C(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 11))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_INTX_EN4_1_W1C(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000002C88ull) + ((offset) & 15) * 16;
}
#else
#define CVMX_CIU_INTX_EN4_1_W1C(offset) (CVMX_ADD_IO_SEG(0x0001070000002C88ull) + ((offset) & 15) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_INTX_EN4_1_W1S(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 11))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_INTX_EN4_1_W1S(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000006C88ull) + ((offset) & 15) * 16;
}
#else
#define CVMX_CIU_INTX_EN4_1_W1S(offset) (CVMX_ADD_IO_SEG(0x0001070000006C88ull) + ((offset) & 15) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_INTX_SUM0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN30XX) && ((offset <= 1) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN31XX) && ((offset <= 3) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN38XX) && ((offset <= 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN50XX) && ((offset <= 3) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 7) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 23) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 11) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 19) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7) || (offset == 32))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7) || (offset == 32)))))
		cvmx_warn("CVMX_CIU_INTX_SUM0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_CIU_INTX_SUM0(offset) (CVMX_ADD_IO_SEG(0x0001070000000000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_INTX_SUM4(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN50XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 11))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_INTX_SUM4(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000C00ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_INTX_SUM4(offset) (CVMX_ADD_IO_SEG(0x0001070000000C00ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_INT_DBG_SEL CVMX_CIU_INT_DBG_SEL_FUNC()
static inline uint64_t CVMX_CIU_INT_DBG_SEL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_CIU_INT_DBG_SEL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010700000007D0ull);
}
#else
#define CVMX_CIU_INT_DBG_SEL (CVMX_ADD_IO_SEG(0x00010700000007D0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_INT_SUM1 CVMX_CIU_INT_SUM1_FUNC()
static inline uint64_t CVMX_CIU_INT_SUM1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_CIU_INT_SUM1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000000108ull);
}
#else
#define CVMX_CIU_INT_SUM1 (CVMX_ADD_IO_SEG(0x0001070000000108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_MBOX_CLRX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
			if ((offset <= 11))
				return CVMX_ADD_IO_SEG(0x0001070000000680ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return CVMX_ADD_IO_SEG(0x0001070000000680ull) + ((offset) & 3) * 8;
			break;
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x0001070000000680ull) + ((offset) & 1) * 8;
			break;
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
			if ((offset <= 15))
				return CVMX_ADD_IO_SEG(0x0001070000000680ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset <= 9))
				return CVMX_ADD_IO_SEG(0x0001070000000680ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return CVMX_ADD_IO_SEG(0x0001070000000680ull) + ((offset) & 7) * 8;
			break;
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001070000000680ull) + ((offset) & 0) * 8;
			break;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return CVMX_ADD_IO_SEG(0x0001070100100600ull) + ((offset) & 31) * 8;
			break;
	}
	cvmx_warn("CVMX_CIU_MBOX_CLRX (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000680ull) + ((offset) & 3) * 8;
}
#else
static inline uint64_t CVMX_CIU_MBOX_CLRX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000680ull) + (offset) * 8;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000680ull) + (offset) * 8;
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000680ull) + (offset) * 8;
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000680ull) + (offset) * 8;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000680ull) + (offset) * 8;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000680ull) + (offset) * 8;
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000680ull) + (offset) * 8;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070100100600ull) + (offset) * 8;
	}
	return CVMX_ADD_IO_SEG(0x0001070000000680ull) + (offset) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_MBOX_SETX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
			if ((offset <= 11))
				return CVMX_ADD_IO_SEG(0x0001070000000600ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return CVMX_ADD_IO_SEG(0x0001070000000600ull) + ((offset) & 3) * 8;
			break;
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x0001070000000600ull) + ((offset) & 1) * 8;
			break;
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
			if ((offset <= 15))
				return CVMX_ADD_IO_SEG(0x0001070000000600ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset <= 9))
				return CVMX_ADD_IO_SEG(0x0001070000000600ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return CVMX_ADD_IO_SEG(0x0001070000000600ull) + ((offset) & 7) * 8;
			break;
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001070000000600ull) + ((offset) & 0) * 8;
			break;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return CVMX_ADD_IO_SEG(0x0001070100100400ull) + ((offset) & 31) * 8;
			break;
	}
	cvmx_warn("CVMX_CIU_MBOX_SETX (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000600ull) + ((offset) & 3) * 8;
}
#else
static inline uint64_t CVMX_CIU_MBOX_SETX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000600ull) + (offset) * 8;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000600ull) + (offset) * 8;
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000600ull) + (offset) * 8;
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000600ull) + (offset) * 8;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000600ull) + (offset) * 8;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000600ull) + (offset) * 8;
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000600ull) + (offset) * 8;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070100100400ull) + (offset) * 8;
	}
	return CVMX_ADD_IO_SEG(0x0001070000000600ull) + (offset) * 8;
}
#endif
#define CVMX_CIU_NMI (CVMX_ADD_IO_SEG(0x0001070000000718ull))
#define CVMX_CIU_PCI_INTA (CVMX_ADD_IO_SEG(0x0001070000000750ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_PP_BIST_STAT CVMX_CIU_PP_BIST_STAT_FUNC()
static inline uint64_t CVMX_CIU_PP_BIST_STAT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_CIU_PP_BIST_STAT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010700000007E0ull);
}
#else
#define CVMX_CIU_PP_BIST_STAT (CVMX_ADD_IO_SEG(0x00010700000007E0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_PP_DBG CVMX_CIU_PP_DBG_FUNC()
static inline uint64_t CVMX_CIU_PP_DBG_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000708ull);
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return CVMX_ADD_IO_SEG(0x0001010000000120ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return CVMX_ADD_IO_SEG(0x0001010000000120ull);
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001010000000120ull);
			break;
	}
	cvmx_warn("CVMX_CIU_PP_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000120ull);
}
#else
#define CVMX_CIU_PP_DBG CVMX_CIU_PP_DBG_FUNC()
static inline uint64_t CVMX_CIU_PP_DBG_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000708ull);
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001010000000120ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001010000000120ull);
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001010000000120ull);
	}
	return CVMX_ADD_IO_SEG(0x0001010000000120ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_PP_POKEX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
			if ((offset <= 11))
				return CVMX_ADD_IO_SEG(0x0001070000000580ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return CVMX_ADD_IO_SEG(0x0001070000000580ull) + ((offset) & 3) * 8;
			break;
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x0001070000000580ull) + ((offset) & 1) * 8;
			break;
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
			if ((offset <= 15))
				return CVMX_ADD_IO_SEG(0x0001070000000580ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset <= 9))
				return CVMX_ADD_IO_SEG(0x0001070000000580ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return CVMX_ADD_IO_SEG(0x0001070000000580ull) + ((offset) & 7) * 8;
			break;
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001070000000580ull) + ((offset) & 0) * 8;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 47))
					return CVMX_ADD_IO_SEG(0x0001010000030000ull) + ((offset) & 63) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 47))
					return CVMX_ADD_IO_SEG(0x0001010000030000ull) + ((offset) & 63) * 8;

			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 15))
				return CVMX_ADD_IO_SEG(0x0001010000030000ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return CVMX_ADD_IO_SEG(0x0001070100100200ull) + ((offset) & 31) * 8;
			break;
	}
	cvmx_warn("CVMX_CIU_PP_POKEX (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001010000030000ull) + ((offset) & 15) * 8;
}
#else
static inline uint64_t CVMX_CIU_PP_POKEX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000580ull) + (offset) * 8;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000580ull) + (offset) * 8;
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000580ull) + (offset) * 8;
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000580ull) + (offset) * 8;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000580ull) + (offset) * 8;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000580ull) + (offset) * 8;
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000580ull) + (offset) * 8;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001010000030000ull) + (offset) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001010000030000ull) + (offset) * 8;

		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001010000030000ull) + (offset) * 8;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070100100200ull) + (offset) * 8;
	}
	return CVMX_ADD_IO_SEG(0x0001010000030000ull) + (offset) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_PP_RST CVMX_CIU_PP_RST_FUNC()
static inline uint64_t CVMX_CIU_PP_RST_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000700ull);
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return CVMX_ADD_IO_SEG(0x0001010000000100ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return CVMX_ADD_IO_SEG(0x0001010000000100ull);
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001010000000100ull);
			break;
	}
	cvmx_warn("CVMX_CIU_PP_RST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000100ull);
}
#else
#define CVMX_CIU_PP_RST CVMX_CIU_PP_RST_FUNC()
static inline uint64_t CVMX_CIU_PP_RST_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000700ull);
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001010000000100ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001010000000100ull);
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001010000000100ull);
	}
	return CVMX_ADD_IO_SEG(0x0001010000000100ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_PP_RST_PENDING CVMX_CIU_PP_RST_PENDING_FUNC()
static inline uint64_t CVMX_CIU_PP_RST_PENDING_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return CVMX_ADD_IO_SEG(0x0001010000000110ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return CVMX_ADD_IO_SEG(0x0001010000000110ull);
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001010000000110ull);
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000740ull);
			break;
	}
	cvmx_warn("CVMX_CIU_PP_RST_PENDING not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000110ull);
}
#else
#define CVMX_CIU_PP_RST_PENDING CVMX_CIU_PP_RST_PENDING_FUNC()
static inline uint64_t CVMX_CIU_PP_RST_PENDING_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001010000000110ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001010000000110ull);
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001010000000110ull);
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000740ull);
	}
	return CVMX_ADD_IO_SEG(0x0001010000000110ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_QLM0 CVMX_CIU_QLM0_FUNC()
static inline uint64_t CVMX_CIU_QLM0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_CIU_QLM0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000000780ull);
}
#else
#define CVMX_CIU_QLM0 (CVMX_ADD_IO_SEG(0x0001070000000780ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_QLM1 CVMX_CIU_QLM1_FUNC()
static inline uint64_t CVMX_CIU_QLM1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_CIU_QLM1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000000788ull);
}
#else
#define CVMX_CIU_QLM1 (CVMX_ADD_IO_SEG(0x0001070000000788ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_QLM2 CVMX_CIU_QLM2_FUNC()
static inline uint64_t CVMX_CIU_QLM2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_CIU_QLM2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000000790ull);
}
#else
#define CVMX_CIU_QLM2 (CVMX_ADD_IO_SEG(0x0001070000000790ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_QLM3 CVMX_CIU_QLM3_FUNC()
static inline uint64_t CVMX_CIU_QLM3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_CIU_QLM3 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000000798ull);
}
#else
#define CVMX_CIU_QLM3 (CVMX_ADD_IO_SEG(0x0001070000000798ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_QLM4 CVMX_CIU_QLM4_FUNC()
static inline uint64_t CVMX_CIU_QLM4_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_CIU_QLM4 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010700000007A0ull);
}
#else
#define CVMX_CIU_QLM4 (CVMX_ADD_IO_SEG(0x00010700000007A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_QLM_DCOK CVMX_CIU_QLM_DCOK_FUNC()
static inline uint64_t CVMX_CIU_QLM_DCOK_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX)))
		cvmx_warn("CVMX_CIU_QLM_DCOK not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000000760ull);
}
#else
#define CVMX_CIU_QLM_DCOK (CVMX_ADD_IO_SEG(0x0001070000000760ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_QLM_JTGC CVMX_CIU_QLM_JTGC_FUNC()
static inline uint64_t CVMX_CIU_QLM_JTGC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_CIU_QLM_JTGC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000000768ull);
}
#else
#define CVMX_CIU_QLM_JTGC (CVMX_ADD_IO_SEG(0x0001070000000768ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_QLM_JTGD CVMX_CIU_QLM_JTGD_FUNC()
static inline uint64_t CVMX_CIU_QLM_JTGD_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_CIU_QLM_JTGD not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000000770ull);
}
#else
#define CVMX_CIU_QLM_JTGD (CVMX_ADD_IO_SEG(0x0001070000000770ull))
#endif
#define CVMX_CIU_SOFT_BIST (CVMX_ADD_IO_SEG(0x0001070000000738ull))
#define CVMX_CIU_SOFT_PRST (CVMX_ADD_IO_SEG(0x0001070000000748ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_SOFT_PRST1 CVMX_CIU_SOFT_PRST1_FUNC()
static inline uint64_t CVMX_CIU_SOFT_PRST1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_CIU_SOFT_PRST1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000000758ull);
}
#else
#define CVMX_CIU_SOFT_PRST1 (CVMX_ADD_IO_SEG(0x0001070000000758ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_SOFT_PRST2 CVMX_CIU_SOFT_PRST2_FUNC()
static inline uint64_t CVMX_CIU_SOFT_PRST2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN66XX)))
		cvmx_warn("CVMX_CIU_SOFT_PRST2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010700000007D8ull);
}
#else
#define CVMX_CIU_SOFT_PRST2 (CVMX_ADD_IO_SEG(0x00010700000007D8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_SOFT_PRST3 CVMX_CIU_SOFT_PRST3_FUNC()
static inline uint64_t CVMX_CIU_SOFT_PRST3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN66XX)))
		cvmx_warn("CVMX_CIU_SOFT_PRST3 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010700000007E0ull);
}
#else
#define CVMX_CIU_SOFT_PRST3 (CVMX_ADD_IO_SEG(0x00010700000007E0ull))
#endif
#define CVMX_CIU_SOFT_RST (CVMX_ADD_IO_SEG(0x0001070000000740ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_SUM1_IOX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_CIU_SUM1_IOX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000008600ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_CIU_SUM1_IOX_INT(offset) (CVMX_ADD_IO_SEG(0x0001070000008600ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_SUM1_PPX_IP2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_SUM1_PPX_IP2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000008000ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_SUM1_PPX_IP2(offset) (CVMX_ADD_IO_SEG(0x0001070000008000ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_SUM1_PPX_IP3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_SUM1_PPX_IP3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000008200ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_SUM1_PPX_IP3(offset) (CVMX_ADD_IO_SEG(0x0001070000008200ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_SUM1_PPX_IP4(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_SUM1_PPX_IP4(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000008400ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_SUM1_PPX_IP4(offset) (CVMX_ADD_IO_SEG(0x0001070000008400ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_SUM2_IOX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_CIU_SUM2_IOX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000008E00ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_CIU_SUM2_IOX_INT(offset) (CVMX_ADD_IO_SEG(0x0001070000008E00ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_SUM2_PPX_IP2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_SUM2_PPX_IP2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000008800ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_SUM2_PPX_IP2(offset) (CVMX_ADD_IO_SEG(0x0001070000008800ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_SUM2_PPX_IP3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_SUM2_PPX_IP3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000008A00ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_SUM2_PPX_IP3(offset) (CVMX_ADD_IO_SEG(0x0001070000008A00ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_SUM2_PPX_IP4(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_CIU_SUM2_PPX_IP4(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000008C00ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_SUM2_PPX_IP4(offset) (CVMX_ADD_IO_SEG(0x0001070000008C00ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_TIMX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN30XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN31XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN38XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN50XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 9)))))
		cvmx_warn("CVMX_CIU_TIMX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000480ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU_TIMX(offset) (CVMX_ADD_IO_SEG(0x0001070000000480ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU_TIM_MULTI_CAST CVMX_CIU_TIM_MULTI_CAST_FUNC()
static inline uint64_t CVMX_CIU_TIM_MULTI_CAST_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00010700000004F0ull);
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x000107000000C200ull);
			break;
	}
	cvmx_warn("CVMX_CIU_TIM_MULTI_CAST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x000107000000C200ull);
}
#else
#define CVMX_CIU_TIM_MULTI_CAST CVMX_CIU_TIM_MULTI_CAST_FUNC()
static inline uint64_t CVMX_CIU_TIM_MULTI_CAST_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00010700000004F0ull);
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x000107000000C200ull);
	}
	return CVMX_ADD_IO_SEG(0x000107000000C200ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU_WDOGX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
			if ((offset <= 11))
				return CVMX_ADD_IO_SEG(0x0001070000000500ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return CVMX_ADD_IO_SEG(0x0001070000000500ull) + ((offset) & 3) * 8;
			break;
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x0001070000000500ull) + ((offset) & 1) * 8;
			break;
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
			if ((offset <= 15))
				return CVMX_ADD_IO_SEG(0x0001070000000500ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset <= 9))
				return CVMX_ADD_IO_SEG(0x0001070000000500ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return CVMX_ADD_IO_SEG(0x0001070000000500ull) + ((offset) & 7) * 8;
			break;
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001070000000500ull) + ((offset) & 0) * 8;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 47))
					return CVMX_ADD_IO_SEG(0x0001010000020000ull) + ((offset) & 63) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 47))
					return CVMX_ADD_IO_SEG(0x0001010000020000ull) + ((offset) & 63) * 8;

			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 15))
				return CVMX_ADD_IO_SEG(0x0001010000020000ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return CVMX_ADD_IO_SEG(0x0001070100100000ull) + ((offset) & 31) * 8;
			break;
	}
	cvmx_warn("CVMX_CIU_WDOGX (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001010000020000ull) + ((offset) & 15) * 8;
}
#else
static inline uint64_t CVMX_CIU_WDOGX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000500ull) + (offset) * 8;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000500ull) + (offset) * 8;
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000500ull) + (offset) * 8;
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000500ull) + (offset) * 8;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000500ull) + (offset) * 8;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000500ull) + (offset) * 8;
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000500ull) + (offset) * 8;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001010000020000ull) + (offset) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001010000020000ull) + (offset) * 8;

		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001010000020000ull) + (offset) * 8;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070100100000ull) + (offset) * 8;
	}
	return CVMX_ADD_IO_SEG(0x0001010000020000ull) + (offset) * 8;
}
#endif

/**
 * cvmx_ciu_bist
 */
union cvmx_ciu_bist {
	uint64_t u64;
	struct cvmx_ciu_bist_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t bist                         : 7;  /**< BIST Results.
                                                         HW sets a bit in BIST for for memory that fails
                                                         BIST. */
#else
	uint64_t bist                         : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_ciu_bist_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t bist                         : 4;  /**< BIST Results.
                                                         HW sets a bit in BIST for for memory that fails
                                                         BIST. */
#else
	uint64_t bist                         : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} cn30xx;
	struct cvmx_ciu_bist_cn30xx           cn31xx;
	struct cvmx_ciu_bist_cn30xx           cn38xx;
	struct cvmx_ciu_bist_cn30xx           cn38xxp2;
	struct cvmx_ciu_bist_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t bist                         : 2;  /**< BIST Results.
                                                         HW sets a bit in BIST for for memory that fails
                                                         BIST. */
#else
	uint64_t bist                         : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} cn50xx;
	struct cvmx_ciu_bist_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t bist                         : 3;  /**< BIST Results.
                                                         HW sets a bit in BIST for for memory that fails
                                                         BIST. */
#else
	uint64_t bist                         : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} cn52xx;
	struct cvmx_ciu_bist_cn52xx           cn52xxp1;
	struct cvmx_ciu_bist_cn30xx           cn56xx;
	struct cvmx_ciu_bist_cn30xx           cn56xxp1;
	struct cvmx_ciu_bist_cn30xx           cn58xx;
	struct cvmx_ciu_bist_cn30xx           cn58xxp1;
	struct cvmx_ciu_bist_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t bist                         : 6;  /**< BIST Results.
                                                         HW sets a bit in BIST for for memory that fails
                                                         BIST. */
#else
	uint64_t bist                         : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} cn61xx;
	struct cvmx_ciu_bist_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t bist                         : 5;  /**< BIST Results.
                                                         HW sets a bit in BIST for for memory that fails
                                                         BIST. */
#else
	uint64_t bist                         : 5;
	uint64_t reserved_5_63                : 59;
#endif
	} cn63xx;
	struct cvmx_ciu_bist_cn63xx           cn63xxp1;
	struct cvmx_ciu_bist_cn61xx           cn66xx;
	struct cvmx_ciu_bist_s                cn68xx;
	struct cvmx_ciu_bist_s                cn68xxp1;
	struct cvmx_ciu_bist_cn52xx           cn70xx;
	struct cvmx_ciu_bist_cn52xx           cn70xxp1;
	struct cvmx_ciu_bist_cn61xx           cnf71xx;
};
typedef union cvmx_ciu_bist cvmx_ciu_bist_t;

/**
 * cvmx_ciu_block_int
 *
 * CIU_BLOCK_INT = CIU Blocks Interrupt
 *
 * The interrupt lines from the various chip blocks.
 */
union cvmx_ciu_block_int {
	uint64_t u64;
	struct cvmx_ciu_block_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt
                                                         See SRIO3_INT_REG, SRIO3_INT2_REG */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt
                                                         See SRIO2_INT_REG, SRIO2_INT2_REG */
	uint64_t reserved_43_59               : 17;
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         See CIU_INT_SUM1[PTP] */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t dfm                          : 1;  /**< DFM interrupt
                                                         See DFM_FNT_STAT */
	uint64_t reserved_34_39               : 6;
	uint64_t srio1                        : 1;  /**< SRIO1 interrupt
                                                         See SRIO1_INT_REG */
	uint64_t srio0                        : 1;  /**< SRIO0 interrupt
                                                         See SRIO0_INT_REG, SRIO0_INT2_REG */
	uint64_t reserved_31_31               : 1;
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t reserved_29_29               : 1;
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_27_27               : 1;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t reserved_24_24               : 1;
	uint64_t asxpcs1                      : 1;  /**< See PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t asxpcs0                      : 1;  /**< See PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t reserved_21_21               : 1;
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t reserved_18_19               : 2;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t reserved_15_15               : 1;
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t reserved_8_8                 : 1;
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t gmx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG */
	uint64_t gmx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
#else
	uint64_t mio                          : 1;
	uint64_t gmx0                         : 1;
	uint64_t gmx1                         : 1;
	uint64_t sli                          : 1;
	uint64_t key                          : 1;
	uint64_t fpa                          : 1;
	uint64_t dfa                          : 1;
	uint64_t zip                          : 1;
	uint64_t reserved_8_8                 : 1;
	uint64_t ipd                          : 1;
	uint64_t pko                          : 1;
	uint64_t tim                          : 1;
	uint64_t pow                          : 1;
	uint64_t usb                          : 1;
	uint64_t rad                          : 1;
	uint64_t reserved_15_15               : 1;
	uint64_t l2c                          : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_18_19               : 2;
	uint64_t pip                          : 1;
	uint64_t reserved_21_21               : 1;
	uint64_t asxpcs0                      : 1;
	uint64_t asxpcs1                      : 1;
	uint64_t reserved_24_24               : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_27_27               : 1;
	uint64_t agl                          : 1;
	uint64_t reserved_29_29               : 1;
	uint64_t iob                          : 1;
	uint64_t reserved_31_31               : 1;
	uint64_t srio0                        : 1;
	uint64_t srio1                        : 1;
	uint64_t reserved_34_39               : 6;
	uint64_t dfm                          : 1;
	uint64_t dpi                          : 1;
	uint64_t ptp                          : 1;
	uint64_t reserved_43_59               : 17;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_ciu_block_int_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         See CIU_INT_SUM1[PTP] */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t reserved_31_40               : 10;
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t reserved_29_29               : 1;
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_27_27               : 1;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t reserved_24_24               : 1;
	uint64_t asxpcs1                      : 1;  /**< See PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t asxpcs0                      : 1;  /**< See PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t reserved_21_21               : 1;
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t reserved_18_19               : 2;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t reserved_15_15               : 1;
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t reserved_8_8                 : 1;
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t gmx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG */
	uint64_t gmx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
#else
	uint64_t mio                          : 1;
	uint64_t gmx0                         : 1;
	uint64_t gmx1                         : 1;
	uint64_t sli                          : 1;
	uint64_t key                          : 1;
	uint64_t fpa                          : 1;
	uint64_t dfa                          : 1;
	uint64_t zip                          : 1;
	uint64_t reserved_8_8                 : 1;
	uint64_t ipd                          : 1;
	uint64_t pko                          : 1;
	uint64_t tim                          : 1;
	uint64_t pow                          : 1;
	uint64_t usb                          : 1;
	uint64_t rad                          : 1;
	uint64_t reserved_15_15               : 1;
	uint64_t l2c                          : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_18_19               : 2;
	uint64_t pip                          : 1;
	uint64_t reserved_21_21               : 1;
	uint64_t asxpcs0                      : 1;
	uint64_t asxpcs1                      : 1;
	uint64_t reserved_24_24               : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_27_27               : 1;
	uint64_t agl                          : 1;
	uint64_t reserved_29_29               : 1;
	uint64_t iob                          : 1;
	uint64_t reserved_31_40               : 10;
	uint64_t dpi                          : 1;
	uint64_t ptp                          : 1;
	uint64_t reserved_43_63               : 21;
#endif
	} cn61xx;
	struct cvmx_ciu_block_int_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         See CIU_INT_SUM1[PTP] */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t dfm                          : 1;  /**< DFM interrupt
                                                         See DFM_FNT_STAT */
	uint64_t reserved_34_39               : 6;
	uint64_t srio1                        : 1;  /**< SRIO1 interrupt
                                                         See SRIO1_INT_REG, SRIO1_INT2_REG */
	uint64_t srio0                        : 1;  /**< SRIO0 interrupt
                                                         See SRIO0_INT_REG, SRIO0_INT2_REG */
	uint64_t reserved_31_31               : 1;
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t reserved_29_29               : 1;
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_27_27               : 1;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t reserved_23_24               : 2;
	uint64_t asxpcs0                      : 1;  /**< See PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t reserved_21_21               : 1;
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t reserved_18_19               : 2;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t reserved_15_15               : 1;
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t reserved_8_8                 : 1;
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t reserved_2_2                 : 1;
	uint64_t gmx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
#else
	uint64_t mio                          : 1;
	uint64_t gmx0                         : 1;
	uint64_t reserved_2_2                 : 1;
	uint64_t sli                          : 1;
	uint64_t key                          : 1;
	uint64_t fpa                          : 1;
	uint64_t dfa                          : 1;
	uint64_t zip                          : 1;
	uint64_t reserved_8_8                 : 1;
	uint64_t ipd                          : 1;
	uint64_t pko                          : 1;
	uint64_t tim                          : 1;
	uint64_t pow                          : 1;
	uint64_t usb                          : 1;
	uint64_t rad                          : 1;
	uint64_t reserved_15_15               : 1;
	uint64_t l2c                          : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_18_19               : 2;
	uint64_t pip                          : 1;
	uint64_t reserved_21_21               : 1;
	uint64_t asxpcs0                      : 1;
	uint64_t reserved_23_24               : 2;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_27_27               : 1;
	uint64_t agl                          : 1;
	uint64_t reserved_29_29               : 1;
	uint64_t iob                          : 1;
	uint64_t reserved_31_31               : 1;
	uint64_t srio0                        : 1;
	uint64_t srio1                        : 1;
	uint64_t reserved_34_39               : 6;
	uint64_t dfm                          : 1;
	uint64_t dpi                          : 1;
	uint64_t ptp                          : 1;
	uint64_t reserved_43_63               : 21;
#endif
	} cn63xx;
	struct cvmx_ciu_block_int_cn63xx      cn63xxp1;
	struct cvmx_ciu_block_int_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt
                                                         See SRIO3_INT_REG, SRIO3_INT2_REG */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt
                                                         See SRIO2_INT_REG, SRIO2_INT2_REG */
	uint64_t reserved_43_59               : 17;
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         See CIU_INT_SUM1[PTP] */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t dfm                          : 1;  /**< DFM interrupt
                                                         See DFM_FNT_STAT */
	uint64_t reserved_33_39               : 7;
	uint64_t srio0                        : 1;  /**< SRIO0 interrupt
                                                         See SRIO0_INT_REG, SRIO0_INT2_REG */
	uint64_t reserved_31_31               : 1;
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t reserved_29_29               : 1;
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_27_27               : 1;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t reserved_24_24               : 1;
	uint64_t asxpcs1                      : 1;  /**< See PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t asxpcs0                      : 1;  /**< See PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t reserved_21_21               : 1;
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t reserved_18_19               : 2;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t reserved_15_15               : 1;
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t reserved_8_8                 : 1;
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t gmx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG */
	uint64_t gmx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
#else
	uint64_t mio                          : 1;
	uint64_t gmx0                         : 1;
	uint64_t gmx1                         : 1;
	uint64_t sli                          : 1;
	uint64_t key                          : 1;
	uint64_t fpa                          : 1;
	uint64_t dfa                          : 1;
	uint64_t zip                          : 1;
	uint64_t reserved_8_8                 : 1;
	uint64_t ipd                          : 1;
	uint64_t pko                          : 1;
	uint64_t tim                          : 1;
	uint64_t pow                          : 1;
	uint64_t usb                          : 1;
	uint64_t rad                          : 1;
	uint64_t reserved_15_15               : 1;
	uint64_t l2c                          : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_18_19               : 2;
	uint64_t pip                          : 1;
	uint64_t reserved_21_21               : 1;
	uint64_t asxpcs0                      : 1;
	uint64_t asxpcs1                      : 1;
	uint64_t reserved_24_24               : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_27_27               : 1;
	uint64_t agl                          : 1;
	uint64_t reserved_29_29               : 1;
	uint64_t iob                          : 1;
	uint64_t reserved_31_31               : 1;
	uint64_t srio0                        : 1;
	uint64_t reserved_33_39               : 7;
	uint64_t dfm                          : 1;
	uint64_t dpi                          : 1;
	uint64_t ptp                          : 1;
	uint64_t reserved_43_59               : 17;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} cn66xx;
	struct cvmx_ciu_block_int_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         See CIU_INT_SUM1[PTP] */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t reserved_31_40               : 10;
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t reserved_27_29               : 3;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t reserved_23_24               : 2;
	uint64_t asxpcs0                      : 1;  /**< See PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t reserved_21_21               : 1;
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t reserved_18_19               : 2;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t reserved_15_15               : 1;
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t reserved_6_8                 : 3;
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t reserved_2_2                 : 1;
	uint64_t gmx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
#else
	uint64_t mio                          : 1;
	uint64_t gmx0                         : 1;
	uint64_t reserved_2_2                 : 1;
	uint64_t sli                          : 1;
	uint64_t key                          : 1;
	uint64_t fpa                          : 1;
	uint64_t reserved_6_8                 : 3;
	uint64_t ipd                          : 1;
	uint64_t pko                          : 1;
	uint64_t tim                          : 1;
	uint64_t pow                          : 1;
	uint64_t usb                          : 1;
	uint64_t rad                          : 1;
	uint64_t reserved_15_15               : 1;
	uint64_t l2c                          : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_18_19               : 2;
	uint64_t pip                          : 1;
	uint64_t reserved_21_21               : 1;
	uint64_t asxpcs0                      : 1;
	uint64_t reserved_23_24               : 2;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_27_29               : 3;
	uint64_t iob                          : 1;
	uint64_t reserved_31_40               : 10;
	uint64_t dpi                          : 1;
	uint64_t ptp                          : 1;
	uint64_t reserved_43_63               : 21;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_block_int cvmx_ciu_block_int_t;

/**
 * cvmx_ciu_cib_l2c_en#
 */
union cvmx_ciu_cib_l2c_enx {
	uint64_t u64;
	struct cvmx_ciu_cib_l2c_enx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t cbcx_int_ioccmddbe           : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t cbcx_int_ioccmdsbe           : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t cbcx_int_rsddbe              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t cbcx_int_rsdsbe              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t mcix_int_vbfdbe              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t mcix_int_vbfsbe              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_rtgdbe              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_rtgsbe              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_rddislmc            : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_wrdislmc            : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_bigrd               : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_bigwr               : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_holerd              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_holewr              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_noway               : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_tagdbe              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_tagsbe              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_fbfdbe              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_fbfsbe              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_sbfdbe              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_sbfsbe              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_l2ddbe              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
	uint64_t tadx_int_l2dsbe              : 1;  /**< Enable corresponding CIU_CIB_L2C_RAW bit. */
#else
	uint64_t tadx_int_l2dsbe              : 1;
	uint64_t tadx_int_l2ddbe              : 1;
	uint64_t tadx_int_sbfsbe              : 1;
	uint64_t tadx_int_sbfdbe              : 1;
	uint64_t tadx_int_fbfsbe              : 1;
	uint64_t tadx_int_fbfdbe              : 1;
	uint64_t tadx_int_tagsbe              : 1;
	uint64_t tadx_int_tagdbe              : 1;
	uint64_t tadx_int_noway               : 1;
	uint64_t tadx_int_holewr              : 1;
	uint64_t tadx_int_holerd              : 1;
	uint64_t tadx_int_bigwr               : 1;
	uint64_t tadx_int_bigrd               : 1;
	uint64_t tadx_int_wrdislmc            : 1;
	uint64_t tadx_int_rddislmc            : 1;
	uint64_t tadx_int_rtgsbe              : 1;
	uint64_t tadx_int_rtgdbe              : 1;
	uint64_t mcix_int_vbfsbe              : 1;
	uint64_t mcix_int_vbfdbe              : 1;
	uint64_t cbcx_int_rsdsbe              : 1;
	uint64_t cbcx_int_rsddbe              : 1;
	uint64_t cbcx_int_ioccmdsbe           : 1;
	uint64_t cbcx_int_ioccmddbe           : 1;
	uint64_t reserved_23_63               : 41;
#endif
	} s;
	struct cvmx_ciu_cib_l2c_enx_s         cn70xx;
	struct cvmx_ciu_cib_l2c_enx_s         cn70xxp1;
};
typedef union cvmx_ciu_cib_l2c_enx cvmx_ciu_cib_l2c_enx_t;

/**
 * cvmx_ciu_cib_l2c_raw#
 */
union cvmx_ciu_cib_l2c_rawx {
	uint64_t u64;
	struct cvmx_ciu_cib_l2c_rawx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t cbcx_int_ioccmddbe           : 1;  /**< Set when L2C_CBC0_INT[IOCCMDDBE] set. Edge-sensitive interrupt, so software should clear
                                                         [CBCX_INT_IOCCMDDBE] before clearing L2C_CBC0_INT[IOCCMDDBE]. */
	uint64_t cbcx_int_ioccmdsbe           : 1;  /**< Set when L2C_CBC0_INT[IOCCMDSBE] set. Edge-sensitive interrupt, so software should clear
                                                         [CBCX_INT_IOCCMDSBE] before clearing L2C_CBC0_INT[IOCCMDSBE]. */
	uint64_t cbcx_int_rsddbe              : 1;  /**< Set when L2C_CBC0_INT[RSDDBE] set. Edge-sensitive interrupt, so software should clear
                                                         [CBCX_INT_RSDDBE] before clearing L2C_CBC0_INT[RSDDBE]. */
	uint64_t cbcx_int_rsdsbe              : 1;  /**< Set when L2C_CBC0_INT[RSDSBE] set. Edge-sensitive interrupt, so software should clear
                                                         [CBCX_INT_RSDSBE] before clearing L2C_CBC0_INT[RSDSBE]. */
	uint64_t mcix_int_vbfdbe              : 1;  /**< Set when L2C_MCI0_INT[VBFDBE] set. Edge-sensitive interrupt, so software should clear
                                                         [MCIX_INT_VBFDBE] before clearing L2C_MCI0_INT[VBFDBE]. */
	uint64_t mcix_int_vbfsbe              : 1;  /**< Set when L2C_MCI0_INT[VBFSBE] set. Edge-sensitive interrupt, so software should clear
                                                         [MCIX_INT_VBFSBE] before clearing L2C_MCI0_INT[VBFSBE]. */
	uint64_t tadx_int_rtgdbe              : 1;  /**< Set when L2C_TAD0_INT[RTGDBE] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_RTGDBE] before clearing L2C_TAD0_INT[RTGDBE]. */
	uint64_t tadx_int_rtgsbe              : 1;  /**< Set when L2C_TAD0_INT[RTGSBE] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_RTGSBE] before clearing L2C_TAD0_INT[RTGSBE]. */
	uint64_t tadx_int_rddislmc            : 1;  /**< Set when L2C_TAD0_INT[RDDISLMC] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_RDDISLMC] before clearing L2C_TAD0_INT[RDDISLMC]. */
	uint64_t tadx_int_wrdislmc            : 1;  /**< Set when L2C_TAD0_INT[WRDISLMC] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_WRDISLMC] before clearing L2C_TAD0_INT[WRDISLMC]. */
	uint64_t tadx_int_bigrd               : 1;  /**< Set when L2C_TAD0_INT[BIGRD] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_BIGRD] before clearing L2C_TAD0_INT[BIGRD]. */
	uint64_t tadx_int_bigwr               : 1;  /**< Set when L2C_TAD0_INT[BIGWR] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_BIGWR] before clearing L2C_TAD0_INT[BIGWR]. */
	uint64_t tadx_int_holerd              : 1;  /**< Set when L2C_TAD0_INT[HOLERD] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_HOLERD] before clearing L2C_TAD0_INT[HOLERD]. */
	uint64_t tadx_int_holewr              : 1;  /**< Set when L2C_TAD0_INT[HOLEWR] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_HOLEWR] before clearing L2C_TAD0_INT[HOLEWR]. */
	uint64_t tadx_int_noway               : 1;  /**< Set when L2C_TAD0_INT[NOWAY] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_NOWAY] before clearing L2C_TAD0_INT[NOWAY]. */
	uint64_t tadx_int_tagdbe              : 1;  /**< Set when L2C_TAD0_INT[TAGDBE] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_TAGDBE] before clearing L2C_TAD0_INT[TAGDBE]. */
	uint64_t tadx_int_tagsbe              : 1;  /**< Set when L2C_TAD0_INT[TAGSBE] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_TAGSBE] before clearing L2C_TAD0_INT[TAGSBE]. */
	uint64_t tadx_int_fbfdbe              : 1;  /**< Set when L2C_TAD0_INT[FBFDBE] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_FBFDBE] before clearing L2C_TAD0_INT[FBFDBE]. */
	uint64_t tadx_int_fbfsbe              : 1;  /**< Set when L2C_TAD0_INT[FBFSBE] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_FBFSBE] before clearing L2C_TAD0_INT[FBFSBE]. */
	uint64_t tadx_int_sbfdbe              : 1;  /**< Set when L2C_TAD0_INT[SBFDBE] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_SBFDBE] before clearing L2C_TAD0_INT[SBFDBE]. */
	uint64_t tadx_int_sbfsbe              : 1;  /**< Set when L2C_TAD0_INT[SBFSBE] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_SBFSBE] before clearing L2C_TAD0_INT[SBFSBE]. */
	uint64_t tadx_int_l2ddbe              : 1;  /**< Set when L2C_TAD0_INT[L2DDBE] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_L2DDBE] before clearing L2C_TAD0_INT[L2DDBE]. */
	uint64_t tadx_int_l2dsbe              : 1;  /**< Set when L2C_TAD0_INT[L2DSBE] set. Edge-sensitive interrupt, so software should clear
                                                         [TADX_INT_L2DSBE] before clearing L2C_TAD0_INT[L2DSBE]. */
#else
	uint64_t tadx_int_l2dsbe              : 1;
	uint64_t tadx_int_l2ddbe              : 1;
	uint64_t tadx_int_sbfsbe              : 1;
	uint64_t tadx_int_sbfdbe              : 1;
	uint64_t tadx_int_fbfsbe              : 1;
	uint64_t tadx_int_fbfdbe              : 1;
	uint64_t tadx_int_tagsbe              : 1;
	uint64_t tadx_int_tagdbe              : 1;
	uint64_t tadx_int_noway               : 1;
	uint64_t tadx_int_holewr              : 1;
	uint64_t tadx_int_holerd              : 1;
	uint64_t tadx_int_bigwr               : 1;
	uint64_t tadx_int_bigrd               : 1;
	uint64_t tadx_int_wrdislmc            : 1;
	uint64_t tadx_int_rddislmc            : 1;
	uint64_t tadx_int_rtgsbe              : 1;
	uint64_t tadx_int_rtgdbe              : 1;
	uint64_t mcix_int_vbfsbe              : 1;
	uint64_t mcix_int_vbfdbe              : 1;
	uint64_t cbcx_int_rsdsbe              : 1;
	uint64_t cbcx_int_rsddbe              : 1;
	uint64_t cbcx_int_ioccmdsbe           : 1;
	uint64_t cbcx_int_ioccmddbe           : 1;
	uint64_t reserved_23_63               : 41;
#endif
	} s;
	struct cvmx_ciu_cib_l2c_rawx_s        cn70xx;
	struct cvmx_ciu_cib_l2c_rawx_s        cn70xxp1;
};
typedef union cvmx_ciu_cib_l2c_rawx cvmx_ciu_cib_l2c_rawx_t;

/**
 * cvmx_ciu_cib_lmc#_en#
 */
union cvmx_ciu_cib_lmcx_enx {
	uint64_t u64;
	struct cvmx_ciu_cib_lmcx_enx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t int_ddr_err                  : 1;  /**< Enable corresponding CIU_CIB_LMC_RAW bit. */
	uint64_t int_dlc_ded                  : 1;  /**< Enable corresponding CIU_CIB_LMC_RAW bit. */
	uint64_t int_dlc_sec                  : 1;  /**< Enable corresponding CIU_CIB_LMC_RAW bit. */
	uint64_t int_ded_errx                 : 4;  /**< Enable corresponding CIU_CIB_LMC_RAW bit. */
	uint64_t int_sec_errx                 : 4;  /**< Enable corresponding CIU_CIB_LMC_RAW bit. */
	uint64_t int_nxm_wr_err               : 1;  /**< Enable corresponding CIU_CIB_LMC_RAW bit. */
#else
	uint64_t int_nxm_wr_err               : 1;
	uint64_t int_sec_errx                 : 4;
	uint64_t int_ded_errx                 : 4;
	uint64_t int_dlc_sec                  : 1;
	uint64_t int_dlc_ded                  : 1;
	uint64_t int_ddr_err                  : 1;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_ciu_cib_lmcx_enx_s        cn70xx;
	struct cvmx_ciu_cib_lmcx_enx_s        cn70xxp1;
};
typedef union cvmx_ciu_cib_lmcx_enx cvmx_ciu_cib_lmcx_enx_t;

/**
 * cvmx_ciu_cib_lmc#_raw#
 */
union cvmx_ciu_cib_lmcx_rawx {
	uint64_t u64;
	struct cvmx_ciu_cib_lmcx_rawx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t int_ddr_err                  : 1;  /**< Set when LMC0_INT[DDR_ERR] set. Edge-sensitive interrupt, so software should clear
                                                         [INT_DDR_ERR] before clearing LMC0_INT[DDR_ERR]. */
	uint64_t int_dlc_ded                  : 1;  /**< Set when LMC0_INT[DLCRAM_DED_ERR] set. Edge-sensitive interrupt, so software should clear
                                                         [INT_DLC_DED] before clearing LMC0_INT[DLCRAM_DED_ERR]. */
	uint64_t int_dlc_sec                  : 1;  /**< Set when LMC0_INT[DLCRAM_SEC_ERR] set. Edge-sensitive interrupt, so software should clear
                                                         [INT_DLC_SEC] before clearing LMC0_INT[DLCRAM_SEC_ERR]. */
	uint64_t int_ded_errx                 : 4;  /**< Set when LMC0_INT[DED_ERR<b>] set. Edge-sensitive interrupts, so software should clear
                                                         [INT_DED_ERRX<b>] before clearing LMC0_INT[DED_ERR<b>]. */
	uint64_t int_sec_errx                 : 4;  /**< Set when LMC0_INT[SEC_ERR<b>] set. Edge-sensitive interrupts, so software should clear
                                                         [INT_SEC_ERRX<b>] before clearing LMC0_INT[SEC_ERR<b>]. */
	uint64_t int_nxm_wr_err               : 1;  /**< Set when LMC0_INT[NXM_WR_ERR] set. Edge-sensitive interrupt, so software should clear
                                                         [INT_NXM_WR_ERR] before clearing LMC0_INT[NXM_WR_ERR]. */
#else
	uint64_t int_nxm_wr_err               : 1;
	uint64_t int_sec_errx                 : 4;
	uint64_t int_ded_errx                 : 4;
	uint64_t int_dlc_sec                  : 1;
	uint64_t int_dlc_ded                  : 1;
	uint64_t int_ddr_err                  : 1;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_ciu_cib_lmcx_rawx_s       cn70xx;
	struct cvmx_ciu_cib_lmcx_rawx_s       cn70xxp1;
};
typedef union cvmx_ciu_cib_lmcx_rawx cvmx_ciu_cib_lmcx_rawx_t;

/**
 * cvmx_ciu_cib_ocla#_en#
 */
union cvmx_ciu_cib_oclax_enx {
	uint64_t u64;
	struct cvmx_ciu_cib_oclax_enx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t state_ddrfull                : 1;  /**< Enable corresponding CIU_CIB_OCLA_RAW bit. */
	uint64_t state_wmark                  : 1;  /**< Enable corresponding CIU_CIB_OCLA_RAW bit. */
	uint64_t state_overfull               : 1;  /**< Enable corresponding CIU_CIB_OCLA_RAW bit. */
	uint64_t state_trigfull               : 1;  /**< Enable corresponding CIU_CIB_OCLA_RAW bit. */
	uint64_t state_captured               : 1;  /**< Enable corresponding CIU_CIB_OCLA_RAW bit. */
	uint64_t state_fsm1_int               : 1;  /**< Enable corresponding CIU_CIB_OCLA_RAW bit. */
	uint64_t state_fsm0_int               : 1;  /**< Enable corresponding CIU_CIB_OCLA_RAW bit. */
	uint64_t state_mcdx                   : 3;  /**< Enable corresponding CIU_CIB_OCLA_RAW bit. */
	uint64_t state_trig                   : 1;  /**< Enable corresponding CIU_CIB_OCLA_RAW bit. */
	uint64_t state_ovflx                  : 4;  /**< Enable corresponding CIU_CIB_OCLA_RAW bit. */
#else
	uint64_t state_ovflx                  : 4;
	uint64_t state_trig                   : 1;
	uint64_t state_mcdx                   : 3;
	uint64_t state_fsm0_int               : 1;
	uint64_t state_fsm1_int               : 1;
	uint64_t state_captured               : 1;
	uint64_t state_trigfull               : 1;
	uint64_t state_overfull               : 1;
	uint64_t state_wmark                  : 1;
	uint64_t state_ddrfull                : 1;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_ciu_cib_oclax_enx_s       cn70xx;
	struct cvmx_ciu_cib_oclax_enx_s       cn70xxp1;
};
typedef union cvmx_ciu_cib_oclax_enx cvmx_ciu_cib_oclax_enx_t;

/**
 * cvmx_ciu_cib_ocla#_raw#
 */
union cvmx_ciu_cib_oclax_rawx {
	uint64_t u64;
	struct cvmx_ciu_cib_oclax_rawx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t state_ddrfull                : 1;  /**< Set when OCLA0_STATE_INT[DDRFULL] set. Edge-sensitive interrupt, so software should clear
                                                         [STATE_DDRFULL] before clearing OCLA0_STATE_INT[DDRFULL]. */
	uint64_t state_wmark                  : 1;  /**< Set when OCLA0_STATE_INT[WMARK] set. Edge-sensitive interrupt, so software should clear
                                                         [STATE_WMARK] before clearing OCLA0_STATE_INT[WMARK]. */
	uint64_t state_overfull               : 1;  /**< Set when OCLA0_STATE_INT[OVERFULL] set. Edge-sensitive interrupt, so software should clear
                                                         [STATE_OVERFULL] before clearing OCLA0_STATE_INT[OVERFULL]. */
	uint64_t state_trigfull               : 1;  /**< Set when OCLA0_STATE_INT[TRIGFULL] set. Edge-sensitive interrupt, so software should clear
                                                         [STATE_TRIGFULL] before clearing OCLA0_STATE_INT[TRIGFULL]. */
	uint64_t state_captured               : 1;  /**< Set when OCLA0_STATE_INT[CAPTURED] set. Edge-sensitive interrupt, so software should clear
                                                         [STATE_CAPTURED] before clearing OCLA0_STATE_INT[CAPTURED]. */
	uint64_t state_fsm1_int               : 1;  /**< Set when OCLA0_STATE_INT[FSM1_INT] set. Edge-sensitive interrupt, so software should clear
                                                         [STATE_FSM1_INT] before clearing OCLA0_STATE_INT[FSM1_INT]. */
	uint64_t state_fsm0_int               : 1;  /**< Set when OCLA0_STATE_INT[FSM0_INT] set. Edge-sensitive interrupt, so software should clear
                                                         [STATE_FSM0_INT] before clearing OCLA0_STATE_INT[FSM0_INT]. */
	uint64_t state_mcdx                   : 3;  /**< Set when OCLA0_STATE_INT[MCD<b>] set. Edge-sensitive interrupts, so software should clear
                                                         [STATE_MCDX<b>] before clearing OCLA0_STATE_INT[MCD<b>]. */
	uint64_t state_trig                   : 1;  /**< Set when OCLA0_STATE_INT[TRIG] set. Edge-sensitive interrupt, so software should clear
                                                         [STATE_TRIG] before clearing OCLA0_STATE_INT[TRIG]. */
	uint64_t state_ovflx                  : 4;  /**< Set when OCLA0_STATE_INT[OVFL<b>] set. Edge-sensitive interrupts, so software should clear
                                                         [STATE_OVFLX<b>] before clearing OCLA0_STATE_INT[OVFL<b>]. */
#else
	uint64_t state_ovflx                  : 4;
	uint64_t state_trig                   : 1;
	uint64_t state_mcdx                   : 3;
	uint64_t state_fsm0_int               : 1;
	uint64_t state_fsm1_int               : 1;
	uint64_t state_captured               : 1;
	uint64_t state_trigfull               : 1;
	uint64_t state_overfull               : 1;
	uint64_t state_wmark                  : 1;
	uint64_t state_ddrfull                : 1;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_ciu_cib_oclax_rawx_s      cn70xx;
	struct cvmx_ciu_cib_oclax_rawx_s      cn70xxp1;
};
typedef union cvmx_ciu_cib_oclax_rawx cvmx_ciu_cib_oclax_rawx_t;

/**
 * cvmx_ciu_cib_rst_en#
 */
union cvmx_ciu_cib_rst_enx {
	uint64_t u64;
	struct cvmx_ciu_cib_rst_enx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t int_perstx                   : 3;  /**< Enable corresponding CIU_CIB_RST_RAW bit. */
	uint64_t int_linkx                    : 3;  /**< Enable corresponding CIU_CIB_RST_RAW bit. */
#else
	uint64_t int_linkx                    : 3;
	uint64_t int_perstx                   : 3;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_ciu_cib_rst_enx_s         cn70xx;
	struct cvmx_ciu_cib_rst_enx_s         cn70xxp1;
};
typedef union cvmx_ciu_cib_rst_enx cvmx_ciu_cib_rst_enx_t;

/**
 * cvmx_ciu_cib_rst_raw#
 */
union cvmx_ciu_cib_rst_rawx {
	uint64_t u64;
	struct cvmx_ciu_cib_rst_rawx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t int_perstx                   : 3;  /**< Set when RST_INT[PERST<a>] set. Edge-sensitive interrupts, so software should clear
                                                         [INT_PERSTX<a>] before clearing RST_INT[PERST<a>]. */
	uint64_t int_linkx                    : 3;  /**< Set when RST_INT[RST_LINK<a>] set. Edge-sensitive interrupts, so software should clear
                                                         [INT_LINKX<a>] before clearing RST_INT[RST_LINK<a>]. */
#else
	uint64_t int_linkx                    : 3;
	uint64_t int_perstx                   : 3;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_ciu_cib_rst_rawx_s        cn70xx;
	struct cvmx_ciu_cib_rst_rawx_s        cn70xxp1;
};
typedef union cvmx_ciu_cib_rst_rawx cvmx_ciu_cib_rst_rawx_t;

/**
 * cvmx_ciu_cib_sata_en#
 */
union cvmx_ciu_cib_sata_enx {
	uint64_t u64;
	struct cvmx_ciu_cib_sata_enx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t uahc_pme_req_ip              : 1;  /**< Enable corresponding CIU_CIB_SATA_RAW bit. */
	uint64_t uahc_intrq_ip                : 1;  /**< Enable corresponding CIU_CIB_SATA_RAW bit. */
	uint64_t intstat_xm_bad_dma           : 1;  /**< Enable corresponding CIU_CIB_SATA_RAW bit. */
	uint64_t intstat_xs_ncb_oob           : 1;  /**< Enable corresponding CIU_CIB_SATA_RAW bit. */
#else
	uint64_t intstat_xs_ncb_oob           : 1;
	uint64_t intstat_xm_bad_dma           : 1;
	uint64_t uahc_intrq_ip                : 1;
	uint64_t uahc_pme_req_ip              : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_ciu_cib_sata_enx_s        cn70xx;
	struct cvmx_ciu_cib_sata_enx_s        cn70xxp1;
};
typedef union cvmx_ciu_cib_sata_enx cvmx_ciu_cib_sata_enx_t;

/**
 * cvmx_ciu_cib_sata_raw#
 */
union cvmx_ciu_cib_sata_rawx {
	uint64_t u64;
	struct cvmx_ciu_cib_sata_rawx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t uahc_pme_req_ip              : 1;  /**< Summary for chip-internal level-sensitive interrupt that is asserted any of
                                                         SATA_UAHC_P_IS[CPDS,DMPS,PCS] are set, and also asserts when both SATA_UAHC_P_IS[SDBS] and
                                                         SATA_UAHC_PX_SNTF[PMN] are set.
                                                         Hardware sets [UAHC_PME_REQ_IP] sometime after the underlying interrupt condition changes
                                                         from de-asserting to asserting, and clears [UAHC_PME_REQ_IP] sometime after the underlying
                                                         interrupt condition changes from asserting to de-asserting.
                                                         R/W1C, but software need not clear [UAHC_PME_REQ_IP], and perhaps should only ever clear
                                                         [UAHC_PME_REQ_IP]
                                                         when the underlying interrupt condition is known to be asserted. */
	uint64_t uahc_intrq_ip                : 1;  /**< Summary for chip-internal level-sensitive interrupt that is asserted when any bit in
                                                         SATA_UAHC_GBL_IS[IPS] is set.
                                                         Hardware sets [UAHC_INTRQ_IP] sometime after the underlying interrupt condition changes
                                                         from de-asserting to asserting, and clears [UAHC_INTRQ_IP] sometime after the underlying
                                                         interrupt condition changes from asserting to de-asserting.
                                                         R/W1C, but software need not clear [UAHC_INTRQ_IP], and perhaps should only ever clear
                                                         [UAHC_INTRQ_IP]
                                                         when the underlying interrupt condition is known to be asserted. */
	uint64_t intstat_xm_bad_dma           : 1;  /**< Set when SATA_UCTL_INTSTAT[XM_BAD_DMA] set. Edge-sensitive interrupt, so software should
                                                         clear [INTSTAT_XM_BAD_DMA] before clearing SATA_UCTL_INTSTAT[XM_BAD_DMA]. */
	uint64_t intstat_xs_ncb_oob           : 1;  /**< Set when SATA_UCTL_INTSTAT[XS_NCB_OOB] set. Edge-sensitive interrupt, so software should
                                                         clear [INTSTAT_XS_NCB_OOB] before clearing SATA_UCTL_INTSTAT[XS_NCB_OOB]. */
#else
	uint64_t intstat_xs_ncb_oob           : 1;
	uint64_t intstat_xm_bad_dma           : 1;
	uint64_t uahc_intrq_ip                : 1;
	uint64_t uahc_pme_req_ip              : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_ciu_cib_sata_rawx_s       cn70xx;
	struct cvmx_ciu_cib_sata_rawx_s       cn70xxp1;
};
typedef union cvmx_ciu_cib_sata_rawx cvmx_ciu_cib_sata_rawx_t;

/**
 * cvmx_ciu_cib_usbdrd#_en#
 */
union cvmx_ciu_cib_usbdrdx_enx {
	uint64_t u64;
	struct cvmx_ciu_cib_usbdrdx_enx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t uahc_dev_int                 : 1;  /**< Enable corresponding CIU_CIB_USBDRD(0..1)_RAW bit. */
	uint64_t uahc_imanx_ip                : 1;  /**< Enable corresponding CIU_CIB_USBDRD(0..1)_RAW bit. */
	uint64_t uahc_usbsts_hse              : 1;  /**< Enable corresponding CIU_CIB_USBDRD(0..1)_RAW bit. */
	uint64_t intstat_ram2_dbe             : 1;  /**< Enable corresponding CIU_CIB_USBDRD(0..1)_RAW bit. */
	uint64_t intstat_ram2_sbe             : 1;  /**< Enable corresponding CIU_CIB_USBDRD(0..1)_RAW bit. */
	uint64_t intstat_ram1_dbe             : 1;  /**< Enable corresponding CIU_CIB_USBDRD(0..1)_RAW bit. */
	uint64_t intstat_ram1_sbe             : 1;  /**< Enable corresponding CIU_CIB_USBDRD(0..1)_RAW bit. */
	uint64_t intstat_ram0_dbe             : 1;  /**< Enable corresponding CIU_CIB_USBDRD(0..1)_RAW bit. */
	uint64_t intstat_ram0_sbe             : 1;  /**< Enable corresponding CIU_CIB_USBDRD(0..1)_RAW bit. */
	uint64_t intstat_xm_bad_dma           : 1;  /**< Enable corresponding CIU_CIB_USBDRD(0..1)_RAW bit. */
	uint64_t intstat_xs_ncb_oob           : 1;  /**< Enable corresponding CIU_CIB_USBDRD(0..1)_RAW bit. */
#else
	uint64_t intstat_xs_ncb_oob           : 1;
	uint64_t intstat_xm_bad_dma           : 1;
	uint64_t intstat_ram0_sbe             : 1;
	uint64_t intstat_ram0_dbe             : 1;
	uint64_t intstat_ram1_sbe             : 1;
	uint64_t intstat_ram1_dbe             : 1;
	uint64_t intstat_ram2_sbe             : 1;
	uint64_t intstat_ram2_dbe             : 1;
	uint64_t uahc_usbsts_hse              : 1;
	uint64_t uahc_imanx_ip                : 1;
	uint64_t uahc_dev_int                 : 1;
	uint64_t reserved_11_63               : 53;
#endif
	} s;
	struct cvmx_ciu_cib_usbdrdx_enx_s     cn70xx;
	struct cvmx_ciu_cib_usbdrdx_enx_s     cn70xxp1;
};
typedef union cvmx_ciu_cib_usbdrdx_enx cvmx_ciu_cib_usbdrdx_enx_t;

/**
 * cvmx_ciu_cib_usbdrd#_raw#
 */
union cvmx_ciu_cib_usbdrdx_rawx {
	uint64_t u64;
	struct cvmx_ciu_cib_usbdrdx_rawx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t uahc_dev_int                 : 1;  /**< Summary for chip-internal level-sensitive interrupt that is asserted when corresponding
                                                         (USBDRD(0..1)_UAHC_GEVNTCOUNT[EVNTCOUNT]!=0).
                                                         Hardware sets [UAHC_DEV_INT] sometime after corresponding
                                                         USBDRD(0..1)_UAHC_GEVNTCOUNT[EVNTCOUNT] changes
                                                         from zero, and clears [UAHC_DEV_INT] sometime after corresponding
                                                         USBDRD(0..1)_UAHC_GEVNTCOUNT[EVNTCOUNT] changes to zero.
                                                         R/W1C, but software need not clear [UAHC_DEV_INT], and perhaps should only ever clear
                                                         [UAHC_DEV_INT]
                                                         when corresponding USBDRD(0..1)_UAHC_GEVNTCOUNT[EVNTCOUNT] is known to be non-zero. */
	uint64_t uahc_imanx_ip                : 1;  /**< Summary for chip-internal level-sensitive interrupt that is asserted when corresponding
                                                         USBDRD(0..1)_UAHC_IMAN(0..0)[IP] is set.
                                                         Hardware sets [UAHC_IMANX_IP] sometime after corresponding
                                                         USBDRD(0..1)_UAHC_IMAN(0..0)[IP] changes
                                                         0->1, and clears [UAHC_IMANX_IP] sometime after corresponding
                                                         USBDRD(0..1)_UAHC_IMAN(0..0)[IP] changes 1->0.
                                                         R/W1C, but software need not clear [UAHC_IMANX_IP], and perhaps should only ever clear
                                                         [UAHC_IMANX_IP]
                                                         when corresponding USBDRD(0..1)_UAHC_IMAN(0..0)[IP] is known to be set. */
	uint64_t uahc_usbsts_hse              : 1;  /**< Summary for chip-internal level-sensitive interrupt that is asserted when corresponding
                                                         USBDRD(0..1)_UAHC_USBSTS[HSE] is set.
                                                         Hardware sets [UAHC_USBSTS_HSE] sometime after corresponding USBDRD(0..1)_UAHC_USBSTS[HSE]
                                                         changes
                                                         0->1, and clears [UAHC_USBSTS_HSE] sometime after corresponding
                                                         USBDRD(0..1)_UAHC_USBSTS[HSE] changes 1->0.
                                                         R/W1C, but software need not clear [UAHC_USBSTS_HSE], and perhaps should only ever clear
                                                         [UAHC_USBSTS_HSE]
                                                         when corresponding USBDRD(0..1)_UAHC_USBSTS[HSE] is known to be set. */
	uint64_t intstat_ram2_dbe             : 1;  /**< Set when corresponding USBDRD(0..1)_UCTL_INTSTAT[RAM2_DBE] set. Edge-sensitive interrupt,
                                                         so software should clear [INTSTAT_RAM2_DBE] before clearing corresponding
                                                         USBDRD(0..1)_UCTL_INTSTAT[RAM2_DBE]. */
	uint64_t intstat_ram2_sbe             : 1;  /**< Set when corresponding USBDRD(0..1)_UCTL_INTSTAT[RAM2_SBE] set. Edge-sensitive interrupt,
                                                         so software should clear [INTSTAT_RAM2_SBE] before clearing corresponding
                                                         USBDRD(0..1)_UCTL_INTSTAT[RAM2_SBE]. */
	uint64_t intstat_ram1_dbe             : 1;  /**< Set when corresponding USBDRD(0..1)_UCTL_INTSTAT[RAM1_DBE] set. Edge-sensitive interrupt,
                                                         so software should clear [INTSTAT_RAM1_DBE] before clearing corresponding
                                                         USBDRD(0..1)_UCTL_INTSTAT[RAM1_DBE]. */
	uint64_t intstat_ram1_sbe             : 1;  /**< Set when corresponding USBDRD(0..1)_UCTL_INTSTAT[RAM1_SBE] set. Edge-sensitive interrupt,
                                                         so software should clear [INTSTAT_RAM1_SBE] before clearing corresponding
                                                         USBDRD(0..1)_UCTL_INTSTAT[RAM1_SBE]. */
	uint64_t intstat_ram0_dbe             : 1;  /**< Set when corresponding USBDRD(0..1)_UCTL_INTSTAT[RAM0_DBE] set. Edge-sensitive interrupt,
                                                         so software should clear [INTSTAT_RAM0_DBE] before clearing corresponding
                                                         USBDRD(0..1)_UCTL_INTSTAT[RAM0_DBE]. */
	uint64_t intstat_ram0_sbe             : 1;  /**< Set when corresponding USBDRD(0..1)_UCTL_INTSTAT[RAM0_SBE] set. Edge-sensitive interrupt,
                                                         so software should clear [INTSTAT_RAM0_SBE] before clearing corresponding
                                                         USBDRD(0..1)_UCTL_INTSTAT[RAM0_SBE]. */
	uint64_t intstat_xm_bad_dma           : 1;  /**< Set when corresponding USBDRD(0..1)_UCTL_INTSTAT[XM_BAD_DMA] set. Edge-sensitive
                                                         interrupt, so software should clear [INTSTAT_XM_BAD_DMA] before clearing corresponding
                                                         USBDRD(0..1)_UCTL_INTSTAT[XM_BAD_DMA]. */
	uint64_t intstat_xs_ncb_oob           : 1;  /**< Set when corresponding USBDRD(0..1)_UCTL_INTSTAT[XS_NCB_OOB] set. Edge-sensitive
                                                         interrupt, so software should clear [INTSTAT_XS_NCB_OOB] before clearing corresponding
                                                         USBDRD(0..1)_UCTL_INTSTAT[XS_NCB_OOB]. */
#else
	uint64_t intstat_xs_ncb_oob           : 1;
	uint64_t intstat_xm_bad_dma           : 1;
	uint64_t intstat_ram0_sbe             : 1;
	uint64_t intstat_ram0_dbe             : 1;
	uint64_t intstat_ram1_sbe             : 1;
	uint64_t intstat_ram1_dbe             : 1;
	uint64_t intstat_ram2_sbe             : 1;
	uint64_t intstat_ram2_dbe             : 1;
	uint64_t uahc_usbsts_hse              : 1;
	uint64_t uahc_imanx_ip                : 1;
	uint64_t uahc_dev_int                 : 1;
	uint64_t reserved_11_63               : 53;
#endif
	} s;
	struct cvmx_ciu_cib_usbdrdx_rawx_s    cn70xx;
	struct cvmx_ciu_cib_usbdrdx_rawx_s    cn70xxp1;
};
typedef union cvmx_ciu_cib_usbdrdx_rawx cvmx_ciu_cib_usbdrdx_rawx_t;

/**
 * cvmx_ciu_dint
 */
union cvmx_ciu_dint {
	uint64_t u64;
	struct cvmx_ciu_dint_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t dint                         : 48; /**< Writing a 1 to a bit sends a DINT pulse to corresponding core vector. */
#else
	uint64_t dint                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_ciu_dint_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t dint                         : 1;  /**< Send DINT pulse to PP vector */
#else
	uint64_t dint                         : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} cn30xx;
	struct cvmx_ciu_dint_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t dint                         : 2;  /**< Send DINT pulse to PP vector */
#else
	uint64_t dint                         : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} cn31xx;
	struct cvmx_ciu_dint_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t dint                         : 16; /**< Send DINT pulse to PP vector */
#else
	uint64_t dint                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn38xx;
	struct cvmx_ciu_dint_cn38xx           cn38xxp2;
	struct cvmx_ciu_dint_cn31xx           cn50xx;
	struct cvmx_ciu_dint_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t dint                         : 4;  /**< Send DINT pulse to PP vector */
#else
	uint64_t dint                         : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} cn52xx;
	struct cvmx_ciu_dint_cn52xx           cn52xxp1;
	struct cvmx_ciu_dint_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t dint                         : 12; /**< Send DINT pulse to PP vector */
#else
	uint64_t dint                         : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} cn56xx;
	struct cvmx_ciu_dint_cn56xx           cn56xxp1;
	struct cvmx_ciu_dint_cn38xx           cn58xx;
	struct cvmx_ciu_dint_cn38xx           cn58xxp1;
	struct cvmx_ciu_dint_cn52xx           cn61xx;
	struct cvmx_ciu_dint_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t dint                         : 6;  /**< Send DINT pulse to PP vector */
#else
	uint64_t dint                         : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} cn63xx;
	struct cvmx_ciu_dint_cn63xx           cn63xxp1;
	struct cvmx_ciu_dint_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t dint                         : 10; /**< Send DINT pulse to PP vector */
#else
	uint64_t dint                         : 10;
	uint64_t reserved_10_63               : 54;
#endif
	} cn66xx;
	struct cvmx_ciu_dint_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t dint                         : 32; /**< Send DINT pulse to PP vector */
#else
	uint64_t dint                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn68xx;
	struct cvmx_ciu_dint_cn68xx           cn68xxp1;
	struct cvmx_ciu_dint_cn52xx           cn70xx;
	struct cvmx_ciu_dint_cn52xx           cn70xxp1;
	struct cvmx_ciu_dint_cn38xx           cn73xx;
	struct cvmx_ciu_dint_s                cn78xx;
	struct cvmx_ciu_dint_s                cn78xxp1;
	struct cvmx_ciu_dint_cn52xx           cnf71xx;
	struct cvmx_ciu_dint_cn38xx           cnf75xx;
};
typedef union cvmx_ciu_dint cvmx_ciu_dint_t;

/**
 * cvmx_ciu_en2_io#_int
 *
 * CIU_EN2_IO0_INT is for PEM0, CIU_EN2_IO1_INT is reserved.
 *
 */
union cvmx_ciu_en2_iox_int {
	uint64_t u64;
	struct cvmx_ciu_en2_iox_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< SATA interrupt enable */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_en2_iox_int_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_en2_iox_int_cn61xx    cn66xx;
	struct cvmx_ciu_en2_iox_int_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< SATA interrupt enable */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_en2_iox_int_cn70xx    cn70xxp1;
	struct cvmx_ciu_en2_iox_int_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_en2_iox_int cvmx_ciu_en2_iox_int_t;

/**
 * cvmx_ciu_en2_io#_int_w1c
 *
 * CIU_EN2_IO0_INT_W1C is for PEM0, CIU_EN2_IO1_INT_W1C is reserved.
 *
 */
union cvmx_ciu_en2_iox_int_w1c {
	uint64_t u64;
	struct cvmx_ciu_en2_iox_int_w1c_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to clear BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to clear AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to clear OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to clear SATA interrupt enable */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< Write 1 to clear ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to clear EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_en2_iox_int_w1c_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_en2_iox_int_w1c_cn61xx cn66xx;
	struct cvmx_ciu_en2_iox_int_w1c_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to clear BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to clear AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to clear OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to clear SATA interrupt enable */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_en2_iox_int_w1c_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_iox_int_w1c_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< Write 1 to clear ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to clear EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_en2_iox_int_w1c cvmx_ciu_en2_iox_int_w1c_t;

/**
 * cvmx_ciu_en2_io#_int_w1s
 *
 * CIU_EN2_IO0_INT_W1S is for PEM0, CIU_EN2_IO1_INT_W1S is reserved.
 *
 */
union cvmx_ciu_en2_iox_int_w1s {
	uint64_t u64;
	struct cvmx_ciu_en2_iox_int_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to set BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to set AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to set OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to set SATA interrupt enable */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< Write 1 to set ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to set EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_en2_iox_int_w1s_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_en2_iox_int_w1s_cn61xx cn66xx;
	struct cvmx_ciu_en2_iox_int_w1s_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to set BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to set AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to set OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to set SATA interrupt enable */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_en2_iox_int_w1s_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_iox_int_w1s_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< Write 1 to set ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to set EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_en2_iox_int_w1s cvmx_ciu_en2_iox_int_w1s_t;

/**
 * cvmx_ciu_en2_pp#_ip2
 *
 * Notes:
 * These SUM2 CSR's did not exist prior to pass 1.2. CIU_TIM4-9 did not exist prior to pass 1.2.
 *
 */
union cvmx_ciu_en2_ppx_ip2 {
	uint64_t u64;
	struct cvmx_ciu_en2_ppx_ip2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< SATA interrupt enable */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_en2_ppx_ip2_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip2_cn61xx    cn66xx;
	struct cvmx_ciu_en2_ppx_ip2_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< SATA interrupt enable */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip2_cn70xx    cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip2_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_en2_ppx_ip2 cvmx_ciu_en2_ppx_ip2_t;

/**
 * cvmx_ciu_en2_pp#_ip2_w1c
 *
 * Write-1-to-clear version of the CIU_EN2_PP(IO)X_IPx(INT) register, read back corresponding
 * CIU_EN2_PP(IO)X_IPx(INT) value.
 */
union cvmx_ciu_en2_ppx_ip2_w1c {
	uint64_t u64;
	struct cvmx_ciu_en2_ppx_ip2_w1c_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to clear BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to clear AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to clear OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to clear SATA interrupt enable */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< Write 1 to clear ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to clear EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_en2_ppx_ip2_w1c_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip2_w1c_cn61xx cn66xx;
	struct cvmx_ciu_en2_ppx_ip2_w1c_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to clear BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to clear AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to clear OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to clear SATA interrupt enable */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip2_w1c_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip2_w1c_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< Write 1 to clear ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to clear EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_en2_ppx_ip2_w1c cvmx_ciu_en2_ppx_ip2_w1c_t;

/**
 * cvmx_ciu_en2_pp#_ip2_w1s
 *
 * Write-1-to-set version of the CIU_EN2_PP(IO)X_IPx(INT) register, read back corresponding
 * CIU_EN2_PP(IO)X_IPx(INT) value.
 */
union cvmx_ciu_en2_ppx_ip2_w1s {
	uint64_t u64;
	struct cvmx_ciu_en2_ppx_ip2_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to set BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to set AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to set OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to set SATA interrupt enable */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< Write 1 to set ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to set EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_en2_ppx_ip2_w1s_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip2_w1s_cn61xx cn66xx;
	struct cvmx_ciu_en2_ppx_ip2_w1s_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to set BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to set AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to set OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to set SATA interrupt enable */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip2_w1s_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip2_w1s_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< Write 1 to set ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to set EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_en2_ppx_ip2_w1s cvmx_ciu_en2_ppx_ip2_w1s_t;

/**
 * cvmx_ciu_en2_pp#_ip3
 *
 * Notes:
 * These SUM2 CSR's did not exist prior to pass 1.2. CIU_TIM4-9 did not exist prior to pass 1.2.
 *
 */
union cvmx_ciu_en2_ppx_ip3 {
	uint64_t u64;
	struct cvmx_ciu_en2_ppx_ip3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< SATA interrupt enable */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_en2_ppx_ip3_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip3_cn61xx    cn66xx;
	struct cvmx_ciu_en2_ppx_ip3_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< SATA interrupt enable */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip3_cn70xx    cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip3_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_en2_ppx_ip3 cvmx_ciu_en2_ppx_ip3_t;

/**
 * cvmx_ciu_en2_pp#_ip3_w1c
 *
 * Notes:
 * Write-1-to-clear version of the CIU_EN2_PP(IO)X_IPx(INT) register, read back corresponding
 * CIU_EN2_PP(IO)X_IPx(INT) value.
 */
union cvmx_ciu_en2_ppx_ip3_w1c {
	uint64_t u64;
	struct cvmx_ciu_en2_ppx_ip3_w1c_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to clear BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to clear AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to clear OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to clear SATA interrupt enable */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< Write 1 to clear ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to clear EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_en2_ppx_ip3_w1c_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip3_w1c_cn61xx cn66xx;
	struct cvmx_ciu_en2_ppx_ip3_w1c_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to clear BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to clear AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to clear OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to clear SATA interrupt enable */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip3_w1c_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip3_w1c_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< Write 1 to clear ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to clear EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_en2_ppx_ip3_w1c cvmx_ciu_en2_ppx_ip3_w1c_t;

/**
 * cvmx_ciu_en2_pp#_ip3_w1s
 *
 * Notes:
 * Write-1-to-set version of the CIU_EN2_PP(IO)X_IPx(INT) register, read back corresponding
 * CIU_EN2_PP(IO)X_IPx(INT) value.
 */
union cvmx_ciu_en2_ppx_ip3_w1s {
	uint64_t u64;
	struct cvmx_ciu_en2_ppx_ip3_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to set BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to set AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to set OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to set SATA interrupt enable */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< Write 1 to set ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to set EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_en2_ppx_ip3_w1s_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip3_w1s_cn61xx cn66xx;
	struct cvmx_ciu_en2_ppx_ip3_w1s_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to set BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to set AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to set OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to set SATA interrupt enable */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip3_w1s_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip3_w1s_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< Write 1 to set ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to set EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_en2_ppx_ip3_w1s cvmx_ciu_en2_ppx_ip3_w1s_t;

/**
 * cvmx_ciu_en2_pp#_ip4
 *
 * Notes:
 * These SUM2 CSR's did not exist prior to pass 1.2. CIU_TIM4-9 did not exist prior to pass 1.2.
 *
 */
union cvmx_ciu_en2_ppx_ip4 {
	uint64_t u64;
	struct cvmx_ciu_en2_ppx_ip4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< SATA interrupt enable */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_en2_ppx_ip4_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip4_cn61xx    cn66xx;
	struct cvmx_ciu_en2_ppx_ip4_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< SATA interrupt enable */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip4_cn70xx    cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip4_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_en2_ppx_ip4 cvmx_ciu_en2_ppx_ip4_t;

/**
 * cvmx_ciu_en2_pp#_ip4_w1c
 *
 * Notes:
 * Write-1-to-clear version of the CIU_EN2_PP(IO)X_IPx(INT) register, read back corresponding
 * CIU_EN2_PP(IO)X_IPx(INT) value.
 */
union cvmx_ciu_en2_ppx_ip4_w1c {
	uint64_t u64;
	struct cvmx_ciu_en2_ppx_ip4_w1c_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to clear BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to clear AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to clear OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to clear SATA interrupt enable */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< Write 1 to clear ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to clear EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_en2_ppx_ip4_w1c_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip4_w1c_cn61xx cn66xx;
	struct cvmx_ciu_en2_ppx_ip4_w1c_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to clear BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to clear AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to clear OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to clear SATA interrupt enable */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip4_w1c_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip4_w1c_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< Write 1 to clear ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to clear EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to clear General timer 4-9 interrupt enable */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_en2_ppx_ip4_w1c cvmx_ciu_en2_ppx_ip4_w1c_t;

/**
 * cvmx_ciu_en2_pp#_ip4_w1s
 *
 * Notes:
 * Write-1-to-set version of the CIU_EN2_PP(IO)X_IPx(INT) register, read back corresponding
 * CIU_EN2_PP(IO)X_IPx(INT) value.
 */
union cvmx_ciu_en2_ppx_ip4_w1s {
	uint64_t u64;
	struct cvmx_ciu_en2_ppx_ip4_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to set BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to set AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to set OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to set SATA interrupt enable */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< Write 1 to set ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to set EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_en2_ppx_ip4_w1s_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_en2_ppx_ip4_w1s_cn61xx cn66xx;
	struct cvmx_ciu_en2_ppx_ip4_w1s_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< Write 1 to set BCH interrupt enable */
	uint64_t agl_drp                      : 1;  /**< Write 1 to set AGL_DRP interrupt enable */
	uint64_t ocla                         : 1;  /**< Write 1 to set OCLA interrupt enable */
	uint64_t sata                         : 1;  /**< Write 1 to set SATA interrupt enable */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_en2_ppx_ip4_w1s_cn70xx cn70xxp1;
	struct cvmx_ciu_en2_ppx_ip4_w1s_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< Write 1 to set ENDOR PHY interrupts enable */
	uint64_t eoi                          : 1;  /**< Write 1 to set EOI rsl interrupt enable */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< Write 1 to set General timer 4-9 interrupt enables */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_en2_ppx_ip4_w1s cvmx_ciu_en2_ppx_ip4_w1s_t;

/**
 * cvmx_ciu_fuse
 */
union cvmx_ciu_fuse {
	uint64_t u64;
	struct cvmx_ciu_fuse_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t fuse                         : 48; /**< Each bit set indicates a physical core is present. FUSE bits <15..0> correspond to PP core
                                                         PP15, 14, 13 ... 2, 1, 0. */
#else
	uint64_t fuse                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_ciu_fuse_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t fuse                         : 1;  /**< Physical PP is present */
#else
	uint64_t fuse                         : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} cn30xx;
	struct cvmx_ciu_fuse_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t fuse                         : 2;  /**< Physical PP is present */
#else
	uint64_t fuse                         : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} cn31xx;
	struct cvmx_ciu_fuse_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t fuse                         : 16; /**< Physical PP is present */
#else
	uint64_t fuse                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn38xx;
	struct cvmx_ciu_fuse_cn38xx           cn38xxp2;
	struct cvmx_ciu_fuse_cn31xx           cn50xx;
	struct cvmx_ciu_fuse_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t fuse                         : 4;  /**< Physical PP is present */
#else
	uint64_t fuse                         : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} cn52xx;
	struct cvmx_ciu_fuse_cn52xx           cn52xxp1;
	struct cvmx_ciu_fuse_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t fuse                         : 12; /**< Physical PP is present */
#else
	uint64_t fuse                         : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} cn56xx;
	struct cvmx_ciu_fuse_cn56xx           cn56xxp1;
	struct cvmx_ciu_fuse_cn38xx           cn58xx;
	struct cvmx_ciu_fuse_cn38xx           cn58xxp1;
	struct cvmx_ciu_fuse_cn52xx           cn61xx;
	struct cvmx_ciu_fuse_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t fuse                         : 6;  /**< Physical PP is present */
#else
	uint64_t fuse                         : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} cn63xx;
	struct cvmx_ciu_fuse_cn63xx           cn63xxp1;
	struct cvmx_ciu_fuse_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t fuse                         : 10; /**< Physical PP is present */
#else
	uint64_t fuse                         : 10;
	uint64_t reserved_10_63               : 54;
#endif
	} cn66xx;
	struct cvmx_ciu_fuse_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t fuse                         : 32; /**< Physical PP is present */
#else
	uint64_t fuse                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn68xx;
	struct cvmx_ciu_fuse_cn68xx           cn68xxp1;
	struct cvmx_ciu_fuse_cn52xx           cn70xx;
	struct cvmx_ciu_fuse_cn52xx           cn70xxp1;
	struct cvmx_ciu_fuse_cn38xx           cn73xx;
	struct cvmx_ciu_fuse_s                cn78xx;
	struct cvmx_ciu_fuse_s                cn78xxp1;
	struct cvmx_ciu_fuse_cn52xx           cnf71xx;
	struct cvmx_ciu_fuse_cn38xx           cnf75xx;
};
typedef union cvmx_ciu_fuse cvmx_ciu_fuse_t;

/**
 * cvmx_ciu_gstop
 */
union cvmx_ciu_gstop {
	uint64_t u64;
	struct cvmx_ciu_gstop_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t gstop                        : 1;  /**< GSTOP bit */
#else
	uint64_t gstop                        : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_ciu_gstop_s               cn30xx;
	struct cvmx_ciu_gstop_s               cn31xx;
	struct cvmx_ciu_gstop_s               cn38xx;
	struct cvmx_ciu_gstop_s               cn38xxp2;
	struct cvmx_ciu_gstop_s               cn50xx;
	struct cvmx_ciu_gstop_s               cn52xx;
	struct cvmx_ciu_gstop_s               cn52xxp1;
	struct cvmx_ciu_gstop_s               cn56xx;
	struct cvmx_ciu_gstop_s               cn56xxp1;
	struct cvmx_ciu_gstop_s               cn58xx;
	struct cvmx_ciu_gstop_s               cn58xxp1;
	struct cvmx_ciu_gstop_s               cn61xx;
	struct cvmx_ciu_gstop_s               cn63xx;
	struct cvmx_ciu_gstop_s               cn63xxp1;
	struct cvmx_ciu_gstop_s               cn66xx;
	struct cvmx_ciu_gstop_s               cn68xx;
	struct cvmx_ciu_gstop_s               cn68xxp1;
	struct cvmx_ciu_gstop_s               cn70xx;
	struct cvmx_ciu_gstop_s               cn70xxp1;
	struct cvmx_ciu_gstop_s               cnf71xx;
};
typedef union cvmx_ciu_gstop cvmx_ciu_gstop_t;

/**
 * cvmx_ciu_int#_en0
 *
 * CIU_INT0_EN0:  PP0/IP2
 * CIU_INT1_EN0:  PP0/IP3
 * CIU_INT2_EN0:  PP1/IP2
 * CIU_INT3_EN0:  PP1/IP3
 * CIU_INT4_EN0:  PP2/IP2
 * CIU_INT5_EN0:  PP2/IP3
 * CIU_INT6_EN0:  PP3/IP2
 * CIU_INT7_EN0:  PP3/IP3
 * - .....
 * (hole)
 * CIU_INT32_EN0: IO 0 (PEM0)
 * CIU_INT33_EN0: IO 1 (reserved in o70).
 */
union cvmx_ciu_intx_en0 {
	uint64_t u64;
	struct cvmx_ciu_intx_en0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt enable */
	uint64_t mii                          : 1;  /**< RGMII/MII/MIX Interface 0 Interrupt enable */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt enable */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< General timer interrupt enables */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt enable */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Two UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCIe interrupt enables */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupt enables */
	uint64_t workq                        : 16; /**< 16 work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} s;
	struct cvmx_ciu_intx_en0_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_59_63               : 5;
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t usb                          : 1;  /**< USB interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop */
	uint64_t reserved_47_47               : 1;
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t reserved_47_47               : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t reserved_59_63               : 5;
#endif
	} cn30xx;
	struct cvmx_ciu_intx_en0_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_59_63               : 5;
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t usb                          : 1;  /**< USB interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t reserved_59_63               : 5;
#endif
	} cn31xx;
	struct cvmx_ciu_intx_en0_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_63               : 8;
#endif
	} cn38xx;
	struct cvmx_ciu_intx_en0_cn38xx       cn38xxp2;
	struct cvmx_ciu_intx_en0_cn30xx       cn50xx;
	struct cvmx_ciu_intx_en0_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn52xx;
	struct cvmx_ciu_intx_en0_cn52xx       cn52xxp1;
	struct cvmx_ciu_intx_en0_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn56xx;
	struct cvmx_ciu_intx_en0_cn56xx       cn56xxp1;
	struct cvmx_ciu_intx_en0_cn38xx       cn58xx;
	struct cvmx_ciu_intx_en0_cn38xx       cn58xxp1;
	struct cvmx_ciu_intx_en0_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt enable */
	uint64_t mii                          : 1;  /**< RGMII/MIX Interface 0 Interrupt enable */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt enable */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt enable */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Two UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCIe interrupt enables */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupt enables */
	uint64_t workq                        : 16; /**< 16 work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_intx_en0_cn52xx       cn63xx;
	struct cvmx_ciu_intx_en0_cn52xx       cn63xxp1;
	struct cvmx_ciu_intx_en0_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt enable */
	uint64_t mii                          : 1;  /**< RGMII/MII/MIX Interface 0 Interrupt enable */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt enable */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt enable */
	uint64_t reserved_57_57               : 1;
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt enable */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCIe/sRIO MSI enables */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Two UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCIe/sRIO interrupt enables */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupt enables */
	uint64_t workq                        : 16; /**< 16 work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_57               : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_intx_en0_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt enable */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt enable */
	uint64_t reserved_56_56               : 1;
	uint64_t timer                        : 4;  /**< General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt enable */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop interrupt enable */
	uint64_t reserved_46_47               : 2;
	uint64_t twsi                         : 1;  /**< TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Two UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCIe interrupt enables */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupt enables */
	uint64_t workq                        : 16; /**< 16 work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t reserved_46_47               : 2;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_56               : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_intx_en0_cn70xx       cn70xxp1;
	struct cvmx_ciu_intx_en0_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt enable */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt enable */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Two UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCIe interrupt enables */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupt enables */
	uint64_t workq                        : 16; /**< 16 work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_intx_en0 cvmx_ciu_intx_en0_t;

/**
 * cvmx_ciu_int#_en0_w1c
 *
 * Write-1-to-clear version of the CIU_INTx_EN0 register, read back corresponding CIU_INTx_EN0
 * value.
 * CIU_INT33_EN0_W1C is reserved.
 */
union cvmx_ciu_intx_en0_w1c {
	uint64_t u64;
	struct cvmx_ciu_intx_en0_w1c_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to clear Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t mii                          : 1;  /**< Write 1 to clr RGMII/MII/MIX Interface 0 Interrupt
                                                         enable */
	uint64_t ipdppthr                     : 1;  /**< Write 1 to clear IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to clear POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to clear 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to clear MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to clear PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< Write 1 to clear General timer interrupt enables */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< Write 1 to clear IPD QOS packet drop interrupt
                                                         enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to clear GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to clear Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to clear RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to clear TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to clear PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to clear PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to clear UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to clear mailbox/PCIe interrupt
                                                         enables */
	uint64_t gpio                         : 16; /**< Write 1s to clear GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to clear work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} s;
	struct cvmx_ciu_intx_en0_w1c_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn52xx;
	struct cvmx_ciu_intx_en0_w1c_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn56xx;
	struct cvmx_ciu_intx_en0_w1c_cn58xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_63               : 8;
#endif
	} cn58xx;
	struct cvmx_ciu_intx_en0_w1c_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to clear Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t mii                          : 1;  /**< Write 1 to clr RGMII/MIX Interface 0 Interrupt
                                                         enable */
	uint64_t ipdppthr                     : 1;  /**< Write 1 to clear IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to clear POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to clear 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to clear MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to clear PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< Write 1 to clear General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to clear IPD QOS packet drop interrupt
                                                         enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to clear GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to clear Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to clear RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to clear TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to clear PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to clear PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to clear UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to clear mailbox/PCIe interrupt
                                                         enables */
	uint64_t gpio                         : 16; /**< Write 1s to clear GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to clear work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_intx_en0_w1c_cn52xx   cn63xx;
	struct cvmx_ciu_intx_en0_w1c_cn52xx   cn63xxp1;
	struct cvmx_ciu_intx_en0_w1c_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to clear Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t mii                          : 1;  /**< Write 1 to clr RGMII/MII/MIX Interface 0 Interrupt
                                                         enable */
	uint64_t ipdppthr                     : 1;  /**< Write 1 to clear IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to clear POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< Write 1 to clear 2nd TWSI Interrupt */
	uint64_t mpi                          : 1;  /**< Write 1 to clear MPI/SPI interrupt */
	uint64_t reserved_57_57               : 1;
	uint64_t usb                          : 1;  /**< Write 1 to clear USB EHCI or OHCI Interrupt */
	uint64_t timer                        : 4;  /**< Write 1 to clear General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to clear IPD QOS packet drop interrupt
                                                         enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to clear GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to clear Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to clear RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to clear TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to clear PCIe/sRIO MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to clear PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to clear UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to clear mailbox/PCIe/sRIO interrupt
                                                         enables */
	uint64_t gpio                         : 16; /**< Write 1s to clear GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to clear work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_57               : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_intx_en0_w1c_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to clear Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< Write 1 to clear IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to clear POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to clear 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to clear MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to clear PCM/TDM interrupt enable */
	uint64_t reserved_56_56               : 1;
	uint64_t timer                        : 4;  /**< Write 1 to clear General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to clear IPD QOS packet drop interrupt
                                                         enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to clear GMX packet drop interrupt enable */
	uint64_t reserved_46_47               : 2;
	uint64_t twsi                         : 1;  /**< Write 1 to clear TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to clear PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to clear PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to clear UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to clear mailbox/PCIe interrupt
                                                         enables */
	uint64_t gpio                         : 16; /**< Write 1s to clear GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to clear work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t reserved_46_47               : 2;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_56               : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_intx_en0_w1c_cn70xx   cn70xxp1;
	struct cvmx_ciu_intx_en0_w1c_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to clear Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< Write 1 to clear IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to clear POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to clear 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to clear MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to clear PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< Write 1 to clear General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to clear IPD QOS packet drop interrupt
                                                         enable */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< Write 1 to clear GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to clear Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to clear RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to clear TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to clear PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to clear PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to clear UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to clear mailbox/PCIe interrupt
                                                         enables */
	uint64_t gpio                         : 16; /**< Write 1s to clear GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to clear work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_intx_en0_w1c cvmx_ciu_intx_en0_w1c_t;

/**
 * cvmx_ciu_int#_en0_w1s
 *
 * Write-1-to-set version of the CIU_INTx_EN0 register, read back corresponding CIU_INTx_EN0
 * value.
 * CIU_INT33_EN0_W1S is reserved.
 */
union cvmx_ciu_intx_en0_w1s {
	uint64_t u64;
	struct cvmx_ciu_intx_en0_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to set Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t mii                          : 1;  /**< Write 1 to set RGMII/MII/MIX Interface 0 Interrupt
                                                         enable */
	uint64_t ipdppthr                     : 1;  /**< Write 1 to set IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to set POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to set 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to set MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to set PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< Write 1 to set General timer interrupt enables */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< Write 1 to set IPD QOS packet drop interrupt
                                                         enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to set GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to set Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to set RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to set TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to set PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to set PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to set UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to set mailbox/PCIe interrupt
                                                         enables */
	uint64_t gpio                         : 16; /**< Write 1s to set GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to set work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} s;
	struct cvmx_ciu_intx_en0_w1s_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn52xx;
	struct cvmx_ciu_intx_en0_w1s_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn56xx;
	struct cvmx_ciu_intx_en0_w1s_cn58xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_63               : 8;
#endif
	} cn58xx;
	struct cvmx_ciu_intx_en0_w1s_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to set Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t mii                          : 1;  /**< Write 1 to set RGMII/MIX Interface 0 Interrupt
                                                         enable */
	uint64_t ipdppthr                     : 1;  /**< Write 1 to set IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to set POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to set 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to set MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to set PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< Write 1 to set General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to set IPD QOS packet drop interrupt
                                                         enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to set GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to set Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to set RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to set TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to set PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to set PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to set UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to set mailbox/PCIe interrupt
                                                         enables */
	uint64_t gpio                         : 16; /**< Write 1s to set GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to set work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_intx_en0_w1s_cn52xx   cn63xx;
	struct cvmx_ciu_intx_en0_w1s_cn52xx   cn63xxp1;
	struct cvmx_ciu_intx_en0_w1s_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to set Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t mii                          : 1;  /**< Write 1 to set RGMII/MII/MIX Interface 0 Interrupt
                                                         enable */
	uint64_t ipdppthr                     : 1;  /**< Write 1 to set IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to set POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< Write 1 to set 2nd TWSI Interrupt */
	uint64_t mpi                          : 1;  /**< Write 1 to set MPI/SPI interrupt */
	uint64_t reserved_57_57               : 1;
	uint64_t usb                          : 1;  /**< Write 1 to set USB EHCI or OHCI Interrupt */
	uint64_t timer                        : 4;  /**< Write 1 to set General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to set IPD QOS packet drop interrupt
                                                         enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to set GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to set Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to set RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to set TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to set PCIe/sRIO MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to set PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to set UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to set mailbox/PCIe/sRIO interrupt
                                                         enables */
	uint64_t gpio                         : 16; /**< Write 1s to set GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to set work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_57               : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_intx_en0_w1s_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to set Boot bus DMA engines Interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< Write 1 to set IPD per-port counter threshold interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to set POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to set 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to set MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to set PCM/TDM interrupt enable */
	uint64_t reserved_56_56               : 1;
	uint64_t timer                        : 4;  /**< Write 1 to set General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to set IPD QOS packet drop interrupt
                                                         enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to set GMX packet drop interrupt enable */
	uint64_t reserved_46_47               : 2;
	uint64_t twsi                         : 1;  /**< Write 1 to set TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to set PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to set PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to set UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to set mailbox/PCIe interrupt
                                                         enables */
	uint64_t gpio                         : 16; /**< Write 1s to set GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to set work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t reserved_46_47               : 2;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_56               : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_intx_en0_w1s_cn70xx   cn70xxp1;
	struct cvmx_ciu_intx_en0_w1s_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to set Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< Write 1 to set IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to set POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to set 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to set MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to set PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< Write 1 to set General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to set IPD QOS packet drop interrupt
                                                         enable */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< Write 1 to set GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to set Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to set RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to set TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to set PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to set PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to set UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to set mailbox/PCIe interrupt
                                                         enables */
	uint64_t gpio                         : 16; /**< Write 1s to set GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to set work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_intx_en0_w1s cvmx_ciu_intx_en0_w1s_t;

/**
 * cvmx_ciu_int#_en1
 *
 * Enables for CIU_SUM1_PPX_IPx  or CIU_SUM1_IOX_INT
 * CIU_INT0_EN1:  PP0/IP2
 * CIU_INT1_EN1:  PP0/IP3
 * CIU_INT2_EN1:  PP1/IP2
 * CIU_INT3_EN1:  PP1/IP3
 * CIU_INT4_EN1:  PP2/IP2
 * CIU_INT5_EN1:  PP2/IP3
 * CIU_INT6_EN1:  PP3/IP2
 * CIU_INT7_EN1:  PP3/IP3
 * - .....
 * (hole)
 * CIU_INT32_EN1: IO0 (PEM0)
 * CIU_INT33_EN1: IO1 (Reserved for o70)
 *
 * PPx/IP2 will be raised when...
 *
 * n = x*2
 * PPx/IP2 = |([CIU_SUM2_PPx_IP2,CIU_SUM1_PPx_IP2, CIU_INTn_SUM0] &
 * [CIU_EN2_PPx_IP2,CIU_INTn_EN1, CIU_INTn_EN0])
 *
 * PPx/IP3 will be raised when...
 *
 * n = x*2 + 1
 * PPx/IP3 =  |([CIU_SUM2_PPx_IP3,CIU_SUM1_PPx_IP3, CIU_INTn_SUM0] &
 * [CIU_EN2_PPx_IP3,CIU_INTn_EN1, CIU_INTn_EN0])
 *
 * PPx/IP4 will be raised when...
 * PPx/IP4 = |([CIU_SUM1_PPx_IP4, CIU_INTx_SUM4] & [CIU_INTx_EN4_1, CIU_INTx_EN4_0])
 *
 * PCI/INT will be raised when...
 *
 * PCI/INT0 (PEM0)
 * PCI/INT0 = |([CIU_SUM2_IO0_INT,CIU_SUM1_IO0_INT, CIU_INT32_SUM0] &
 * [CIU_EN2_IO0_INT,CIU_INT32_EN1, CIU_INT32_EN0])
 *
 * PCI/INT1 is reserved for o70.
 * PCI/INT1 = |([CIU_SUM2_IO1_INT,CIU_SUM1_IO1_INT, CIU_INT33_SUM0] &
 * [CIU_EN2_IO1_INT,CIU_INT33_EN1, CIU_INT33_EN0])
 */
union cvmx_ciu_intx_en1 {
	uint64_t u64;
	struct cvmx_ciu_intx_en1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt enable */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt enable */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt enable */
	uint64_t srio1                        : 1;  /**< SRIO1 interrupt enable */
	uint64_t reserved_50_50               : 1;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< PTP interrupt enable */
	uint64_t agl                          : 1;  /**< AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< DPI interrupt enable */
	uint64_t sli                          : 1;  /**< SLI interrupt enable */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< DFA interrupt enable */
	uint64_t key                          : 1;  /**< KEY interrupt enable */
	uint64_t rad                          : 1;  /**< RAD interrupt enable */
	uint64_t tim                          : 1;  /**< TIM interrupt enable */
	uint64_t zip                          : 1;  /**< ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< PKO interrupt enable */
	uint64_t pip                          : 1;  /**< PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< L2C interrupt enable */
	uint64_t pow                          : 1;  /**< POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< FPA interrupt enable */
	uint64_t iob                          : 1;  /**< IOB interrupt enable */
	uint64_t mio                          : 1;  /**< MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt enable */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt enable */
	uint64_t usb1                         : 1;  /**< USBDRD1 summary interrupt enable vector. */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t wdog                         : 16; /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 16;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_50               : 1;
	uint64_t srio1                        : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} s;
	struct cvmx_ciu_intx_en1_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t wdog                         : 1;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} cn30xx;
	struct cvmx_ciu_intx_en1_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t wdog                         : 2;  /**< Watchdog summary interrupt enable vectory */
#else
	uint64_t wdog                         : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} cn31xx;
	struct cvmx_ciu_intx_en1_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t wdog                         : 16; /**< Watchdog summary interrupt enable vectory */
#else
	uint64_t wdog                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn38xx;
	struct cvmx_ciu_intx_en1_cn38xx       cn38xxp2;
	struct cvmx_ciu_intx_en1_cn31xx       cn50xx;
	struct cvmx_ciu_intx_en1_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t nand                         : 1;  /**< NAND Flash Controller */
	uint64_t mii1                         : 1;  /**< Second MII Interrupt */
	uint64_t usb1                         : 1;  /**< Second USB Interrupt */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t reserved_4_15                : 12;
	uint64_t wdog                         : 4;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_15                : 12;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn52xx;
	struct cvmx_ciu_intx_en1_cn52xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t mii1                         : 1;  /**< Second MII Interrupt */
	uint64_t usb1                         : 1;  /**< Second USB Interrupt */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t reserved_4_15                : 12;
	uint64_t wdog                         : 4;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_15                : 12;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t reserved_19_63               : 45;
#endif
	} cn52xxp1;
	struct cvmx_ciu_intx_en1_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t wdog                         : 12; /**< Watchdog summary interrupt enable vectory */
#else
	uint64_t wdog                         : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} cn56xx;
	struct cvmx_ciu_intx_en1_cn56xx       cn56xxp1;
	struct cvmx_ciu_intx_en1_cn38xx       cn58xx;
	struct cvmx_ciu_intx_en1_cn38xx       cn58xxp1;
	struct cvmx_ciu_intx_en1_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt enable */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< PTP interrupt enable */
	uint64_t agl                          : 1;  /**< AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< DPI interrupt enable */
	uint64_t sli                          : 1;  /**< SLI interrupt enable */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< DFA interrupt enable */
	uint64_t key                          : 1;  /**< KEY interrupt enable */
	uint64_t rad                          : 1;  /**< RAD interrupt enable */
	uint64_t tim                          : 1;  /**< TIM interrupt enable */
	uint64_t zip                          : 1;  /**< ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< PKO interrupt enable */
	uint64_t pip                          : 1;  /**< PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< L2C interrupt enable */
	uint64_t pow                          : 1;  /**< POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< FPA interrupt enable */
	uint64_t iob                          : 1;  /**< IOB interrupt enable */
	uint64_t mio                          : 1;  /**< MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt enable */
	uint64_t mii1                         : 1;  /**< RGMII/MIX Interface 1 Interrupt enable */
	uint64_t reserved_4_17                : 14;
	uint64_t wdog                         : 4;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_17                : 14;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_intx_en1_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt enable */
	uint64_t reserved_57_62               : 6;
	uint64_t dfm                          : 1;  /**< DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt enable */
	uint64_t srio1                        : 1;  /**< SRIO1 interrupt enable */
	uint64_t srio0                        : 1;  /**< SRIO0 interrupt enable */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< PTP interrupt enable */
	uint64_t agl                          : 1;  /**< AGL interrupt enable */
	uint64_t reserved_37_45               : 9;
	uint64_t agx0                         : 1;  /**< GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< DPI interrupt enable */
	uint64_t sli                          : 1;  /**< SLI interrupt enable */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< DFA interrupt enable */
	uint64_t key                          : 1;  /**< KEY interrupt enable */
	uint64_t rad                          : 1;  /**< RAD interrupt enable */
	uint64_t tim                          : 1;  /**< TIM interrupt enable */
	uint64_t zip                          : 1;  /**< ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< PKO interrupt enable */
	uint64_t pip                          : 1;  /**< PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< L2C interrupt enable */
	uint64_t pow                          : 1;  /**< POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< FPA interrupt enable */
	uint64_t iob                          : 1;  /**< IOB interrupt enable */
	uint64_t mio                          : 1;  /**< MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< NAND Flash Controller interrupt enable */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt enable */
	uint64_t reserved_6_17                : 12;
	uint64_t wdog                         : 6;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 6;
	uint64_t reserved_6_17                : 12;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_45               : 9;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t srio1                        : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_62               : 6;
	uint64_t rst                          : 1;
#endif
	} cn63xx;
	struct cvmx_ciu_intx_en1_cn63xx       cn63xxp1;
	struct cvmx_ciu_intx_en1_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt enable */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt enable */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt enable */
	uint64_t reserved_51_51               : 1;
	uint64_t srio0                        : 1;  /**< SRIO0 interrupt enable */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< PTP interrupt enable */
	uint64_t agl                          : 1;  /**< AGL interrupt enable */
	uint64_t reserved_38_45               : 8;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< DPI interrupt enable */
	uint64_t sli                          : 1;  /**< SLI interrupt enable */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< DFA interrupt enable */
	uint64_t key                          : 1;  /**< KEY interrupt enable */
	uint64_t rad                          : 1;  /**< RAD interrupt enable */
	uint64_t tim                          : 1;  /**< TIM interrupt enable */
	uint64_t zip                          : 1;  /**< ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< PKO interrupt enable */
	uint64_t pip                          : 1;  /**< PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< L2C interrupt enable */
	uint64_t pow                          : 1;  /**< POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< FPA interrupt enable */
	uint64_t iob                          : 1;  /**< IOB interrupt enable */
	uint64_t mio                          : 1;  /**< MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< NAND Flash Controller interrupt enable */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt enable */
	uint64_t reserved_10_17               : 8;
	uint64_t wdog                         : 10; /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 10;
	uint64_t reserved_10_17               : 8;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_45               : 8;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_intx_en1_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt enable */
	uint64_t reserved_51_51               : 1;
	uint64_t pem2                         : 1;  /**< PEM2 interrupt enable */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< PTP interrupt enable */
	uint64_t agl                          : 1;  /**< AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI_DMA interrupt enable */
	uint64_t reserved_39_38               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< DPI interrupt enable */
	uint64_t sli                          : 1;  /**< SLI interrupt enable */
	uint64_t usb                          : 1;  /**< USBDRD0 interrupt enable */
	uint64_t dfa                          : 1;  /**< DFA interrupt enable */
	uint64_t key                          : 1;  /**< KEY interrupt enable */
	uint64_t rad                          : 1;  /**< RAD interrupt enable */
	uint64_t tim                          : 1;  /**< TIM interrupt enable */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< PKO interrupt enable */
	uint64_t pip                          : 1;  /**< PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< L2C interrupt enable */
	uint64_t pow                          : 1;  /**< POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< FPA interrupt enable */
	uint64_t iob                          : 1;  /**< IOB interrupt enable */
	uint64_t mio                          : 1;  /**< MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< NAND / EMMC Controller interrupt enable */
	uint64_t reserved_18_18               : 1;
	uint64_t usb1                         : 1;  /**< USBDRD1 summary interrupt enable vector. */
	uint64_t reserved_4_16                : 13;
	uint64_t wdog                         : 4;  /**< Watchdog summary interrupt enable vector. */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_16                : 13;
	uint64_t usb1                         : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_39_38               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t pem2                         : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_intx_en1_cn70xx       cn70xxp1;
	struct cvmx_ciu_intx_en1_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt enable */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< PTP interrupt enable */
	uint64_t reserved_41_46               : 6;
	uint64_t dpi_dma                      : 1;  /**< DPI_DMA interrupt enable */
	uint64_t reserved_37_39               : 3;
	uint64_t agx0                         : 1;  /**< GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< DPI interrupt enable */
	uint64_t sli                          : 1;  /**< SLI interrupt enable */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt enable */
	uint64_t reserved_32_32               : 1;
	uint64_t key                          : 1;  /**< KEY interrupt enable */
	uint64_t rad                          : 1;  /**< RAD interrupt enable */
	uint64_t tim                          : 1;  /**< TIM interrupt enable */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< PKO interrupt enable */
	uint64_t pip                          : 1;  /**< PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< L2C interrupt enable */
	uint64_t pow                          : 1;  /**< POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< FPA interrupt enable */
	uint64_t iob                          : 1;  /**< IOB interrupt enable */
	uint64_t mio                          : 1;  /**< MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt enable */
	uint64_t reserved_4_18                : 15;
	uint64_t wdog                         : 4;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_18                : 15;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t reserved_32_32               : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_39               : 3;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_46               : 6;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_intx_en1 cvmx_ciu_intx_en1_t;

/**
 * cvmx_ciu_int#_en1_w1c
 *
 * Write-1-to-clear version of the CIU_INTX_EN1 register, read back corresponding CIU_INTX_EN1
 * value.
 * CIU_INT33_EN1_W1C is reserved.
 */
union cvmx_ciu_intx_en1_w1c {
	uint64_t u64;
	struct cvmx_ciu_intx_en1_w1c_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to clear MIO RST interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< Write 1 to clear SRIO3 interrupt enable */
	uint64_t srio2                        : 1;  /**< Write 1 to clear SRIO2 interrupt enable */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< Write 1 to clear DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< Write 1 to clear LMC0 interrupt enable */
	uint64_t srio1                        : 1;  /**< Write 1 to clear SRIO1 interrupt enable */
	uint64_t reserved_50_50               : 1;
	uint64_t pem1                         : 1;  /**< Write 1 to clear PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to clear PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to clear PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to clear AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to clear DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< Write 1 to clear GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to clear GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to clear DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to clear SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to clear DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to clear KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to clear RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to clear TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to clear ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to clear PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to clear PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to clear IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to clear L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to clear POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to clear FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to clear IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to clear MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to clear EMMC Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to clear RGMII/MII/MIX Interface 1
                                                         Interrupt enable */
	uint64_t usb1                         : 1;  /**< Write 1s to clear USBDRD1 summary interrupt enable */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t wdog                         : 16; /**< Write 1s to clear Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 16;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_50               : 1;
	uint64_t srio1                        : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} s;
	struct cvmx_ciu_intx_en1_w1c_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t nand                         : 1;  /**< NAND Flash Controller */
	uint64_t mii1                         : 1;  /**< Second MII Interrupt */
	uint64_t usb1                         : 1;  /**< Second USB Interrupt */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t reserved_4_15                : 12;
	uint64_t wdog                         : 4;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_15                : 12;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn52xx;
	struct cvmx_ciu_intx_en1_w1c_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t wdog                         : 12; /**< Watchdog summary interrupt enable vectory */
#else
	uint64_t wdog                         : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} cn56xx;
	struct cvmx_ciu_intx_en1_w1c_cn58xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t wdog                         : 16; /**< Watchdog summary interrupt enable vectory */
#else
	uint64_t wdog                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn58xx;
	struct cvmx_ciu_intx_en1_w1c_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to clear MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< Write 1 to clear LMC0 interrupt enable */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< Write 1 to clear PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to clear PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to clear PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to clear AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to clear DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< Write 1 to clear GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to clear GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to clear DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to clear SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to clear DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to clear KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to clear RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to clear TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to clear ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to clear PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to clear PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to clear IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to clear L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to clear POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to clear FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to clear IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to clear MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to clear EMMC Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to clear RGMII/MIX Interface 1
                                                         Interrupt enable */
	uint64_t reserved_4_17                : 14;
	uint64_t wdog                         : 4;  /**< Write 1s to clear Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_17                : 14;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_intx_en1_w1c_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to clear MIO RST interrupt enable */
	uint64_t reserved_57_62               : 6;
	uint64_t dfm                          : 1;  /**< Write 1 to clear DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< Write 1 to clear LMC0 interrupt enable */
	uint64_t srio1                        : 1;  /**< Write 1 to clear SRIO1 interrupt enable */
	uint64_t srio0                        : 1;  /**< Write 1 to clear SRIO0 interrupt enable */
	uint64_t pem1                         : 1;  /**< Write 1 to clear PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to clear PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to clear PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to clear AGL interrupt enable */
	uint64_t reserved_37_45               : 9;
	uint64_t agx0                         : 1;  /**< Write 1 to clear GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to clear DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to clear SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to clear DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to clear KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to clear RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to clear TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to clear ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to clear PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to clear PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to clear IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to clear L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to clear POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to clear FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to clear IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to clear MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to clear NAND Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to clear RGMII/MII/MIX Interface 1
                                                         Interrupt enable */
	uint64_t reserved_6_17                : 12;
	uint64_t wdog                         : 6;  /**< Write 1s to clear Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 6;
	uint64_t reserved_6_17                : 12;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_45               : 9;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t srio1                        : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_62               : 6;
	uint64_t rst                          : 1;
#endif
	} cn63xx;
	struct cvmx_ciu_intx_en1_w1c_cn63xx   cn63xxp1;
	struct cvmx_ciu_intx_en1_w1c_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to clear MIO RST interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< Write 1 to clear SRIO3 interrupt enable */
	uint64_t srio2                        : 1;  /**< Write 1 to clear SRIO2 interrupt enable */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< Write 1 to clear DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< Write 1 to clear LMC0 interrupt enable */
	uint64_t reserved_51_51               : 1;
	uint64_t srio0                        : 1;  /**< Write 1 to clear SRIO0 interrupt enable */
	uint64_t pem1                         : 1;  /**< Write 1 to clear PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to clear PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to clear PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to clear AGL interrupt enable */
	uint64_t reserved_38_45               : 8;
	uint64_t agx1                         : 1;  /**< Write 1 to clear GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to clear GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to clear DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to clear SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to clear DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to clear KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to clear RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to clear TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to clear ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to clear PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to clear PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to clear IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to clear L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to clear POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to clear FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to clear IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to clear MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to clear NAND Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to clear RGMII/MII/MIX Interface 1
                                                         Interrupt enable */
	uint64_t reserved_10_17               : 8;
	uint64_t wdog                         : 10; /**< Write 1s to clear Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 10;
	uint64_t reserved_10_17               : 8;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_45               : 8;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_intx_en1_w1c_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to clear MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< Write 1 to clear LMC0 interrupt enable */
	uint64_t reserved_51_51               : 1;
	uint64_t pem2                         : 1;  /**< Write 1 to clear PEM2 interrupt enable */
	uint64_t pem1                         : 1;  /**< Write 1 to clear PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to clear PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to clear PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to clear AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to clear DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< Write 1 to clear GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to clear GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to clear DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to clear SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USBDRD0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to clear DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to clear KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to clear RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to clear TIM interrupt enable */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< Write 1 to clear PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to clear PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to clear IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to clear L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to clear POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to clear FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to clear IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to clear MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to clear NAND / EMMC Controller interrupt
                                                         enable */
	uint64_t reserved_18_18               : 1;
	uint64_t usb1                         : 1;  /**< Write 1s to clear USBDRD1 summary interrupt enable */
	uint64_t reserved_4_16                : 13;
	uint64_t wdog                         : 4;  /**< Write 1s to clear Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_16                : 13;
	uint64_t usb1                         : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t pem2                         : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_intx_en1_w1c_cn70xx   cn70xxp1;
	struct cvmx_ciu_intx_en1_w1c_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to clear MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< Write 1 to clear LMC0 interrupt enable */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< Write 1 to clear PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to clear PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to clear PTP interrupt enable */
	uint64_t reserved_41_46               : 6;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to clear DPI_DMA interrupt enable */
	uint64_t reserved_37_39               : 3;
	uint64_t agx0                         : 1;  /**< Write 1 to clear GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to clear DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to clear SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB UCTL0 interrupt enable */
	uint64_t reserved_32_32               : 1;
	uint64_t key                          : 1;  /**< Write 1 to clear KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to clear RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to clear TIM interrupt enable */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< Write 1 to clear PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to clear PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to clear IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to clear L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to clear POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to clear FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to clear IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to clear MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to clear EMMC Flash Controller interrupt
                                                         enable */
	uint64_t reserved_4_18                : 15;
	uint64_t wdog                         : 4;  /**< Write 1s to clear Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_18                : 15;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t reserved_32_32               : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_39               : 3;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_46               : 6;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_intx_en1_w1c cvmx_ciu_intx_en1_w1c_t;

/**
 * cvmx_ciu_int#_en1_w1s
 *
 * Write-1-to-set version of the CIU_INTX_EN1 register, read back corresponding CIU_INTX_EN1
 * value.
 * CIU_INT33_EN1_W1S is reserved.
 */
union cvmx_ciu_intx_en1_w1s {
	uint64_t u64;
	struct cvmx_ciu_intx_en1_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to set MIO RST interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< Write 1 to set SRIO3 interrupt enable */
	uint64_t srio2                        : 1;  /**< Write 1 to set SRIO2 interrupt enable */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< Write 1 to set DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< Write 1 to set LMC0 interrupt enable */
	uint64_t srio1                        : 1;  /**< Write 1 to set SRIO1 interrupt enable */
	uint64_t reserved_50_50               : 1;
	uint64_t pem1                         : 1;  /**< Write 1 to set PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to set PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to set PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to set AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to set DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< Write 1 to set GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to set GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to set DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to set SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to set DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to set KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to set RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to set TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to set ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to set PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to set PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to set IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to set L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to set POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to set FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to set IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to set MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to set EMMC Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to set RGMII/MII/MIX Interface 1 Interrupt
                                                         enable */
	uint64_t usb1                         : 1;  /**< Write 1s to set USBDRD1 summary interrupt enable */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t wdog                         : 16; /**< Write 1s to set Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 16;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_50               : 1;
	uint64_t srio1                        : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} s;
	struct cvmx_ciu_intx_en1_w1s_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t nand                         : 1;  /**< NAND Flash Controller */
	uint64_t mii1                         : 1;  /**< Second MII Interrupt */
	uint64_t usb1                         : 1;  /**< Second USB Interrupt */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t reserved_4_15                : 12;
	uint64_t wdog                         : 4;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_15                : 12;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn52xx;
	struct cvmx_ciu_intx_en1_w1s_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t wdog                         : 12; /**< Watchdog summary interrupt enable vectory */
#else
	uint64_t wdog                         : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} cn56xx;
	struct cvmx_ciu_intx_en1_w1s_cn58xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t wdog                         : 16; /**< Watchdog summary interrupt enable vectory */
#else
	uint64_t wdog                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn58xx;
	struct cvmx_ciu_intx_en1_w1s_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to set MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< Write 1 to set LMC0 interrupt enable */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< Write 1 to set PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to set PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to set PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to set AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to set DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< Write 1 to set GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to set GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to set DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to set SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to set DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to set KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to set RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to set TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to set ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to set PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to set PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to set IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to set L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to set POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to set FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to set IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to set MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to set EMMC Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to set RGMII/MIX Interface 1 Interrupt
                                                         enable */
	uint64_t reserved_4_17                : 14;
	uint64_t wdog                         : 4;  /**< Write 1s to set Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_17                : 14;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_intx_en1_w1s_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to set MIO RST interrupt enable */
	uint64_t reserved_57_62               : 6;
	uint64_t dfm                          : 1;  /**< Write 1 to set DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< Write 1 to set LMC0 interrupt enable */
	uint64_t srio1                        : 1;  /**< Write 1 to set SRIO1 interrupt enable */
	uint64_t srio0                        : 1;  /**< Write 1 to set SRIO0 interrupt enable */
	uint64_t pem1                         : 1;  /**< Write 1 to set PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to set PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to set PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to set AGL interrupt enable */
	uint64_t reserved_37_45               : 9;
	uint64_t agx0                         : 1;  /**< Write 1 to set GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to set DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to set SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to set DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to set KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to set RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to set TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to set ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to set PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to set PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to set IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to set L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to set POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to set FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to set IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to set MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to set NAND Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to set RGMII/MII/MIX Interface 1 Interrupt
                                                         enable */
	uint64_t reserved_6_17                : 12;
	uint64_t wdog                         : 6;  /**< Write 1s to set Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 6;
	uint64_t reserved_6_17                : 12;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_45               : 9;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t srio1                        : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_62               : 6;
	uint64_t rst                          : 1;
#endif
	} cn63xx;
	struct cvmx_ciu_intx_en1_w1s_cn63xx   cn63xxp1;
	struct cvmx_ciu_intx_en1_w1s_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to set MIO RST interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< Write 1 to set SRIO3 interrupt enable */
	uint64_t srio2                        : 1;  /**< Write 1 to set SRIO2 interrupt enable */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< Write 1 to set DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< Write 1 to set LMC0 interrupt enable */
	uint64_t reserved_51_51               : 1;
	uint64_t srio0                        : 1;  /**< Write 1 to set SRIO0 interrupt enable */
	uint64_t pem1                         : 1;  /**< Write 1 to set PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to set PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to set PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to set AGL interrupt enable */
	uint64_t reserved_38_45               : 8;
	uint64_t agx1                         : 1;  /**< Write 1 to set GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to set GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to set DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to set SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to set DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to set KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to set RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to set TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to set ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to set PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to set PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to set IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to set L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to set POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to set FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to set IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to set MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to set NAND Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to set RGMII/MII/MIX Interface 1 Interrupt
                                                         enable */
	uint64_t reserved_10_17               : 8;
	uint64_t wdog                         : 10; /**< Write 1s to set Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 10;
	uint64_t reserved_10_17               : 8;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_45               : 8;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_intx_en1_w1s_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to set MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< Write 1 to set LMC0 interrupt enable */
	uint64_t reserved_51_51               : 1;
	uint64_t pem2                         : 1;  /**< Write 1 to set PEM2 interrupt enable */
	uint64_t pem1                         : 1;  /**< Write 1 to set PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to set PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to set PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to set AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to set DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< Write 1 to set GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to set GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to set DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to set SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USBDRD0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to set DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to set KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to set RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to set TIM interrupt enable */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< Write 1 to set PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to set PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to set IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to set L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to set POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to set FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to set IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to set MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to set NAND / EMMC Controller interrupt
                                                         enable */
	uint64_t reserved_18_18               : 1;
	uint64_t usb1                         : 1;  /**< Write 1s to set USBDRD1 summary interrupt enable */
	uint64_t reserved_4_16                : 13;
	uint64_t wdog                         : 4;  /**< Write 1s to set Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_16                : 13;
	uint64_t usb1                         : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t pem2                         : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_intx_en1_w1s_cn70xx   cn70xxp1;
	struct cvmx_ciu_intx_en1_w1s_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to set MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< Write 1 to set LMC0 interrupt enable */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< Write 1 to set PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to set PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to set PTP interrupt enable */
	uint64_t reserved_41_46               : 6;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to set DPI_DMA interrupt enable */
	uint64_t reserved_37_39               : 3;
	uint64_t agx0                         : 1;  /**< Write 1 to set GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to set DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to set SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB UCTL0 interrupt enable */
	uint64_t reserved_32_32               : 1;
	uint64_t key                          : 1;  /**< Write 1 to set KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to set RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to set TIM interrupt enable */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< Write 1 to set PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to set PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to set IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to set L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to set POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to set FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to set IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to set MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to set EMMC Flash Controller interrupt
                                                         enable */
	uint64_t reserved_4_18                : 15;
	uint64_t wdog                         : 4;  /**< Write 1s to set Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_18                : 15;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t reserved_32_32               : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_39               : 3;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_46               : 6;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_intx_en1_w1s cvmx_ciu_intx_en1_w1s_t;

/**
 * cvmx_ciu_int#_en4_0
 *
 * CIU_INT0_EN4_0:   PP0  /IP4
 * CIU_INT1_EN4_0:   PP1  /IP4
 * - ...
 * CIU_INT3_EN4_0:   PP3  /IP4
 */
union cvmx_ciu_intx_en4_0 {
	uint64_t u64;
	struct cvmx_ciu_intx_en4_0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt enable */
	uint64_t mii                          : 1;  /**< RGMII/MII/MIX Interface 0 Interrupt enable */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt enable */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< General timer interrupt enables */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt enable */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Two UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupt enables */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupt enables */
	uint64_t workq                        : 16; /**< 16 work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} s;
	struct cvmx_ciu_intx_en4_0_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_59_63               : 5;
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t usb                          : 1;  /**< USB interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop */
	uint64_t reserved_47_47               : 1;
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t reserved_47_47               : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t reserved_59_63               : 5;
#endif
	} cn50xx;
	struct cvmx_ciu_intx_en4_0_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn52xx;
	struct cvmx_ciu_intx_en4_0_cn52xx     cn52xxp1;
	struct cvmx_ciu_intx_en4_0_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn56xx;
	struct cvmx_ciu_intx_en4_0_cn56xx     cn56xxp1;
	struct cvmx_ciu_intx_en4_0_cn58xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_63               : 8;
#endif
	} cn58xx;
	struct cvmx_ciu_intx_en4_0_cn58xx     cn58xxp1;
	struct cvmx_ciu_intx_en4_0_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt enable */
	uint64_t mii                          : 1;  /**< RGMII/MIX Interface 0 Interrupt enable */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt enable */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt enable */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Two UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupt enables */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupt enables */
	uint64_t workq                        : 16; /**< 16 work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_intx_en4_0_cn52xx     cn63xx;
	struct cvmx_ciu_intx_en4_0_cn52xx     cn63xxp1;
	struct cvmx_ciu_intx_en4_0_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt enable */
	uint64_t mii                          : 1;  /**< RGMII/MII/MIX Interface 0 Interrupt enable */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt enable */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt enable */
	uint64_t reserved_57_57               : 1;
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt enable */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCIe/sRIO MSI enables */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Two UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupt enables */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupt enables */
	uint64_t workq                        : 16; /**< 16 work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_57               : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_intx_en4_0_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt        enable */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt enable */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt enable */
	uint64_t reserved_56_56               : 1;
	uint64_t timer                        : 4;  /**< General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt enable */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop interrupt enable */
	uint64_t reserved_46_47               : 2;
	uint64_t twsi                         : 1;  /**< TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Two UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupt enables */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupt enables */
	uint64_t workq                        : 16; /**< 16 work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t reserved_46_47               : 2;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_56               : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_intx_en4_0_cn70xx     cn70xxp1;
	struct cvmx_ciu_intx_en4_0_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt enable */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt enable */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Two UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupt enables */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupt enables */
	uint64_t workq                        : 16; /**< 16 work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_intx_en4_0 cvmx_ciu_intx_en4_0_t;

/**
 * cvmx_ciu_int#_en4_0_w1c
 *
 * Write-1-to-clear version of the CIU_INTx_EN4_0 register, read back corresponding
 * CIU_INTx_EN4_0 value.
 */
union cvmx_ciu_intx_en4_0_w1c {
	uint64_t u64;
	struct cvmx_ciu_intx_en4_0_w1c_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to clear Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t mii                          : 1;  /**< Write 1 to clr RGMII/MII/MIX Interface 0 Interrupt
                                                         enable */
	uint64_t ipdppthr                     : 1;  /**< Write 1 to clear IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to clear POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to clear 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to clear MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to clear PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< Write 1 to clear General timer interrupt enables */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< Write 1 to clear IPD QOS packet drop interrupt
                                                         enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to clear GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to clear Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to clear RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to clear TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to clear PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to clear PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to clear UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to clear mailbox interrupt enables */
	uint64_t gpio                         : 16; /**< Write 1s to clear GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to clear work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} s;
	struct cvmx_ciu_intx_en4_0_w1c_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn52xx;
	struct cvmx_ciu_intx_en4_0_w1c_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn56xx;
	struct cvmx_ciu_intx_en4_0_w1c_cn58xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_63               : 8;
#endif
	} cn58xx;
	struct cvmx_ciu_intx_en4_0_w1c_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to clear Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t mii                          : 1;  /**< Write 1 to clr RGMII/MIX Interface 0 Interrupt
                                                         enable */
	uint64_t ipdppthr                     : 1;  /**< Write 1 to clear IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to clear POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to clear 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to clear MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to clear PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< Write 1 to clear General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to clear IPD QOS packet drop interrupt
                                                         enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to clear GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to clear Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to clear RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to clear TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to clear PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to clear PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to clear UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to clear mailbox interrupt enables */
	uint64_t gpio                         : 16; /**< Write 1s to clear GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to clear work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_intx_en4_0_w1c_cn52xx cn63xx;
	struct cvmx_ciu_intx_en4_0_w1c_cn52xx cn63xxp1;
	struct cvmx_ciu_intx_en4_0_w1c_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to clear Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t mii                          : 1;  /**< Write 1 to clr RGMII/MII/MIX Interface 0 Interrupt
                                                         enable */
	uint64_t ipdppthr                     : 1;  /**< Write 1 to clear IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to clear POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< Write 1 to clear 2nd TWSI Interrupt */
	uint64_t mpi                          : 1;  /**< Write 1 to clear MPI/SPI interrupt */
	uint64_t reserved_57_57               : 1;
	uint64_t usb                          : 1;  /**< Write 1 to clear USB EHCI or OHCI Interrupt */
	uint64_t timer                        : 4;  /**< Write 1 to clear General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to clear IPD QOS packet drop interrupt
                                                         enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to clear GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to clear Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to clear RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to clear TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to clear PCIe/sRIO MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to clear PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to clear UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to clear mailbox interrupt enables */
	uint64_t gpio                         : 16; /**< Write 1s to clear GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to clear work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_57               : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_intx_en4_0_w1c_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to clear Boot bus DMA engines Interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< Write 1 to clear IPD per-port counter threshold interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to clear POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to clear 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to clear MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to clear PCM/TDM interrupt enable */
	uint64_t reserved_56_56               : 1;
	uint64_t timer                        : 4;  /**< Write 1 to clear General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to clear IPD QOS packet drop interrupt enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to clear GMX packet drop interrupt enable */
	uint64_t reserved_46_47               : 2;
	uint64_t twsi                         : 1;  /**< Write 1 to clear TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to clear PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to clear PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to clear UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to clear mailbox interrupt enables */
	uint64_t gpio                         : 16; /**< Write 1s to clear GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to clear work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t reserved_46_47               : 2;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_56               : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_intx_en4_0_w1c_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en4_0_w1c_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to clear Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< Write 1 to clear IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to clear POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to clear 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to clear MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to clear PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< Write 1 to clear General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to clear IPD QOS packet drop interrupt
                                                         enable */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< Write 1 to clear GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to clear Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to clear RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to clear TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to clear PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to clear PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to clear UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to clear mailbox interrupt enables */
	uint64_t gpio                         : 16; /**< Write 1s to clear GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to clear work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_intx_en4_0_w1c cvmx_ciu_intx_en4_0_w1c_t;

/**
 * cvmx_ciu_int#_en4_0_w1s
 *
 * Write-1-to-set version of the CIU_INTX_EN4_0 register, read back corresponding CIU_INTX_EN4_0
 * value.
 */
union cvmx_ciu_intx_en4_0_w1s {
	uint64_t u64;
	struct cvmx_ciu_intx_en4_0_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to set Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t mii                          : 1;  /**< Write 1 to set RGMII/MII/MIX Interface 0 Interrupt
                                                         enable */
	uint64_t ipdppthr                     : 1;  /**< Write 1 to set IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to set POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to set 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to set MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to set PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< Write 1 to set General timer interrupt enables */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< Write 1 to set IPD QOS packet drop interrupt
                                                         enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to set GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to set Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to set RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to set TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to set PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to set PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to set UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to set mailbox interrupt enables */
	uint64_t gpio                         : 16; /**< Write 1s to set GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to set work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} s;
	struct cvmx_ciu_intx_en4_0_w1s_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn52xx;
	struct cvmx_ciu_intx_en4_0_w1s_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn56xx;
	struct cvmx_ciu_intx_en4_0_w1s_cn58xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< PCI MSI */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox/PCI interrupts */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_63               : 8;
#endif
	} cn58xx;
	struct cvmx_ciu_intx_en4_0_w1s_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to set Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t mii                          : 1;  /**< Write 1 to set RGMII/MIX Interface 0 Interrupt
                                                         enable */
	uint64_t ipdppthr                     : 1;  /**< Write 1 to set IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to set POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to set 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to set MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to set PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< Write 1 to set General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to set IPD QOS packet drop interrupt
                                                         enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to set GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to set Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to set RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to set TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to set PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to set PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to set UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to set mailbox interrupt enables */
	uint64_t gpio                         : 16; /**< Write 1s to set GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to set work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_intx_en4_0_w1s_cn52xx cn63xx;
	struct cvmx_ciu_intx_en4_0_w1s_cn52xx cn63xxp1;
	struct cvmx_ciu_intx_en4_0_w1s_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to set Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t mii                          : 1;  /**< Write 1 to set RGMII/MII/MIX Interface 0 Interrupt
                                                         enable */
	uint64_t ipdppthr                     : 1;  /**< Write 1 to set IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to set POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< Write 1 to set 2nd TWSI Interrupt */
	uint64_t mpi                          : 1;  /**< Write 1 to set MPI/SPI interrupt */
	uint64_t reserved_57_57               : 1;
	uint64_t usb                          : 1;  /**< Write 1 to set USB EHCI or OHCI Interrupt */
	uint64_t timer                        : 4;  /**< Write 1 to set General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to set IPD QOS packet drop interrupt
                                                         enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to set GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to set Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to set RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to set TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to set PCIe/sRIO MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to set PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to set UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to set mailbox interrupt enables */
	uint64_t gpio                         : 16; /**< Write 1s to set GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to set work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_57               : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_intx_en4_0_w1s_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to set Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< Write 1 to set IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to set POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to set 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to set MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to set PCM/TDM interrupt enable */
	uint64_t reserved_56_56               : 1;
	uint64_t timer                        : 4;  /**< Write 1 to set General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to set IPD QOS packet drop interrupt
                                                         enable */
	uint64_t gmx_drp                      : 2;  /**< Write 1 to set GMX packet drop interrupt enable */
	uint64_t reserved_46_47               : 2;
	uint64_t twsi                         : 1;  /**< Write 1 to set TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to set PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to set PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to set UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to set mailbox interrupt enables */
	uint64_t gpio                         : 16; /**< Write 1s to set GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to set work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t reserved_46_47               : 2;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_56               : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_intx_en4_0_w1s_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en4_0_w1s_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Write 1 to set Boot bus DMA engines Interrupt
                                                         enable */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< Write 1 to set IPD per-port counter threshold
                                                         interrupt enable */
	uint64_t powiq                        : 1;  /**< Write 1 to set POW IQ interrupt enable */
	uint64_t twsi2                        : 1;  /**< Write 1 to set 2nd TWSI Interrupt enable */
	uint64_t mpi                          : 1;  /**< Write 1 to set MPI/SPI interrupt enable */
	uint64_t pcm                          : 1;  /**< Write 1 to set PCM/TDM interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB EHCI or OHCI Interrupt enable */
	uint64_t timer                        : 4;  /**< Write 1 to set General timer interrupt enables */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< Write 1 to set IPD QOS packet drop interrupt
                                                         enable */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< Write 1 to set GMX packet drop interrupt enable */
	uint64_t trace                        : 1;  /**< Write 1 to set Trace buffer interrupt enable */
	uint64_t rml                          : 1;  /**< Write 1 to set RML Interrupt enable */
	uint64_t twsi                         : 1;  /**< Write 1 to set TWSI Interrupt enable */
	uint64_t reserved_44_44               : 1;
	uint64_t pci_msi                      : 4;  /**< Write 1s to set PCIe MSI enables */
	uint64_t pci_int                      : 4;  /**< Write 1s to set PCIe INTA/B/C/D enables */
	uint64_t uart                         : 2;  /**< Write 1s to set UART interrupt enables */
	uint64_t mbox                         : 2;  /**< Write 1s to set mailbox interrupt enables */
	uint64_t gpio                         : 16; /**< Write 1s to set GPIO interrupt enables */
	uint64_t workq                        : 16; /**< Write 1s to set work queue interrupt enables */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t reserved_44_44               : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_intx_en4_0_w1s cvmx_ciu_intx_en4_0_w1s_t;

/**
 * cvmx_ciu_int#_en4_1
 *
 * PPx/IP4 will be raised when...
 * PPx/IP4 = |([CIU_SUM1_PPx_IP4, CIU_INTx_SUM4] & [CIU_INTx_EN4_1, CIU_INTx_EN4_0])
 */
union cvmx_ciu_intx_en4_1 {
	uint64_t u64;
	struct cvmx_ciu_intx_en4_1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt enable */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt enable */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt enable */
	uint64_t srio1                        : 1;  /**< SRIO1 interrupt enable */
	uint64_t reserved_50_50               : 1;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< PTP interrupt enable */
	uint64_t agl                          : 1;  /**< AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< DPI interrupt enable */
	uint64_t sli                          : 1;  /**< SLI interrupt enable */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< DFA interrupt enable */
	uint64_t key                          : 1;  /**< KEY interrupt enable */
	uint64_t rad                          : 1;  /**< RAD interrupt enable */
	uint64_t tim                          : 1;  /**< TIM interrupt enable */
	uint64_t zip                          : 1;  /**< ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< PKO interrupt enable */
	uint64_t pip                          : 1;  /**< PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< L2C interrupt enable */
	uint64_t pow                          : 1;  /**< POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< FPA interrupt enable */
	uint64_t iob                          : 1;  /**< IOB interrupt enable */
	uint64_t mio                          : 1;  /**< MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt enable */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt enable */
	uint64_t usb1                         : 1;  /**< USBDRD1 summary interrupt enable vector */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t wdog                         : 16; /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 16;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_50               : 1;
	uint64_t srio1                        : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} s;
	struct cvmx_ciu_intx_en4_1_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t wdog                         : 2;  /**< Watchdog summary interrupt enable vectory */
#else
	uint64_t wdog                         : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} cn50xx;
	struct cvmx_ciu_intx_en4_1_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t nand                         : 1;  /**< NAND Flash Controller */
	uint64_t mii1                         : 1;  /**< Second MII Interrupt */
	uint64_t usb1                         : 1;  /**< Second USB Interrupt */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t reserved_4_15                : 12;
	uint64_t wdog                         : 4;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_15                : 12;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn52xx;
	struct cvmx_ciu_intx_en4_1_cn52xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t mii1                         : 1;  /**< Second MII Interrupt */
	uint64_t usb1                         : 1;  /**< Second USB Interrupt */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t reserved_4_15                : 12;
	uint64_t wdog                         : 4;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_15                : 12;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t reserved_19_63               : 45;
#endif
	} cn52xxp1;
	struct cvmx_ciu_intx_en4_1_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t wdog                         : 12; /**< Watchdog summary interrupt enable vectory */
#else
	uint64_t wdog                         : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} cn56xx;
	struct cvmx_ciu_intx_en4_1_cn56xx     cn56xxp1;
	struct cvmx_ciu_intx_en4_1_cn58xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t wdog                         : 16; /**< Watchdog summary interrupt enable vectory */
#else
	uint64_t wdog                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn58xx;
	struct cvmx_ciu_intx_en4_1_cn58xx     cn58xxp1;
	struct cvmx_ciu_intx_en4_1_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt enable */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< PTP interrupt enable */
	uint64_t agl                          : 1;  /**< AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< DPI interrupt enable */
	uint64_t sli                          : 1;  /**< SLI interrupt enable */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< DFA interrupt enable */
	uint64_t key                          : 1;  /**< KEY interrupt enable */
	uint64_t rad                          : 1;  /**< RAD interrupt enable */
	uint64_t tim                          : 1;  /**< TIM interrupt enable */
	uint64_t zip                          : 1;  /**< ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< PKO interrupt enable */
	uint64_t pip                          : 1;  /**< PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< L2C interrupt enable */
	uint64_t pow                          : 1;  /**< POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< FPA interrupt enable */
	uint64_t iob                          : 1;  /**< IOB interrupt enable */
	uint64_t mio                          : 1;  /**< MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt enable */
	uint64_t mii1                         : 1;  /**< RGMII/MIX Interface 1 Interrupt enable */
	uint64_t reserved_4_17                : 14;
	uint64_t wdog                         : 4;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_17                : 14;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_intx_en4_1_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt enable */
	uint64_t reserved_57_62               : 6;
	uint64_t dfm                          : 1;  /**< DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt enable */
	uint64_t srio1                        : 1;  /**< SRIO1 interrupt enable */
	uint64_t srio0                        : 1;  /**< SRIO0 interrupt enable */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< PTP interrupt enable */
	uint64_t agl                          : 1;  /**< AGL interrupt enable */
	uint64_t reserved_37_45               : 9;
	uint64_t agx0                         : 1;  /**< GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< DPI interrupt enable */
	uint64_t sli                          : 1;  /**< SLI interrupt enable */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< DFA interrupt enable */
	uint64_t key                          : 1;  /**< KEY interrupt enable */
	uint64_t rad                          : 1;  /**< RAD interrupt enable */
	uint64_t tim                          : 1;  /**< TIM interrupt enable */
	uint64_t zip                          : 1;  /**< ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< PKO interrupt enable */
	uint64_t pip                          : 1;  /**< PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< L2C interrupt enable */
	uint64_t pow                          : 1;  /**< POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< FPA interrupt enable */
	uint64_t iob                          : 1;  /**< IOB interrupt enable */
	uint64_t mio                          : 1;  /**< MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< NAND Flash Controller interrupt enable */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt enable */
	uint64_t reserved_6_17                : 12;
	uint64_t wdog                         : 6;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 6;
	uint64_t reserved_6_17                : 12;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_45               : 9;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t srio1                        : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_62               : 6;
	uint64_t rst                          : 1;
#endif
	} cn63xx;
	struct cvmx_ciu_intx_en4_1_cn63xx     cn63xxp1;
	struct cvmx_ciu_intx_en4_1_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt enable */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt enable */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt enable */
	uint64_t reserved_51_51               : 1;
	uint64_t srio0                        : 1;  /**< SRIO0 interrupt enable */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< PTP interrupt enable */
	uint64_t agl                          : 1;  /**< AGL interrupt enable */
	uint64_t reserved_38_45               : 8;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< DPI interrupt enable */
	uint64_t sli                          : 1;  /**< SLI interrupt enable */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< DFA interrupt enable */
	uint64_t key                          : 1;  /**< KEY interrupt enable */
	uint64_t rad                          : 1;  /**< RAD interrupt enable */
	uint64_t tim                          : 1;  /**< TIM interrupt enable */
	uint64_t zip                          : 1;  /**< ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< PKO interrupt enable */
	uint64_t pip                          : 1;  /**< PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< L2C interrupt enable */
	uint64_t pow                          : 1;  /**< POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< FPA interrupt enable */
	uint64_t iob                          : 1;  /**< IOB interrupt enable */
	uint64_t mio                          : 1;  /**< MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< NAND Flash Controller interrupt enable */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt enable */
	uint64_t reserved_10_17               : 8;
	uint64_t wdog                         : 10; /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 10;
	uint64_t reserved_10_17               : 8;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_45               : 8;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_intx_en4_1_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt enable */
	uint64_t reserved_51_51               : 1;
	uint64_t pem2                         : 1;  /**< PEM2 interrupt enable */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< PTP interrupt enable */
	uint64_t agl                          : 1;  /**< AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI_DMA interrupt enable */
	uint64_t reserved_39_38               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< DPI interrupt enable */
	uint64_t sli                          : 1;  /**< SLI interrupt enable */
	uint64_t usb                          : 1;  /**< USBDRD0 interrupt enable */
	uint64_t dfa                          : 1;  /**< DFA interrupt enable */
	uint64_t key                          : 1;  /**< KEY interrupt enable */
	uint64_t rad                          : 1;  /**< RAD interrupt enable */
	uint64_t tim                          : 1;  /**< TIM interrupt enable */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< PKO interrupt enable */
	uint64_t pip                          : 1;  /**< PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< L2C interrupt enable */
	uint64_t pow                          : 1;  /**< POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< FPA interrupt enable */
	uint64_t iob                          : 1;  /**< IOB interrupt enable */
	uint64_t mio                          : 1;  /**< MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< NAND / EMMC Controller interrupt enable */
	uint64_t reserved_18_18               : 1;
	uint64_t usb1                         : 1;  /**< USBDRD1 summary interrupt enable vector */
	uint64_t reserved_4_16                : 13;
	uint64_t wdog                         : 4;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_16                : 13;
	uint64_t usb1                         : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_39_38               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t pem2                         : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_intx_en4_1_cn70xx     cn70xxp1;
	struct cvmx_ciu_intx_en4_1_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt enable */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< PTP interrupt enable */
	uint64_t reserved_41_46               : 6;
	uint64_t dpi_dma                      : 1;  /**< DPI_DMA interrupt enable */
	uint64_t reserved_37_39               : 3;
	uint64_t agx0                         : 1;  /**< GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< DPI interrupt enable */
	uint64_t sli                          : 1;  /**< SLI interrupt enable */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt enable */
	uint64_t reserved_32_32               : 1;
	uint64_t key                          : 1;  /**< KEY interrupt enable */
	uint64_t rad                          : 1;  /**< RAD interrupt enable */
	uint64_t tim                          : 1;  /**< TIM interrupt enable */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< PKO interrupt enable */
	uint64_t pip                          : 1;  /**< PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< L2C interrupt enable */
	uint64_t pow                          : 1;  /**< POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< FPA interrupt enable */
	uint64_t iob                          : 1;  /**< IOB interrupt enable */
	uint64_t mio                          : 1;  /**< MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt enable */
	uint64_t reserved_4_18                : 15;
	uint64_t wdog                         : 4;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_18                : 15;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t reserved_32_32               : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_39               : 3;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_46               : 6;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_intx_en4_1 cvmx_ciu_intx_en4_1_t;

/**
 * cvmx_ciu_int#_en4_1_w1c
 *
 * Write-1-to-clear version of the CIU_INTX_EN4_1 register, read back corresponding
 * CIU_INTX_EN4_1 value.
 */
union cvmx_ciu_intx_en4_1_w1c {
	uint64_t u64;
	struct cvmx_ciu_intx_en4_1_w1c_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to clear MIO RST interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< Write 1 to clear SRIO3 interrupt enable */
	uint64_t srio2                        : 1;  /**< Write 1 to clear SRIO2 interrupt enable */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< Write 1 to clear DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< Write 1 to clear LMC0 interrupt enable */
	uint64_t srio1                        : 1;  /**< Write 1 to clear SRIO1 interrupt enable */
	uint64_t reserved_50_50               : 1;
	uint64_t pem1                         : 1;  /**< Write 1 to clear PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to clear PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to clear PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to clear AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to clear DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< Write 1 to clear GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to clear GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to clear DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to clear SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to clear DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to clear KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to clear RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to clear TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to clear ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to clear PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to clear PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to clear IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to clear L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to clear POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to clear FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to clear IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to clear MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to clear EMMC Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to clear RGMII/MII/MIX Interface 1
                                                         Interrupt enable */
	uint64_t usb1                         : 1;  /**< Write 1s to clear USBDRD1 summary interrupt enable */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t wdog                         : 16; /**< Write 1s to clear Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 16;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_50               : 1;
	uint64_t srio1                        : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} s;
	struct cvmx_ciu_intx_en4_1_w1c_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t nand                         : 1;  /**< NAND Flash Controller */
	uint64_t mii1                         : 1;  /**< Second MII Interrupt */
	uint64_t usb1                         : 1;  /**< Second USB Interrupt */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t reserved_4_15                : 12;
	uint64_t wdog                         : 4;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_15                : 12;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn52xx;
	struct cvmx_ciu_intx_en4_1_w1c_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t wdog                         : 12; /**< Watchdog summary interrupt enable vectory */
#else
	uint64_t wdog                         : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} cn56xx;
	struct cvmx_ciu_intx_en4_1_w1c_cn58xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t wdog                         : 16; /**< Watchdog summary interrupt enable vectory */
#else
	uint64_t wdog                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn58xx;
	struct cvmx_ciu_intx_en4_1_w1c_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to clear MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< Write 1 to clear LMC0 interrupt enable */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< Write 1 to clear PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to clear PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to clear PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to clear AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to clear DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< Write 1 to clear GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to clear GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to clear DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to clear SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to clear DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to clear KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to clear RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to clear TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to clear ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to clear PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to clear PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to clear IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to clear L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to clear POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to clear FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to clear IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to clear MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to clear EMMC Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to clear RGMII/MIX Interface 1
                                                         Interrupt enable */
	uint64_t reserved_4_17                : 14;
	uint64_t wdog                         : 4;  /**< Write 1s to clear Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_17                : 14;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_intx_en4_1_w1c_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to clear MIO RST interrupt enable */
	uint64_t reserved_57_62               : 6;
	uint64_t dfm                          : 1;  /**< Write 1 to clear DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< Write 1 to clear LMC0 interrupt enable */
	uint64_t srio1                        : 1;  /**< Write 1 to clear SRIO1 interrupt enable */
	uint64_t srio0                        : 1;  /**< Write 1 to clear SRIO0 interrupt enable */
	uint64_t pem1                         : 1;  /**< Write 1 to clear PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to clear PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to clear PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to clear AGL interrupt enable */
	uint64_t reserved_37_45               : 9;
	uint64_t agx0                         : 1;  /**< Write 1 to clear GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to clear DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to clear SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to clear DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to clear KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to clear RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to clear TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to clear ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to clear PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to clear PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to clear IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to clear L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to clear POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to clear FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to clear IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to clear MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to clear NAND Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to clear RGMII/MII/MIX Interface 1
                                                         Interrupt enable */
	uint64_t reserved_6_17                : 12;
	uint64_t wdog                         : 6;  /**< Write 1s to clear Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 6;
	uint64_t reserved_6_17                : 12;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_45               : 9;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t srio1                        : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_62               : 6;
	uint64_t rst                          : 1;
#endif
	} cn63xx;
	struct cvmx_ciu_intx_en4_1_w1c_cn63xx cn63xxp1;
	struct cvmx_ciu_intx_en4_1_w1c_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to clear MIO RST interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< Write 1 to clear SRIO3 interrupt enable */
	uint64_t srio2                        : 1;  /**< Write 1 to clear SRIO2 interrupt enable */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< Write 1 to clear DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< Write 1 to clear LMC0 interrupt enable */
	uint64_t reserved_51_51               : 1;
	uint64_t srio0                        : 1;  /**< Write 1 to clear SRIO0 interrupt enable */
	uint64_t pem1                         : 1;  /**< Write 1 to clear PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to clear PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to clear PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to clear AGL interrupt enable */
	uint64_t reserved_38_45               : 8;
	uint64_t agx1                         : 1;  /**< Write 1 to clear GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to clear GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to clear DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to clear SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to clear DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to clear KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to clear RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to clear TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to clear ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to clear PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to clear PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to clear IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to clear L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to clear POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to clear FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to clear IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to clear MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to clear NAND Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to clear RGMII/MII/MIX Interface 1
                                                         Interrupt enable */
	uint64_t reserved_10_17               : 8;
	uint64_t wdog                         : 10; /**< Write 1s to clear Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 10;
	uint64_t reserved_10_17               : 8;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_45               : 8;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_intx_en4_1_w1c_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to clear MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< Write 1 to clear LMC0 interrupt enable */
	uint64_t reserved_51_51               : 1;
	uint64_t pem2                         : 1;  /**< Write 1 to clear PEM2 interrupt enable */
	uint64_t pem1                         : 1;  /**< Write 1 to clear PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to clear PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to clear PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to clear AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to clear DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< Write 1 to clear GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to clear GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to clear DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to clear SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USBDRD0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to clear DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to clear KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to clear RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to clear TIM interrupt enable */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< Write 1 to clear PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to clear PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to clear IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to clear L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to clear POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to clear FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to clear IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to clear MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to clear NAND / EMMC Controller interrupt
                                                         enable */
	uint64_t reserved_18_18               : 1;
	uint64_t usb1                         : 1;  /**< Write 1s to clear USBDRD1 summary interrupt enable */
	uint64_t reserved_4_16                : 13;
	uint64_t wdog                         : 4;  /**< Write 1s to clear Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_16                : 13;
	uint64_t usb1                         : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t pem2                         : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_intx_en4_1_w1c_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en4_1_w1c_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to clear MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< Write 1 to clear LMC0 interrupt enable */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< Write 1 to clear PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to clear PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to clear PTP interrupt enable */
	uint64_t reserved_41_46               : 6;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to clear DPI_DMA interrupt enable */
	uint64_t reserved_37_39               : 3;
	uint64_t agx0                         : 1;  /**< Write 1 to clear GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to clear DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to clear SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to clear USB UCTL0 interrupt enable */
	uint64_t reserved_32_32               : 1;
	uint64_t key                          : 1;  /**< Write 1 to clear KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to clear RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to clear TIM interrupt enable */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< Write 1 to clear PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to clear PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to clear IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to clear L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to clear POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to clear FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to clear IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to clear MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to clear EMMC Flash Controller interrupt
                                                         enable */
	uint64_t reserved_4_18                : 15;
	uint64_t wdog                         : 4;  /**< Write 1s to clear Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_18                : 15;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t reserved_32_32               : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_39               : 3;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_46               : 6;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_intx_en4_1_w1c cvmx_ciu_intx_en4_1_w1c_t;

/**
 * cvmx_ciu_int#_en4_1_w1s
 *
 * Write-1-to-set version of the CIU_INTX_EN4_1 register, read back corresponding CIU_INTX_EN4_1
 * value.
 */
union cvmx_ciu_intx_en4_1_w1s {
	uint64_t u64;
	struct cvmx_ciu_intx_en4_1_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to set MIO RST interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< Write 1 to set SRIO3 interrupt enable */
	uint64_t srio2                        : 1;  /**< Write 1 to set SRIO2 interrupt enable */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< Write 1 to set DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< Write 1 to set LMC0 interrupt enable */
	uint64_t srio1                        : 1;  /**< Write 1 to set SRIO1 interrupt enable */
	uint64_t reserved_50_50               : 1;
	uint64_t pem1                         : 1;  /**< Write 1 to set PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to set PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to set PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to set AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to set DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< Write 1 to set GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to set GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to set DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to set SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to set DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to set KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to set RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to set TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to set ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to set PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to set PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to set IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to set L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to set POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to set FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to set IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to set MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to set EMMC Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to set RGMII/MII/MIX Interface 1 Interrupt
                                                         enable */
	uint64_t usb1                         : 1;  /**< Write 1s to set USBDRD1 summary interrupt enable */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t wdog                         : 16; /**< Write 1s to set Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 16;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_50               : 1;
	uint64_t srio1                        : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} s;
	struct cvmx_ciu_intx_en4_1_w1s_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t nand                         : 1;  /**< NAND Flash Controller */
	uint64_t mii1                         : 1;  /**< Second MII Interrupt */
	uint64_t usb1                         : 1;  /**< Second USB Interrupt */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t reserved_4_15                : 12;
	uint64_t wdog                         : 4;  /**< Watchdog summary interrupt enable vector */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_15                : 12;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn52xx;
	struct cvmx_ciu_intx_en4_1_w1s_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t wdog                         : 12; /**< Watchdog summary interrupt enable vectory */
#else
	uint64_t wdog                         : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} cn56xx;
	struct cvmx_ciu_intx_en4_1_w1s_cn58xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t wdog                         : 16; /**< Watchdog summary interrupt enable vectory */
#else
	uint64_t wdog                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn58xx;
	struct cvmx_ciu_intx_en4_1_w1s_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to set MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< Write 1 to set LMC0 interrupt enable */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< Write 1 to set PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to set PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to set PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to set AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to set DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< Write 1 to set GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to set GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to set DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to set SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to set DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to set KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to set RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to set TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to set ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to set PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to set PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to set IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to set L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to set POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to set FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to set IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to set MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to set EMMC Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to set RGMII/MIX Interface 1 Interrupt
                                                         enable */
	uint64_t reserved_4_17                : 14;
	uint64_t wdog                         : 4;  /**< Write 1s to set Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_17                : 14;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_intx_en4_1_w1s_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to set MIO RST interrupt enable */
	uint64_t reserved_57_62               : 6;
	uint64_t dfm                          : 1;  /**< Write 1 to set DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< Write 1 to set LMC0 interrupt enable */
	uint64_t srio1                        : 1;  /**< Write 1 to set SRIO1 interrupt enable */
	uint64_t srio0                        : 1;  /**< Write 1 to set SRIO0 interrupt enable */
	uint64_t pem1                         : 1;  /**< Write 1 to set PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to set PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to set PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to set AGL interrupt enable */
	uint64_t reserved_37_45               : 9;
	uint64_t agx0                         : 1;  /**< Write 1 to set GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to set DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to set SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to set DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to set KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to set RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to set TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to set ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to set PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to set PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to set IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to set L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to set POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to set FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to set IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to set MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to set NAND Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to set RGMII/MII/MIX Interface 1 Interrupt
                                                         enable */
	uint64_t reserved_6_17                : 12;
	uint64_t wdog                         : 6;  /**< Write 1s to set Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 6;
	uint64_t reserved_6_17                : 12;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_45               : 9;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t srio1                        : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_62               : 6;
	uint64_t rst                          : 1;
#endif
	} cn63xx;
	struct cvmx_ciu_intx_en4_1_w1s_cn63xx cn63xxp1;
	struct cvmx_ciu_intx_en4_1_w1s_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to set MIO RST interrupt enable */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< Write 1 to set SRIO3 interrupt enable */
	uint64_t srio2                        : 1;  /**< Write 1 to set SRIO2 interrupt enable */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< Write 1 to set DFM interrupt enable */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< Write 1 to set LMC0 interrupt enable */
	uint64_t reserved_51_51               : 1;
	uint64_t srio0                        : 1;  /**< Write 1 to set SRIO0 interrupt enable */
	uint64_t pem1                         : 1;  /**< Write 1 to set PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to set PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to set PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to set AGL interrupt enable */
	uint64_t reserved_38_45               : 8;
	uint64_t agx1                         : 1;  /**< Write 1 to set GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to set GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to set DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to set SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB UCTL0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to set DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to set KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to set RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to set TIM interrupt enable */
	uint64_t zip                          : 1;  /**< Write 1 to set ZIP interrupt enable */
	uint64_t pko                          : 1;  /**< Write 1 to set PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to set PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to set IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to set L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to set POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to set FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to set IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to set MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to set NAND Flash Controller interrupt
                                                         enable */
	uint64_t mii1                         : 1;  /**< Write 1 to set RGMII/MII/MIX Interface 1 Interrupt
                                                         enable */
	uint64_t reserved_10_17               : 8;
	uint64_t wdog                         : 10; /**< Write 1s to set Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 10;
	uint64_t reserved_10_17               : 8;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_45               : 8;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_intx_en4_1_w1s_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to set MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< Write 1 to set LMC0 interrupt enable */
	uint64_t reserved_51_51               : 1;
	uint64_t pem2                         : 1;  /**< Write 1 to set PEM2 interrupt enable */
	uint64_t pem1                         : 1;  /**< Write 1 to set PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to set PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to set PTP interrupt enable */
	uint64_t agl                          : 1;  /**< Write 1 to set AGL interrupt enable */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to set DPI_DMA interrupt enable */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< Write 1 to set GMX1 interrupt enable */
	uint64_t agx0                         : 1;  /**< Write 1 to set GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to set DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to set SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USBDRD0 interrupt enable */
	uint64_t dfa                          : 1;  /**< Write 1 to set DFA interrupt enable */
	uint64_t key                          : 1;  /**< Write 1 to set KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to set RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to set TIM interrupt enable */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< Write 1 to set PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to set PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to set IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to set L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to set POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to set FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to set IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to set MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to set NAND / EMMC Controller interrupt
                                                         enable */
	uint64_t reserved_18_18               : 1;
	uint64_t usb1                         : 1;  /**< Write 1s to set USBDRD1 summary interrupt enable */
	uint64_t reserved_4_16                : 13;
	uint64_t wdog                         : 4;  /**< Write 1s to set Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_16                : 13;
	uint64_t usb1                         : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t pem2                         : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_intx_en4_1_w1s_cn70xx cn70xxp1;
	struct cvmx_ciu_intx_en4_1_w1s_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< Write 1 to set MIO RST interrupt enable */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< Write 1 to set LMC0 interrupt enable */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< Write 1 to set PEM1 interrupt enable */
	uint64_t pem0                         : 1;  /**< Write 1 to set PEM0 interrupt enable */
	uint64_t ptp                          : 1;  /**< Write 1 to set PTP interrupt enable */
	uint64_t reserved_41_46               : 6;
	uint64_t dpi_dma                      : 1;  /**< Write 1 to set DPI_DMA interrupt enable */
	uint64_t reserved_37_39               : 3;
	uint64_t agx0                         : 1;  /**< Write 1 to set GMX0 interrupt enable */
	uint64_t dpi                          : 1;  /**< Write 1 to set DPI interrupt enable */
	uint64_t sli                          : 1;  /**< Write 1 to set SLI interrupt enable */
	uint64_t usb                          : 1;  /**< Write 1 to set USB UCTL0 interrupt enable */
	uint64_t reserved_32_32               : 1;
	uint64_t key                          : 1;  /**< Write 1 to set KEY interrupt enable */
	uint64_t rad                          : 1;  /**< Write 1 to set RAD interrupt enable */
	uint64_t tim                          : 1;  /**< Write 1 to set TIM interrupt enable */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< Write 1 to set PKO interrupt enable */
	uint64_t pip                          : 1;  /**< Write 1 to set PIP interrupt enable */
	uint64_t ipd                          : 1;  /**< Write 1 to set IPD interrupt enable */
	uint64_t l2c                          : 1;  /**< Write 1 to set L2C interrupt enable */
	uint64_t pow                          : 1;  /**< Write 1 to set POW err interrupt enable */
	uint64_t fpa                          : 1;  /**< Write 1 to set FPA interrupt enable */
	uint64_t iob                          : 1;  /**< Write 1 to set IOB interrupt enable */
	uint64_t mio                          : 1;  /**< Write 1 to set MIO boot interrupt enable */
	uint64_t nand                         : 1;  /**< Write 1 to set EMMC Flash Controller interrupt
                                                         enable */
	uint64_t reserved_4_18                : 15;
	uint64_t wdog                         : 4;  /**< Write 1s to set Watchdog summary interrupt enable */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_18                : 15;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t reserved_32_32               : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_39               : 3;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_46               : 6;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_intx_en4_1_w1s cvmx_ciu_intx_en4_1_w1s_t;

/**
 * cvmx_ciu_int#_sum0
 *
 * The remaining IP4 summary bits will be CIU_INTX_SUM4.
 * CIU_INT0_SUM0:  PP0/IP2
 * CIU_INT1_SUM0:  PP0/IP3
 * CIU_INT2_SUM0:  PP1/IP2
 * CIU_INT3_SUM0:  PP1/IP3
 * CIU_INT4_SUM0:  PP2/IP2
 * CIU_INT5_SUM0:  PP2/IP3
 * CIU_INT6_SUM0:  PP3/IP2
 * CIU_INT7_SUM0:  PP3/IP3
 *  - .....
 * (hole)
 * CIU_INT32_SUM0: IO 0 (PEM0).
 * CIU_INT33_SUM0: IO 1 (Reserved in o70, in seperate address group)
 */
union cvmx_ciu_intx_sum0 {
	uint64_t u64;
	struct cvmx_ciu_intx_sum0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt
                                                         See MIO_BOOT_DMA_INT*, MIO_NDF_DMA_INT */
	uint64_t mii                          : 1;  /**< RGMII/MII/MIX Interface 0 Interrupt
                                                         See MIX0_ISR */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt
                                                         See IPD_PORT_QOS_INT* */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt
                                                         See POW_IQ_INT */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt
                                                         See MIO_TWS1_INT */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt, Set when MPI transaction
                                                         finished, see MPI_CFG[INT_ENA] and MPI_STS[BUSY] */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt
                                                         See UAHC0_EHCI_USBSTS UAHC0_OHCI0_HCINTERRUPTSTATUS */
	uint64_t timer                        : 4;  /**< General timer 0-3 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_SUM2_*[TIMER] field implement all 10 CIU_TIM*
                                                         interrupts. */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt
                                                         Set any time PIP/IPD drops a packet */
	uint64_t gmx_drp                      : 2;  /**< GMX0/1 packet drop interrupt
                                                         Set any time corresponding GMX0/1 drops a packet */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt
                                                         See TRA_INT_STATUS */
	uint64_t rml                          : 1;  /**< RML Interrupt
                                                         This interrupt will assert if any bit within
                                                         CIU_BLOCK_INT is asserted. */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt
                                                         See MIO_TWS0_INT */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM1_PPX_IPx (CIU_SUM1_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_INTx_EN is set
                                                         PPs use CIU_INTx_SUM0 where x=0-7
                                                         PCIe uses the CIU_INTx_SUM0 where x=32-33.
                                                         Note that WDOG_SUM only summarizes the SUM1/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCIe MSI
                                                         See SLI_MSI_RCVn for bit <40+n> */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the SLI chapter of the spec
                                                         PCI_INT<3> = INTD
                                                         PCI_INT<2> = INTC
                                                         PCI_INT<1> = INTB
                                                         PCI_INT<0> = INTA */
	uint64_t uart                         : 2;  /**< Two UART interrupts
                                                         See MIO_UARTn_IIR[IID] for bit <34+n> */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-11
                                                          [33] is the or of <31:16>
                                                          [32] is the or of <15:0>
                                                         Two PCIe internal interrupts for entries 32-33
                                                          which equal CIU_PCI_INTA[INT] */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts
                                                         When GPIO_MULTI_CAST[EN] == 1
                                                         Write 1 to clear either the per PP or common GPIO
                                                         edge-triggered interrupts,depending on mode.
                                                         See GPIO_MULTI_CAST for all details.
                                                         When GPIO_MULTI_CAST[EN] == 0
                                                         Read Only, retain the same behavior as o63. */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         See POW_WQ_INT[WQ_INT]
                                                          1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} s;
	struct cvmx_ciu_intx_sum0_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_59_63               : 5;
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t usb                          : 1;  /**< USB interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop */
	uint64_t reserved_47_47               : 1;
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t wdog_sum                     : 1;  /**< Watchdog summary
                                                         PPs use CIU_INTx_SUM0 where x=0-1.
                                                         PCI uses the CIU_INTx_SUM0 where x=32.
                                                         Even INTx registers report WDOG to IP2
                                                         Odd INTx registers report WDOG to IP3 */
	uint64_t pci_msi                      : 4;  /**< PCI MSI
                                                         [43] is the or of <63:48>
                                                         [42] is the or of <47:32>
                                                         [41] is the or of <31:16>
                                                         [40] is the or of <15:0> */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-31
                                                          [33] is the or of <31:16>
                                                          [32] is the or of <15:0>
                                                         Two PCI internal interrupts for entry 32
                                                          CIU_PCI_INTA */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t reserved_47_47               : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t reserved_59_63               : 5;
#endif
	} cn30xx;
	struct cvmx_ciu_intx_sum0_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_59_63               : 5;
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t usb                          : 1;  /**< USB interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t wdog_sum                     : 1;  /**< Watchdog summary
                                                         PPs use CIU_INTx_SUM0 where x=0-3.
                                                         PCI uses the CIU_INTx_SUM0 where x=32.
                                                         Even INTx registers report WDOG to IP2
                                                         Odd INTx registers report WDOG to IP3 */
	uint64_t pci_msi                      : 4;  /**< PCI MSI
                                                         [43] is the or of <63:48>
                                                         [42] is the or of <47:32>
                                                         [41] is the or of <31:16>
                                                         [40] is the or of <15:0> */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-31
                                                          [33] is the or of <31:16>
                                                          [32] is the or of <15:0>
                                                         Two PCI internal interrupts for entry 32
                                                          CIU_PCI_INTA */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t reserved_59_63               : 5;
#endif
	} cn31xx;
	struct cvmx_ciu_intx_sum0_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt
                                                         KEY_ZERO will be set when the external ZERO_KEYS
                                                         pin is sampled high.  KEY_ZERO is cleared by SW */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t wdog_sum                     : 1;  /**< Watchdog summary
                                                         PPs use CIU_INTx_SUM0 where x=0-31.
                                                         PCI uses the CIU_INTx_SUM0 where x=32.
                                                         Even INTx registers report WDOG to IP2
                                                         Odd INTx registers report WDOG to IP3 */
	uint64_t pci_msi                      : 4;  /**< PCI MSI
                                                         [43] is the or of <63:48>
                                                         [42] is the or of <47:32>
                                                         [41] is the or of <31:16>
                                                         [40] is the or of <15:0> */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-31
                                                          [33] is the or of <31:16>
                                                          [32] is the or of <15:0>
                                                         Two PCI internal interrupts for entry 32
                                                          CIU_PCI_INTA */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_63               : 8;
#endif
	} cn38xx;
	struct cvmx_ciu_intx_sum0_cn38xx      cn38xxp2;
	struct cvmx_ciu_intx_sum0_cn30xx      cn50xx;
	struct cvmx_ciu_intx_sum0_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_INT_SUM1 bit is set and corresponding
                                                         enable bit in CIU_INTx_EN is set, where x
                                                         is the same as x in this CIU_INTx_SUM0.
                                                         PPs use CIU_INTx_SUM0 where x=0-7.
                                                         PCI uses the CIU_INTx_SUM0 where x=32.
                                                         Even INTx registers report WDOG to IP2
                                                         Odd INTx registers report WDOG to IP3
                                                         Note that WDOG_SUM only summarizes the SUM/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCI MSI
                                                         Refer to "Receiving Message-Signalled
                                                         Interrupts" in the PCIe chapter of the spec */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the PCIe chapter of the spec */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-7
                                                          [33] is the or of <31:16>
                                                          [32] is the or of <15:0>
                                                         Two PCI internal interrupts for entry 32
                                                          CIU_PCI_INTA */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn52xx;
	struct cvmx_ciu_intx_sum0_cn52xx      cn52xxp1;
	struct cvmx_ciu_intx_sum0_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt
                                                         KEY_ZERO will be set when the external ZERO_KEYS
                                                         pin is sampled high.  KEY_ZERO is cleared by SW */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t wdog_sum                     : 1;  /**< Watchdog summary
                                                         PPs use CIU_INTx_SUM0 where x=0-23.
                                                         PCI uses the CIU_INTx_SUM0 where x=32.
                                                         Even INTx registers report WDOG to IP2
                                                         Odd INTx registers report WDOG to IP3 */
	uint64_t pci_msi                      : 4;  /**< PCI MSI
                                                         Refer to "Receiving Message-Signalled
                                                         Interrupts" in the PCIe chapter of the spec */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the PCIe chapter of the spec */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-23
                                                          [33] is the or of <31:16>
                                                          [32] is the or of <15:0>
                                                         Two PCI internal interrupts for entry 32
                                                          CIU_PCI_INTA */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn56xx;
	struct cvmx_ciu_intx_sum0_cn56xx      cn56xxp1;
	struct cvmx_ciu_intx_sum0_cn38xx      cn58xx;
	struct cvmx_ciu_intx_sum0_cn38xx      cn58xxp1;
	struct cvmx_ciu_intx_sum0_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt
                                                         See MIO_BOOT_DMA_INT*, MIO_NDF_DMA_INT */
	uint64_t mii                          : 1;  /**< RGMII/MIX Interface 0 Interrupt
                                                         See MIX0_ISR */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt
                                                         See IPD_PORT_QOS_INT* */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt
                                                         See POW_IQ_INT */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt
                                                         See MIO_TWS1_INT */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt, Set when MPI transaction
                                                         finished, see MPI_CFG[INT_ENA] and MPI_STS[BUSY] */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt
                                                         See UAHC0_EHCI_USBSTS UAHC0_OHCI0_HCINTERRUPTSTATUS */
	uint64_t timer                        : 4;  /**< General timer 0-3 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_SUM2_*[TIMER] field implement all 10 CIU_TIM*
                                                         interrupts. */
	uint64_t sum2                         : 1;  /**< SUM2&EN2 SUMMARY bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM2_PPX_IPx (CIU_SUM2_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_EN2_PPX_IPx
                                                         (CIU_EN2_IOX_INT) is set.
                                                         Note that SUM2 only summarizes the SUM2/EN2
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt
                                                         Set any time PIP/IPD drops a packet */
	uint64_t gmx_drp                      : 2;  /**< GMX0/1 packet drop interrupt
                                                         Set any time corresponding GMX0/1 drops a packet */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt
                                                         See TRA_INT_STATUS */
	uint64_t rml                          : 1;  /**< RML Interrupt
                                                         This interrupt will assert if any bit within
                                                         CIU_BLOCK_INT is asserted. */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt
                                                         See MIO_TWS0_INT */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM1_PPX_IPx (CIU_SUM1_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_INTx_EN is set
                                                         PPs use CIU_INTx_SUM0 where x=0-7
                                                         PCIe uses the CIU_INTx_SUM0 where x=32-33.
                                                         Note that WDOG_SUM only summarizes the SUM1/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCIe MSI
                                                         See SLI_MSI_RCVn for bit <40+n> */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the SLI chapter of the spec
                                                         PCI_INT<3> = INTD
                                                         PCI_INT<2> = INTC
                                                         PCI_INT<1> = INTB
                                                         PCI_INT<0> = INTA */
	uint64_t uart                         : 2;  /**< Two UART interrupts
                                                         See MIO_UARTn_IIR[IID] for bit <34+n> */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-11
                                                          [33] is the or of <31:16>
                                                          [32] is the or of <15:0>
                                                         Two PCIe internal interrupts for entries 32-33
                                                          which equal CIU_PCI_INTA[INT] */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts
                                                         When GPIO_MULTI_CAST[EN] == 1
                                                         Write 1 to clear either the per PP or common GPIO
                                                         edge-triggered interrupts,depending on mode.
                                                         See GPIO_MULTI_CAST for all details.
                                                         When GPIO_MULTI_CAST[EN] == 0
                                                         Read Only, retain the same behavior as o63. */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         See POW_WQ_INT[WQ_INT]
                                                          1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t sum2                         : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_intx_sum0_cn52xx      cn63xx;
	struct cvmx_ciu_intx_sum0_cn52xx      cn63xxp1;
	struct cvmx_ciu_intx_sum0_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt
                                                         See MIO_BOOT_DMA_INT*, MIO_NDF_DMA_INT */
	uint64_t mii                          : 1;  /**< RGMII/MII/MIX Interface 0 Interrupt
                                                         See MIX0_ISR */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt
                                                         See IPD_PORT_QOS_INT* */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt
                                                         See POW_IQ_INT */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt
                                                         See MIO_TWS1_INT */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt, Set when MPI transaction
                                                         finished, see MPI_CFG[INT_ENA] and MPI_STS[BUSY] */
	uint64_t reserved_57_57               : 1;
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt
                                                         See UAHC0_EHCI_USBSTS UAHC0_OHCI0_HCINTERRUPTSTATUS */
	uint64_t timer                        : 4;  /**< General timer 0-3 interrupts.
                                                         Prior to pass 1.2 or
                                                          when CIU_TIM_MULTI_CAST[EN]==0, this interrupt is
                                                          common for all PP/IRQs, writing '1' to any PP/IRQ
                                                          will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                          are set at the same time, but clearing is per
                                                          cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_SUM2_*[TIMER] field implement all 10 CIU_TIM*
                                                         interrupts. */
	uint64_t sum2                         : 1;  /**< SUM2&EN2 SUMMARY bit
                                                          In pass 1.2 and subsequent passes,
                                                          this read-only bit reads as a one whenever any
                                                          CIU_SUM2_PPX_IPx (CIU_SUM2_IOX_INT)  bit is set
                                                          and corresponding enable bit in CIU_EN2_PPX_IPx
                                                          (CIU_EN2_IOX_INT) is set.
                                                          Note that SUM2 only summarizes the SUM2/EN2
                                                          result and does not have a corresponding enable
                                                          bit, so does not directly contribute to
                                                          interrupts.
                                                         Prior to pass 1.2, SUM2 did not exist and this
                                                          bit reads as zero. */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt
                                                         Set any time PIP/IPD drops a packet */
	uint64_t gmx_drp                      : 2;  /**< GMX0/1 packet drop interrupt
                                                         Set any time corresponding GMX0/1 drops a packet */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt
                                                         See TRA_INT_STATUS */
	uint64_t rml                          : 1;  /**< RML Interrupt
                                                         This interrupt will assert if any bit within
                                                         CIU_BLOCK_INT is asserted. */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt
                                                         See MIO_TWS0_INT */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM1_PPX_IPx (CIU_SUM1_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_INTx_EN is set
                                                         PPs use CIU_INTx_SUM0 where x=0-19
                                                         PCIe/sRIO uses the CIU_INTx_SUM0 where x=32-33.
                                                         Note that WDOG_SUM only summarizes the SUM1/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCIe/sRIO MSI
                                                         See SLI_MSI_RCVn for bit <40+n> */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the SLI chapter of the spec
                                                         PCI_INT<3> = INTD
                                                         PCI_INT<2> = INTC
                                                         PCI_INT<1> = INTB
                                                         PCI_INT<0> = INTA */
	uint64_t uart                         : 2;  /**< Two UART interrupts
                                                         See MIO_UARTn_IIR[IID] for bit <34+n> */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-11
                                                          [33] is the or of <31:16>
                                                          [32] is the or of <15:0>
                                                         Two PCIe/sRIO internal interrupts for entries 32-33
                                                          which equal CIU_PCI_INTA[INT] */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         See POW_WQ_INT[WQ_INT]
                                                          1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t sum2                         : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_57               : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_intx_sum0_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt
                                                         See MIO_NDF_DMA_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt
                                                         See IPD_PORT_QOS_INT* */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt
                                                         See POW_IQ_INT */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt
                                                         See MIO_TWS1_INT */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt, Set when MPI transaction
                                                         finished, see MPI_CFG[INT_ENA] and MPI_STS[BUSY] */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t reserved_56_56               : 1;
	uint64_t timer                        : 4;  /**< General timer 0-3 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         TIMERn (n=0..3) will clear all TIMERn(n=0..3) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERn(n=0..3)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_SUM2_*[TIMER] field implement all 10 CIU_TIM*
                                                         interrupts. */
	uint64_t sum2                         : 1;  /**< SUM2&EN2 SUMMARY bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM2_PPX_IPx (CIU_SUM2_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_EN2_PPX_IPx
                                                         (CIU_EN2_IOX_INT) is set.
                                                         Note that SUM2 only summarizes the SUM2/EN2
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt
                                                         Set any time PIP/IPD drops a packet */
	uint64_t gmx_drp                      : 2;  /**< GMX0/1 packet drop interrupt
                                                         Set any time corresponding GMX0/1 drops a packet */
	uint64_t reserved_46_47               : 2;
	uint64_t twsi                         : 1;  /**< TWSI Interrupt
                                                         See MIO_TWS0_INT */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM1_PPX_IPx (CIU_SUM1_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_INTx_EN is set
                                                         PPs use CIU_INTx_SUM0 where x=0-7
                                                         PCIe uses the CIU_INTx_SUM0 where x=32-33.
                                                         Note that WDOG_SUM only summarizes the SUM1/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCIe MSI
                                                         See SLI_MSI_RCVn for bit <40+n> */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the SLI chapter of the spec
                                                         PCI_INT<3> = INTD
                                                         PCI_INT<2> = INTC
                                                         PCI_INT<1> = INTB
                                                         PCI_INT<0> = INTA */
	uint64_t uart                         : 2;  /**< Two UART interrupts
                                                         See MIO_UARTn_IIR[IID] for bit <34+n> */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts        for entries 0-11
                                                         [33] is the or of <31:16>
                                                         [32] is the or of <15:0>
                                                         Two PCIe internal interrupts for entries 32-33
                                                         which equal CIU_PCI_INTA[INT] */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts
                                                         When GPIO_MULTI_CAST[EN] == 1
                                                         Write 1 to clear either the per PP or common GPIO
                                                         edge-triggered interrupts,depending on mode.
                                                         See GPIO_MULTI_CAST for all details.
                                                         When GPIO_MULTI_CAST[EN] == 0
                                                         Read Only, retain the same behavior as o63. */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         See POW_WQ_INT[WQ_INT]
                                                         1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t reserved_46_47               : 2;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t sum2                         : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_56               : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_intx_sum0_cn70xx      cn70xxp1;
	struct cvmx_ciu_intx_sum0_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt
                                                         See MIO_BOOT_DMA_INT*, MIO_NDF_DMA_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt
                                                         See IPD_PORT_QOS_INT* */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt
                                                         See POW_IQ_INT */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt
                                                         See MIO_TWS1_INT */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt, Set when MPI transaction
                                                         finished, see MPI_CFG[INT_ENA] and MPI_STS[BUSY] */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt
                                                         See UAHC0_EHCI_USBSTS UAHC0_OHCI0_HCINTERRUPTSTATUS */
	uint64_t timer                        : 4;  /**< General timer 0-3 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_SUM2_*[TIMER] field implement all 10 CIU_TIM*
                                                         interrupts. */
	uint64_t sum2                         : 1;  /**< SUM2&EN2 SUMMARY bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM2_PPX_IPx (CIU_SUM2_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_EN2_PPX_IPx
                                                         (CIU_EN2_IOX_INT) is set.
                                                         Note that SUM2 only summarizes the SUM2/EN2
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt
                                                         Set any time PIP/IPD drops a packet */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX0/1 packet drop interrupt
                                                         Set any time corresponding GMX0/1 drops a packet */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt
                                                         See TRA_INT_STATUS */
	uint64_t rml                          : 1;  /**< RML Interrupt
                                                         This interrupt will assert if any bit within
                                                         CIU_BLOCK_INT is asserted. */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt
                                                         See MIO_TWS0_INT */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM1_PPX_IPx (CIU_SUM1_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_INTx_EN is set
                                                         PPs use CIU_INTx_SUM0 where x=0-7
                                                         PCIe uses the CIU_INTx_SUM0 where x=32-33.
                                                         Note that WDOG_SUM only summarizes the SUM1/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCIe MSI
                                                         See SLI_MSI_RCVn for bit <40+n> */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the SLI chapter of the spec
                                                         PCI_INT<3> = INTD
                                                         PCI_INT<2> = INTC
                                                         PCI_INT<1> = INTB
                                                         PCI_INT<0> = INTA */
	uint64_t uart                         : 2;  /**< Two UART interrupts
                                                         See MIO_UARTn_IIR[IID] for bit <34+n> */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-11
                                                          [33] is the or of <31:16>
                                                          [32] is the or of <15:0>
                                                         Two PCIe internal interrupts for entries 32-33
                                                          which equal CIU_PCI_INTA[INT] */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts
                                                         When GPIO_MULTI_CAST[EN] == 1
                                                         Write 1 to clear either the per PP or common GPIO
                                                         edge-triggered interrupts,depending on mode.
                                                         See GPIO_MULTI_CAST for all details.
                                                         When GPIO_MULTI_CAST[EN] == 0
                                                         Read Only, retain the same behavior as o63. */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         See POW_WQ_INT[WQ_INT]
                                                          1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t sum2                         : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_intx_sum0 cvmx_ciu_intx_sum0_t;

/**
 * cvmx_ciu_int#_sum4
 *
 * CIU_INT0_SUM4:   PP0  /IP4
 * CIU_INT1_SUM4:   PP1  /IP4
 * - ...
 * CIU_INT3_SUM4:   PP3  /IP4
 */
union cvmx_ciu_intx_sum4 {
	uint64_t u64;
	struct cvmx_ciu_intx_sum4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt
                                                         See MIO_BOOT_DMA_INT*, MIO_NDF_DMA_INT */
	uint64_t mii                          : 1;  /**< RGMII/MII/MIX Interface 0 Interrupt
                                                         See MIX0_ISR */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt
                                                         See IPD_PORT_QOS_INT* */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt
                                                         See POW_IQ_INT */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt
                                                         See MIO_TWS1_INT */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt
                                                         See UAHC0_EHCI_USBSTS UAHC0_OHCI0_HCINTERRUPTSTATUS */
	uint64_t timer                        : 4;  /**< General timer 0-3 interrupts
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_SUM2_*[TIMER] field implement all 10 CIU_TIM*
                                                         interrupts. */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt
                                                         Set any time PIP/IPD drops a packet */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop interrupt
                                                         Set any time corresponding GMX drops a packet */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt
                                                         See TRA_INT_STATUS */
	uint64_t rml                          : 1;  /**< RML Interrupt
                                                         This bit is set when any bit is set in
                                                         CIU_BLOCK_INT. */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt
                                                         See MIO_TWS0_INT */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM1_PPX_IPx (CIU_SUM1_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_INTx_EN is set
                                                         PPs use CIU_INTx_SUM0 where x=0-19
                                                         PCIe uses the CIU_INTx_SUM0 where x=32-33.
                                                         Note that WDOG_SUM only summarizes the SUM1/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCIe MSI
                                                         See SLI_MSI_RCVn for bit <40+n> */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the SLI chapter of the spec
                                                         PCI_INT<3> = INTD
                                                         PCI_INT<2> = INTC
                                                         PCI_INT<1> = INTB
                                                         PCI_INT<0> = INTA */
	uint64_t uart                         : 2;  /**< Two UART interrupts
                                                         See MIO_UARTn_IIR[IID] for bit <34+n> */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-5
                                                         [33] is the or of <31:16>
                                                         [32] is the or of <15:0> */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts
                                                         When GPIO_MULTI_CAST[EN] == 1
                                                         Write 1 to clear either the per PP interrupt or
                                                         common GPIO interrupt for all PP/IOs,depending
                                                         on mode setting. This will apply to all 16 GPIOs.
                                                         See GPIO_MULTI_CAST for all details
                                                         When GPIO_MULTI_CAST[EN] == 0
                                                         Read Only, retain the same behavior as o63. */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         See POW_WQ_INT[WQ_INT]
                                                          1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} s;
	struct cvmx_ciu_intx_sum4_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_59_63               : 5;
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t usb                          : 1;  /**< USB interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop */
	uint64_t reserved_47_47               : 1;
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t wdog_sum                     : 1;  /**< Watchdog summary
                                                         PPs use CIU_INTx_SUM4 where x=0-1. */
	uint64_t pci_msi                      : 4;  /**< PCI MSI
                                                         [43] is the or of <63:48>
                                                         [42] is the or of <47:32>
                                                         [41] is the or of <31:16>
                                                         [40] is the or of <15:0> */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-31
                                                          [33] is the or of <31:16>
                                                          [32] is the or of <15:0>
                                                         Two PCI internal interrupts for entry 32
                                                          CIU_PCI_INTA */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t reserved_47_47               : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t reserved_59_63               : 5;
#endif
	} cn50xx;
	struct cvmx_ciu_intx_sum4_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN4_1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_INT_SUM1 bit is set and corresponding
                                                         enable bit in CIU_INTx_EN4_1 is set, where x
                                                         is the same as x in this CIU_INTx_SUM4.
                                                         PPs use CIU_INTx_SUM4 for IP4, where x=PPid.
                                                         Note that WDOG_SUM only summarizes the SUM/EN4_1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCI MSI
                                                         Refer to "Receiving Message-Signalled
                                                         Interrupts" in the PCIe chapter of the spec */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the PCIe chapter of the spec */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-3
                                                         [33] is the or of <31:16>
                                                         [32] is the or of <15:0> */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn52xx;
	struct cvmx_ciu_intx_sum4_cn52xx      cn52xxp1;
	struct cvmx_ciu_intx_sum4_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt */
	uint64_t mii                          : 1;  /**< MII Interface Interrupt */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB Interrupt */
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt
                                                         KEY_ZERO will be set when the external ZERO_KEYS
                                                         pin is sampled high.  KEY_ZERO is cleared by SW */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t wdog_sum                     : 1;  /**< Watchdog summary
                                                         These registers report WDOG to IP4 */
	uint64_t pci_msi                      : 4;  /**< PCI MSI
                                                         Refer to "Receiving Message-Signalled
                                                         Interrupts" in the PCIe chapter of the spec */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the PCIe chapter of the spec */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-11
                                                         [33] is the or of <31:16>
                                                         [32] is the or of <15:0> */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn56xx;
	struct cvmx_ciu_intx_sum4_cn56xx      cn56xxp1;
	struct cvmx_ciu_intx_sum4_cn58xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t timer                        : 4;  /**< General timer interrupts */
	uint64_t key_zero                     : 1;  /**< Key Zeroization interrupt
                                                         KEY_ZERO will be set when the external ZERO_KEYS
                                                         pin is sampled high.  KEY_ZERO is cleared by SW */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop */
	uint64_t trace                        : 1;  /**< L2C has the CMB trace buffer */
	uint64_t rml                          : 1;  /**< RML Interrupt */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt */
	uint64_t wdog_sum                     : 1;  /**< Watchdog summary
                                                         These registers report WDOG to IP4 */
	uint64_t pci_msi                      : 4;  /**< PCI MSI
                                                         [43] is the or of <63:48>
                                                         [42] is the or of <47:32>
                                                         [41] is the or of <31:16>
                                                         [40] is the or of <15:0> */
	uint64_t pci_int                      : 4;  /**< PCI INTA/B/C/D */
	uint64_t uart                         : 2;  /**< Two UART interrupts */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-31
                                                          [33] is the or of <31:16>
                                                          [32] is the or of <15:0>
                                                         Two PCI internal interrupts for entry 32
                                                          CIU_PCI_INTA */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t key_zero                     : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_63               : 8;
#endif
	} cn58xx;
	struct cvmx_ciu_intx_sum4_cn58xx      cn58xxp1;
	struct cvmx_ciu_intx_sum4_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt
                                                         See MIO_BOOT_DMA_INT*, MIO_NDF_DMA_INT */
	uint64_t mii                          : 1;  /**< RGMII/MIX Interface 0 Interrupt
                                                         See MIX0_ISR */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt
                                                         See IPD_PORT_QOS_INT* */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt
                                                         See POW_IQ_INT */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt
                                                         See MIO_TWS1_INT */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt
                                                         See UAHC0_EHCI_USBSTS UAHC0_OHCI0_HCINTERRUPTSTATUS */
	uint64_t timer                        : 4;  /**< General timer 0-3 interrupts
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_SUM2_*[TIMER] field implement all 10 CIU_TIM*
                                                         interrupts. */
	uint64_t sum2                         : 1;  /**< SUM2&EN2 SUMMARY bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM2_PPX_IPx (CIU_SUM2_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_EN2_PPX_IPx
                                                         (CIU_EN2_IOX_INT) is set.
                                                         Note that WDOG_SUM only summarizes the SUM2/EN2
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt
                                                         Set any time PIP/IPD drops a packet */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop interrupt
                                                         Set any time corresponding GMX drops a packet */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt
                                                         See TRA_INT_STATUS */
	uint64_t rml                          : 1;  /**< RML Interrupt
                                                         This bit is set when any bit is set in
                                                         CIU_BLOCK_INT. */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt
                                                         See MIO_TWS0_INT */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM1_PPX_IPx (CIU_SUM1_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_INTx_EN is set
                                                         PPs use CIU_INTx_SUM0 where x=0-19
                                                         PCIe uses the CIU_INTx_SUM0 where x=32-33.
                                                         Note that WDOG_SUM only summarizes the SUM1/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCIe MSI
                                                         See SLI_MSI_RCVn for bit <40+n> */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the SLI chapter of the spec
                                                         PCI_INT<3> = INTD
                                                         PCI_INT<2> = INTC
                                                         PCI_INT<1> = INTB
                                                         PCI_INT<0> = INTA */
	uint64_t uart                         : 2;  /**< Two UART interrupts
                                                         See MIO_UARTn_IIR[IID] for bit <34+n> */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-5
                                                         [33] is the or of <31:16>
                                                         [32] is the or of <15:0> */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts
                                                         When GPIO_MULTI_CAST[EN] == 1
                                                         Write 1 to clear either the per PP interrupt or
                                                         common GPIO interrupt for all PP/IOs,depending
                                                         on mode setting. This will apply to all 16 GPIOs.
                                                         See GPIO_MULTI_CAST for all details
                                                         When GPIO_MULTI_CAST[EN] == 0
                                                         Read Only, retain the same behavior as o63. */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         See POW_WQ_INT[WQ_INT]
                                                          1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t sum2                         : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_intx_sum4_cn52xx      cn63xx;
	struct cvmx_ciu_intx_sum4_cn52xx      cn63xxp1;
	struct cvmx_ciu_intx_sum4_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt
                                                         See MIO_BOOT_DMA_INT*, MIO_NDF_DMA_INT */
	uint64_t mii                          : 1;  /**< RGMII/MII/MIX Interface 0 Interrupt
                                                         See MIX0_ISR */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt
                                                         See IPD_PORT_QOS_INT* */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt
                                                         See POW_IQ_INT */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt
                                                         See MIO_TWS1_INT */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt */
	uint64_t reserved_57_57               : 1;
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt
                                                         See UAHC0_EHCI_USBSTS UAHC0_OHCI0_HCINTERRUPTSTATUS */
	uint64_t timer                        : 4;  /**< General timer 0-3 interrupts.
                                                         Prior to pass 1.2 or
                                                          when CIU_TIM_MULTI_CAST[EN]==0, this interrupt is
                                                          common for all PP/IRQs, writing '1' to any PP/IRQ
                                                          will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                          are set at the same time, but clearing is per
                                                          cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_SUM2_*[TIMER] field implement all 10 CIU_TIM*
                                                         interrupts. */
	uint64_t sum2                         : 1;  /**< SUM2&EN2 SUMMARY bit
                                                          In pass 1.2 and subsequent passes,
                                                          this read-only bit reads as a one whenever any
                                                          CIU_SUM2_PPX_IPx (CIU_SUM2_IOX_INT)  bit is set
                                                          and corresponding enable bit in CIU_EN2_PPX_IPx
                                                          (CIU_EN2_IOX_INT) is set.
                                                          Note that WDOG_SUM only summarizes the SUM2/EN2
                                                          result and does not have a corresponding enable
                                                          bit, so does not directly contribute to
                                                          interrupts.
                                                         Prior to pass 1.2, SUM2 did not exist and this
                                                          bit reads as zero. */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt
                                                         Set any time PIP/IPD drops a packet */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop interrupt
                                                         Set any time corresponding GMX drops a packet */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt
                                                         See TRA_INT_STATUS */
	uint64_t rml                          : 1;  /**< RML Interrupt
                                                         This bit is set when any bit is set in
                                                         CIU_BLOCK_INT. */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt
                                                         See MIO_TWS0_INT */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM1_PPX_IPx (CIU_SUM1_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_INTx_EN is set
                                                         PPs use CIU_INTx_SUM0 where x=0-19
                                                         PCIe/sRIO uses the CIU_INTx_SUM0 where x=32-33.
                                                         Note that WDOG_SUM only summarizes the SUM1/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCIe/sRIO MSI
                                                         See SLI_MSI_RCVn for bit <40+n> */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the SLI chapter of the spec
                                                         PCI_INT<3> = INTD
                                                         PCI_INT<2> = INTC
                                                         PCI_INT<1> = INTB
                                                         PCI_INT<0> = INTA */
	uint64_t uart                         : 2;  /**< Two UART interrupts
                                                         See MIO_UARTn_IIR[IID] for bit <34+n> */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-5
                                                         [33] is the or of <31:16>
                                                         [32] is the or of <15:0> */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         See POW_WQ_INT[WQ_INT]
                                                          1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t sum2                         : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_57               : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_intx_sum4_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt
                                                         See MIO_NDF_DMA_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt
                                                         See IPD_PORT_QOS_INT* */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt
                                                         See POW_IQ_INT */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt
                                                         See MIO_TWS1_INT */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t reserved_56_56               : 1;
	uint64_t timer                        : 4;  /**< General timer 0-3 interrupts
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_SUM2_*[TIMER] field implement all 10 CIU_TIM*
                                                         interrupts. */
	uint64_t sum2                         : 1;  /**< SUM2&EN2 SUMMARY bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM2_PPX_IPx (CIU_SUM2_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_EN2_PPX_IPx
                                                         (CIU_EN2_IOX_INT) is set.
                                                         Note that WDOG_SUM only summarizes the SUM2/EN2
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt
                                                         Set any time PIP/IPD drops a packet */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop interrupt
                                                         Set any time corresponding GMX drops a packet */
	uint64_t reserved_46_47               : 2;
	uint64_t twsi                         : 1;  /**< TWSI Interrupt
                                                         See MIO_TWS0_INT */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM1_PPX_IPx (CIU_SUM1_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_INTx_EN is set
                                                         PPs use CIU_INTx_SUM0 where x=0-19
                                                         PCIe uses the CIU_INTx_SUM0 where x=32-33.
                                                         Note that WDOG_SUM only summarizes the SUM1/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCIe MSI
                                                         See SLI_MSI_RCVn for bit <40+n> */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the SLI chapter of the spec
                                                         PCI_INT<3> = INTD
                                                         PCI_INT<2> = INTC
                                                         PCI_INT<1> = INTB
                                                         PCI_INT<0> = INTA */
	uint64_t uart                         : 2;  /**< Two UART interrupts
                                                         See MIO_UARTn_IIR[IID] for bit <34+n> */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts        for entries 0-5
                                                         [33] is the or of <31:16>
                                                         [32] is the or of <15:0> */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts
                                                         When GPIO_MULTI_CAST[EN] == 1
                                                         Write 1 to clear either the per PP interrupt or
                                                         common GPIO interrupt for all PP/IOs,depending
                                                         on mode setting. This will apply to all 16 GPIOs.
                                                         See GPIO_MULTI_CAST for all details
                                                         When GPIO_MULTI_CAST[EN] == 0
                                                         Read Only, retain the same behavior as o63. */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         See POW_WQ_INT[WQ_INT]
                                                         1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t reserved_46_47               : 2;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t sum2                         : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_56               : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_intx_sum4_cn70xx      cn70xxp1;
	struct cvmx_ciu_intx_sum4_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt
                                                         See MIO_BOOT_DMA_INT*, MIO_NDF_DMA_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt
                                                         See IPD_PORT_QOS_INT* */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt
                                                         See POW_IQ_INT */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt
                                                         See MIO_TWS1_INT */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt
                                                         See UAHC0_EHCI_USBSTS UAHC0_OHCI0_HCINTERRUPTSTATUS */
	uint64_t timer                        : 4;  /**< General timer 0-3 interrupts
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_SUM2_*[TIMER] field implement all 10 CIU_TIM*
                                                         interrupts. */
	uint64_t sum2                         : 1;  /**< SUM2&EN2 SUMMARY bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM2_PPX_IPx (CIU_SUM2_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_EN2_PPX_IPx
                                                         (CIU_EN2_IOX_INT) is set.
                                                         Note that WDOG_SUM only summarizes the SUM2/EN2
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt
                                                         Set any time PIP/IPD drops a packet */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop interrupt
                                                         Set any time corresponding GMX drops a packet */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt
                                                         See TRA_INT_STATUS */
	uint64_t rml                          : 1;  /**< RML Interrupt
                                                         This bit is set when any bit is set in
                                                         CIU_BLOCK_INT. */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt
                                                         See MIO_TWS0_INT */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM1_PPX_IPx (CIU_SUM1_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_INTx_EN is set
                                                         PPs use CIU_INTx_SUM0 where x=0-19
                                                         PCIe uses the CIU_INTx_SUM0 where x=32-33.
                                                         Note that WDOG_SUM only summarizes the SUM1/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCIe MSI
                                                         See SLI_MSI_RCVn for bit <40+n> */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the SLI chapter of the spec
                                                         PCI_INT<3> = INTD
                                                         PCI_INT<2> = INTC
                                                         PCI_INT<1> = INTB
                                                         PCI_INT<0> = INTA */
	uint64_t uart                         : 2;  /**< Two UART interrupts
                                                         See MIO_UARTn_IIR[IID] for bit <34+n> */
	uint64_t mbox                         : 2;  /**< Two mailbox interrupts for entries 0-5
                                                         [33] is the or of <31:16>
                                                         [32] is the or of <15:0> */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts
                                                         When GPIO_MULTI_CAST[EN] == 1
                                                         Write 1 to clear either the per PP interrupt or
                                                         common GPIO interrupt for all PP/IOs,depending
                                                         on mode setting. This will apply to all 16 GPIOs.
                                                         See GPIO_MULTI_CAST for all details
                                                         When GPIO_MULTI_CAST[EN] == 0
                                                         Read Only, retain the same behavior as o63. */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         See POW_WQ_INT[WQ_INT]
                                                          1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t sum2                         : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_intx_sum4 cvmx_ciu_intx_sum4_t;

/**
 * cvmx_ciu_int33_sum0
 *
 * This bit is associated with CIU_INTX_SUM0. Reserved for o70 for future expansion.
 *
 */
union cvmx_ciu_int33_sum0 {
	uint64_t u64;
	struct cvmx_ciu_int33_sum0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt
                                                         See MIO_BOOT_DMA_INT*, MIO_NDF_DMA_INT */
	uint64_t mii                          : 1;  /**< RGMII/MII/MIX Interface 0 Interrupt
                                                         See MIX0_ISR */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt
                                                         See IPD_PORT_QOS_INT* */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt
                                                         See POW_IQ_INT */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt
                                                         See MIO_TWS1_INT */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt
                                                         See UAHC0_EHCI_USBSTS UAHC0_OHCI0_HCINTERRUPTSTATUS */
	uint64_t timer                        : 4;  /**< General timer 0-3 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_SUM2_*[TIMER] field implement all 10 CIU_TIM*
                                                         interrupts. */
	uint64_t sum2                         : 1;  /**< SUM2&EN2 SUMMARY bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM2_PPX_IPx (CIU_SUM2_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_EN2_PPX_IPx
                                                         (CIU_EN2_IOX_INT) is set.
                                                         Note that SUM2 only summarizes the SUM2/EN2
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt
                                                         Set any time PIP/IPD drops a packet */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop interrupt
                                                         Set any time corresponding GMX drops a packet */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt
                                                         See TRA_INT_STATUS */
	uint64_t rml                          : 1;  /**< RML Interrupt
                                                         This interrupt will assert if any bit within
                                                         CIU_BLOCK_INT is asserted. */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt
                                                         See MIO_TWS0_INT */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM1_PPX_IPx bit is set and corresponding
                                                         enable bit in CIU_INTx_EN is set, where x
                                                         is the same as x in this CIU_INTx_SUM0.
                                                         PPs use CIU_INTx_SUM0 where x=0-7.
                                                         PCIe uses the CIU_INTx_SUM0 where x=32-33.
                                                         Note that WDOG_SUM only summarizes the SUM1/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCIe MSI
                                                         See SLI_MSI_RCVn for bit <40+n> */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the SLI chapter of the spec
                                                         PCI_INT<3> = INTD
                                                         PCI_INT<2> = INTC
                                                         PCI_INT<1> = INTB
                                                         PCI_INT<0> = INTA */
	uint64_t uart                         : 2;  /**< Two UART interrupts
                                                         See MIO_UARTn_IIR[IID] for bit <34+n> */
	uint64_t mbox                         : 2;  /**< A read-only copy of CIU_PCI_INTA[INT] */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts
                                                         When GPIO_MULTI_CAST[EN] == 1
                                                         Write 1 to clear either the per PP or common GPIO
                                                         edge-triggered interrupts,depending on mode.
                                                         See GPIO_MULTI_CAST for all details.
                                                         When GPIO_MULTI_CAST[EN] == 0
                                                         Read Only, retain the same behavior as o63. */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         See POW_WQ_INT[WQ_INT]
                                                          1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t sum2                         : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} s;
	struct cvmx_ciu_int33_sum0_s          cn61xx;
	struct cvmx_ciu_int33_sum0_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt
                                                         See MIO_BOOT_DMA_INT*, MIO_NDF_DMA_INT */
	uint64_t mii                          : 1;  /**< RGMII/MII/MIX Interface 0 Interrupt
                                                         See MIX0_ISR */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt
                                                         See IPD_PORT_QOS_INT* */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt
                                                         See POW_IQ_INT */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt
                                                         See MIO_TWS1_INT */
	uint64_t reserved_57_58               : 2;
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt
                                                         See UAHC0_EHCI_USBSTS UAHC0_OHCI0_HCINTERRUPTSTATUS */
	uint64_t timer                        : 4;  /**< General timer interrupts
                                                         Set any time the corresponding CIU timer expires */
	uint64_t reserved_51_51               : 1;
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt
                                                         Set any time PIP/IPD drops a packet */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop interrupt
                                                         Set any time corresponding GMX drops a packet */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt
                                                         See TRA_INT_STATUS */
	uint64_t rml                          : 1;  /**< RML Interrupt
                                                         This interrupt will assert if any bit within
                                                         CIU_BLOCK_INT is asserted. */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt
                                                         See MIO_TWS0_INT */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_INT_SUM1 bit is set and corresponding
                                                         enable bit in CIU_INTx_EN is set, where x
                                                         is the same as x in this CIU_INTx_SUM0.
                                                         PPs use CIU_INTx_SUM0 where x=0-11.
                                                         PCIe/sRIO uses the CIU_INTx_SUM0 where x=32-33.
                                                         Even INTx registers report WDOG to IP2
                                                         Odd INTx registers report WDOG to IP3
                                                         Note that WDOG_SUM only summarizes the SUM/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCIe/sRIO MSI
                                                         See SLI_MSI_RCVn for bit <40+n> */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the SLI chapter of the spec
                                                         PCI_INT<3> = INTD
                                                         PCI_INT<2> = INTC
                                                         PCI_INT<1> = INTB
                                                         PCI_INT<0> = INTA */
	uint64_t uart                         : 2;  /**< Two UART interrupts
                                                         See MIO_UARTn_IIR[IID] for bit <34+n> */
	uint64_t mbox                         : 2;  /**< A read-only copy of CIU_PCI_INTA[INT] */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         See POW_WQ_INT[WQ_INT]
                                                          1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_58               : 2;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn63xx;
	struct cvmx_ciu_int33_sum0_cn63xx     cn63xxp1;
	struct cvmx_ciu_int33_sum0_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt
                                                         See MIO_BOOT_DMA_INT*, MIO_NDF_DMA_INT */
	uint64_t mii                          : 1;  /**< RGMII/MII/MIX Interface 0 Interrupt
                                                         See MIX0_ISR */
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt
                                                         See IPD_PORT_QOS_INT* */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt
                                                         See POW_IQ_INT */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt
                                                         See MIO_TWS1_INT */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt */
	uint64_t reserved_57_57               : 1;
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt
                                                         See UAHC0_EHCI_USBSTS UAHC0_OHCI0_HCINTERRUPTSTATUS */
	uint64_t timer                        : 4;  /**< General timer 0-3 interrupts.
                                                         Prior to pass 1.2 or
                                                          when CIU_TIM_MULTI_CAST[EN]==0, this interrupt is
                                                          common for all PP/IRQs, writing '1' to any PP/IRQ
                                                          will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                          are set at the same time, but clearing is per
                                                          cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_SUM2_*[TIMER] field implement all 10 CIU_TIM*
                                                         interrupts. */
	uint64_t sum2                         : 1;  /**< SUM2&EN2 SUMMARY bit
                                                          In pass 1.2 and subsequent passes,
                                                          this read-only bit reads as a one whenever any
                                                          CIU_SUM2_PPX_IPx (CIU_SUM2_IOX_INT)  bit is set
                                                          and corresponding enable bit in CIU_EN2_PPX_IPx
                                                          (CIU_EN2_IOX_INT) is set.
                                                          Note that SUM2 only summarizes the SUM2/EN2
                                                          result and does not have a corresponding enable
                                                          bit, so does not directly contribute to
                                                          interrupts.
                                                         Prior to pass 1.2, SUM2 did not exist and this
                                                          bit reads as zero. */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt
                                                         Set any time PIP/IPD drops a packet */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop interrupt
                                                         Set any time corresponding GMX drops a packet */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt
                                                         See TRA_INT_STATUS */
	uint64_t rml                          : 1;  /**< RML Interrupt
                                                         This interrupt will assert if any bit within
                                                         CIU_BLOCK_INT is asserted. */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt
                                                         See MIO_TWS0_INT */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM1_PPX_IPx (CIU_SUM1_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_INTx_EN is set
                                                         PPs use CIU_INTx_SUM0 where x=0-19
                                                         PCIe/sRIO uses the CIU_INTx_SUM0 where x=32-33.
                                                         Note that WDOG_SUM only summarizes the SUM1/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCIe/sRIO MSI
                                                         See SLI_MSI_RCVn for bit <40+n> */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the SLI chapter of the spec
                                                         PCI_INT<3> = INTD
                                                         PCI_INT<2> = INTC
                                                         PCI_INT<1> = INTB
                                                         PCI_INT<0> = INTA */
	uint64_t uart                         : 2;  /**< Two UART interrupts
                                                         See MIO_UARTn_IIR[IID] for bit <34+n> */
	uint64_t mbox                         : 2;  /**< A read-only copy of CIU_PCI_INTA[INT] */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         See POW_WQ_INT[WQ_INT]
                                                          1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t sum2                         : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t reserved_57_57               : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t mii                          : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_int33_sum0_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt
                                                         See MIO_NDF_DMA_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt
                                                         See IPD_PORT_QOS_INT* */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt
                                                         See POW_IQ_INT */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt
                                                         See MIO_TWS1_INT */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t reserved_56_56               : 1;
	uint64_t timer                        : 4;  /**< General timer 0-3 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_SUM2_*[TIMER] field implement all 10 CIU_TIM*
                                                         interrupts. */
	uint64_t sum2                         : 1;  /**< SUM2&EN2 SUMMARY bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM2_PPX_IPx (CIU_SUM2_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_EN2_PPX_IPx
                                                         (CIU_EN2_IOX_INT) is set.
                                                         Note that SUM2 only summarizes the SUM2/EN2
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt
                                                         Set any time PIP/IPD drops a packet */
	uint64_t gmx_drp                      : 2;  /**< GMX packet drop interrupt
                                                         Set any time corresponding GMX drops a packet */
	uint64_t reserved_47_46               : 2;
	uint64_t twsi                         : 1;  /**< TWSI Interrupt
                                                         See MIO_TWS0_INT */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM1_PPX_IPx bit is set and corresponding
                                                         enable bit in CIU_INTx_EN is set, where x
                                                         is the same as x in this CIU_INTx_SUM0.
                                                         PPs use CIU_INTx_SUM0 where x=0-7.
                                                         PCIe uses the CIU_INTx_SUM0 where x=32-33.
                                                         Note that WDOG_SUM only summarizes the SUM1/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCIe MSI
                                                         See SLI_MSI_RCVn for bit <40+n> */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the SLI chapter of the spec
                                                         PCI_INT<3> = INTD
                                                         PCI_INT<2> = INTC
                                                         PCI_INT<1> = INTB
                                                         PCI_INT<0> = INTA */
	uint64_t uart                         : 2;  /**< Two UART interrupts
                                                         See MIO_UARTn_IIR[IID] for bit <34+n> */
	uint64_t mbox                         : 2;  /**< A read-only copy of CIU_PCI_INTA[INT] */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts
                                                         When GPIO_MULTI_CAST[EN] == 1
                                                         Write 1 to clear either the per PP or common GPIO
                                                         edge-triggered interrupts,depending on mode.
                                                         See GPIO_MULTI_CAST for all details.
                                                         When GPIO_MULTI_CAST[EN] == 0
                                                         Read Only, retain the same behavior as o63. */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         See POW_WQ_INT[WQ_INT]
                                                         1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t reserved_47_46               : 2;
	uint64_t gmx_drp                      : 2;
	uint64_t ipd_drp                      : 1;
	uint64_t sum2                         : 1;
	uint64_t timer                        : 4;
	uint64_t reserved_56_56               : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_int33_sum0_cn70xx     cn70xxp1;
	struct cvmx_ciu_int33_sum0_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bootdma                      : 1;  /**< Boot bus DMA engines Interrupt
                                                         See MIO_BOOT_DMA_INT*, MIO_NDF_DMA_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t ipdppthr                     : 1;  /**< IPD per-port counter threshold interrupt
                                                         See IPD_PORT_QOS_INT* */
	uint64_t powiq                        : 1;  /**< POW IQ interrupt
                                                         See POW_IQ_INT */
	uint64_t twsi2                        : 1;  /**< 2nd TWSI Interrupt
                                                         See MIO_TWS1_INT */
	uint64_t mpi                          : 1;  /**< MPI/SPI interrupt */
	uint64_t pcm                          : 1;  /**< PCM/TDM interrupt */
	uint64_t usb                          : 1;  /**< USB EHCI or OHCI Interrupt
                                                         See UAHC0_EHCI_USBSTS UAHC0_OHCI0_HCINTERRUPTSTATUS */
	uint64_t timer                        : 4;  /**< General timer 0-3 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_SUM2_*[TIMER] field implement all 10 CIU_TIM*
                                                         interrupts. */
	uint64_t sum2                         : 1;  /**< SUM2&EN2 SUMMARY bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM2_PPX_IPx (CIU_SUM2_IOX_INT)  bit is set
                                                         and corresponding enable bit in CIU_EN2_PPX_IPx
                                                         (CIU_EN2_IOX_INT) is set.
                                                         Note that SUM2 only summarizes the SUM2/EN2
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t ipd_drp                      : 1;  /**< IPD QOS packet drop interrupt
                                                         Set any time PIP/IPD drops a packet */
	uint64_t reserved_49_49               : 1;
	uint64_t gmx_drp                      : 1;  /**< GMX packet drop interrupt
                                                         Set any time corresponding GMX drops a packet */
	uint64_t trace                        : 1;  /**< Trace buffer interrupt
                                                         See TRA_INT_STATUS */
	uint64_t rml                          : 1;  /**< RML Interrupt
                                                         This interrupt will assert if any bit within
                                                         CIU_BLOCK_INT is asserted. */
	uint64_t twsi                         : 1;  /**< TWSI Interrupt
                                                         See MIO_TWS0_INT */
	uint64_t wdog_sum                     : 1;  /**< SUM1&EN1 summary bit
                                                         This read-only bit reads as a one whenever any
                                                         CIU_SUM1_PPX_IPx bit is set and corresponding
                                                         enable bit in CIU_INTx_EN is set, where x
                                                         is the same as x in this CIU_INTx_SUM0.
                                                         PPs use CIU_INTx_SUM0 where x=0-7.
                                                         PCIe uses the CIU_INTx_SUM0 where x=32-33.
                                                         Note that WDOG_SUM only summarizes the SUM1/EN1
                                                         result and does not have a corresponding enable
                                                         bit, so does not directly contribute to
                                                         interrupts. */
	uint64_t pci_msi                      : 4;  /**< PCIe MSI
                                                         See SLI_MSI_RCVn for bit <40+n> */
	uint64_t pci_int                      : 4;  /**< PCIe INTA/B/C/D
                                                         Refer to "Receiving Emulated INTA/INTB/
                                                         INTC/INTD" in the SLI chapter of the spec
                                                         PCI_INT<3> = INTD
                                                         PCI_INT<2> = INTC
                                                         PCI_INT<1> = INTB
                                                         PCI_INT<0> = INTA */
	uint64_t uart                         : 2;  /**< Two UART interrupts
                                                         See MIO_UARTn_IIR[IID] for bit <34+n> */
	uint64_t mbox                         : 2;  /**< A read-only copy of CIU_PCI_INTA[INT] */
	uint64_t gpio                         : 16; /**< 16 GPIO interrupts
                                                         When GPIO_MULTI_CAST[EN] == 1
                                                         Write 1 to clear either the per PP or common GPIO
                                                         edge-triggered interrupts,depending on mode.
                                                         See GPIO_MULTI_CAST for all details.
                                                         When GPIO_MULTI_CAST[EN] == 0
                                                         Read Only, retain the same behavior as o63. */
	uint64_t workq                        : 16; /**< 16 work queue interrupts
                                                         See POW_WQ_INT[WQ_INT]
                                                          1 bit/group. A copy of the R/W1C bit in the POW. */
#else
	uint64_t workq                        : 16;
	uint64_t gpio                         : 16;
	uint64_t mbox                         : 2;
	uint64_t uart                         : 2;
	uint64_t pci_int                      : 4;
	uint64_t pci_msi                      : 4;
	uint64_t wdog_sum                     : 1;
	uint64_t twsi                         : 1;
	uint64_t rml                          : 1;
	uint64_t trace                        : 1;
	uint64_t gmx_drp                      : 1;
	uint64_t reserved_49_49               : 1;
	uint64_t ipd_drp                      : 1;
	uint64_t sum2                         : 1;
	uint64_t timer                        : 4;
	uint64_t usb                          : 1;
	uint64_t pcm                          : 1;
	uint64_t mpi                          : 1;
	uint64_t twsi2                        : 1;
	uint64_t powiq                        : 1;
	uint64_t ipdppthr                     : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t bootdma                      : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_int33_sum0 cvmx_ciu_int33_sum0_t;

/**
 * cvmx_ciu_int_dbg_sel
 */
union cvmx_ciu_int_dbg_sel {
	uint64_t u64;
	struct cvmx_ciu_int_dbg_sel_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t sel                          : 3;  /**< Selects if all or the specific interrupt is
                                                         presented on the debug port.
                                                         0=erst_n
                                                         1=start_bist
                                                         2=toggle at sclk/2 freq
                                                         3=All PP interrupt bits are ORed together
                                                         4=Only the selected virtual  PP/IRQ is selected */
	uint64_t reserved_10_15               : 6;
	uint64_t irq                          : 2;  /**< Which IRQ to select
                                                         0=IRQ2
                                                         1=IRQ3
                                                         2=IRQ4 */
	uint64_t reserved_5_7                 : 3;
	uint64_t pp                           : 5;  /**< Which PP to select */
#else
	uint64_t pp                           : 5;
	uint64_t reserved_5_7                 : 3;
	uint64_t irq                          : 2;
	uint64_t reserved_10_15               : 6;
	uint64_t sel                          : 3;
	uint64_t reserved_19_63               : 45;
#endif
	} s;
	struct cvmx_ciu_int_dbg_sel_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t sel                          : 3;  /**< Selects if all or the specific interrupt is
                                                         presented on the debug port.
                                                         0=erst_n
                                                         1=start_bist
                                                         2=toggle at sclk/2 freq
                                                         3=All PP interrupt bits are ORed together
                                                         4=Only the selected virtual  PP/IRQ is selected */
	uint64_t reserved_10_15               : 6;
	uint64_t irq                          : 2;  /**< Which IRQ to select
                                                         0=IRQ2
                                                         1=IRQ3
                                                         2=IRQ4 */
	uint64_t reserved_4_7                 : 4;
	uint64_t pp                           : 4;  /**< Which PP to select */
#else
	uint64_t pp                           : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t irq                          : 2;
	uint64_t reserved_10_15               : 6;
	uint64_t sel                          : 3;
	uint64_t reserved_19_63               : 45;
#endif
	} cn61xx;
	struct cvmx_ciu_int_dbg_sel_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t sel                          : 3;  /**< Selects if all or the specific interrupt is
                                                         presented on the debug port.
                                                         0=erst_n
                                                         1=start_bist
                                                         2=toggle at sclk/2 freq
                                                         3=All PP interrupt bits are ORed together
                                                         4=Only the selected physical PP/IRQ is selected */
	uint64_t reserved_10_15               : 6;
	uint64_t irq                          : 2;  /**< Which IRQ to select
                                                         0=IRQ2
                                                         1=IRQ3
                                                         2=IRQ4 */
	uint64_t reserved_3_7                 : 5;
	uint64_t pp                           : 3;  /**< Which PP to select */
#else
	uint64_t pp                           : 3;
	uint64_t reserved_3_7                 : 5;
	uint64_t irq                          : 2;
	uint64_t reserved_10_15               : 6;
	uint64_t sel                          : 3;
	uint64_t reserved_19_63               : 45;
#endif
	} cn63xx;
	struct cvmx_ciu_int_dbg_sel_cn61xx    cn66xx;
	struct cvmx_ciu_int_dbg_sel_s         cn68xx;
	struct cvmx_ciu_int_dbg_sel_s         cn68xxp1;
	struct cvmx_ciu_int_dbg_sel_cn61xx    cnf71xx;
};
typedef union cvmx_ciu_int_dbg_sel cvmx_ciu_int_dbg_sel_t;

/**
 * cvmx_ciu_int_sum1
 *
 * CIU_INT_SUM1 is kept to keep backward compatible.
 * Refer to CIU_SUM1_PPX_IPx which is the one should use.
 */
union cvmx_ciu_int_sum1 {
	uint64_t u64;
	struct cvmx_ciu_int_sum1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt
                                                         See SRIO3_INT_REG, SRIO3_INT2_REG */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt
                                                         See SRIO2_INT_REG, SRIO2_INT2_REG */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< DFM Interrupt
                                                         See DFM_FNT_STAT */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t srio1                        : 1;  /**< SRIO1 interrupt
                                                         See SRIO1_INT_REG */
	uint64_t reserved_50_50               : 1;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_38_45               : 8;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt
                                                         See  EMMC interrupt */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t usb1                         : 1;  /**< USBDRD1 Interrupt.  Value equals ((CIU_CIB_USBDRD(1)_RAW & CIU_CIB_USBDRD(1)_EN) != 0).
                                                         See CIU_CIB_USBDRD(1)_RAW and CIU_CIB_USBDRD(1)_EN. */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t wdog                         : 16; /**< Per PP watchdog interrupts */
#else
	uint64_t wdog                         : 16;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_45               : 8;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_50               : 1;
	uint64_t srio1                        : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} s;
	struct cvmx_ciu_int_sum1_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t wdog                         : 1;  /**< 1 watchdog interrupt */
#else
	uint64_t wdog                         : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} cn30xx;
	struct cvmx_ciu_int_sum1_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t wdog                         : 2;  /**< 2 watchdog interrupts */
#else
	uint64_t wdog                         : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} cn31xx;
	struct cvmx_ciu_int_sum1_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t wdog                         : 16; /**< 16 watchdog interrupts */
#else
	uint64_t wdog                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn38xx;
	struct cvmx_ciu_int_sum1_cn38xx       cn38xxp2;
	struct cvmx_ciu_int_sum1_cn31xx       cn50xx;
	struct cvmx_ciu_int_sum1_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t nand                         : 1;  /**< NAND Flash Controller */
	uint64_t mii1                         : 1;  /**< Second MII Interrupt */
	uint64_t usb1                         : 1;  /**< Second USB Interrupt */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t reserved_4_15                : 12;
	uint64_t wdog                         : 4;  /**< 4 watchdog interrupts */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_15                : 12;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn52xx;
	struct cvmx_ciu_int_sum1_cn52xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t mii1                         : 1;  /**< Second MII Interrupt */
	uint64_t usb1                         : 1;  /**< Second USB Interrupt */
	uint64_t uart2                        : 1;  /**< Third UART interrupt */
	uint64_t reserved_4_15                : 12;
	uint64_t wdog                         : 4;  /**< 4 watchdog interrupts */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_15                : 12;
	uint64_t uart2                        : 1;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t reserved_19_63               : 45;
#endif
	} cn52xxp1;
	struct cvmx_ciu_int_sum1_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t wdog                         : 12; /**< 12 watchdog interrupts */
#else
	uint64_t wdog                         : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} cn56xx;
	struct cvmx_ciu_int_sum1_cn56xx       cn56xxp1;
	struct cvmx_ciu_int_sum1_cn38xx       cn58xx;
	struct cvmx_ciu_int_sum1_cn38xx       cn58xxp1;
	struct cvmx_ciu_int_sum1_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_38_45               : 8;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt
                                                         See  EMMC interrupt */
	uint64_t mii1                         : 1;  /**< RGMII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t reserved_4_17                : 14;
	uint64_t wdog                         : 4;  /**< Per PP watchdog interrupts */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_17                : 14;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_45               : 8;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_int_sum1_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_57_62               : 6;
	uint64_t dfm                          : 1;  /**< DFM Interrupt
                                                         See DFM_FNT_STAT */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t srio1                        : 1;  /**< SRIO1 interrupt
                                                         See SRIO1_INT_REG, SRIO1_INT2_REG */
	uint64_t srio0                        : 1;  /**< SRIO0 interrupt
                                                         See SRIO0_INT_REG, SRIO0_INT2_REG */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_37_45               : 9;
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< NAND Flash Controller interrupt
                                                         See NDF_INT */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t reserved_6_17                : 12;
	uint64_t wdog                         : 6;  /**< 6 watchdog interrupts */
#else
	uint64_t wdog                         : 6;
	uint64_t reserved_6_17                : 12;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_45               : 9;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t srio1                        : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_62               : 6;
	uint64_t rst                          : 1;
#endif
	} cn63xx;
	struct cvmx_ciu_int_sum1_cn63xx       cn63xxp1;
	struct cvmx_ciu_int_sum1_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt
                                                         See SRIO3_INT_REG, SRIO3_INT2_REG */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt
                                                         See SRIO2_INT_REG, SRIO2_INT2_REG */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< DFM Interrupt
                                                         See DFM_FNT_STAT */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_51_51               : 1;
	uint64_t srio0                        : 1;  /**< SRIO0 interrupt
                                                         See SRIO0_INT_REG, SRIO0_INT2_REG */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_38_45               : 8;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< NAND Flash Controller interrupt
                                                         See NDF_INT */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t reserved_10_17               : 8;
	uint64_t wdog                         : 10; /**< 10 watchdog interrupts */
#else
	uint64_t wdog                         : 10;
	uint64_t reserved_10_17               : 8;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_45               : 8;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_int_sum1_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< RST interrupt. Value equals ((CIU_CIB_RST_RAW & CIU_CIB_RST_EN) != 0).
                                                         See CIU_CIB_RST_RAW and CIU_CIB_RST_EN. */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt. Value equals ((CIU_CIB_LMC(0)_RAW & CIU_CIB_LMC(0)_EN) != 0).
                                                         See CIU_CIB_LMC(0)_RAW and CIU_CIB_LMC(0)_EN. */
	uint64_t reserved_51_51               : 1;
	uint64_t pem2                         : 1;  /**< PEM2 interrupt
                                                         See PEM2_INT_SUM (enabled by PEM2_INT_ENB) */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero
                                                         See MIO_PTP_EVT_CNT for details. */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_38_45               : 8;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USBDRD0 Interrupt.  Value equals ((CIU_CIB_USBDRD(0)_RAW & CIU_CIB_USBDRD(0)_EN) != 0).
                                                         See CIU_CIB_USBDRD(0)_RAW and CIU_CIB_USBDRD(0)_EN. */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt. Value equals ((CIU_CIB_L2C_RAW & CIU_CIB_L2C_EN) != 0).
                                                         See CIU_CIB_L2C_RAW and CIU_CIB_L2C_EN. */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< NAND / EMMC Controller interrupt
                                                         See  NAND / EMMC interrupt */
	uint64_t reserved_18_18               : 1;
	uint64_t usb1                         : 1;  /**< USBDRD1 Interrupt.  Value equals ((CIU_CIB_USBDRD(1)_RAW & CIU_CIB_USBDRD(1)_EN) != 0).
                                                         See CIU_CIB_USBDRD(1)_RAW and CIU_CIB_USBDRD(1)_EN. */
	uint64_t reserved_4_16                : 13;
	uint64_t wdog                         : 4;  /**< Watchdog interrupts. Bit 0 for PP0 watchdog, and Bit n for PPn. */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_16                : 13;
	uint64_t usb1                         : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_45               : 8;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t pem2                         : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_int_sum1_cn70xx       cn70xxp1;
	struct cvmx_ciu_int_sum1_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t reserved_37_46               : 10;
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t reserved_32_32               : 1;
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt
                                                         See  EMMC interrupt */
	uint64_t reserved_4_18                : 15;
	uint64_t wdog                         : 4;  /**< Per PP watchdog interrupts */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_18                : 15;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t reserved_32_32               : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_46               : 10;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_int_sum1 cvmx_ciu_int_sum1_t;

/**
 * cvmx_ciu_intr_slowdown
 */
union cvmx_ciu_intr_slowdown {
	uint64_t u64;
	struct cvmx_ciu_intr_slowdown_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t ctl                          : 3;  /**< Slow down CIU interrupt walker processing time. IRQ2/3/4 for all 48 cores are sent to the
                                                         core (MRC) in a serial bus to reduce global routing. There is no backpressure mechanism
                                                         designed for this scheme. It will only be a problem when SCLK is faster; this Control will
                                                         slow down the data send out rate in INTR interface to PP. With different a setting, clock
                                                         rate ratio can handle:
                                                         SLOWDOWN sclk_freq/aclk_freq ratio
                                                         0 3
                                                         1 6
                                                         n 3*2n */
#else
	uint64_t ctl                          : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_ciu_intr_slowdown_s       cn70xx;
	struct cvmx_ciu_intr_slowdown_s       cn70xxp1;
};
typedef union cvmx_ciu_intr_slowdown cvmx_ciu_intr_slowdown_t;

/**
 * cvmx_ciu_mbox_clr#
 */
union cvmx_ciu_mbox_clrx {
	uint64_t u64;
	struct cvmx_ciu_mbox_clrx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t bits                         : 32; /**< On writes, clr corresponding bit in MBOX register
                                                         on reads, return the MBOX register */
#else
	uint64_t bits                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_ciu_mbox_clrx_s           cn30xx;
	struct cvmx_ciu_mbox_clrx_s           cn31xx;
	struct cvmx_ciu_mbox_clrx_s           cn38xx;
	struct cvmx_ciu_mbox_clrx_s           cn38xxp2;
	struct cvmx_ciu_mbox_clrx_s           cn50xx;
	struct cvmx_ciu_mbox_clrx_s           cn52xx;
	struct cvmx_ciu_mbox_clrx_s           cn52xxp1;
	struct cvmx_ciu_mbox_clrx_s           cn56xx;
	struct cvmx_ciu_mbox_clrx_s           cn56xxp1;
	struct cvmx_ciu_mbox_clrx_s           cn58xx;
	struct cvmx_ciu_mbox_clrx_s           cn58xxp1;
	struct cvmx_ciu_mbox_clrx_s           cn61xx;
	struct cvmx_ciu_mbox_clrx_s           cn63xx;
	struct cvmx_ciu_mbox_clrx_s           cn63xxp1;
	struct cvmx_ciu_mbox_clrx_s           cn66xx;
	struct cvmx_ciu_mbox_clrx_s           cn68xx;
	struct cvmx_ciu_mbox_clrx_s           cn68xxp1;
	struct cvmx_ciu_mbox_clrx_s           cn70xx;
	struct cvmx_ciu_mbox_clrx_s           cn70xxp1;
	struct cvmx_ciu_mbox_clrx_s           cnf71xx;
};
typedef union cvmx_ciu_mbox_clrx cvmx_ciu_mbox_clrx_t;

/**
 * cvmx_ciu_mbox_set#
 */
union cvmx_ciu_mbox_setx {
	uint64_t u64;
	struct cvmx_ciu_mbox_setx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t bits                         : 32; /**< On writes, set corresponding bit in MBOX register
                                                         on reads, return the MBOX register */
#else
	uint64_t bits                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_ciu_mbox_setx_s           cn30xx;
	struct cvmx_ciu_mbox_setx_s           cn31xx;
	struct cvmx_ciu_mbox_setx_s           cn38xx;
	struct cvmx_ciu_mbox_setx_s           cn38xxp2;
	struct cvmx_ciu_mbox_setx_s           cn50xx;
	struct cvmx_ciu_mbox_setx_s           cn52xx;
	struct cvmx_ciu_mbox_setx_s           cn52xxp1;
	struct cvmx_ciu_mbox_setx_s           cn56xx;
	struct cvmx_ciu_mbox_setx_s           cn56xxp1;
	struct cvmx_ciu_mbox_setx_s           cn58xx;
	struct cvmx_ciu_mbox_setx_s           cn58xxp1;
	struct cvmx_ciu_mbox_setx_s           cn61xx;
	struct cvmx_ciu_mbox_setx_s           cn63xx;
	struct cvmx_ciu_mbox_setx_s           cn63xxp1;
	struct cvmx_ciu_mbox_setx_s           cn66xx;
	struct cvmx_ciu_mbox_setx_s           cn68xx;
	struct cvmx_ciu_mbox_setx_s           cn68xxp1;
	struct cvmx_ciu_mbox_setx_s           cn70xx;
	struct cvmx_ciu_mbox_setx_s           cn70xxp1;
	struct cvmx_ciu_mbox_setx_s           cnf71xx;
};
typedef union cvmx_ciu_mbox_setx cvmx_ciu_mbox_setx_t;

/**
 * cvmx_ciu_nmi
 */
union cvmx_ciu_nmi {
	uint64_t u64;
	struct cvmx_ciu_nmi_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t nmi                          : 32; /**< Send NMI pulse to PP vector */
#else
	uint64_t nmi                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_ciu_nmi_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t nmi                          : 1;  /**< Send NMI pulse to PP vector */
#else
	uint64_t nmi                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} cn30xx;
	struct cvmx_ciu_nmi_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t nmi                          : 2;  /**< Send NMI pulse to PP vector */
#else
	uint64_t nmi                          : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} cn31xx;
	struct cvmx_ciu_nmi_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t nmi                          : 16; /**< Send NMI pulse to PP vector */
#else
	uint64_t nmi                          : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn38xx;
	struct cvmx_ciu_nmi_cn38xx            cn38xxp2;
	struct cvmx_ciu_nmi_cn31xx            cn50xx;
	struct cvmx_ciu_nmi_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t nmi                          : 4;  /**< Send NMI pulse to PP vector */
#else
	uint64_t nmi                          : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} cn52xx;
	struct cvmx_ciu_nmi_cn52xx            cn52xxp1;
	struct cvmx_ciu_nmi_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t nmi                          : 12; /**< Send NMI pulse to PP vector */
#else
	uint64_t nmi                          : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} cn56xx;
	struct cvmx_ciu_nmi_cn56xx            cn56xxp1;
	struct cvmx_ciu_nmi_cn38xx            cn58xx;
	struct cvmx_ciu_nmi_cn38xx            cn58xxp1;
	struct cvmx_ciu_nmi_cn52xx            cn61xx;
	struct cvmx_ciu_nmi_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t nmi                          : 6;  /**< Send NMI pulse to PP vector */
#else
	uint64_t nmi                          : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} cn63xx;
	struct cvmx_ciu_nmi_cn63xx            cn63xxp1;
	struct cvmx_ciu_nmi_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t nmi                          : 10; /**< Send NMI pulse to PP vector */
#else
	uint64_t nmi                          : 10;
	uint64_t reserved_10_63               : 54;
#endif
	} cn66xx;
	struct cvmx_ciu_nmi_s                 cn68xx;
	struct cvmx_ciu_nmi_s                 cn68xxp1;
	struct cvmx_ciu_nmi_cn52xx            cn70xx;
	struct cvmx_ciu_nmi_cn52xx            cn70xxp1;
	struct cvmx_ciu_nmi_cn52xx            cnf71xx;
};
typedef union cvmx_ciu_nmi cvmx_ciu_nmi_t;

/**
 * cvmx_ciu_pci_inta
 */
union cvmx_ciu_pci_inta {
	uint64_t u64;
	struct cvmx_ciu_pci_inta_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t intr                         : 2;  /**< PCIe interrupt
                                                         These bits are observed in CIU_INTX_SUM0<33:32>
                                                         where X=32-33 */
#else
	uint64_t intr                         : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_ciu_pci_inta_s            cn30xx;
	struct cvmx_ciu_pci_inta_s            cn31xx;
	struct cvmx_ciu_pci_inta_s            cn38xx;
	struct cvmx_ciu_pci_inta_s            cn38xxp2;
	struct cvmx_ciu_pci_inta_s            cn50xx;
	struct cvmx_ciu_pci_inta_s            cn52xx;
	struct cvmx_ciu_pci_inta_s            cn52xxp1;
	struct cvmx_ciu_pci_inta_s            cn56xx;
	struct cvmx_ciu_pci_inta_s            cn56xxp1;
	struct cvmx_ciu_pci_inta_s            cn58xx;
	struct cvmx_ciu_pci_inta_s            cn58xxp1;
	struct cvmx_ciu_pci_inta_s            cn61xx;
	struct cvmx_ciu_pci_inta_s            cn63xx;
	struct cvmx_ciu_pci_inta_s            cn63xxp1;
	struct cvmx_ciu_pci_inta_s            cn66xx;
	struct cvmx_ciu_pci_inta_s            cn68xx;
	struct cvmx_ciu_pci_inta_s            cn68xxp1;
	struct cvmx_ciu_pci_inta_s            cn70xx;
	struct cvmx_ciu_pci_inta_s            cn70xxp1;
	struct cvmx_ciu_pci_inta_s            cnf71xx;
};
typedef union cvmx_ciu_pci_inta cvmx_ciu_pci_inta_t;

/**
 * cvmx_ciu_pp_bist_stat
 */
union cvmx_ciu_pp_bist_stat {
	uint64_t u64;
	struct cvmx_ciu_pp_bist_stat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t pp_bist                      : 32; /**< Physical PP BIST status */
#else
	uint64_t pp_bist                      : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_ciu_pp_bist_stat_s        cn68xx;
	struct cvmx_ciu_pp_bist_stat_s        cn68xxp1;
};
typedef union cvmx_ciu_pp_bist_stat cvmx_ciu_pp_bist_stat_t;

/**
 * cvmx_ciu_pp_dbg
 */
union cvmx_ciu_pp_dbg {
	uint64_t u64;
	struct cvmx_ciu_pp_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t ppdbg                        : 48; /**< Debug[DM] value for each core, whether the cores are in debug mode or not. */
#else
	uint64_t ppdbg                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_ciu_pp_dbg_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ppdbg                        : 1;  /**< Debug[DM] value for each PP
                                                         whether the PP's are in debug mode or not */
#else
	uint64_t ppdbg                        : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} cn30xx;
	struct cvmx_ciu_pp_dbg_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t ppdbg                        : 2;  /**< Debug[DM] value for each PP
                                                         whether the PP's are in debug mode or not */
#else
	uint64_t ppdbg                        : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} cn31xx;
	struct cvmx_ciu_pp_dbg_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t ppdbg                        : 16; /**< Debug[DM] value for each PP
                                                         whether the PP's are in debug mode or not */
#else
	uint64_t ppdbg                        : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn38xx;
	struct cvmx_ciu_pp_dbg_cn38xx         cn38xxp2;
	struct cvmx_ciu_pp_dbg_cn31xx         cn50xx;
	struct cvmx_ciu_pp_dbg_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t ppdbg                        : 4;  /**< Debug[DM] value for each PP
                                                         whether the PP's are in debug mode or not */
#else
	uint64_t ppdbg                        : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} cn52xx;
	struct cvmx_ciu_pp_dbg_cn52xx         cn52xxp1;
	struct cvmx_ciu_pp_dbg_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t ppdbg                        : 12; /**< Debug[DM] value for each PP
                                                         whether the PP's are in debug mode or not */
#else
	uint64_t ppdbg                        : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} cn56xx;
	struct cvmx_ciu_pp_dbg_cn56xx         cn56xxp1;
	struct cvmx_ciu_pp_dbg_cn38xx         cn58xx;
	struct cvmx_ciu_pp_dbg_cn38xx         cn58xxp1;
	struct cvmx_ciu_pp_dbg_cn52xx         cn61xx;
	struct cvmx_ciu_pp_dbg_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t ppdbg                        : 6;  /**< Debug[DM] value for each PP
                                                         whether the PP's are in debug mode or not */
#else
	uint64_t ppdbg                        : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} cn63xx;
	struct cvmx_ciu_pp_dbg_cn63xx         cn63xxp1;
	struct cvmx_ciu_pp_dbg_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t ppdbg                        : 10; /**< Debug[DM] value for each PP
                                                         whether the PP's are in debug mode or not */
#else
	uint64_t ppdbg                        : 10;
	uint64_t reserved_10_63               : 54;
#endif
	} cn66xx;
	struct cvmx_ciu_pp_dbg_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t ppdbg                        : 32; /**< Debug[DM] value for each PP
                                                         whether the PP's are in debug mode or not */
#else
	uint64_t ppdbg                        : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn68xx;
	struct cvmx_ciu_pp_dbg_cn68xx         cn68xxp1;
	struct cvmx_ciu_pp_dbg_cn52xx         cn70xx;
	struct cvmx_ciu_pp_dbg_cn52xx         cn70xxp1;
	struct cvmx_ciu_pp_dbg_cn38xx         cn73xx;
	struct cvmx_ciu_pp_dbg_s              cn78xx;
	struct cvmx_ciu_pp_dbg_s              cn78xxp1;
	struct cvmx_ciu_pp_dbg_cn52xx         cnf71xx;
	struct cvmx_ciu_pp_dbg_cn38xx         cnf75xx;
};
typedef union cvmx_ciu_pp_dbg cvmx_ciu_pp_dbg_t;

/**
 * cvmx_ciu_pp_poke#
 *
 * CIU_PP_POKE for CIU_WDOG
 *
 */
union cvmx_ciu_pp_pokex {
	uint64_t u64;
	struct cvmx_ciu_pp_pokex_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t poke                         : 64; /**< Core poke. Writing any value to this register does the following:
                                                         * clears any pending interrupt generated by the associated watchdog.
                                                         * resets CIU_WDOG()[STATE] to 0x0.
                                                         * sets CIU_WDOG()[CNT] to ( CIU_WDOG()[LEN] << 8).
                                                         Reading this register returns the associated CIU_WDOG() register. */
#else
	uint64_t poke                         : 64;
#endif
	} s;
	struct cvmx_ciu_pp_pokex_s            cn30xx;
	struct cvmx_ciu_pp_pokex_s            cn31xx;
	struct cvmx_ciu_pp_pokex_s            cn38xx;
	struct cvmx_ciu_pp_pokex_s            cn38xxp2;
	struct cvmx_ciu_pp_pokex_s            cn50xx;
	struct cvmx_ciu_pp_pokex_s            cn52xx;
	struct cvmx_ciu_pp_pokex_s            cn52xxp1;
	struct cvmx_ciu_pp_pokex_s            cn56xx;
	struct cvmx_ciu_pp_pokex_s            cn56xxp1;
	struct cvmx_ciu_pp_pokex_s            cn58xx;
	struct cvmx_ciu_pp_pokex_s            cn58xxp1;
	struct cvmx_ciu_pp_pokex_s            cn61xx;
	struct cvmx_ciu_pp_pokex_s            cn63xx;
	struct cvmx_ciu_pp_pokex_s            cn63xxp1;
	struct cvmx_ciu_pp_pokex_s            cn66xx;
	struct cvmx_ciu_pp_pokex_s            cn68xx;
	struct cvmx_ciu_pp_pokex_s            cn68xxp1;
	struct cvmx_ciu_pp_pokex_s            cn70xx;
	struct cvmx_ciu_pp_pokex_s            cn70xxp1;
	struct cvmx_ciu_pp_pokex_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t poke                         : 1;  /**< Core poke. Writing any value to this register does the following:
                                                         * clears any pending interrupt generated by the associated watchdog.
                                                         * resets CIU_WDOG()[STATE] to 0x0.
                                                         * sets CIU_WDOG()[CNT] to ( CIU_WDOG()[LEN] << 8).
                                                         Reading this register returns the associated CIU_WDOG() register. */
#else
	uint64_t poke                         : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} cn73xx;
	struct cvmx_ciu_pp_pokex_cn73xx       cn78xx;
	struct cvmx_ciu_pp_pokex_cn73xx       cn78xxp1;
	struct cvmx_ciu_pp_pokex_s            cnf71xx;
	struct cvmx_ciu_pp_pokex_cn73xx       cnf75xx;
};
typedef union cvmx_ciu_pp_pokex cvmx_ciu_pp_pokex_t;

/**
 * cvmx_ciu_pp_rst
 *
 * This register contains the reset control for each core. A 1 holds a core in reset, 0 release
 * from reset. It resets to all ones when REMOTE_BOOT is enabled or all ones excluding bit 0 when
 * REMOTE_BOOT is disabled. Writes to this register should occur only if the CIU_PP_RST_PENDING
 * register is cleared.
 */
union cvmx_ciu_pp_rst {
	uint64_t u64;
	struct cvmx_ciu_pp_rst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t rst                          : 47; /**< Core reset for cores 1 and above. Writing a 1 holds the corresponding core in reset,
                                                         writing a 0 releases from reset.
                                                         The upper bits of this field remain accessible but will have no effect if the cores
                                                         are disabled. The number of bits cleared in CIU_FUSE[FUSE] indicate the number of cores. */
	uint64_t rst0                         : 1;  /**< Core reset for core 0, depends on standalone mode. */
#else
	uint64_t rst0                         : 1;
	uint64_t rst                          : 47;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_ciu_pp_rst_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t rst0                         : 1;  /**< PP Rst for PP0
                                                         depends on standalone mode */
#else
	uint64_t rst0                         : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} cn30xx;
	struct cvmx_ciu_pp_rst_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t rst                          : 1;  /**< PP Rst for PP1 */
	uint64_t rst0                         : 1;  /**< PP Rst for PP0
                                                         depends on standalone mode */
#else
	uint64_t rst0                         : 1;
	uint64_t rst                          : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} cn31xx;
	struct cvmx_ciu_pp_rst_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t rst                          : 15; /**< PP Rst for PP's 15-1 */
	uint64_t rst0                         : 1;  /**< PP Rst for PP0
                                                         depends on standalone mode */
#else
	uint64_t rst0                         : 1;
	uint64_t rst                          : 15;
	uint64_t reserved_16_63               : 48;
#endif
	} cn38xx;
	struct cvmx_ciu_pp_rst_cn38xx         cn38xxp2;
	struct cvmx_ciu_pp_rst_cn31xx         cn50xx;
	struct cvmx_ciu_pp_rst_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t rst                          : 3;  /**< PP Rst for PP's 11-1 */
	uint64_t rst0                         : 1;  /**< PP Rst for PP0
                                                         depends on standalone mode */
#else
	uint64_t rst0                         : 1;
	uint64_t rst                          : 3;
	uint64_t reserved_4_63                : 60;
#endif
	} cn52xx;
	struct cvmx_ciu_pp_rst_cn52xx         cn52xxp1;
	struct cvmx_ciu_pp_rst_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t rst                          : 11; /**< PP Rst for PP's 11-1 */
	uint64_t rst0                         : 1;  /**< PP Rst for PP0
                                                         depends on standalone mode */
#else
	uint64_t rst0                         : 1;
	uint64_t rst                          : 11;
	uint64_t reserved_12_63               : 52;
#endif
	} cn56xx;
	struct cvmx_ciu_pp_rst_cn56xx         cn56xxp1;
	struct cvmx_ciu_pp_rst_cn38xx         cn58xx;
	struct cvmx_ciu_pp_rst_cn38xx         cn58xxp1;
	struct cvmx_ciu_pp_rst_cn52xx         cn61xx;
	struct cvmx_ciu_pp_rst_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t rst                          : 5;  /**< PP Rst for PP's 5-1 */
	uint64_t rst0                         : 1;  /**< PP Rst for PP0
                                                         depends on standalone mode */
#else
	uint64_t rst0                         : 1;
	uint64_t rst                          : 5;
	uint64_t reserved_6_63                : 58;
#endif
	} cn63xx;
	struct cvmx_ciu_pp_rst_cn63xx         cn63xxp1;
	struct cvmx_ciu_pp_rst_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t rst                          : 9;  /**< PP Rst for PP's 9-1 */
	uint64_t rst0                         : 1;  /**< PP Rst for PP0
                                                         depends on standalone mode */
#else
	uint64_t rst0                         : 1;
	uint64_t rst                          : 9;
	uint64_t reserved_10_63               : 54;
#endif
	} cn66xx;
	struct cvmx_ciu_pp_rst_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t rst                          : 31; /**< PP Rst for PP's 31-1 */
	uint64_t rst0                         : 1;  /**< PP Rst for PP0
                                                         depends on standalone mode */
#else
	uint64_t rst0                         : 1;
	uint64_t rst                          : 31;
	uint64_t reserved_32_63               : 32;
#endif
	} cn68xx;
	struct cvmx_ciu_pp_rst_cn68xx         cn68xxp1;
	struct cvmx_ciu_pp_rst_cn52xx         cn70xx;
	struct cvmx_ciu_pp_rst_cn52xx         cn70xxp1;
	struct cvmx_ciu_pp_rst_cn38xx         cn73xx;
	struct cvmx_ciu_pp_rst_s              cn78xx;
	struct cvmx_ciu_pp_rst_s              cn78xxp1;
	struct cvmx_ciu_pp_rst_cn52xx         cnf71xx;
	struct cvmx_ciu_pp_rst_cn38xx         cnf75xx;
};
typedef union cvmx_ciu_pp_rst cvmx_ciu_pp_rst_t;

/**
 * cvmx_ciu_pp_rst_pending
 *
 * This register contains the reset status for each core.
 *
 */
union cvmx_ciu_pp_rst_pending {
	uint64_t u64;
	struct cvmx_ciu_pp_rst_pending_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t pend                         : 48; /**< Set if corresponding core is waiting to change its reset state. Normally a reset change
                                                         occurs immediately but if RST_PP_POWER[GATE] bit is set and the core is released from
                                                         reset a delay of 64K core clocks between each core reset will apply to satisfy power
                                                         management.
                                                         The upper bits of this field remain accessible but will have no effect if the cores
                                                         are disabled. The number of bits cleared in CIU_FUSE[FUSE] indicate the number of cores. */
#else
	uint64_t pend                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_ciu_pp_rst_pending_s      cn70xx;
	struct cvmx_ciu_pp_rst_pending_s      cn70xxp1;
	struct cvmx_ciu_pp_rst_pending_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t pend                         : 16; /**< Set if corresponding core is waiting to change its reset state. Normally a reset change
                                                         occurs immediately but if RST_PP_POWER[GATE] bit is set and the core is released from
                                                         reset a delay of 64K core clocks between each core reset will apply to satisfy power
                                                         management.
                                                         The upper bits of this field remain accessible but will have no effect if the cores
                                                         are disabled. The number of bits cleared in CIU_FUSE[FUSE] indicate the number of cores. */
#else
	uint64_t pend                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn73xx;
	struct cvmx_ciu_pp_rst_pending_s      cn78xx;
	struct cvmx_ciu_pp_rst_pending_s      cn78xxp1;
	struct cvmx_ciu_pp_rst_pending_cn73xx cnf75xx;
};
typedef union cvmx_ciu_pp_rst_pending cvmx_ciu_pp_rst_pending_t;

/**
 * cvmx_ciu_qlm0
 *
 * Notes:
 * This register is only reset by cold reset.
 *
 */
union cvmx_ciu_qlm0 {
	uint64_t u64;
	struct cvmx_ciu_qlm0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t g2bypass                     : 1;  /**< QLM0 PCIE Gen2 tx bypass enable */
	uint64_t reserved_53_62               : 10;
	uint64_t g2deemph                     : 5;  /**< QLM0 PCIE Gen2 tx bypass de-emphasis value */
	uint64_t reserved_45_47               : 3;
	uint64_t g2margin                     : 5;  /**< QLM0 PCIE Gen2 tx bypass margin (amplitude) value */
	uint64_t reserved_32_39               : 8;
	uint64_t txbypass                     : 1;  /**< QLM0 transmitter bypass enable */
	uint64_t reserved_21_30               : 10;
	uint64_t txdeemph                     : 5;  /**< QLM0 transmitter bypass de-emphasis value */
	uint64_t reserved_13_15               : 3;
	uint64_t txmargin                     : 5;  /**< QLM0 transmitter bypass margin (amplitude) value */
	uint64_t reserved_4_7                 : 4;
	uint64_t lane_en                      : 4;  /**< QLM0 lane enable mask */
#else
	uint64_t lane_en                      : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t txmargin                     : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t txdeemph                     : 5;
	uint64_t reserved_21_30               : 10;
	uint64_t txbypass                     : 1;
	uint64_t reserved_32_39               : 8;
	uint64_t g2margin                     : 5;
	uint64_t reserved_45_47               : 3;
	uint64_t g2deemph                     : 5;
	uint64_t reserved_53_62               : 10;
	uint64_t g2bypass                     : 1;
#endif
	} s;
	struct cvmx_ciu_qlm0_s                cn61xx;
	struct cvmx_ciu_qlm0_s                cn63xx;
	struct cvmx_ciu_qlm0_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t txbypass                     : 1;  /**< QLM0 transmitter bypass enable */
	uint64_t reserved_20_30               : 11;
	uint64_t txdeemph                     : 4;  /**< QLM0 transmitter bypass de-emphasis value */
	uint64_t reserved_13_15               : 3;
	uint64_t txmargin                     : 5;  /**< QLM0 transmitter bypass margin (amplitude) value */
	uint64_t reserved_4_7                 : 4;
	uint64_t lane_en                      : 4;  /**< QLM0 lane enable mask */
#else
	uint64_t lane_en                      : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t txmargin                     : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t txdeemph                     : 4;
	uint64_t reserved_20_30               : 11;
	uint64_t txbypass                     : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} cn63xxp1;
	struct cvmx_ciu_qlm0_s                cn66xx;
	struct cvmx_ciu_qlm0_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t txbypass                     : 1;  /**< QLMx transmitter bypass enable */
	uint64_t reserved_21_30               : 10;
	uint64_t txdeemph                     : 5;  /**< QLMx transmitter bypass de-emphasis value */
	uint64_t reserved_13_15               : 3;
	uint64_t txmargin                     : 5;  /**< QLMx transmitter bypass margin (amplitude) value */
	uint64_t reserved_4_7                 : 4;
	uint64_t lane_en                      : 4;  /**< QLMx lane enable mask */
#else
	uint64_t lane_en                      : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t txmargin                     : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t txdeemph                     : 5;
	uint64_t reserved_21_30               : 10;
	uint64_t txbypass                     : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} cn68xx;
	struct cvmx_ciu_qlm0_cn68xx           cn68xxp1;
	struct cvmx_ciu_qlm0_s                cnf71xx;
};
typedef union cvmx_ciu_qlm0 cvmx_ciu_qlm0_t;

/**
 * cvmx_ciu_qlm1
 *
 * Notes:
 * This register is only reset by cold reset.
 *
 */
union cvmx_ciu_qlm1 {
	uint64_t u64;
	struct cvmx_ciu_qlm1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t g2bypass                     : 1;  /**< QLM1 PCIE Gen2 tx bypass enable */
	uint64_t reserved_53_62               : 10;
	uint64_t g2deemph                     : 5;  /**< QLM1 PCIE Gen2 tx bypass de-emphasis value */
	uint64_t reserved_45_47               : 3;
	uint64_t g2margin                     : 5;  /**< QLM1 PCIE Gen2 tx bypass margin (amplitude) value */
	uint64_t reserved_32_39               : 8;
	uint64_t txbypass                     : 1;  /**< QLM1 transmitter bypass enable */
	uint64_t reserved_21_30               : 10;
	uint64_t txdeemph                     : 5;  /**< QLM1 transmitter bypass de-emphasis value */
	uint64_t reserved_13_15               : 3;
	uint64_t txmargin                     : 5;  /**< QLM1 transmitter bypass margin (amplitude) value */
	uint64_t reserved_4_7                 : 4;
	uint64_t lane_en                      : 4;  /**< QLM1 lane enable mask */
#else
	uint64_t lane_en                      : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t txmargin                     : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t txdeemph                     : 5;
	uint64_t reserved_21_30               : 10;
	uint64_t txbypass                     : 1;
	uint64_t reserved_32_39               : 8;
	uint64_t g2margin                     : 5;
	uint64_t reserved_45_47               : 3;
	uint64_t g2deemph                     : 5;
	uint64_t reserved_53_62               : 10;
	uint64_t g2bypass                     : 1;
#endif
	} s;
	struct cvmx_ciu_qlm1_s                cn61xx;
	struct cvmx_ciu_qlm1_s                cn63xx;
	struct cvmx_ciu_qlm1_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t txbypass                     : 1;  /**< QLM1 transmitter bypass enable */
	uint64_t reserved_20_30               : 11;
	uint64_t txdeemph                     : 4;  /**< QLM1 transmitter bypass de-emphasis value */
	uint64_t reserved_13_15               : 3;
	uint64_t txmargin                     : 5;  /**< QLM1 transmitter bypass margin (amplitude) value */
	uint64_t reserved_4_7                 : 4;
	uint64_t lane_en                      : 4;  /**< QLM1 lane enable mask */
#else
	uint64_t lane_en                      : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t txmargin                     : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t txdeemph                     : 4;
	uint64_t reserved_20_30               : 11;
	uint64_t txbypass                     : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} cn63xxp1;
	struct cvmx_ciu_qlm1_s                cn66xx;
	struct cvmx_ciu_qlm1_s                cn68xx;
	struct cvmx_ciu_qlm1_s                cn68xxp1;
	struct cvmx_ciu_qlm1_s                cnf71xx;
};
typedef union cvmx_ciu_qlm1 cvmx_ciu_qlm1_t;

/**
 * cvmx_ciu_qlm2
 *
 * Notes:
 * This register is only reset by cold reset.
 *
 */
union cvmx_ciu_qlm2 {
	uint64_t u64;
	struct cvmx_ciu_qlm2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t g2bypass                     : 1;  /**< QLMx PCIE Gen2 tx bypass enable */
	uint64_t reserved_53_62               : 10;
	uint64_t g2deemph                     : 5;  /**< QLMx PCIE Gen2 tx bypass de-emphasis value */
	uint64_t reserved_45_47               : 3;
	uint64_t g2margin                     : 5;  /**< QLMx PCIE Gen2 tx bypass margin (amplitude) value */
	uint64_t reserved_32_39               : 8;
	uint64_t txbypass                     : 1;  /**< QLM2 transmitter bypass enable */
	uint64_t reserved_21_30               : 10;
	uint64_t txdeemph                     : 5;  /**< QLM2 transmitter bypass de-emphasis value */
	uint64_t reserved_13_15               : 3;
	uint64_t txmargin                     : 5;  /**< QLM2 transmitter bypass margin (amplitude) value */
	uint64_t reserved_4_7                 : 4;
	uint64_t lane_en                      : 4;  /**< QLM2 lane enable mask */
#else
	uint64_t lane_en                      : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t txmargin                     : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t txdeemph                     : 5;
	uint64_t reserved_21_30               : 10;
	uint64_t txbypass                     : 1;
	uint64_t reserved_32_39               : 8;
	uint64_t g2margin                     : 5;
	uint64_t reserved_45_47               : 3;
	uint64_t g2deemph                     : 5;
	uint64_t reserved_53_62               : 10;
	uint64_t g2bypass                     : 1;
#endif
	} s;
	struct cvmx_ciu_qlm2_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t txbypass                     : 1;  /**< QLM2 transmitter bypass enable */
	uint64_t reserved_21_30               : 10;
	uint64_t txdeemph                     : 5;  /**< QLM2 transmitter bypass de-emphasis value */
	uint64_t reserved_13_15               : 3;
	uint64_t txmargin                     : 5;  /**< QLM2 transmitter bypass margin (amplitude) value */
	uint64_t reserved_4_7                 : 4;
	uint64_t lane_en                      : 4;  /**< QLM2 lane enable mask */
#else
	uint64_t lane_en                      : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t txmargin                     : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t txdeemph                     : 5;
	uint64_t reserved_21_30               : 10;
	uint64_t txbypass                     : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} cn61xx;
	struct cvmx_ciu_qlm2_cn61xx           cn63xx;
	struct cvmx_ciu_qlm2_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t txbypass                     : 1;  /**< QLM2 transmitter bypass enable */
	uint64_t reserved_20_30               : 11;
	uint64_t txdeemph                     : 4;  /**< QLM2 transmitter bypass de-emphasis value */
	uint64_t reserved_13_15               : 3;
	uint64_t txmargin                     : 5;  /**< QLM2 transmitter bypass margin (amplitude) value */
	uint64_t reserved_4_7                 : 4;
	uint64_t lane_en                      : 4;  /**< QLM2 lane enable mask */
#else
	uint64_t lane_en                      : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t txmargin                     : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t txdeemph                     : 4;
	uint64_t reserved_20_30               : 11;
	uint64_t txbypass                     : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} cn63xxp1;
	struct cvmx_ciu_qlm2_cn61xx           cn66xx;
	struct cvmx_ciu_qlm2_s                cn68xx;
	struct cvmx_ciu_qlm2_s                cn68xxp1;
	struct cvmx_ciu_qlm2_cn61xx           cnf71xx;
};
typedef union cvmx_ciu_qlm2 cvmx_ciu_qlm2_t;

/**
 * cvmx_ciu_qlm3
 *
 * Notes:
 * This register is only reset by cold reset.
 *
 */
union cvmx_ciu_qlm3 {
	uint64_t u64;
	struct cvmx_ciu_qlm3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t g2bypass                     : 1;  /**< QLMx PCIE Gen2 tx bypass enable */
	uint64_t reserved_53_62               : 10;
	uint64_t g2deemph                     : 5;  /**< QLMx PCIE Gen2 tx bypass de-emphasis value */
	uint64_t reserved_45_47               : 3;
	uint64_t g2margin                     : 5;  /**< QLMx PCIE Gen2 tx bypass margin (amplitude) value */
	uint64_t reserved_32_39               : 8;
	uint64_t txbypass                     : 1;  /**< QLMx transmitter bypass enable */
	uint64_t reserved_21_30               : 10;
	uint64_t txdeemph                     : 5;  /**< QLMx transmitter bypass de-emphasis value */
	uint64_t reserved_13_15               : 3;
	uint64_t txmargin                     : 5;  /**< QLMx transmitter bypass margin (amplitude) value */
	uint64_t reserved_4_7                 : 4;
	uint64_t lane_en                      : 4;  /**< QLMx lane enable mask */
#else
	uint64_t lane_en                      : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t txmargin                     : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t txdeemph                     : 5;
	uint64_t reserved_21_30               : 10;
	uint64_t txbypass                     : 1;
	uint64_t reserved_32_39               : 8;
	uint64_t g2margin                     : 5;
	uint64_t reserved_45_47               : 3;
	uint64_t g2deemph                     : 5;
	uint64_t reserved_53_62               : 10;
	uint64_t g2bypass                     : 1;
#endif
	} s;
	struct cvmx_ciu_qlm3_s                cn68xx;
	struct cvmx_ciu_qlm3_s                cn68xxp1;
};
typedef union cvmx_ciu_qlm3 cvmx_ciu_qlm3_t;

/**
 * cvmx_ciu_qlm4
 *
 * Notes:
 * This register is only reset by cold reset.
 *
 */
union cvmx_ciu_qlm4 {
	uint64_t u64;
	struct cvmx_ciu_qlm4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t g2bypass                     : 1;  /**< QLMx PCIE Gen2 tx bypass enable */
	uint64_t reserved_53_62               : 10;
	uint64_t g2deemph                     : 5;  /**< QLMx PCIE Gen2 tx bypass de-emphasis value */
	uint64_t reserved_45_47               : 3;
	uint64_t g2margin                     : 5;  /**< QLMx PCIE Gen2 tx bypass margin (amplitude) value */
	uint64_t reserved_32_39               : 8;
	uint64_t txbypass                     : 1;  /**< QLMx transmitter bypass enable */
	uint64_t reserved_21_30               : 10;
	uint64_t txdeemph                     : 5;  /**< QLMx transmitter bypass de-emphasis value */
	uint64_t reserved_13_15               : 3;
	uint64_t txmargin                     : 5;  /**< QLMx transmitter bypass margin (amplitude) value */
	uint64_t reserved_4_7                 : 4;
	uint64_t lane_en                      : 4;  /**< QLMx lane enable mask */
#else
	uint64_t lane_en                      : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t txmargin                     : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t txdeemph                     : 5;
	uint64_t reserved_21_30               : 10;
	uint64_t txbypass                     : 1;
	uint64_t reserved_32_39               : 8;
	uint64_t g2margin                     : 5;
	uint64_t reserved_45_47               : 3;
	uint64_t g2deemph                     : 5;
	uint64_t reserved_53_62               : 10;
	uint64_t g2bypass                     : 1;
#endif
	} s;
	struct cvmx_ciu_qlm4_s                cn68xx;
	struct cvmx_ciu_qlm4_s                cn68xxp1;
};
typedef union cvmx_ciu_qlm4 cvmx_ciu_qlm4_t;

/**
 * cvmx_ciu_qlm_dcok
 */
union cvmx_ciu_qlm_dcok {
	uint64_t u64;
	struct cvmx_ciu_qlm_dcok_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t qlm_dcok                     : 4;  /**< Re-assert dcok for each QLM. The value in this
                                                         field is "anded" with the pll_dcok pin and then
                                                         sent to each QLM (0..3). */
#else
	uint64_t qlm_dcok                     : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_ciu_qlm_dcok_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t qlm_dcok                     : 2;  /**< Re-assert dcok for each QLM. The value in this
                                                         field is "anded" with the pll_dcok pin and then
                                                         sent to each QLM (0..3). */
#else
	uint64_t qlm_dcok                     : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} cn52xx;
	struct cvmx_ciu_qlm_dcok_cn52xx       cn52xxp1;
	struct cvmx_ciu_qlm_dcok_s            cn56xx;
	struct cvmx_ciu_qlm_dcok_s            cn56xxp1;
};
typedef union cvmx_ciu_qlm_dcok cvmx_ciu_qlm_dcok_t;

/**
 * cvmx_ciu_qlm_jtgc
 */
union cvmx_ciu_qlm_jtgc {
	uint64_t u64;
	struct cvmx_ciu_qlm_jtgc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t bypass_ext                   : 1;  /**< BYPASS Field extension to select QLM 4
                                                         Selects which QLM JTAG shift chains are bypassed
                                                         by the QLM JTAG data register (CIU_QLM_JTGD) (one
                                                         bit per QLM) */
	uint64_t reserved_11_15               : 5;
	uint64_t clk_div                      : 3;  /**< Clock divider for QLM JTAG operations.  eclk is
                                                         divided by 2^(CLK_DIV + 2) */
	uint64_t reserved_7_7                 : 1;
	uint64_t mux_sel                      : 3;  /**< Selects which QLM JTAG shift out is shifted into
                                                         the QLM JTAG shift register: CIU_QLM_JTGD[SHFT_REG] */
	uint64_t bypass                       : 4;  /**< Selects which QLM JTAG shift chains are bypassed
                                                         by the QLM JTAG data register (CIU_QLM_JTGD) (one
                                                         bit per QLM) */
#else
	uint64_t bypass                       : 4;
	uint64_t mux_sel                      : 3;
	uint64_t reserved_7_7                 : 1;
	uint64_t clk_div                      : 3;
	uint64_t reserved_11_15               : 5;
	uint64_t bypass_ext                   : 1;
	uint64_t reserved_17_63               : 47;
#endif
	} s;
	struct cvmx_ciu_qlm_jtgc_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t clk_div                      : 3;  /**< Clock divider for QLM JTAG operations.  eclk is
                                                         divided by 2^(CLK_DIV + 2) */
	uint64_t reserved_5_7                 : 3;
	uint64_t mux_sel                      : 1;  /**< Selects which QLM JTAG shift out is shifted into
                                                         the QLM JTAG shift register: CIU_QLM_JTGD[SHFT_REG] */
	uint64_t reserved_2_3                 : 2;
	uint64_t bypass                       : 2;  /**< Selects which QLM JTAG shift chains are bypassed
                                                         by the QLM JTAG data register (CIU_QLM_JTGD) (one
                                                         bit per QLM) */
#else
	uint64_t bypass                       : 2;
	uint64_t reserved_2_3                 : 2;
	uint64_t mux_sel                      : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t clk_div                      : 3;
	uint64_t reserved_11_63               : 53;
#endif
	} cn52xx;
	struct cvmx_ciu_qlm_jtgc_cn52xx       cn52xxp1;
	struct cvmx_ciu_qlm_jtgc_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t clk_div                      : 3;  /**< Clock divider for QLM JTAG operations.  eclk is
                                                         divided by 2^(CLK_DIV + 2) */
	uint64_t reserved_6_7                 : 2;
	uint64_t mux_sel                      : 2;  /**< Selects which QLM JTAG shift out is shifted into
                                                         the QLM JTAG shift register: CIU_QLM_JTGD[SHFT_REG] */
	uint64_t bypass                       : 4;  /**< Selects which QLM JTAG shift chains are bypassed
                                                         by the QLM JTAG data register (CIU_QLM_JTGD) (one
                                                         bit per QLM) */
#else
	uint64_t bypass                       : 4;
	uint64_t mux_sel                      : 2;
	uint64_t reserved_6_7                 : 2;
	uint64_t clk_div                      : 3;
	uint64_t reserved_11_63               : 53;
#endif
	} cn56xx;
	struct cvmx_ciu_qlm_jtgc_cn56xx       cn56xxp1;
	struct cvmx_ciu_qlm_jtgc_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t clk_div                      : 3;  /**< Clock divider for QLM JTAG operations.  eclk is
                                                         divided by 2^(CLK_DIV + 2) */
	uint64_t reserved_6_7                 : 2;
	uint64_t mux_sel                      : 2;  /**< Selects which QLM JTAG shift out is shifted into
                                                         the QLM JTAG shift register: CIU_QLM_JTGD[SHFT_REG] */
	uint64_t reserved_3_3                 : 1;
	uint64_t bypass                       : 3;  /**< Selects which QLM JTAG shift chains are bypassed
                                                         by the QLM JTAG data register (CIU_QLM_JTGD) (one
                                                         bit per QLM) */
#else
	uint64_t bypass                       : 3;
	uint64_t reserved_3_3                 : 1;
	uint64_t mux_sel                      : 2;
	uint64_t reserved_6_7                 : 2;
	uint64_t clk_div                      : 3;
	uint64_t reserved_11_63               : 53;
#endif
	} cn61xx;
	struct cvmx_ciu_qlm_jtgc_cn61xx       cn63xx;
	struct cvmx_ciu_qlm_jtgc_cn61xx       cn63xxp1;
	struct cvmx_ciu_qlm_jtgc_cn61xx       cn66xx;
	struct cvmx_ciu_qlm_jtgc_s            cn68xx;
	struct cvmx_ciu_qlm_jtgc_s            cn68xxp1;
	struct cvmx_ciu_qlm_jtgc_cn61xx       cnf71xx;
};
typedef union cvmx_ciu_qlm_jtgc cvmx_ciu_qlm_jtgc_t;

/**
 * cvmx_ciu_qlm_jtgd
 */
union cvmx_ciu_qlm_jtgd {
	uint64_t u64;
	struct cvmx_ciu_qlm_jtgd_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t capture                      : 1;  /**< Perform JTAG capture operation (self-clearing when
                                                         op completes) */
	uint64_t shift                        : 1;  /**< Perform JTAG shift operation (self-clearing when
                                                         op completes) */
	uint64_t update                       : 1;  /**< Perform JTAG update operation (self-clearing when
                                                         op completes) */
	uint64_t reserved_45_60               : 16;
	uint64_t select                       : 5;  /**< Selects which QLM JTAG shift chains the JTAG
                                                         operations are performed on */
	uint64_t reserved_37_39               : 3;
	uint64_t shft_cnt                     : 5;  /**< QLM JTAG shift count (encoded in -1 notation) */
	uint64_t shft_reg                     : 32; /**< QLM JTAG shift register */
#else
	uint64_t shft_reg                     : 32;
	uint64_t shft_cnt                     : 5;
	uint64_t reserved_37_39               : 3;
	uint64_t select                       : 5;
	uint64_t reserved_45_60               : 16;
	uint64_t update                       : 1;
	uint64_t shift                        : 1;
	uint64_t capture                      : 1;
#endif
	} s;
	struct cvmx_ciu_qlm_jtgd_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t capture                      : 1;  /**< Perform JTAG capture operation (self-clearing when
                                                         op completes) */
	uint64_t shift                        : 1;  /**< Perform JTAG shift operation (self-clearing when
                                                         op completes) */
	uint64_t update                       : 1;  /**< Perform JTAG update operation (self-clearing when
                                                         op completes) */
	uint64_t reserved_42_60               : 19;
	uint64_t select                       : 2;  /**< Selects which QLM JTAG shift chains the JTAG
                                                         operations are performed on */
	uint64_t reserved_37_39               : 3;
	uint64_t shft_cnt                     : 5;  /**< QLM JTAG shift count (encoded in -1 notation) */
	uint64_t shft_reg                     : 32; /**< QLM JTAG shift register */
#else
	uint64_t shft_reg                     : 32;
	uint64_t shft_cnt                     : 5;
	uint64_t reserved_37_39               : 3;
	uint64_t select                       : 2;
	uint64_t reserved_42_60               : 19;
	uint64_t update                       : 1;
	uint64_t shift                        : 1;
	uint64_t capture                      : 1;
#endif
	} cn52xx;
	struct cvmx_ciu_qlm_jtgd_cn52xx       cn52xxp1;
	struct cvmx_ciu_qlm_jtgd_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t capture                      : 1;  /**< Perform JTAG capture operation (self-clearing when
                                                         op completes) */
	uint64_t shift                        : 1;  /**< Perform JTAG shift operation (self-clearing when
                                                         op completes) */
	uint64_t update                       : 1;  /**< Perform JTAG update operation (self-clearing when
                                                         op completes) */
	uint64_t reserved_44_60               : 17;
	uint64_t select                       : 4;  /**< Selects which QLM JTAG shift chains the JTAG
                                                         operations are performed on */
	uint64_t reserved_37_39               : 3;
	uint64_t shft_cnt                     : 5;  /**< QLM JTAG shift count (encoded in -1 notation) */
	uint64_t shft_reg                     : 32; /**< QLM JTAG shift register */
#else
	uint64_t shft_reg                     : 32;
	uint64_t shft_cnt                     : 5;
	uint64_t reserved_37_39               : 3;
	uint64_t select                       : 4;
	uint64_t reserved_44_60               : 17;
	uint64_t update                       : 1;
	uint64_t shift                        : 1;
	uint64_t capture                      : 1;
#endif
	} cn56xx;
	struct cvmx_ciu_qlm_jtgd_cn56xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t capture                      : 1;  /**< Perform JTAG capture operation (self-clearing when
                                                         op completes) */
	uint64_t shift                        : 1;  /**< Perform JTAG shift operation (self-clearing when
                                                         op completes) */
	uint64_t update                       : 1;  /**< Perform JTAG update operation (self-clearing when
                                                         op completes) */
	uint64_t reserved_37_60               : 24;
	uint64_t shft_cnt                     : 5;  /**< QLM JTAG shift count (encoded in -1 notation) */
	uint64_t shft_reg                     : 32; /**< QLM JTAG shift register */
#else
	uint64_t shft_reg                     : 32;
	uint64_t shft_cnt                     : 5;
	uint64_t reserved_37_60               : 24;
	uint64_t update                       : 1;
	uint64_t shift                        : 1;
	uint64_t capture                      : 1;
#endif
	} cn56xxp1;
	struct cvmx_ciu_qlm_jtgd_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t capture                      : 1;  /**< Perform JTAG capture operation (self-clearing when
                                                         op completes) */
	uint64_t shift                        : 1;  /**< Perform JTAG shift operation (self-clearing when
                                                         op completes) */
	uint64_t update                       : 1;  /**< Perform JTAG update operation (self-clearing when
                                                         op completes) */
	uint64_t reserved_43_60               : 18;
	uint64_t select                       : 3;  /**< Selects which QLM JTAG shift chains the JTAG
                                                         operations are performed on */
	uint64_t reserved_37_39               : 3;
	uint64_t shft_cnt                     : 5;  /**< QLM JTAG shift count (encoded in -1 notation) */
	uint64_t shft_reg                     : 32; /**< QLM JTAG shift register */
#else
	uint64_t shft_reg                     : 32;
	uint64_t shft_cnt                     : 5;
	uint64_t reserved_37_39               : 3;
	uint64_t select                       : 3;
	uint64_t reserved_43_60               : 18;
	uint64_t update                       : 1;
	uint64_t shift                        : 1;
	uint64_t capture                      : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_qlm_jtgd_cn61xx       cn63xx;
	struct cvmx_ciu_qlm_jtgd_cn61xx       cn63xxp1;
	struct cvmx_ciu_qlm_jtgd_cn61xx       cn66xx;
	struct cvmx_ciu_qlm_jtgd_s            cn68xx;
	struct cvmx_ciu_qlm_jtgd_s            cn68xxp1;
	struct cvmx_ciu_qlm_jtgd_cn61xx       cnf71xx;
};
typedef union cvmx_ciu_qlm_jtgd cvmx_ciu_qlm_jtgd_t;

/**
 * cvmx_ciu_soft_bist
 */
union cvmx_ciu_soft_bist {
	uint64_t u64;
	struct cvmx_ciu_soft_bist_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t soft_bist                    : 1;  /**< Reserved */
#else
	uint64_t soft_bist                    : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_ciu_soft_bist_s           cn30xx;
	struct cvmx_ciu_soft_bist_s           cn31xx;
	struct cvmx_ciu_soft_bist_s           cn38xx;
	struct cvmx_ciu_soft_bist_s           cn38xxp2;
	struct cvmx_ciu_soft_bist_s           cn50xx;
	struct cvmx_ciu_soft_bist_s           cn52xx;
	struct cvmx_ciu_soft_bist_s           cn52xxp1;
	struct cvmx_ciu_soft_bist_s           cn56xx;
	struct cvmx_ciu_soft_bist_s           cn56xxp1;
	struct cvmx_ciu_soft_bist_s           cn58xx;
	struct cvmx_ciu_soft_bist_s           cn58xxp1;
	struct cvmx_ciu_soft_bist_s           cn61xx;
	struct cvmx_ciu_soft_bist_s           cn63xx;
	struct cvmx_ciu_soft_bist_s           cn63xxp1;
	struct cvmx_ciu_soft_bist_s           cn66xx;
	struct cvmx_ciu_soft_bist_s           cn68xx;
	struct cvmx_ciu_soft_bist_s           cn68xxp1;
	struct cvmx_ciu_soft_bist_s           cn70xx;
	struct cvmx_ciu_soft_bist_s           cn70xxp1;
	struct cvmx_ciu_soft_bist_s           cnf71xx;
};
typedef union cvmx_ciu_soft_bist cvmx_ciu_soft_bist_t;

/**
 * cvmx_ciu_soft_prst
 */
union cvmx_ciu_soft_prst {
	uint64_t u64;
	struct cvmx_ciu_soft_prst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t host64                       : 1;  /**< PCX Host Mode Device Capability (0=32b/1=64b) */
	uint64_t npi                          : 1;  /**< When PCI soft reset is asserted, also reset the
                                                         NPI and PNI logic */
	uint64_t soft_prst                    : 1;  /**< Resets the PCIe logic in all modes, not just
                                                         RC mode. The reset value is based on the
                                                         corresponding MIO_RST_CTL[PRTMODE] CSR field:
                                                          If PRTMODE == 0, then SOFT_PRST resets to 0
                                                          If PRTMODE != 0, then SOFT_PRST resets to 1
                                                         When OCTEON is configured to drive the PERST*_L
                                                         chip pin (ie. MIO_RST_CTL0[RST_DRV] is set), this
                                                         controls the PERST*_L chip pin. */
#else
	uint64_t soft_prst                    : 1;
	uint64_t npi                          : 1;
	uint64_t host64                       : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_ciu_soft_prst_s           cn30xx;
	struct cvmx_ciu_soft_prst_s           cn31xx;
	struct cvmx_ciu_soft_prst_s           cn38xx;
	struct cvmx_ciu_soft_prst_s           cn38xxp2;
	struct cvmx_ciu_soft_prst_s           cn50xx;
	struct cvmx_ciu_soft_prst_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t soft_prst                    : 1;  /**< Reset the PCI bus.  Only works when Octane is
                                                         configured as a HOST. When OCTEON is a PCI host
                                                         (i.e. when PCI_HOST_MODE = 1), This controls
                                                         PCI_RST_L. Refer to section 10.11.1. */
#else
	uint64_t soft_prst                    : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} cn52xx;
	struct cvmx_ciu_soft_prst_cn52xx      cn52xxp1;
	struct cvmx_ciu_soft_prst_cn52xx      cn56xx;
	struct cvmx_ciu_soft_prst_cn52xx      cn56xxp1;
	struct cvmx_ciu_soft_prst_s           cn58xx;
	struct cvmx_ciu_soft_prst_s           cn58xxp1;
	struct cvmx_ciu_soft_prst_cn52xx      cn61xx;
	struct cvmx_ciu_soft_prst_cn52xx      cn63xx;
	struct cvmx_ciu_soft_prst_cn52xx      cn63xxp1;
	struct cvmx_ciu_soft_prst_cn52xx      cn66xx;
	struct cvmx_ciu_soft_prst_cn52xx      cn68xx;
	struct cvmx_ciu_soft_prst_cn52xx      cn68xxp1;
	struct cvmx_ciu_soft_prst_cn52xx      cnf71xx;
};
typedef union cvmx_ciu_soft_prst cvmx_ciu_soft_prst_t;

/**
 * cvmx_ciu_soft_prst1
 */
union cvmx_ciu_soft_prst1 {
	uint64_t u64;
	struct cvmx_ciu_soft_prst1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t soft_prst                    : 1;  /**< Resets the PCIe logic in all modes, not just
                                                         RC mode. The reset value is based on the
                                                         corresponding MIO_RST_CTL[PRTMODE] CSR field:
                                                          If PRTMODE == 0, then SOFT_PRST resets to 0
                                                          If PRTMODE != 0, then SOFT_PRST resets to 1
                                                         In o61, this PRST initial value is always '1' as
                                                         PEM1 always running on host mode. */
#else
	uint64_t soft_prst                    : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_ciu_soft_prst1_s          cn52xx;
	struct cvmx_ciu_soft_prst1_s          cn52xxp1;
	struct cvmx_ciu_soft_prst1_s          cn56xx;
	struct cvmx_ciu_soft_prst1_s          cn56xxp1;
	struct cvmx_ciu_soft_prst1_s          cn61xx;
	struct cvmx_ciu_soft_prst1_s          cn63xx;
	struct cvmx_ciu_soft_prst1_s          cn63xxp1;
	struct cvmx_ciu_soft_prst1_s          cn66xx;
	struct cvmx_ciu_soft_prst1_s          cn68xx;
	struct cvmx_ciu_soft_prst1_s          cn68xxp1;
	struct cvmx_ciu_soft_prst1_s          cnf71xx;
};
typedef union cvmx_ciu_soft_prst1 cvmx_ciu_soft_prst1_t;

/**
 * cvmx_ciu_soft_prst2
 */
union cvmx_ciu_soft_prst2 {
	uint64_t u64;
	struct cvmx_ciu_soft_prst2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t soft_prst                    : 1;  /**< Resets the      sRIO logic in all modes, not just
                                                         RC mode. The reset value is based on the
                                                         corresponding MIO_RST_CNTL[PRTMODE] CSR field:
                                                          If PRTMODE == 0, then SOFT_PRST resets to 0
                                                          If PRTMODE != 0, then SOFT_PRST resets to 1 */
#else
	uint64_t soft_prst                    : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_ciu_soft_prst2_s          cn66xx;
};
typedef union cvmx_ciu_soft_prst2 cvmx_ciu_soft_prst2_t;

/**
 * cvmx_ciu_soft_prst3
 */
union cvmx_ciu_soft_prst3 {
	uint64_t u64;
	struct cvmx_ciu_soft_prst3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t soft_prst                    : 1;  /**< Resets the      sRIO logic in all modes, not just
                                                         RC mode. The reset value is based on the
                                                         corresponding MIO_RST_CNTL[PRTMODE] CSR field:
                                                          If PRTMODE == 0, then SOFT_PRST resets to 0
                                                          If PRTMODE != 0, then SOFT_PRST resets to 1 */
#else
	uint64_t soft_prst                    : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_ciu_soft_prst3_s          cn66xx;
};
typedef union cvmx_ciu_soft_prst3 cvmx_ciu_soft_prst3_t;

/**
 * cvmx_ciu_soft_rst
 */
union cvmx_ciu_soft_rst {
	uint64_t u64;
	struct cvmx_ciu_soft_rst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t soft_rst                     : 1;  /**< Resets Octeon
                                                         When soft reseting Octeon from a remote PCIe
                                                         host, always read CIU_SOFT_RST (and wait for
                                                         result) before writing SOFT_RST to '1'. */
#else
	uint64_t soft_rst                     : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_ciu_soft_rst_s            cn30xx;
	struct cvmx_ciu_soft_rst_s            cn31xx;
	struct cvmx_ciu_soft_rst_s            cn38xx;
	struct cvmx_ciu_soft_rst_s            cn38xxp2;
	struct cvmx_ciu_soft_rst_s            cn50xx;
	struct cvmx_ciu_soft_rst_s            cn52xx;
	struct cvmx_ciu_soft_rst_s            cn52xxp1;
	struct cvmx_ciu_soft_rst_s            cn56xx;
	struct cvmx_ciu_soft_rst_s            cn56xxp1;
	struct cvmx_ciu_soft_rst_s            cn58xx;
	struct cvmx_ciu_soft_rst_s            cn58xxp1;
	struct cvmx_ciu_soft_rst_s            cn61xx;
	struct cvmx_ciu_soft_rst_s            cn63xx;
	struct cvmx_ciu_soft_rst_s            cn63xxp1;
	struct cvmx_ciu_soft_rst_s            cn66xx;
	struct cvmx_ciu_soft_rst_s            cn68xx;
	struct cvmx_ciu_soft_rst_s            cn68xxp1;
	struct cvmx_ciu_soft_rst_s            cnf71xx;
};
typedef union cvmx_ciu_soft_rst cvmx_ciu_soft_rst_t;

/**
 * cvmx_ciu_sum1_io#_int
 *
 * CIU_SUM1_IO0_INT is for PEM0, CIU_SUM1_IO1_INT is reserved.
 *
 */
union cvmx_ciu_sum1_iox_int {
	uint64_t u64;
	struct cvmx_ciu_sum1_iox_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt
                                                         See SRIO3_INT_REG, SRIO3_INT2_REG */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt
                                                         See SRIO2_INT_REG, SRIO2_INT2_REG */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< DFM Interrupt
                                                         See DFM_FNT_STAT */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion  interrupt
                                                         TBD, See DPI DMA instruction completion */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt
                                                         See EMMC interrupt */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t usb1                         : 1;  /**< USBDRD1 interrupt.  Value equals ((CIU_CIB_USBDRD(1)_RAW & CIU_CIB_USBDRD(1)_EN) != 0).
                                                         See CIU_CIB_USBDRD(1)_RAW and CIU_CIB_USBDRD(1)_EN. */
	uint64_t reserved_10_16               : 7;
	uint64_t wdog                         : 10; /**< Per PP watchdog interrupts */
#else
	uint64_t wdog                         : 10;
	uint64_t reserved_10_16               : 7;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} s;
	struct cvmx_ciu_sum1_iox_int_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion  interrupt
                                                         TBD, See DPI DMA instruction completion */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt
                                                         See EMMC interrupt */
	uint64_t mii1                         : 1;  /**< RGMII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t reserved_4_17                : 14;
	uint64_t wdog                         : 4;  /**< Per PP watchdog interrupts */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_17                : 14;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_sum1_iox_int_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt
                                                         See SRIO3_INT_REG, SRIO3_INT2_REG */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt
                                                         See SRIO2_INT_REG, SRIO2_INT2_REG */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< DFM Interrupt
                                                         See DFM_FNT_STAT */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_51_51               : 1;
	uint64_t srio0                        : 1;  /**< SRIO0 interrupt
                                                         See SRIO0_INT_REG, SRIO0_INT2_REG */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_38_45               : 8;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< NAND Flash Controller interrupt
                                                         See NDF_INT */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t reserved_10_17               : 8;
	uint64_t wdog                         : 10; /**< 10 watchdog interrupts */
#else
	uint64_t wdog                         : 10;
	uint64_t reserved_10_17               : 8;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_45               : 8;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_sum1_iox_int_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< RST interrupt. Value equals ((CIU_CIB_RST_RAW & CIU_CIB_RST_EN) != 0).
                                                         See CIU_CIB_RST_RAW and CIU_CIB_RST_EN. */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt. Value equals ((CIU_CIB_LMC(0)_RAW & CIU_CIB_LMC(0)_EN) != 0).
                                                         See CIU_CIB_LMC(0)_RAW and CIU_CIB_LMC(0)_EN. */
	uint64_t reserved_51_51               : 1;
	uint64_t pem2                         : 1;  /**< PEM2 interrupt
                                                         See PEM2_INT_SUM (enabled by PEM2_INT_ENB) */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero.
                                                         See MIO_PTP_EVT_CNT for details. */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion interrupt.
                                                         This bit is different for each CIU_SUM1_PPx.
                                                         See DPI_DMA_PP*_CNT. */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USBDRD0 interrupt.  Value equals ((CIU_CIB_USBDRD(0)_RAW & CIU_CIB_USBDRD(0)_EN) != 0).
                                                         See CIU_CIB_USBDRD(0)_RAW and CIU_CIB_USBDRD(0)_EN. */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt. Value equals ((CIU_CIB_L2C_RAW & CIU_CIB_L2C_EN) != 0).
                                                         See CIU_CIB_L2C_RAW and CIU_CIB_L2C_EN. */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< NAND / EMMC Controller interrupt
                                                         See NAND / EMMC interrupt */
	uint64_t reserved_18_18               : 1;
	uint64_t usb1                         : 1;  /**< USBDRD1 interrupt.  Value equals ((CIU_CIB_USBDRD(1)_RAW & CIU_CIB_USBDRD(1)_EN) != 0).
                                                         See CIU_CIB_USBDRD(1)_RAW and CIU_CIB_USBDRD(1)_EN. */
	uint64_t reserved_4_16                : 13;
	uint64_t wdog                         : 4;  /**< Watchdog interrupts, bit 0 is watchdog for PP0, ..., bit x for PPx. */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_16                : 13;
	uint64_t usb1                         : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t pem2                         : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_sum1_iox_int_cn70xx   cn70xxp1;
	struct cvmx_ciu_sum1_iox_int_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t reserved_41_46               : 6;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion  interrupt
                                                         TBD, See DPI DMA instruction completion */
	uint64_t reserved_37_39               : 3;
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t reserved_32_32               : 1;
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt
                                                         See EMMC interrupt */
	uint64_t reserved_4_18                : 15;
	uint64_t wdog                         : 4;  /**< Per PP watchdog interrupts */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_18                : 15;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t reserved_32_32               : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_39               : 3;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_46               : 6;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_sum1_iox_int cvmx_ciu_sum1_iox_int_t;

/**
 * cvmx_ciu_sum1_pp#_ip2
 *
 * SUM1 becomes per IPx in o65/6 and afterwards. Only Field <40> DPI_DMA will have
 * different value per PP(IP) for  $CIU_SUM1_PPx_IPy, and <40> DPI_DMA will always
 * be zero for  $CIU_SUM1_IOX_INT. All other fields ([63:41] and [39:0]) values  are idential for
 * different PPs, same value as $CIU_INT_SUM1.
 * Write to any IRQ's PTP fields will clear PTP for all IRQ's PTP field.
 */
union cvmx_ciu_sum1_ppx_ip2 {
	uint64_t u64;
	struct cvmx_ciu_sum1_ppx_ip2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt
                                                         See SRIO3_INT_REG, SRIO3_INT2_REG */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt
                                                         See SRIO2_INT_REG, SRIO2_INT2_REG */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< DFM Interrupt
                                                         See DFM_FNT_STAT */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion  interrupt
                                                         TBD, See DPI DMA instruction completion */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt
                                                         See EMMC interrupt */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t usb1                         : 1;  /**< USBDRD1 interrupt.  Value equals ((CIU_CIB_USBDRD(1)_RAW & CIU_CIB_USBDRD(1)_EN) != 0).
                                                         See CIU_CIB_USBDRD(1)_RAW and CIU_CIB_USBDRD(1)_EN. */
	uint64_t reserved_10_16               : 7;
	uint64_t wdog                         : 10; /**< Per PP watchdog interrupts */
#else
	uint64_t wdog                         : 10;
	uint64_t reserved_10_16               : 7;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} s;
	struct cvmx_ciu_sum1_ppx_ip2_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion  interrupt
                                                         TBD, See DPI DMA instruction completion */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt
                                                         See EMMC interrupt */
	uint64_t mii1                         : 1;  /**< RGMII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t reserved_4_17                : 14;
	uint64_t wdog                         : 4;  /**< Per PP watchdog interrupts */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_17                : 14;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_sum1_ppx_ip2_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt
                                                         See SRIO3_INT_REG, SRIO3_INT2_REG */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt
                                                         See SRIO2_INT_REG, SRIO2_INT2_REG */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< DFM Interrupt
                                                         See DFM_FNT_STAT */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_51_51               : 1;
	uint64_t srio0                        : 1;  /**< SRIO0 interrupt
                                                         See SRIO0_INT_REG, SRIO0_INT2_REG */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_38_45               : 8;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< NAND Flash Controller interrupt
                                                         See NDF_INT */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t reserved_10_17               : 8;
	uint64_t wdog                         : 10; /**< 10 watchdog interrupts */
#else
	uint64_t wdog                         : 10;
	uint64_t reserved_10_17               : 8;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_45               : 8;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_sum1_ppx_ip2_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< RST interrupt. Value equals ((CIU_CIB_RST_RAW & CIU_CIB_RST_EN) != 0).
                                                         See CIU_CIB_RST_RAW and CIU_CIB_RST_EN. */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt. Value equals ((CIU_CIB_LMC(0)_RAW & CIU_CIB_LMC(0)_EN) != 0).
                                                         See CIU_CIB_LMC(0)_RAW and CIU_CIB_LMC(0)_EN. */
	uint64_t reserved_51_51               : 1;
	uint64_t pem2                         : 1;  /**< PEM2 interrupt
                                                         See PEM2_INT_SUM (enabled by PEM2_INT_ENB) */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero.
                                                         See MIO_PTP_EVT_CNT for details. */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion interrupt.
                                                         This bit is different for each CIU_SUM1_PPx.
                                                         See DPI_DMA_PP*_CNT. */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USBDRD0 interrupt.  Value equals ((CIU_CIB_USBDRD(0)_RAW & CIU_CIB_USBDRD(0)_EN) != 0).
                                                         See CIU_CIB_USBDRD(0)_RAW and CIU_CIB_USBDRD(0)_EN. */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt. Value equals ((CIU_CIB_L2C_RAW & CIU_CIB_L2C_EN) != 0).
                                                         See CIU_CIB_L2C_RAW and CIU_CIB_L2C_EN. */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< NAND / EMMC Controller interrupt
                                                         See NAND / EMMC interrupt */
	uint64_t reserved_18_18               : 1;
	uint64_t usb1                         : 1;  /**< USBDRD1 interrupt.  Value equals ((CIU_CIB_USBDRD(1)_RAW & CIU_CIB_USBDRD(1)_EN) != 0).
                                                         See CIU_CIB_USBDRD(1)_RAW and CIU_CIB_USBDRD(1)_EN. */
	uint64_t reserved_4_16                : 13;
	uint64_t wdog                         : 4;  /**< Watchdog interrupts, bit 0 is watchdog for PP0, ..., bit x for PPx. */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_16                : 13;
	uint64_t usb1                         : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t pem2                         : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_sum1_ppx_ip2_cn70xx   cn70xxp1;
	struct cvmx_ciu_sum1_ppx_ip2_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t reserved_41_46               : 6;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion  interrupt
                                                         TBD, See DPI DMA instruction completion */
	uint64_t reserved_37_39               : 3;
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t reserved_32_32               : 1;
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt
                                                         See EMMC interrupt */
	uint64_t reserved_4_18                : 15;
	uint64_t wdog                         : 4;  /**< Per PP watchdog interrupts */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_18                : 15;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t reserved_32_32               : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_39               : 3;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_46               : 6;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_sum1_ppx_ip2 cvmx_ciu_sum1_ppx_ip2_t;

/**
 * cvmx_ciu_sum1_pp#_ip3
 *
 * Notes:
 * SUM1 becomes per IPx in o65/6 and afterwards. Only Field <40> DPI_DMA will have
 * different value per PP(IP) for  $CIU_SUM1_PPx_IPy, and <40> DPI_DMA will always
 * be zero for  $CIU_SUM1_IOX_INT. All other fields ([63:41] and [39:0]) values  are idential for
 * different PPs, same value as $CIU_INT_SUM1.
 * Write to any IRQ's PTP fields will clear PTP for all IRQ's PTP field.
 */
union cvmx_ciu_sum1_ppx_ip3 {
	uint64_t u64;
	struct cvmx_ciu_sum1_ppx_ip3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt
                                                         See SRIO3_INT_REG, SRIO3_INT2_REG */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt
                                                         See SRIO2_INT_REG, SRIO2_INT2_REG */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< DFM Interrupt
                                                         See DFM_FNT_STAT */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion  interrupt
                                                         TBD, See DPI DMA instruction completion */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt
                                                         See EMMC interrupt */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t usb1                         : 1;  /**< USBDRD1 interrupt.  Value equals ((CIU_CIB_USBDRD(1)_RAW & CIU_CIB_USBDRD(1)_EN) != 0).
                                                         See CIU_CIB_USBDRD(1)_RAW and CIU_CIB_USBDRD(1)_EN. */
	uint64_t reserved_10_16               : 7;
	uint64_t wdog                         : 10; /**< Per PP watchdog interrupts */
#else
	uint64_t wdog                         : 10;
	uint64_t reserved_10_16               : 7;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} s;
	struct cvmx_ciu_sum1_ppx_ip3_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion  interrupt
                                                         TBD, See DPI DMA instruction completion */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt
                                                         See EMMC interrupt */
	uint64_t mii1                         : 1;  /**< RGMII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t reserved_4_17                : 14;
	uint64_t wdog                         : 4;  /**< Per PP watchdog interrupts */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_17                : 14;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_sum1_ppx_ip3_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt
                                                         See SRIO3_INT_REG, SRIO3_INT2_REG */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt
                                                         See SRIO2_INT_REG, SRIO2_INT2_REG */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< DFM Interrupt
                                                         See DFM_FNT_STAT */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_51_51               : 1;
	uint64_t srio0                        : 1;  /**< SRIO0 interrupt
                                                         See SRIO0_INT_REG, SRIO0_INT2_REG */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_38_45               : 8;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< NAND Flash Controller interrupt
                                                         See NDF_INT */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t reserved_10_17               : 8;
	uint64_t wdog                         : 10; /**< 10 watchdog interrupts */
#else
	uint64_t wdog                         : 10;
	uint64_t reserved_10_17               : 8;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_45               : 8;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_sum1_ppx_ip3_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< RST interrupt. Value equals ((CIU_CIB_RST_RAW & CIU_CIB_RST_EN) != 0).
                                                         See CIU_CIB_RST_RAW and CIU_CIB_RST_EN. */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt. Value equals ((CIU_CIB_LMC(0)_RAW & CIU_CIB_LMC(0)_EN) != 0).
                                                         See CIU_CIB_LMC(0)_RAW and CIU_CIB_LMC(0)_EN. */
	uint64_t reserved_51_51               : 1;
	uint64_t pem2                         : 1;  /**< PEM2 interrupt
                                                         See PEM2_INT_SUM (enabled by PEM2_INT_ENB) */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero.
                                                         See MIO_PTP_EVT_CNT for details. */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion interrupt.
                                                         This bit is different for each CIU_SUM1_PPx.
                                                         See DPI_DMA_PP*_CNT. */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USBDRD0 interrupt.  Value equals ((CIU_CIB_USBDRD(0)_RAW & CIU_CIB_USBDRD(0)_EN) != 0).
                                                         See CIU_CIB_USBDRD(0)_RAW and CIU_CIB_USBDRD(0)_EN. */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt. Value equals ((CIU_CIB_L2C_RAW & CIU_CIB_L2C_EN) != 0).
                                                         See CIU_CIB_L2C_RAW and CIU_CIB_L2C_EN. */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< NAND / EMMC Controller interrupt
                                                         See NAND / EMMC interrupt */
	uint64_t reserved_18_18               : 1;
	uint64_t usb1                         : 1;  /**< USBDRD1 interrupt.  Value equals ((CIU_CIB_USBDRD(1)_RAW & CIU_CIB_USBDRD(1)_EN) != 0).
                                                         See CIU_CIB_USBDRD(1)_RAW and CIU_CIB_USBDRD(1)_EN. */
	uint64_t reserved_4_16                : 13;
	uint64_t wdog                         : 4;  /**< Watchdog interrupts, bit 0 is watchdog for PP0, ..., bit x for PPx. */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_16                : 13;
	uint64_t usb1                         : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t pem2                         : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_sum1_ppx_ip3_cn70xx   cn70xxp1;
	struct cvmx_ciu_sum1_ppx_ip3_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t reserved_41_46               : 6;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion  interrupt
                                                         TBD, See DPI DMA instruction completion */
	uint64_t reserved_37_39               : 3;
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t reserved_32_32               : 1;
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt
                                                         See EMMC interrupt */
	uint64_t reserved_4_18                : 15;
	uint64_t wdog                         : 4;  /**< Per PP watchdog interrupts */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_18                : 15;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t reserved_32_32               : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_39               : 3;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_46               : 6;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_sum1_ppx_ip3 cvmx_ciu_sum1_ppx_ip3_t;

/**
 * cvmx_ciu_sum1_pp#_ip4
 *
 * Notes:
 * SUM1 becomes per IPx in o65/6 and afterwards. Only Field <40> DPI_DMA will have
 * different value per PP(IP) for  $CIU_SUM1_PPx_IPy, and <40> DPI_DMA will always
 * be zero for  $CIU_SUM1_IOX_INT. All other fields ([63:41] and [39:0]) values  are idential for
 * different PPs, same value as $CIU_INT_SUM1.
 * Write to any IRQ's PTP fields will clear PTP for all IRQ's PTP field.
 */
union cvmx_ciu_sum1_ppx_ip4 {
	uint64_t u64;
	struct cvmx_ciu_sum1_ppx_ip4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt
                                                         See SRIO3_INT_REG, SRIO3_INT2_REG */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt
                                                         See SRIO2_INT_REG, SRIO2_INT2_REG */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< DFM Interrupt
                                                         See DFM_FNT_STAT */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion  interrupt
                                                         TBD, See DPI DMA instruction completion */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt
                                                         See EMMC interrupt */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t usb1                         : 1;  /**< USBDRD1 interrupt.  Value equals ((CIU_CIB_USBDRD(1)_RAW & CIU_CIB_USBDRD(1)_EN) != 0).
                                                         See CIU_CIB_USBDRD(1)_RAW and CIU_CIB_USBDRD(1)_EN. */
	uint64_t reserved_10_16               : 7;
	uint64_t wdog                         : 10; /**< Per PP watchdog interrupts */
#else
	uint64_t wdog                         : 10;
	uint64_t reserved_10_16               : 7;
	uint64_t usb1                         : 1;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} s;
	struct cvmx_ciu_sum1_ppx_ip4_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion  interrupt
                                                         TBD, See DPI DMA instruction completion */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt
                                                         See EMMC interrupt */
	uint64_t mii1                         : 1;  /**< RGMII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t reserved_4_17                : 14;
	uint64_t wdog                         : 4;  /**< Per PP watchdog interrupts */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_17                : 14;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn61xx;
	struct cvmx_ciu_sum1_ppx_ip4_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_62_62               : 1;
	uint64_t srio3                        : 1;  /**< SRIO3 interrupt
                                                         See SRIO3_INT_REG, SRIO3_INT2_REG */
	uint64_t srio2                        : 1;  /**< SRIO2 interrupt
                                                         See SRIO2_INT_REG, SRIO2_INT2_REG */
	uint64_t reserved_57_59               : 3;
	uint64_t dfm                          : 1;  /**< DFM Interrupt
                                                         See DFM_FNT_STAT */
	uint64_t reserved_53_55               : 3;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_51_51               : 1;
	uint64_t srio0                        : 1;  /**< SRIO0 interrupt
                                                         See SRIO0_INT_REG, SRIO0_INT2_REG */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_38_45               : 8;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t zip                          : 1;  /**< ZIP interrupt
                                                         See ZIP_ERROR */
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< NAND Flash Controller interrupt
                                                         See NDF_INT */
	uint64_t mii1                         : 1;  /**< RGMII/MII/MIX Interface 1 Interrupt
                                                         See MIX1_ISR */
	uint64_t reserved_10_17               : 8;
	uint64_t wdog                         : 10; /**< 10 watchdog interrupts */
#else
	uint64_t wdog                         : 10;
	uint64_t reserved_10_17               : 8;
	uint64_t mii1                         : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t zip                          : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_45               : 8;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t srio0                        : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_55               : 3;
	uint64_t dfm                          : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t srio2                        : 1;
	uint64_t srio3                        : 1;
	uint64_t reserved_62_62               : 1;
	uint64_t rst                          : 1;
#endif
	} cn66xx;
	struct cvmx_ciu_sum1_ppx_ip4_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< RST interrupt. Value equals ((CIU_CIB_RST_RAW & CIU_CIB_RST_EN) != 0).
                                                         See CIU_CIB_RST_RAW and CIU_CIB_RST_EN. */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt. Value equals ((CIU_CIB_LMC(0)_RAW & CIU_CIB_LMC(0)_EN) != 0).
                                                         See CIU_CIB_LMC(0)_RAW and CIU_CIB_LMC(0)_EN. */
	uint64_t reserved_51_51               : 1;
	uint64_t pem2                         : 1;  /**< PEM2 interrupt
                                                         See PEM2_INT_SUM (enabled by PEM2_INT_ENB) */
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero.
                                                         See MIO_PTP_EVT_CNT for details. */
	uint64_t agl                          : 1;  /**< AGL interrupt
                                                         See AGL_GMX_RX*_INT_REG, AGL_GMX_TX_INT_REG */
	uint64_t reserved_41_45               : 5;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion interrupt.
                                                         This bit is different for each CIU_SUM1_PPx.
                                                         See DPI_DMA_PP*_CNT. */
	uint64_t reserved_38_39               : 2;
	uint64_t agx1                         : 1;  /**< GMX1 interrupt
                                                         See GMX1_RX*_INT_REG, GMX1_TX_INT_REG,
                                                         PCS1_INT*_REG, PCSX1_INT_REG */
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USBDRD0 interrupt.  Value equals ((CIU_CIB_USBDRD(0)_RAW & CIU_CIB_USBDRD(0)_EN) != 0).
                                                         See CIU_CIB_USBDRD(0)_RAW and CIU_CIB_USBDRD(0)_EN. */
	uint64_t dfa                          : 1;  /**< DFA interrupt
                                                         See DFA_ERROR */
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt. Value equals ((CIU_CIB_L2C_RAW & CIU_CIB_L2C_EN) != 0).
                                                         See CIU_CIB_L2C_RAW and CIU_CIB_L2C_EN. */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< NAND / EMMC Controller interrupt
                                                         See NAND / EMMC interrupt */
	uint64_t reserved_18_18               : 1;
	uint64_t usb1                         : 1;  /**< USBDRD1 interrupt.  Value equals ((CIU_CIB_USBDRD(1)_RAW & CIU_CIB_USBDRD(1)_EN) != 0).
                                                         See CIU_CIB_USBDRD(1)_RAW and CIU_CIB_USBDRD(1)_EN. */
	uint64_t reserved_4_16                : 13;
	uint64_t wdog                         : 4;  /**< Watchdog interrupts, bit 0 is watchdog for PP0, ..., bit x for PPx. */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_16                : 13;
	uint64_t usb1                         : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t dfa                          : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t agx1                         : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_45               : 5;
	uint64_t agl                          : 1;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t pem2                         : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cn70xx;
	struct cvmx_ciu_sum1_ppx_ip4_cn70xx   cn70xxp1;
	struct cvmx_ciu_sum1_ppx_ip4_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 1;  /**< MIO RST interrupt
                                                         See MIO_RST_INT */
	uint64_t reserved_53_62               : 10;
	uint64_t lmc0                         : 1;  /**< LMC0 interrupt
                                                         See LMC0_INT */
	uint64_t reserved_50_51               : 2;
	uint64_t pem1                         : 1;  /**< PEM1 interrupt
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB) */
	uint64_t pem0                         : 1;  /**< PEM0 interrupt
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB) */
	uint64_t ptp                          : 1;  /**< PTP interrupt
                                                         Set when HW decrements MIO_PTP_EVT_CNT to zero */
	uint64_t reserved_41_46               : 6;
	uint64_t dpi_dma                      : 1;  /**< DPI DMA instruction completion  interrupt
                                                         TBD, See DPI DMA instruction completion */
	uint64_t reserved_37_39               : 3;
	uint64_t agx0                         : 1;  /**< GMX0 interrupt
                                                         See GMX0_RX*_INT_REG, GMX0_TX_INT_REG,
                                                         PCS0_INT*_REG, PCSX0_INT_REG */
	uint64_t dpi                          : 1;  /**< DPI interrupt
                                                         See DPI_INT_REG */
	uint64_t sli                          : 1;  /**< SLI interrupt
                                                         See SLI_INT_SUM (enabled by SLI_INT_ENB_CIU) */
	uint64_t usb                          : 1;  /**< USB UCTL0 interrupt
                                                         See UCTL0_INT_REG */
	uint64_t reserved_32_32               : 1;
	uint64_t key                          : 1;  /**< KEY interrupt
                                                         See KEY_INT_SUM */
	uint64_t rad                          : 1;  /**< RAD interrupt
                                                         See RAD_REG_ERROR */
	uint64_t tim                          : 1;  /**< TIM interrupt
                                                         See TIM_REG_ERROR */
	uint64_t reserved_28_28               : 1;
	uint64_t pko                          : 1;  /**< PKO interrupt
                                                         See PKO_REG_ERROR */
	uint64_t pip                          : 1;  /**< PIP interrupt
                                                         See PIP_INT_REG */
	uint64_t ipd                          : 1;  /**< IPD interrupt
                                                         See IPD_INT_SUM */
	uint64_t l2c                          : 1;  /**< L2C interrupt
                                                         See L2C_INT_REG */
	uint64_t pow                          : 1;  /**< POW err interrupt
                                                         See POW_ECC_ERR */
	uint64_t fpa                          : 1;  /**< FPA interrupt
                                                         See FPA_INT_SUM */
	uint64_t iob                          : 1;  /**< IOB interrupt
                                                         See IOB_INT_SUM */
	uint64_t mio                          : 1;  /**< MIO boot interrupt
                                                         See MIO_BOOT_ERR */
	uint64_t nand                         : 1;  /**< EMMC Flash Controller interrupt
                                                         See EMMC interrupt */
	uint64_t reserved_4_18                : 15;
	uint64_t wdog                         : 4;  /**< Per PP watchdog interrupts */
#else
	uint64_t wdog                         : 4;
	uint64_t reserved_4_18                : 15;
	uint64_t nand                         : 1;
	uint64_t mio                          : 1;
	uint64_t iob                          : 1;
	uint64_t fpa                          : 1;
	uint64_t pow                          : 1;
	uint64_t l2c                          : 1;
	uint64_t ipd                          : 1;
	uint64_t pip                          : 1;
	uint64_t pko                          : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t tim                          : 1;
	uint64_t rad                          : 1;
	uint64_t key                          : 1;
	uint64_t reserved_32_32               : 1;
	uint64_t usb                          : 1;
	uint64_t sli                          : 1;
	uint64_t dpi                          : 1;
	uint64_t agx0                         : 1;
	uint64_t reserved_37_39               : 3;
	uint64_t dpi_dma                      : 1;
	uint64_t reserved_41_46               : 6;
	uint64_t ptp                          : 1;
	uint64_t pem0                         : 1;
	uint64_t pem1                         : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t lmc0                         : 1;
	uint64_t reserved_53_62               : 10;
	uint64_t rst                          : 1;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_sum1_ppx_ip4 cvmx_ciu_sum1_ppx_ip4_t;

/**
 * cvmx_ciu_sum2_io#_int
 *
 * CIU_SUM2_IO0_INT is for PEM0, CIU_SUM2_IO1_INT is reserved.
 *
 */
union cvmx_ciu_sum2_iox_int {
	uint64_t u64;
	struct cvmx_ciu_sum2_iox_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt. See BCH_GEN_INT. */
	uint64_t agl_drp                      : 1;  /**< AGL parket drop interrupt. Set any time AGL drops a packet. */
	uint64_t ocla                         : 1;  /**< OCLA interrupt summary. Value equals ((CIU_CIB_OCLA(0)_RAW & CIU_CIB_OCLA(0)_EN) != 0).
                                                         See CIU_CIB_OCLA(0)_RAW and CIU_CIB_OCLA(0)_EN. */
	uint64_t sata                         : 1;  /**< SATA interrupt summary. Value equals ((CIU_CIB_SATA(0)_RAW & CIU_CIB_SATA(0)_EN) != 0).
                                                         See CIU_CIB_SATA(0)_RAW and CIU_CIB_SATA(0)_EN. */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts, see ENDOR interrupt status
                                                         register ENDOR_RSTCLK_INTR0(1)_STATUS for details */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt, see EOI_INT_STA */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_sum2_iox_int_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_sum2_iox_int_cn61xx   cn66xx;
	struct cvmx_ciu_sum2_iox_int_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt. See BCH_GEN_INT. */
	uint64_t agl_drp                      : 1;  /**< AGL parket drop interrupt. Set any time AGL drops a packet. */
	uint64_t ocla                         : 1;  /**< OCLA interrupt summary. Value equals ((CIU_CIB_OCLA(0)_RAW & CIU_CIB_OCLA(0)_EN) != 0).
                                                         See CIU_CIB_OCLA(0)_RAW and CIU_CIB_OCLA(0)_EN. */
	uint64_t sata                         : 1;  /**< SATA interrupt summary. Value equals ((CIU_CIB_SATA(0)_RAW & CIU_CIB_SATA(0)_EN) != 0).
                                                         See CIU_CIB_SATA(0)_RAW and CIU_CIB_SATA(0)_EN. */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_sum2_iox_int_cn70xx   cn70xxp1;
	struct cvmx_ciu_sum2_iox_int_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts, see ENDOR interrupt status
                                                         register ENDOR_RSTCLK_INTR0(1)_STATUS for details */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt, see EOI_INT_STA */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_sum2_iox_int cvmx_ciu_sum2_iox_int_t;

/**
 * cvmx_ciu_sum2_pp#_ip2
 *
 * Only TIMER field may have different value per PP(IP).
 * All other fields  values  are idential for different PPs.
 */
union cvmx_ciu_sum2_ppx_ip2 {
	uint64_t u64;
	struct cvmx_ciu_sum2_ppx_ip2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt. See BCH_GEN_INT. */
	uint64_t agl_drp                      : 1;  /**< AGL parket drop interrupt. Set any time AGL drops a packet. */
	uint64_t ocla                         : 1;  /**< OCLA interrupt summary. Value equals ((CIU_CIB_OCLA(0)_RAW & CIU_CIB_OCLA(0)_EN) != 0).
                                                         See CIU_CIB_OCLA(0)_RAW and CIU_CIB_OCLA(0)_EN. */
	uint64_t sata                         : 1;  /**< SATA interrupt summary. Value equals ((CIU_CIB_SATA(0)_RAW & CIU_CIB_SATA(0)_EN) != 0).
                                                         See CIU_CIB_SATA(0)_RAW and CIU_CIB_SATA(0)_EN. */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts, see ENDOR interrupt status
                                                         register ENDOR_RSTCLK_INTR0(1)_STATUS for details */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt, see EOI_INT_STA */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_sum2_ppx_ip2_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_sum2_ppx_ip2_cn61xx   cn66xx;
	struct cvmx_ciu_sum2_ppx_ip2_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt. See BCH_GEN_INT. */
	uint64_t agl_drp                      : 1;  /**< AGL parket drop interrupt. Set any time AGL drops a packet. */
	uint64_t ocla                         : 1;  /**< OCLA interrupt summary. Value equals ((CIU_CIB_OCLA(0)_RAW & CIU_CIB_OCLA(0)_EN) != 0).
                                                         See CIU_CIB_OCLA(0)_RAW and CIU_CIB_OCLA(0)_EN. */
	uint64_t sata                         : 1;  /**< SATA interrupt summary. Value equals ((CIU_CIB_SATA(0)_RAW & CIU_CIB_SATA(0)_EN) != 0).
                                                         See CIU_CIB_SATA(0)_RAW and CIU_CIB_SATA(0)_EN. */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_sum2_ppx_ip2_cn70xx   cn70xxp1;
	struct cvmx_ciu_sum2_ppx_ip2_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts, see ENDOR interrupt status
                                                         register ENDOR_RSTCLK_INTR0(1)_STATUS for details */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt, see EOI_INT_STA */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_sum2_ppx_ip2 cvmx_ciu_sum2_ppx_ip2_t;

/**
 * cvmx_ciu_sum2_pp#_ip3
 *
 * Notes:
 * These SUM2 CSR's did not exist prior to pass 1.2. CIU_TIM4-9 did not exist prior to pass 1.2.
 *
 */
union cvmx_ciu_sum2_ppx_ip3 {
	uint64_t u64;
	struct cvmx_ciu_sum2_ppx_ip3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt. See BCH_GEN_INT. */
	uint64_t agl_drp                      : 1;  /**< AGL parket drop interrupt. Set any time AGL drops a packet. */
	uint64_t ocla                         : 1;  /**< OCLA interrupt summary. Value equals ((CIU_CIB_OCLA(0)_RAW & CIU_CIB_OCLA(0)_EN) != 0).
                                                         See CIU_CIB_OCLA(0)_RAW and CIU_CIB_OCLA(0)_EN. */
	uint64_t sata                         : 1;  /**< SATA interrupt summary. Value equals ((CIU_CIB_SATA(0)_RAW & CIU_CIB_SATA(0)_EN) != 0).
                                                         See CIU_CIB_SATA(0)_RAW and CIU_CIB_SATA(0)_EN. */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts, see ENDOR interrupt status
                                                         register ENDOR_RSTCLK_INTR0(1)_STATUS for details */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt, see EOI_INT_STA */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_sum2_ppx_ip3_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_sum2_ppx_ip3_cn61xx   cn66xx;
	struct cvmx_ciu_sum2_ppx_ip3_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt. See BCH_GEN_INT. */
	uint64_t agl_drp                      : 1;  /**< AGL parket drop interrupt. Set any time AGL drops a packet. */
	uint64_t ocla                         : 1;  /**< OCLA interrupt summary. Value equals ((CIU_CIB_OCLA(0)_RAW & CIU_CIB_OCLA(0)_EN) != 0).
                                                         See CIU_CIB_OCLA(0)_RAW and CIU_CIB_OCLA(0)_EN. */
	uint64_t sata                         : 1;  /**< SATA interrupt summary. Value equals ((CIU_CIB_SATA(0)_RAW & CIU_CIB_SATA(0)_EN) != 0).
                                                         See CIU_CIB_SATA(0)_RAW and CIU_CIB_SATA(0)_EN. */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_sum2_ppx_ip3_cn70xx   cn70xxp1;
	struct cvmx_ciu_sum2_ppx_ip3_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts, see ENDOR interrupt status
                                                         register ENDOR_RSTCLK_INTR0(1)_STATUS for details */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt, see EOI_INT_STA */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_sum2_ppx_ip3 cvmx_ciu_sum2_ppx_ip3_t;

/**
 * cvmx_ciu_sum2_pp#_ip4
 *
 * Notes:
 * These SUM2 CSR's did not exist prior to pass 1.2. CIU_TIM4-9 did not exist prior to pass 1.2.
 *
 */
union cvmx_ciu_sum2_ppx_ip4 {
	uint64_t u64;
	struct cvmx_ciu_sum2_ppx_ip4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt. See BCH_GEN_INT. */
	uint64_t agl_drp                      : 1;  /**< AGL parket drop interrupt. Set any time AGL drops a packet. */
	uint64_t ocla                         : 1;  /**< OCLA interrupt summary. Value equals ((CIU_CIB_OCLA(0)_RAW & CIU_CIB_OCLA(0)_EN) != 0).
                                                         See CIU_CIB_OCLA(0)_RAW and CIU_CIB_OCLA(0)_EN. */
	uint64_t sata                         : 1;  /**< SATA interrupt summary. Value equals ((CIU_CIB_SATA(0)_RAW & CIU_CIB_SATA(0)_EN) != 0).
                                                         See CIU_CIB_SATA(0)_RAW and CIU_CIB_SATA(0)_EN. */
	uint64_t reserved_15_15               : 1;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts, see ENDOR interrupt status
                                                         register ENDOR_RSTCLK_INTR0(1)_STATUS for details */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt, see EOI_INT_STA */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_15               : 1;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ciu_sum2_ppx_ip4_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_ciu_sum2_ppx_ip4_cn61xx   cn66xx;
	struct cvmx_ciu_sum2_ppx_ip4_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bch                          : 1;  /**< BCH interrupt. See BCH_GEN_INT. */
	uint64_t agl_drp                      : 1;  /**< AGL parket drop interrupt. Set any time AGL drops a packet. */
	uint64_t ocla                         : 1;  /**< OCLA interrupt summary. Value equals ((CIU_CIB_OCLA(0)_RAW & CIU_CIB_OCLA(0)_EN) != 0).
                                                         See CIU_CIB_OCLA(0)_RAW and CIU_CIB_OCLA(0)_EN. */
	uint64_t sata                         : 1;  /**< SATA interrupt summary. Value equals ((CIU_CIB_SATA(0)_RAW & CIU_CIB_SATA(0)_EN) != 0).
                                                         See CIU_CIB_SATA(0)_RAW and CIU_CIB_SATA(0)_EN. */
	uint64_t reserved_10_15               : 6;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_15               : 6;
	uint64_t sata                         : 1;
	uint64_t ocla                         : 1;
	uint64_t agl_drp                      : 1;
	uint64_t bch                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn70xx;
	struct cvmx_ciu_sum2_ppx_ip4_cn70xx   cn70xxp1;
	struct cvmx_ciu_sum2_ppx_ip4_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t endor                        : 2;  /**< ENDOR PHY interrupts, see ENDOR interrupt status
                                                         register ENDOR_RSTCLK_INTR0(1)_STATUS for details */
	uint64_t eoi                          : 1;  /**< EOI rsl interrupt, see EOI_INT_STA */
	uint64_t reserved_10_11               : 2;
	uint64_t timer                        : 6;  /**< General timer 4-9 interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 0, this interrupt is
                                                         common for all PP/IRQs, writing '1' to any PP/IRQ
                                                         will clear all TIMERx(x=0..9) interrupts.
                                                         When CIU_TIM_MULTI_CAST[EN] == 1, TIMERx(x=0..9)
                                                         are set at the same time, but clearing are based on
                                                         per cnMIPS core. See CIU_TIM_MULTI_CAST for detail.
                                                         The combination of this field and the
                                                         CIU_INT*_SUM0/4[TIMER] field implement all 10
                                                         CIU_TIM* interrupts. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t timer                        : 6;
	uint64_t reserved_10_11               : 2;
	uint64_t eoi                          : 1;
	uint64_t endor                        : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} cnf71xx;
};
typedef union cvmx_ciu_sum2_ppx_ip4 cvmx_ciu_sum2_ppx_ip4_t;

/**
 * cvmx_ciu_tim#
 *
 * Notes:
 * CIU_TIM4-9 did not exist prior to pass 1.2
 *
 */
union cvmx_ciu_timx {
	uint64_t u64;
	struct cvmx_ciu_timx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_37_63               : 27;
	uint64_t one_shot                     : 1;  /**< One-shot mode */
	uint64_t len                          : 36; /**< Timeout length in core clock cycles
                                                         Periodic interrupts will occur every LEN+1 core
                                                         clock cycles when ONE_SHOT==0
                                                         Timer disabled when LEN==0 */
#else
	uint64_t len                          : 36;
	uint64_t one_shot                     : 1;
	uint64_t reserved_37_63               : 27;
#endif
	} s;
	struct cvmx_ciu_timx_s                cn30xx;
	struct cvmx_ciu_timx_s                cn31xx;
	struct cvmx_ciu_timx_s                cn38xx;
	struct cvmx_ciu_timx_s                cn38xxp2;
	struct cvmx_ciu_timx_s                cn50xx;
	struct cvmx_ciu_timx_s                cn52xx;
	struct cvmx_ciu_timx_s                cn52xxp1;
	struct cvmx_ciu_timx_s                cn56xx;
	struct cvmx_ciu_timx_s                cn56xxp1;
	struct cvmx_ciu_timx_s                cn58xx;
	struct cvmx_ciu_timx_s                cn58xxp1;
	struct cvmx_ciu_timx_s                cn61xx;
	struct cvmx_ciu_timx_s                cn63xx;
	struct cvmx_ciu_timx_s                cn63xxp1;
	struct cvmx_ciu_timx_s                cn66xx;
	struct cvmx_ciu_timx_s                cn68xx;
	struct cvmx_ciu_timx_s                cn68xxp1;
	struct cvmx_ciu_timx_s                cn70xx;
	struct cvmx_ciu_timx_s                cn70xxp1;
	struct cvmx_ciu_timx_s                cnf71xx;
};
typedef union cvmx_ciu_timx cvmx_ciu_timx_t;

/**
 * cvmx_ciu_tim_multi_cast
 *
 * Notes:
 * This register does not exist prior to pass 1.2 silicon. Those earlier chip passes operate as if
 * EN==0.
 */
union cvmx_ciu_tim_multi_cast {
	uint64_t u64;
	struct cvmx_ciu_tim_multi_cast_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t en                           : 1;  /**< General Timer Interrupt Mutli-Cast mode:
                                                         - 0: Timer interrupt is common for all PP/IRQs.
                                                         - 1: Timer interrupts are set at the same time for
                                                            all PP/IRQs, but interrupt clearings can/need
                                                            to be done Individually based on per cnMIPS core.
                                                          Timer interrupts for IOs (X=32,33) will always use
                                                          common interrupts. Clear any of the I/O interrupts
                                                          will clear the common interrupt. */
#else
	uint64_t en                           : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_ciu_tim_multi_cast_s      cn61xx;
	struct cvmx_ciu_tim_multi_cast_s      cn66xx;
	struct cvmx_ciu_tim_multi_cast_s      cn70xx;
	struct cvmx_ciu_tim_multi_cast_s      cn70xxp1;
	struct cvmx_ciu_tim_multi_cast_s      cnf71xx;
};
typedef union cvmx_ciu_tim_multi_cast cvmx_ciu_tim_multi_cast_t;

/**
 * cvmx_ciu_wdog#
 */
union cvmx_ciu_wdogx {
	uint64_t u64;
	struct cvmx_ciu_wdogx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_46_63               : 18;
	uint64_t gstopen                      : 1;  /**< Global-stop enable. */
	uint64_t dstop                        : 1;  /**< Debug-stop enable. */
	uint64_t cnt                          : 24; /**< Number of 256-cycle coprocessor clock intervals until next watchdog expiration.
                                                         Cleared on write to associated CIU_PP_POKE() register. */
	uint64_t len                          : 16; /**< Watchdog time-expiration length. The most-significant 16 bits of a 24-bit decrementer that
                                                         decrements every 256-cycle coprocessor clock interval. Must be set > 0. */
	uint64_t state                        : 2;  /**< Watchdog state. The number of watchdog time expirations since last core poke. Cleared on
                                                         write to associated CIU_PP_POKE() register. */
	uint64_t mode                         : 2;  /**< Watchdog mode:
                                                         0x0 = Off.
                                                         0x1 = Interrupt only.
                                                         0x2 = Interrupt + NMI.
                                                         0x3 = Interrupt + NMI + soft reset. */
#else
	uint64_t mode                         : 2;
	uint64_t state                        : 2;
	uint64_t len                          : 16;
	uint64_t cnt                          : 24;
	uint64_t dstop                        : 1;
	uint64_t gstopen                      : 1;
	uint64_t reserved_46_63               : 18;
#endif
	} s;
	struct cvmx_ciu_wdogx_s               cn30xx;
	struct cvmx_ciu_wdogx_s               cn31xx;
	struct cvmx_ciu_wdogx_s               cn38xx;
	struct cvmx_ciu_wdogx_s               cn38xxp2;
	struct cvmx_ciu_wdogx_s               cn50xx;
	struct cvmx_ciu_wdogx_s               cn52xx;
	struct cvmx_ciu_wdogx_s               cn52xxp1;
	struct cvmx_ciu_wdogx_s               cn56xx;
	struct cvmx_ciu_wdogx_s               cn56xxp1;
	struct cvmx_ciu_wdogx_s               cn58xx;
	struct cvmx_ciu_wdogx_s               cn58xxp1;
	struct cvmx_ciu_wdogx_s               cn61xx;
	struct cvmx_ciu_wdogx_s               cn63xx;
	struct cvmx_ciu_wdogx_s               cn63xxp1;
	struct cvmx_ciu_wdogx_s               cn66xx;
	struct cvmx_ciu_wdogx_s               cn68xx;
	struct cvmx_ciu_wdogx_s               cn68xxp1;
	struct cvmx_ciu_wdogx_s               cn70xx;
	struct cvmx_ciu_wdogx_s               cn70xxp1;
	struct cvmx_ciu_wdogx_s               cn73xx;
	struct cvmx_ciu_wdogx_s               cn78xx;
	struct cvmx_ciu_wdogx_s               cn78xxp1;
	struct cvmx_ciu_wdogx_s               cnf71xx;
	struct cvmx_ciu_wdogx_s               cnf75xx;
};
typedef union cvmx_ciu_wdogx cvmx_ciu_wdogx_t;

#endif
