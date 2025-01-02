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
 * cvmx-usbdrdx-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon usbdrdx.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_USBDRDX_DEFS_H__
#define __CVMX_USBDRDX_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_CAPLENGTH(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_CAPLENGTH(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000000ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_CAPLENGTH(offset) (CVMX_ADD_IO_SEG(0x0001680000000000ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_CONFIG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_CONFIG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000058ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_CONFIG(offset) (CVMX_ADD_IO_SEG(0x0001680000000058ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_CRCR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_CRCR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000038ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_CRCR(offset) (CVMX_ADD_IO_SEG(0x0001680000000038ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_DALEPENA(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_DALEPENA(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C720ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_DALEPENA(offset) (CVMX_ADD_IO_SEG(0x000168000000C720ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_DBOFF(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_DBOFF(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000014ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_DBOFF(offset) (CVMX_ADD_IO_SEG(0x0001680000000014ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_DBX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 64)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 64)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 64)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_DBX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000480ull) + (((offset) & 127) + ((block_id) & 1) * 0x4000000000ull) * 4;
}
#else
#define CVMX_USBDRDX_UAHC_DBX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000480ull) + (((offset) & 127) + ((block_id) & 1) * 0x4000000000ull) * 4)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_DCBAAP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_DCBAAP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000050ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_DCBAAP(offset) (CVMX_ADD_IO_SEG(0x0001680000000050ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_DCFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_DCFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C700ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_DCFG(offset) (CVMX_ADD_IO_SEG(0x000168000000C700ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_DCTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_DCTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C704ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_DCTL(offset) (CVMX_ADD_IO_SEG(0x000168000000C704ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_DEPCMDPAR0_X(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 15)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 15)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 15)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_DEPCMDPAR0_X(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C808ull) + (((offset) & 15) + ((block_id) & 1) * 0x1000000000ull) * 16;
}
#else
#define CVMX_USBDRDX_UAHC_DEPCMDPAR0_X(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C808ull) + (((offset) & 15) + ((block_id) & 1) * 0x1000000000ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_DEPCMDPAR1_X(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 15)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 15)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 15)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_DEPCMDPAR1_X(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C804ull) + (((offset) & 15) + ((block_id) & 1) * 0x1000000000ull) * 16;
}
#else
#define CVMX_USBDRDX_UAHC_DEPCMDPAR1_X(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C804ull) + (((offset) & 15) + ((block_id) & 1) * 0x1000000000ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_DEPCMDPAR2_X(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 15)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 15)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 15)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_DEPCMDPAR2_X(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C800ull) + (((offset) & 15) + ((block_id) & 1) * 0x1000000000ull) * 16;
}
#else
#define CVMX_USBDRDX_UAHC_DEPCMDPAR2_X(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C800ull) + (((offset) & 15) + ((block_id) & 1) * 0x1000000000ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_DEPCMDX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 15)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 15)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 15)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_DEPCMDX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C80Cull) + (((offset) & 15) + ((block_id) & 1) * 0x1000000000ull) * 16;
}
#else
#define CVMX_USBDRDX_UAHC_DEPCMDX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C80Cull) + (((offset) & 15) + ((block_id) & 1) * 0x1000000000ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_DEVTEN(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_DEVTEN(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C708ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_DEVTEN(offset) (CVMX_ADD_IO_SEG(0x000168000000C708ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_DGCMD(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_DGCMD(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C714ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_DGCMD(offset) (CVMX_ADD_IO_SEG(0x000168000000C714ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_DGCMDPAR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_DGCMDPAR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C710ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_DGCMDPAR(offset) (CVMX_ADD_IO_SEG(0x000168000000C710ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_DNCTRL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_DNCTRL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000034ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_DNCTRL(offset) (CVMX_ADD_IO_SEG(0x0001680000000034ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_DSTS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_DSTS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C70Cull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_DSTS(offset) (CVMX_ADD_IO_SEG(0x000168000000C70Cull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_ERDPX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_ERDPX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000478ull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_ERDPX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000478ull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_ERSTBAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_ERSTBAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000470ull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_ERSTBAX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000470ull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_ERSTSZX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_ERSTSZX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000468ull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_ERSTSZX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000468ull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GBUSERRADDR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GBUSERRADDR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C130ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GBUSERRADDR(offset) (CVMX_ADD_IO_SEG(0x000168000000C130ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GCTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GCTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C110ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GCTL(offset) (CVMX_ADD_IO_SEG(0x000168000000C110ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GDBGBMU(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GDBGBMU(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C16Cull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GDBGBMU(offset) (CVMX_ADD_IO_SEG(0x000168000000C16Cull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GDBGEPINFO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GDBGEPINFO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C178ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GDBGEPINFO(offset) (CVMX_ADD_IO_SEG(0x000168000000C178ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GDBGFIFOSPACE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GDBGFIFOSPACE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C160ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GDBGFIFOSPACE(offset) (CVMX_ADD_IO_SEG(0x000168000000C160ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GDBGLNMCC(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GDBGLNMCC(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C168ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GDBGLNMCC(offset) (CVMX_ADD_IO_SEG(0x000168000000C168ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GDBGLSP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GDBGLSP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C174ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GDBGLSP(offset) (CVMX_ADD_IO_SEG(0x000168000000C174ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GDBGLSPMUX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GDBGLSPMUX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C170ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GDBGLSPMUX(offset) (CVMX_ADD_IO_SEG(0x000168000000C170ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GDBGLTSSM(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GDBGLTSSM(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C164ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GDBGLTSSM(offset) (CVMX_ADD_IO_SEG(0x000168000000C164ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GDMAHLRATIO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GDMAHLRATIO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C624ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GDMAHLRATIO(offset) (CVMX_ADD_IO_SEG(0x000168000000C624ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GEVNTADRX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GEVNTADRX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C400ull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GEVNTADRX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C400ull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GEVNTCOUNTX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GEVNTCOUNTX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C40Cull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GEVNTCOUNTX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C40Cull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GEVNTSIZX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GEVNTSIZX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C408ull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GEVNTSIZX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C408ull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GFLADJ(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GFLADJ(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C630ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GFLADJ(offset) (CVMX_ADD_IO_SEG(0x000168000000C630ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GGPIO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GGPIO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C124ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GGPIO(offset) (CVMX_ADD_IO_SEG(0x000168000000C124ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GHWPARAMS0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GHWPARAMS0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C140ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GHWPARAMS0(offset) (CVMX_ADD_IO_SEG(0x000168000000C140ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GHWPARAMS1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GHWPARAMS1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C144ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GHWPARAMS1(offset) (CVMX_ADD_IO_SEG(0x000168000000C144ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GHWPARAMS2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GHWPARAMS2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C148ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GHWPARAMS2(offset) (CVMX_ADD_IO_SEG(0x000168000000C148ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GHWPARAMS3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GHWPARAMS3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C14Cull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GHWPARAMS3(offset) (CVMX_ADD_IO_SEG(0x000168000000C14Cull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GHWPARAMS4(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GHWPARAMS4(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C150ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GHWPARAMS4(offset) (CVMX_ADD_IO_SEG(0x000168000000C150ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GHWPARAMS5(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GHWPARAMS5(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C154ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GHWPARAMS5(offset) (CVMX_ADD_IO_SEG(0x000168000000C154ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GHWPARAMS6(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GHWPARAMS6(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C158ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GHWPARAMS6(offset) (CVMX_ADD_IO_SEG(0x000168000000C158ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GHWPARAMS7(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GHWPARAMS7(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C15Cull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GHWPARAMS7(offset) (CVMX_ADD_IO_SEG(0x000168000000C15Cull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GHWPARAMS8(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GHWPARAMS8(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C600ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GHWPARAMS8(offset) (CVMX_ADD_IO_SEG(0x000168000000C600ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GPMSTS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GPMSTS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C114ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GPMSTS(offset) (CVMX_ADD_IO_SEG(0x000168000000C114ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GPRTBIMAP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GPRTBIMAP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C138ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GPRTBIMAP(offset) (CVMX_ADD_IO_SEG(0x000168000000C138ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GPRTBIMAP_FS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GPRTBIMAP_FS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C188ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GPRTBIMAP_FS(offset) (CVMX_ADD_IO_SEG(0x000168000000C188ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GPRTBIMAP_HS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GPRTBIMAP_HS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C180ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GPRTBIMAP_HS(offset) (CVMX_ADD_IO_SEG(0x000168000000C180ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GRLSID(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GRLSID(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C120ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GRLSID(offset) (CVMX_ADD_IO_SEG(0x000168000000C120ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GRXFIFOPRIHST(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GRXFIFOPRIHST(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C61Cull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GRXFIFOPRIHST(offset) (CVMX_ADD_IO_SEG(0x000168000000C61Cull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GRXFIFOSIZX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 2)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 2)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 2)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GRXFIFOSIZX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C380ull) + (((offset) & 3) + ((block_id) & 1) * 0x4000000000ull) * 4;
}
#else
#define CVMX_USBDRDX_UAHC_GRXFIFOSIZX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C380ull) + (((offset) & 3) + ((block_id) & 1) * 0x4000000000ull) * 4)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GRXTHRCFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GRXTHRCFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C10Cull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GRXTHRCFG(offset) (CVMX_ADD_IO_SEG(0x000168000000C10Cull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GSBUSCFG0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GSBUSCFG0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C100ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GSBUSCFG0(offset) (CVMX_ADD_IO_SEG(0x000168000000C100ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GSBUSCFG1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GSBUSCFG1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C104ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GSBUSCFG1(offset) (CVMX_ADD_IO_SEG(0x000168000000C104ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GSTS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GSTS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C118ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GSTS(offset) (CVMX_ADD_IO_SEG(0x000168000000C118ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GTXFIFOPRIDEV(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GTXFIFOPRIDEV(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C610ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GTXFIFOPRIDEV(offset) (CVMX_ADD_IO_SEG(0x000168000000C610ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GTXFIFOPRIHST(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GTXFIFOPRIHST(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C618ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GTXFIFOPRIHST(offset) (CVMX_ADD_IO_SEG(0x000168000000C618ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GTXFIFOSIZX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 3)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 3)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 3)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GTXFIFOSIZX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C300ull) + (((offset) & 3) + ((block_id) & 1) * 0x4000000000ull) * 4;
}
#else
#define CVMX_USBDRDX_UAHC_GTXFIFOSIZX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C300ull) + (((offset) & 3) + ((block_id) & 1) * 0x4000000000ull) * 4)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GTXTHRCFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GTXTHRCFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C108ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GTXTHRCFG(offset) (CVMX_ADD_IO_SEG(0x000168000000C108ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GUCTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GUCTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C12Cull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GUCTL(offset) (CVMX_ADD_IO_SEG(0x000168000000C12Cull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GUCTL1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GUCTL1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C11Cull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GUCTL1(offset) (CVMX_ADD_IO_SEG(0x000168000000C11Cull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GUID(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GUID(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C128ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GUID(offset) (CVMX_ADD_IO_SEG(0x000168000000C128ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GUSB2I2CCTLX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GUSB2I2CCTLX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C240ull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GUSB2I2CCTLX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C240ull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GUSB2PHYCFGX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GUSB2PHYCFGX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C200ull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GUSB2PHYCFGX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C200ull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_GUSB3PIPECTLX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_GUSB3PIPECTLX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C2C0ull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_GUSB3PIPECTLX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C2C0ull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_HCCPARAMS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_HCCPARAMS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000010ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_HCCPARAMS(offset) (CVMX_ADD_IO_SEG(0x0001680000000010ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_HCSPARAMS1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_HCSPARAMS1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000004ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_HCSPARAMS1(offset) (CVMX_ADD_IO_SEG(0x0001680000000004ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_HCSPARAMS2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_HCSPARAMS2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000008ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_HCSPARAMS2(offset) (CVMX_ADD_IO_SEG(0x0001680000000008ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_HCSPARAMS3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_HCSPARAMS3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000000Cull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_HCSPARAMS3(offset) (CVMX_ADD_IO_SEG(0x000168000000000Cull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_IMANX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_IMANX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000460ull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_IMANX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000460ull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_IMODX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_IMODX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000464ull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_IMODX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000464ull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_MFINDEX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_MFINDEX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000440ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_MFINDEX(offset) (CVMX_ADD_IO_SEG(0x0001680000000440ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_PAGESIZE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_PAGESIZE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000028ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_PAGESIZE(offset) (CVMX_ADD_IO_SEG(0x0001680000000028ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_PORTHLPMC_20X(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_PORTHLPMC_20X(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000042Cull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_PORTHLPMC_20X(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000042Cull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_PORTHLPMC_SSX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 1)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 1)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_PORTHLPMC_SSX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000043Cull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_PORTHLPMC_SSX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000043Cull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_PORTLI_20X(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_PORTLI_20X(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000428ull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_PORTLI_20X(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000428ull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_PORTLI_SSX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 1)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 1)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_PORTLI_SSX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000438ull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_PORTLI_SSX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000438ull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_PORTPMSC_20X(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_PORTPMSC_20X(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000424ull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_PORTPMSC_20X(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000424ull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_PORTPMSC_SSX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 1)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 1)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_PORTPMSC_SSX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000434ull) + ((block_id) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_PORTPMSC_SSX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000434ull) + ((block_id) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_PORTSCX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 1)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UAHC_PORTSCX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000420ull) + (((offset) & 1) + ((block_id) & 1) * 0x1000000000ull) * 16;
}
#else
#define CVMX_USBDRDX_UAHC_PORTSCX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000420ull) + (((offset) & 1) + ((block_id) & 1) * 0x1000000000ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_RTSOFF(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_RTSOFF(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000018ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_RTSOFF(offset) (CVMX_ADD_IO_SEG(0x0001680000000018ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_SUPTPRT2_DW0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_SUPTPRT2_DW0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000890ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_SUPTPRT2_DW0(offset) (CVMX_ADD_IO_SEG(0x0001680000000890ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_SUPTPRT2_DW1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_SUPTPRT2_DW1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000894ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_SUPTPRT2_DW1(offset) (CVMX_ADD_IO_SEG(0x0001680000000894ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_SUPTPRT2_DW2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_SUPTPRT2_DW2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000898ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_SUPTPRT2_DW2(offset) (CVMX_ADD_IO_SEG(0x0001680000000898ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_SUPTPRT2_DW3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_SUPTPRT2_DW3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000089Cull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_SUPTPRT2_DW3(offset) (CVMX_ADD_IO_SEG(0x000168000000089Cull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_SUPTPRT3_DW0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_SUPTPRT3_DW0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016800000008A0ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_SUPTPRT3_DW0(offset) (CVMX_ADD_IO_SEG(0x00016800000008A0ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_SUPTPRT3_DW1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_SUPTPRT3_DW1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016800000008A4ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_SUPTPRT3_DW1(offset) (CVMX_ADD_IO_SEG(0x00016800000008A4ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_SUPTPRT3_DW2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_SUPTPRT3_DW2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016800000008A8ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_SUPTPRT3_DW2(offset) (CVMX_ADD_IO_SEG(0x00016800000008A8ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_SUPTPRT3_DW3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_SUPTPRT3_DW3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016800000008ACull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_SUPTPRT3_DW3(offset) (CVMX_ADD_IO_SEG(0x00016800000008ACull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_USBCMD(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_USBCMD(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000020ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_USBCMD(offset) (CVMX_ADD_IO_SEG(0x0001680000000020ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_USBLEGCTLSTS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_USBLEGCTLSTS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000884ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_USBLEGCTLSTS(offset) (CVMX_ADD_IO_SEG(0x0001680000000884ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_USBLEGSUP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_USBLEGSUP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000880ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_USBLEGSUP(offset) (CVMX_ADD_IO_SEG(0x0001680000000880ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UAHC_USBSTS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UAHC_USBSTS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000024ull) + ((offset) & 1) * 0x10000000000ull;
}
#else
#define CVMX_USBDRDX_UAHC_USBSTS(offset) (CVMX_ADD_IO_SEG(0x0001680000000024ull) + ((offset) & 1) * 0x10000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UCTL_BIST_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UCTL_BIST_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180068000008ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_USBDRDX_UCTL_BIST_STATUS(offset) (CVMX_ADD_IO_SEG(0x0001180068000008ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UCTL_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UCTL_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180068000000ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_USBDRDX_UCTL_CTL(offset) (CVMX_ADD_IO_SEG(0x0001180068000000ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UCTL_ECC(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UCTL_ECC(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800680000F0ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_USBDRDX_UCTL_ECC(offset) (CVMX_ADD_IO_SEG(0x00011800680000F0ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UCTL_HOST_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UCTL_HOST_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800680000E0ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_USBDRDX_UCTL_HOST_CFG(offset) (CVMX_ADD_IO_SEG(0x00011800680000E0ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UCTL_INTSTAT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UCTL_INTSTAT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180068000030ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_USBDRDX_UCTL_INTSTAT(offset) (CVMX_ADD_IO_SEG(0x0001180068000030ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UCTL_PORTX_CFG_HS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UCTL_PORTX_CFG_HS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180068000040ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
#define CVMX_USBDRDX_UCTL_PORTX_CFG_HS(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180068000040ull) + ((block_id) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UCTL_PORTX_CFG_SS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UCTL_PORTX_CFG_SS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180068000048ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
#define CVMX_USBDRDX_UCTL_PORTX_CFG_SS(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180068000048ull) + ((block_id) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UCTL_PORTX_CR_DBG_CFG(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UCTL_PORTX_CR_DBG_CFG(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180068000050ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
#define CVMX_USBDRDX_UCTL_PORTX_CR_DBG_CFG(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180068000050ull) + ((block_id) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UCTL_PORTX_CR_DBG_STATUS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_USBDRDX_UCTL_PORTX_CR_DBG_STATUS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180068000058ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
#define CVMX_USBDRDX_UCTL_PORTX_CR_DBG_STATUS(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180068000058ull) + ((block_id) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UCTL_SHIM_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UCTL_SHIM_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800680000E8ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_USBDRDX_UCTL_SHIM_CFG(offset) (CVMX_ADD_IO_SEG(0x00011800680000E8ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UCTL_SPARE0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_USBDRDX_UCTL_SPARE0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180068000010ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_USBDRDX_UCTL_SPARE0(offset) (CVMX_ADD_IO_SEG(0x0001180068000010ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UCTL_SPARE0_ECO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UCTL_SPARE0_ECO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180068000010ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_USBDRDX_UCTL_SPARE0_ECO(offset) (CVMX_ADD_IO_SEG(0x0001180068000010ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UCTL_SPARE1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_USBDRDX_UCTL_SPARE1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800680000F8ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_USBDRDX_UCTL_SPARE1(offset) (CVMX_ADD_IO_SEG(0x00011800680000F8ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_USBDRDX_UCTL_SPARE1_ECO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_USBDRDX_UCTL_SPARE1_ECO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800680000F8ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_USBDRDX_UCTL_SPARE1_ECO(offset) (CVMX_ADD_IO_SEG(0x00011800680000F8ull) + ((offset) & 1) * 0x1000000ull)
#endif

/**
 * cvmx_usbdrd#_uahc_caplength
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.3.1.
 *
 */
union cvmx_usbdrdx_uahc_caplength {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_caplength_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t hciversion                   : 16; /**< Host controller interface version number. */
	uint32_t reserved_8_15                : 8;
	uint32_t caplength                    : 8;  /**< Capability registers length. */
#else
	uint32_t caplength                    : 8;
	uint32_t reserved_8_15                : 8;
	uint32_t hciversion                   : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_caplength_s  cn70xx;
	struct cvmx_usbdrdx_uahc_caplength_s  cn70xxp1;
	struct cvmx_usbdrdx_uahc_caplength_s  cn73xx;
	struct cvmx_usbdrdx_uahc_caplength_s  cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_caplength cvmx_usbdrdx_uahc_caplength_t;

/**
 * cvmx_usbdrd#_uahc_config
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.7.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_config {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_config_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_8_31                : 24;
	uint32_t maxslotsen                   : 8;  /**< Maximum device slots enabled. */
#else
	uint32_t maxslotsen                   : 8;
	uint32_t reserved_8_31                : 24;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_config_s     cn70xx;
	struct cvmx_usbdrdx_uahc_config_s     cn70xxp1;
	struct cvmx_usbdrdx_uahc_config_s     cn73xx;
	struct cvmx_usbdrdx_uahc_config_s     cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_config cvmx_usbdrdx_uahc_config_t;

/**
 * cvmx_usbdrd#_uahc_crcr
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.5.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_crcr {
	uint64_t u64;
	struct cvmx_usbdrdx_uahc_crcr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cmd_ring_ptr                 : 58; /**< Command ring pointer. */
	uint64_t reserved_4_5                 : 2;
	uint64_t crr                          : 1;  /**< Command ring running. */
	uint64_t ca                           : 1;  /**< Command abort. */
	uint64_t cs                           : 1;  /**< Command stop. */
	uint64_t rcs                          : 1;  /**< Ring cycle state. */
#else
	uint64_t rcs                          : 1;
	uint64_t cs                           : 1;
	uint64_t ca                           : 1;
	uint64_t crr                          : 1;
	uint64_t reserved_4_5                 : 2;
	uint64_t cmd_ring_ptr                 : 58;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_crcr_s       cn70xx;
	struct cvmx_usbdrdx_uahc_crcr_s       cn70xxp1;
	struct cvmx_usbdrdx_uahc_crcr_s       cn73xx;
	struct cvmx_usbdrdx_uahc_crcr_s       cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_crcr cvmx_usbdrdx_uahc_crcr_t;

/**
 * cvmx_usbdrd#_uahc_dalepena
 *
 * This register indicates whether a USB endpoint is active in a given configuration or
 * interface.
 *
 * This register can be reset by IOI reset or USBDRD()_UCTL_CTL[UAHC_RST] or
 * USBDRD()_UAHC_GCTL[CORESOFTRESET] or
 * USBDRD()_UAHC_USBCMD[HCRST] or USBDRD()_UAHC_USBCMD[LHCRST] or
 * USBDRD()_UAHC_DCTL[CSFTRST].
 */
union cvmx_usbdrdx_uahc_dalepena {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_dalepena_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t usbactep                     : 32; /**< This field indicates if a USB endpoint is active in the current configuration
                                                         and interface. It applies to USB IN endpoints 0-15 and OUT endpoints 0-15,
                                                         with one bit for each of the 32 possible endpoints. Even numbers are for
                                                         USB OUT endpoints, and odd numbers are for USB IN endpoints, as
                                                         follows:
                                                          <0> = USB EP0-OUT.
                                                          <1> = USB EP0-IN.
                                                          <2> = USB EP1-OUT.
                                                          <3> = USB EP1-IN.
                                                         The entity programming this register must set bits 0 and 1 because they
                                                         enable control endpoints that map to physical endpoints (resources) after
                                                         USBReset.
                                                         Application software clears these bits for all endpoints (other than EP0-OUT
                                                         and EP0-IN) after detecting a USB reset. After receiving SetConfiguration
                                                         and SetInterface requests, the application must program endpoint registers
                                                         accordingly and set these bits. */
#else
	uint32_t usbactep                     : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_dalepena_s   cn70xx;
	struct cvmx_usbdrdx_uahc_dalepena_s   cn70xxp1;
	struct cvmx_usbdrdx_uahc_dalepena_s   cn73xx;
	struct cvmx_usbdrdx_uahc_dalepena_s   cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_dalepena cvmx_usbdrdx_uahc_dalepena_t;

/**
 * cvmx_usbdrd#_uahc_db#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.6.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_dbx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_dbx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dbstreamid                   : 16; /**< Doorbell stream ID. */
	uint32_t reserved_8_15                : 8;
	uint32_t dbtarget                     : 8;  /**< Doorbell target. */
#else
	uint32_t dbtarget                     : 8;
	uint32_t reserved_8_15                : 8;
	uint32_t dbstreamid                   : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_dbx_s        cn70xx;
	struct cvmx_usbdrdx_uahc_dbx_s        cn70xxp1;
	struct cvmx_usbdrdx_uahc_dbx_s        cn73xx;
	struct cvmx_usbdrdx_uahc_dbx_s        cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_dbx cvmx_usbdrdx_uahc_dbx_t;

/**
 * cvmx_usbdrd#_uahc_dboff
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.3.7.
 *
 */
union cvmx_usbdrdx_uahc_dboff {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_dboff_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dboff                        : 30; /**< Doorbell array offset. */
	uint32_t reserved_0_1                 : 2;
#else
	uint32_t reserved_0_1                 : 2;
	uint32_t dboff                        : 30;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_dboff_s      cn70xx;
	struct cvmx_usbdrdx_uahc_dboff_s      cn70xxp1;
	struct cvmx_usbdrdx_uahc_dboff_s      cn73xx;
	struct cvmx_usbdrdx_uahc_dboff_s      cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_dboff cvmx_usbdrdx_uahc_dboff_t;

/**
 * cvmx_usbdrd#_uahc_dcbaap
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.6.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_dcbaap {
	uint64_t u64;
	struct cvmx_usbdrdx_uahc_dcbaap_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dcbaap                       : 58; /**< Device context base address array pointer. */
	uint64_t reserved_0_5                 : 6;
#else
	uint64_t reserved_0_5                 : 6;
	uint64_t dcbaap                       : 58;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_dcbaap_s     cn70xx;
	struct cvmx_usbdrdx_uahc_dcbaap_s     cn70xxp1;
	struct cvmx_usbdrdx_uahc_dcbaap_s     cn73xx;
	struct cvmx_usbdrdx_uahc_dcbaap_s     cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_dcbaap cvmx_usbdrdx_uahc_dcbaap_t;

/**
 * cvmx_usbdrd#_uahc_dcfg
 *
 * This register configures the core in device mode after power-on or after certain control
 * commands or enumeration. Do not make changes to this register after initial programming.
 *
 * This register can be reset by IOI reset or USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_dcfg {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_dcfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t ignorestreampp               : 1;  /**< This bit only affects stream-capable bulk endpoints.
                                                         When this bit is set to 0x0 and the controller receives a data packet with the
                                                         packet pending (PP) bit set to 0 for OUT endpoints, or it receives an ACK
                                                         with the NumP field set to 0 and PP set to 0 for IN endpoints, the core
                                                         attempts to search for another stream (CStream) to initiate to the host.
                                                         However, there are two situations where this behavior is not optimal:
                                                         * When the host is setting PP=0 even though it has not finished the
                                                         stream, or.
                                                         * When the endpoint on the device is configured with one transfer
                                                         resource and therefore does not have any other streams to initiate to the
                                                         host.
                                                         When this bit is set to 0x1, the core ignores the packet pending bit for the
                                                         purposes of stream selection and does not search for another stream when
                                                         it receives DP(PP=0) or ACK(NumP=0, PP=0). This can enhance the
                                                         performance when the device system bus bandwidth is low */
	uint32_t lpmcap                       : 1;  /**< LPM capable.
                                                         The application uses this bit to control the controller's core LPM
                                                         capabilities. If the core operates as a non-LPM-capable device, it cannot
                                                         respond to LPM transactions.
                                                           0x0 = LPM capability is not enabled.
                                                           0x1 = LPM capability is enabled. */
	uint32_t nump                         : 5;  /**< Number of receive buffers.
                                                         This bit indicates the number of receive buffers to be reported in the ACK
                                                         TP.
                                                         The DWC_usb3 controller uses this field if USBDRD()_UAHC_GRXTHRCFG[USBRXPKTCNTSEL]
                                                         is set to 0x0. The application can program this value based on RxFIFO size,
                                                         buffer sizes programmed in descriptors, and system latency.
                                                         For an OUT endpoint, this field controls the number of receive buffers
                                                         reported in the NumP field of the ACK TP transmitted by the core. */
	uint32_t intrnum                      : 5;  /**< Interrupt number.
                                                         Indicates interrupt/EventQ number on which non-endpoint-specific device-related
                                                         interrupts (see DEVT) are generated. */
	uint32_t reserved_10_11               : 2;
	uint32_t devaddr                      : 7;  /**< Device address.
                                                         The application must perform the following:
                                                          * Program this field after every SetAddress request.
                                                          * Reset this field to zero after USB reset. */
	uint32_t devspd                       : 3;  /**< Device speed.
                                                         Indicates the speed at which the application requires the core to connect, or
                                                         the maximum speed the application can support. However, the actual bus
                                                         speed is determined only after the chirp sequence is completed, and is
                                                         based on the speed of the USB host to which the core is connected.
                                                           0x0 = High-speed (USB 2.0 PHY clock is 30 MHz or 60 MHz).
                                                           0x1 = Full-speed (USB 2.0 PHY clock is 30 MHz or 60 MHz).
                                                           0x4 = SuperSpeed (USB 3.0 PHY clock is 125 MHz or 250 MHz). */
#else
	uint32_t devspd                       : 3;
	uint32_t devaddr                      : 7;
	uint32_t reserved_10_11               : 2;
	uint32_t intrnum                      : 5;
	uint32_t nump                         : 5;
	uint32_t lpmcap                       : 1;
	uint32_t ignorestreampp               : 1;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_dcfg_s       cn70xx;
	struct cvmx_usbdrdx_uahc_dcfg_s       cn70xxp1;
	struct cvmx_usbdrdx_uahc_dcfg_s       cn73xx;
	struct cvmx_usbdrdx_uahc_dcfg_s       cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_dcfg cvmx_usbdrdx_uahc_dcfg_t;

/**
 * cvmx_usbdrd#_uahc_dctl
 *
 * This register controls devlice mode.
 *
 * This register can be reset by IOI reset or USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_dctl {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_dctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t rs                           : 1;  /**< Run/Stop.
                                                         The software writes 1 to this bit to start the device controller operation.
                                                         To stop the device controller operation, the software must remove any active
                                                         transfers and write 0 to this bit. When the controller is stopped, it sets the
                                                         USBDRD()_UAHC_DSTS[DEVCTRLHLT] bit when the core is idle and the lower layer finishes
                                                         the disconnect process.
                                                         The Run/Stop bit must be used in following cases as specified:
                                                            1. After power-on reset and CSR initialization, the software must write 1 to this bit
                                                            to start the device controller. The controller does not signal connect to the host
                                                            until this bit is set.
                                                            2. The software uses this bit to control the device controller to perform a soft
                                                            disconnect. When the software writes 0 to this bit, the host does not see that
                                                            the device is connected. The device controller stays in the disconnected state
                                                            until the software writes 1 to this bit. The minimum duration of keeping this bit
                                                            cleared is 30ms in SuperSpeed and 10ms in High/Full/LowSpeed.
                                                            If the software attempts a connect after the soft disconnect or detects a
                                                            disconnect event, it must set USBDRD()_UAHC_DCTL[ULSTCHNGREQ] to
                                                            "Rx.Detect" before reasserting the Run/Stop bit. */
	uint32_t csftrst                      : 1;  /**< Core soft reset.
                                                         Resets the all clock domains as follows:
                                                         * Clears the interrupts and all the CSRs except the following registers:
                                                           GCTL, GUCTL, GSTS, GSNPSID, GGPIO, GUID, GUSB2PHYCFGn registers,
                                                           GUSB3PIPECTLn registers, DCFG, DCTL, DEVTEN, DSTS.
                                                         * All module state machines (except the SoC Bus Slave Unit) are reset to the
                                                            IDLE state, and all the TxFIFOs and the RxFIFO are flushed.
                                                         * Any transactions on the SoC bus Master are terminated as soon as possible,
                                                           after gracefully completing the last data phase of a SoC bus transfer. Any
                                                           transactions on the USB are terminated immediately.
                                                         The application can write this bit at any time to reset the core. This is a self-clearing
                                                         bit; the core clears this bit after all necessary logic is reset in the core,
                                                         which may take several clocks depending on the corefs current state. Once this
                                                         bit is cleared, the software must wait at least 3 PHY clocks before accessing the
                                                         PHY domain (synchronization delay). Typically, software reset is used during
                                                         software development and also when you dynamically change the PHY selection
                                                         bits in the USB configuration registers listed above. When you change the PHY,
                                                         the corresponding clock for the PHY is selected and used in the PHY domain.
                                                         Once a new clock is selected, the PHY domain must be reset for proper
                                                         operation. */
	uint32_t reserved_29_29               : 1;
	uint32_t hird_thres                   : 5;  /**< HIRD threshold.
                                                         The core asserts output signals utmi_l1_suspend_n and utmi_sleep_n on the basis of this
                                                         signal:
                                                         * The core asserts utmi_l1_suspend_n to put the PHY into Deep Low-Power
                                                           mode in L1 when both of the following are true:
                                                           - HIRD value is greater than or equal to the value in HIRD_Thres[3:0]
                                                           - HIRD_Thres[4] is set to 1'b1.
                                                         * The core asserts utmi_sleep_n on L1 when one of the following is true:
                                                           - If the HIRD value is less than HIRD_Thres[3:0] or
                                                           - HIRD_Thres[4] is set to 1'b0. */
	uint32_t appl1res                     : 1;  /**< LPM response programmed by application.
                                                         Handshake response to LPM token specified by device application. Response
                                                         depends on USBDRD()_UAHC_DCFG[LPMCAP].
                                                          LPMCAP is 0x0 - The core always responds with timeout (that is, no
                                                          response).
                                                          LPMCAP is 0x1 and this bit is 0:
                                                          The core responds with an ACK upon a successful LPM transaction,
                                                              which requires all of the following are satisfied:
                                                              * There are no PID/CRC5 errors in both the EXT token and the LPM token
                                                              (if not true, inactivity results in a timeout ERROR).
                                                              * A valid bLinkState = 0001B (L1) is received in the LPM transaction (else
                                                              STALL).
                                                              * No data is pending in the Transmit FIFO and OUT endpoints not in flow
                                                              controlled state (else NYET).
                                                          LPMCAP is 0x1 and this bit is 1:
                                                         The core responds with an ACK upon a successful LPM, independent
                                                             of transmit FIFO status and OUT endpoint flow control state. The LPM
                                                             transaction is successful if all of the following are satisfied:
                                                             * There are no PID/CRC5 errors in both the EXT token and the LPM token
                                                             (else ERROR).
                                                             * A valid bLinkState = 0001B (L1) is received in the LPM transaction (else
                                                             STALL). */
	uint32_t reserved_20_22               : 3;
	uint32_t keepconnect                  : 1;  /**< Always write 0. */
	uint32_t l1hibernationen              : 1;  /**< Always write 0. */
	uint32_t crs                          : 1;  /**< Controller restore state.
                                                         This command is similar to the USBDRD()_UAHC_USBCMD[CRS] bit in host mode and
                                                         initiates the restore process. When software sets this bit to 1, the controller
                                                         immediately sets USBDRD()_UAHC_DSTS[RSS] to 1. When the controller has finished
                                                         the restore process, it sets USBDRD()_UAHC_DSTS[RSS] to 0.
                                                         Note: When read, this field always returns 0. */
	uint32_t css                          : 1;  /**< Controller save state.
                                                         This command is similar to the USBDRD()_UAHC_USBCMD[CSS] bit in host mode and
                                                         initiates the restore process. When software sets this bit to 1, the controller
                                                         immediately sets USBDRD()_UAHC_DSTS[SSS] to 1. When the controller has finished
                                                         the save process, it sets USBDRD()_UAHC_DSTS[SSS] to 0.
                                                         Note: When read, this field always returns 0. */
	uint32_t reserved_13_15               : 3;
	uint32_t initu2ena                    : 1;  /**< Initiate U2 enable.
                                                          0 = May not initiate U2 (default).
                                                          1 = May initiate U2.
                                                         On USB reset, hardware clears this bit to 0. Software sets this bit after receiving
                                                         SetFeature(U2_ENABLE), and clears this bit when ClearFeature(U2_ENABLE) is
                                                         received.
                                                         If USBDRD()_UAHC_DCTL[ACCEPTU2ENA] is 0, the link immediately exits U2 state. */
	uint32_t acceptu2ena                  : 1;  /**< Accept U2 enable.
                                                          0 = Reject U2 except when Force_LinkPM_Accept bit is set (default).
                                                          1 = Core accepts transition to U2 state if nothing is pending on the
                                                             application side.
                                                         On USB reset, hardware clears this bit to 0. Software sets this bit after receiving
                                                         a SetConfiguration command. */
	uint32_t initu1ena                    : 1;  /**< Initiate U1 enable.
                                                          0 = May not initiate U1 (default).
                                                          1 = May initiate U1.
                                                         On USB reset, hardware clears this bit to 0. Software sets this bit after receiving
                                                         SetFeature(U1_ENABLE), and clears this bit when ClearFeature(U1_ENABLE) is
                                                         received.
                                                         If USBDRD()_UAHC_DCTL[ACCEPTU1ENA] is 0, the link immediately exits U1 state. */
	uint32_t acceptu1ena                  : 1;  /**< Accept U1 enable.
                                                          0 = Reject U1 except when Force_LinkPM_Accept bit is set (default)
                                                          1 = Core accepts transition to U1 state if nothing is pending on the
                                                             application side.
                                                         On USB reset, hardware clears this bit to 0. Software sets this bit after receiving
                                                         a SetConfiguration command. */
	uint32_t ulstchngreq                  : 4;  /**< USB/link state change request.
                                                         Software writes this field to issue a USB/link state change request. A change in
                                                         this field indicates a new request to the core. If software wants to issue the same
                                                         request back-to-back, it must write a 0 to this field between the two requests. The
                                                         result of the state change request is reflected in USBDRD()_UAHC_DSTS[USBLNKST].
                                                         These bits are self-cleared on the MAC Layer exiting suspended state.
                                                         If software is updating other fields of the USBDRD()_UAHC_DCTL register and not
                                                         intending to force any link state change, then it must write a 0 to this field.
                                                         SuperSpeed compliance mode is normally entered and controlled by the remote link
                                                         partner. Refer to the USB3 specification. Alternatively, you can force the local link
                                                         directly into Compliance mode, by resetting the SuperSpeed link with the
                                                         USBDRD()_UAHC_DCTL[RS] bit set to zero. If you then write 0xA to the ULSTCHNGREQ
                                                         field and 1 to USBDRD()_UAHC_DCTL[RS], the Link will go to compliance. Once you
                                                         are in compliance, you may alternately write 0x0 and 0xA to this field to advance
                                                         the compliance pattern.
                                                         In SS mode:
                                                             0x0 = No Action.
                                                             0x4 = SS.Disabled.
                                                             0x5 = Rx.Detect.
                                                             0x6 = SS.Inactive.
                                                             0x8 = Recovery.
                                                             0xA = Compliance.
                                                             Others = Reserved.
                                                         In HS/FS/LS mode:
                                                             0x8 = Remote wakeup request.
                                                             The remote wakeup request should be issued 2us after the device goes into
                                                             suspend state (USBDRD()_UAHC_DSTS[USBLNKST] is 0x3).
                                                             Others = Reserved. */
	uint32_t tstctl                       : 4;  /**< Test control.
                                                         0x0 = Test mode disabled.
                                                         0x1 = Test_J mode.
                                                         0x2 = Test_K mode.
                                                         0x3 = Test_SE0_NAK mode.
                                                         0x4 = Test_Packet mode.
                                                         0x5 = Test_Force_Enable.
                                                         Others = Reserved. */
	uint32_t reserved_0_0                 : 1;
#else
	uint32_t reserved_0_0                 : 1;
	uint32_t tstctl                       : 4;
	uint32_t ulstchngreq                  : 4;
	uint32_t acceptu1ena                  : 1;
	uint32_t initu1ena                    : 1;
	uint32_t acceptu2ena                  : 1;
	uint32_t initu2ena                    : 1;
	uint32_t reserved_13_15               : 3;
	uint32_t css                          : 1;
	uint32_t crs                          : 1;
	uint32_t l1hibernationen              : 1;
	uint32_t keepconnect                  : 1;
	uint32_t reserved_20_22               : 3;
	uint32_t appl1res                     : 1;
	uint32_t hird_thres                   : 5;
	uint32_t reserved_29_29               : 1;
	uint32_t csftrst                      : 1;
	uint32_t rs                           : 1;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_dctl_s       cn70xx;
	struct cvmx_usbdrdx_uahc_dctl_s       cn70xxp1;
	struct cvmx_usbdrdx_uahc_dctl_s       cn73xx;
	struct cvmx_usbdrdx_uahc_dctl_s       cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_dctl cvmx_usbdrdx_uahc_dctl_t;

/**
 * cvmx_usbdrd#_uahc_depcmd#
 *
 * This register enables software to issue physical endpoint-specific commands. This register
 * contains command, control, and status fields relevant to the current generic command,
 * while the USBDRD()_UAHC_DEPCMDPAR* registers provide command parameters and return
 * status information.
 *
 * Several fields (including CMDTYPE) are write-only, so their read values are undefined. After
 * power-on, prior to issuing the first endpoint command, the read value of this register is
 * undefined. In particular, [CMDACT] may be set after power-on. In this case, it is safe
 * to issue an endpoint command.
 *
 * This register can be reset by IOI reset or USBDRD()_UCTL_CTL[UAHC_RST] or
 * USBDRD()_UAHC_GCTL[CORESOFTRESET] or
 * USBDRD()_UAHC_USBCMD[HCRST] or USBDRD()_UAHC_USBCMD[LHCRST] or
 * USBDRD()_UAHC_DCTL[CSFTRST].
 */
union cvmx_usbdrdx_uahc_depcmdx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_depcmdx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t commandparam                 : 16; /**< Command or event parameters.
                                                         When this register is written:
                                                           Command parameters:
                                                             For start transfer command:
                                                             _  - [31:16]: StreamID. The USB StreamID assigned to this transfer
                                                             For start transfer command applied to an isochronous endpoint:
                                                             _  - [31:16]: StartMicroFramNum: Indicates the (micro)frame number to
                                                               which the first TRB applies
                                                             For update transfer, end transfer, and start new configuration
                                                             commands:
                                                             _  - [22:16]: Transfer resource index (XferRscIdx). The hardware-assigned
                                                               transfer resource index for the transfer, which was returned
                                                               in response to the start transfer command. The application
                                                               software-assigned transfer resource index for a start new
                                                               configuration command.
                                                         When this register is read:
                                                             For XferNotReady, XferComplete, and stream events on bulk endpoints:
                                                             _  - [31:16]: StreamID. Applies only to bulk endpoints that support streams. This
                                                                          indicates the StreamID of the transfer for which the event is
                                                                          generated
                                                             For XferInProgress:
                                                             _  - [31:16]: Isochronous microframe number (IsocMicroFrameNum): Indicates the
                                                                          microframe number of the beginning of the interval that generated
                                                                          the XferInProgress event (debug purposes only)
                                                             For XferNotReady events on Isochronous Endpoints:
                                                             _  - [31:16]: Isochronous microframe number (IsocMicroFrameNum). Indicates the
                                                                          microframe number during which the endpoint was not ready
                                                               Note: controller core represents USB bus time as a 14-bit value on the bus and also
                                                               in the DSTS register (USBDRD()_UAHC_DSTS[SOFFN]), but as a 16-bit value in the
                                                               XferNotReady event. Use the 16-bit value to interact with isochronous endpoints via
                                                               the StartXfer command. The extra two bits that the controller core produces will be
                                                               necessary for handling wrap-around conditions in the interaction between software
                                                               and hardware.
                                                            For all EPCmdCmplt events
                                                            _  - [27:24]: Command type. The command type that completed (Valid only in a DEPEVT
                                                                            event. Undefined when read from the
                                                                            USBDRD()_UAHC_DEPCMD()[COMMANDPARAM] field).
                                                            For EPCmdCmplt event in response to start transfer command:
                                                            _  - [22:16]: Transfer resource index (XferRscIdx). The internal hardware transfer
                                                                            resource index assigned to this transfer. This index must be used in
                                                                            all update transfer and end transfer commands. */
	uint32_t cmdstatus                    : 4;  /**< Command completion status.
                                                         Additional information about the completion of this command is available in
                                                         this field.
                                                         Within an XferNotReady event:
                                                         _ [15]: Indicates the reason why the XferNotReady event is generated:
                                                         _ [15] = 0: XferNotActive: Host initiated a transfer, but the requested transfer is not
                                                               present in the hardware.
                                                         _ [15] = 1: XferActive: Host initiated a transfer, the transfer is present, but no valid
                                                         TRBs
                                                               are available
                                                         _ [14]: Not Used
                                                         _ [13:12]: For control endpoints, indicates what stage was requested when the transfer was
                                                           not ready:
                                                         _ [13:12] = 0x1: Control data request
                                                         _ [13:12] = 0x2: Control status request
                                                         Within an XferComplete or XferInProgress event:
                                                         _ [15]: LST bit of the completed TRB (XferComplete only)
                                                         _ [15]: MissedIsoc: Indicates the interval did not complete successfully (XferInProgress
                                                           only)
                                                         _ [14]: IOC bit of the TRB that completed.
                                                         _ [13]: Indicates the TRB completed with a short packet reception or the last packet of an
                                                           isochronous interval
                                                         _ [12]: Reserved.
                                                           If the host aborts the data stage of a control transfer, software may receive a
                                                           XferComplete event with the EventStatus field equal to 0. This is a valid event
                                                           that must be processed as a part of the control transfer programming model.
                                                         Within a stream event:
                                                         _ [15:12] = 0x2: StreamNotFound: This stream event is issued when the stream-capable
                                                         endpoint
                                                               performed a search in its transfer resource cache, but could not find an active
                                                               and ready stream.
                                                         _ [15:12] = 0x1: StreamFound: This stream event is issued when the stream-capable endpoint
                                                         found
                                                               an active and ready stream in its transfer resource cache, and initiated traffic for
                                                               that stream to the host. The ID of the selected Stream is in the EventParam field.
                                                         In response to a Start Transfer command:
                                                         _ [15:12] = 0x2: Indicates expiry of the bus time reflected in the Start Transfer command.
                                                         _ [15:12] = 0x1: Indicates there is no transfer resource available on the endpoint.
                                                         In response to a Set Transfer Resource (DEPXFERCFG) command:
                                                         _ [15:12] = 0x1: Indicates an error has occurred because software is requesting more
                                                         transfer
                                                               resources to be assigned than have been configured in the hardware.
                                                         In response to a end transfer command:
                                                         _ [15:12] = 0x1: Indicates an invalid transfer resource was specified. */
	uint32_t hipri_forcerm                : 1;  /**< HighPriority: Only valid for start transfer command.
                                                         ForceRM: Only valid for end transfer command. */
	uint32_t cmdact                       : 1;  /**< Software sets this bit to 1 to enable the device endpoint controller to
                                                         execute the generic command.
                                                         The device controller sets this bit to 0 when [CMDSTATUS] is valid and
                                                         the endpoint is ready to accept another command. This does not imply that
                                                         all the effects of the previously-issued command have taken place. */
	uint32_t reserved_9_9                 : 1;
	uint32_t cmdioc                       : 1;  /**< Command interrupt on complete.
                                                         When this bit is set, the device controller issues a generic endpoint
                                                         command complete event after executing the command. Note that this
                                                         interrupt is mapped to DEPCFG.IntrNum. When the DEPCFG command is
                                                         executed, the command interrupt on completion goes to the interrupt
                                                         pointed by the USBDRD()_UAHC_DCFG[INTRNUM] in the current command.
                                                         Note: This field must not set to 1 if the USBDRD()_UAHC_DCTL[RS] field is 0. */
	uint32_t reserved_4_7                 : 4;
	uint32_t cmdtyp                       : 4;  /**< Command type.
                                                         Specifies the type of command the software driver is requesting the core to
                                                         perform.
                                                         0x0 = Reserved.
                                                         0x1 = Set endpoint configuration (64 or 96-bit parameter).
                                                         0x2 = Set endpoint transfer resource configuration (32-bit parameter).
                                                         0x3 = Get endpoint state (no parameter needed).
                                                         0x4 = Set stall (no parameter needed).
                                                         0x5 = Clear stall (see set stall, no parameter needed).
                                                         0x6 = Start transfer (64-bit parameter).
                                                         0x7 = Update transfer (no parameter needed).
                                                         0x8 = End transfer (no parameter needed).
                                                         0x9 = Start new configuration (no parameter needed). */
#else
	uint32_t cmdtyp                       : 4;
	uint32_t reserved_4_7                 : 4;
	uint32_t cmdioc                       : 1;
	uint32_t reserved_9_9                 : 1;
	uint32_t cmdact                       : 1;
	uint32_t hipri_forcerm                : 1;
	uint32_t cmdstatus                    : 4;
	uint32_t commandparam                 : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_depcmdx_s    cn70xx;
	struct cvmx_usbdrdx_uahc_depcmdx_s    cn70xxp1;
	struct cvmx_usbdrdx_uahc_depcmdx_s    cn73xx;
	struct cvmx_usbdrdx_uahc_depcmdx_s    cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_depcmdx cvmx_usbdrdx_uahc_depcmdx_t;

/**
 * cvmx_usbdrd#_uahc_depcmdpar0_#
 *
 * This register indicates the physical endpoint command parameter 0. It must be programmed
 * before issuing the command.
 *
 * This register can be reset by IOI reset or USBDRD()_UCTL_CTL[UAHC_RST] or
 * USBDRD()_UAHC_GCTL[CORESOFTRESET] or
 * USBDRD()_UAHC_USBCMD[HCRST] or USBDRD()_UAHC_USBCMD[LHCRST] or
 * USBDRD()_UAHC_DCTL[CSFTRST].
 */
union cvmx_usbdrdx_uahc_depcmdpar0_x {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_depcmdpar0_x_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t param0                       : 32; /**< Physical endpoint command parameter 0 */
#else
	uint32_t param0                       : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_depcmdpar0_x_s cn70xx;
	struct cvmx_usbdrdx_uahc_depcmdpar0_x_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_depcmdpar0_x_s cn73xx;
	struct cvmx_usbdrdx_uahc_depcmdpar0_x_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_depcmdpar0_x cvmx_usbdrdx_uahc_depcmdpar0_x_t;

/**
 * cvmx_usbdrd#_uahc_depcmdpar1_#
 *
 * This register indicates the physical endpoint command parameter 1. It must be programmed
 * before issuing the command.
 *
 * This register can be reset by IOI reset or USBDRD()_UCTL_CTL[UAHC_RST] or
 * USBDRD()_UAHC_GCTL[CORESOFTRESET] or
 * USBDRD()_UAHC_USBCMD[HCRST] or USBDRD()_UAHC_USBCMD[LHCRST] or
 * USBDRD()_UAHC_DCTL[CSFTRST].
 */
union cvmx_usbdrdx_uahc_depcmdpar1_x {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_depcmdpar1_x_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t param1                       : 32; /**< Physical endpoint command parameter 1 */
#else
	uint32_t param1                       : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_depcmdpar1_x_s cn70xx;
	struct cvmx_usbdrdx_uahc_depcmdpar1_x_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_depcmdpar1_x_s cn73xx;
	struct cvmx_usbdrdx_uahc_depcmdpar1_x_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_depcmdpar1_x cvmx_usbdrdx_uahc_depcmdpar1_x_t;

/**
 * cvmx_usbdrd#_uahc_depcmdpar2_#
 *
 * This register indicates the physical endpoint command parameter 2. It must be programmed
 * before issuing the command.
 *
 * This register can be reset by IOI reset or USBDRD()_UCTL_CTL[UAHC_RST] or
 * USBDRD()_UAHC_GCTL[CORESOFTRESET] or
 * USBDRD()_UAHC_USBCMD[HCRST] or USBDRD()_UAHC_USBCMD[LHCRST] or
 * USBDRD()_UAHC_DCTL[CSFTRST].
 */
union cvmx_usbdrdx_uahc_depcmdpar2_x {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_depcmdpar2_x_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t param2                       : 32; /**< Physical endpoint command parameter 2 */
#else
	uint32_t param2                       : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_depcmdpar2_x_s cn70xx;
	struct cvmx_usbdrdx_uahc_depcmdpar2_x_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_depcmdpar2_x_s cn73xx;
	struct cvmx_usbdrdx_uahc_depcmdpar2_x_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_depcmdpar2_x cvmx_usbdrdx_uahc_depcmdpar2_x_t;

/**
 * cvmx_usbdrd#_uahc_devten
 *
 * This register controls the generation of device-specific events.
 * If an enable bit is set to 0, the event will not be generated.
 *
 * This register can be reset by IOI reset or USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_devten {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_devten_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_13_31               : 19;
	uint32_t vndrdevtstrcveden            : 1;  /**< Vendor device test LMP received event. */
	uint32_t reserved_10_11               : 2;
	uint32_t errticerren                  : 1;  /**< Erratic error event enable. */
	uint32_t reserved_8_8                 : 1;
	uint32_t sofen                        : 1;  /**< Start of (micro)frame enable.
                                                         For debug purposes only; normally software must disable this event. */
	uint32_t u3l2l1suspen                 : 1;  /**< U3/L2-L1 suspend event enable. */
	uint32_t hibernationreqevten          : 1;  /**< This bit enables/disables the generation of the hibernation request event. */
	uint32_t wkupevten                    : 1;  /**< Resume/remote wakeup detected event enable. */
	uint32_t ulstcngen                    : 1;  /**< USB/link state change event enable. */
	uint32_t connectdoneen                : 1;  /**< Connection done enable. */
	uint32_t usbrsten                     : 1;  /**< USB reset enable. */
	uint32_t disconnevten                 : 1;  /**< Disconnect detected event enable. */
#else
	uint32_t disconnevten                 : 1;
	uint32_t usbrsten                     : 1;
	uint32_t connectdoneen                : 1;
	uint32_t ulstcngen                    : 1;
	uint32_t wkupevten                    : 1;
	uint32_t hibernationreqevten          : 1;
	uint32_t u3l2l1suspen                 : 1;
	uint32_t sofen                        : 1;
	uint32_t reserved_8_8                 : 1;
	uint32_t errticerren                  : 1;
	uint32_t reserved_10_11               : 2;
	uint32_t vndrdevtstrcveden            : 1;
	uint32_t reserved_13_31               : 19;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_devten_s     cn70xx;
	struct cvmx_usbdrdx_uahc_devten_s     cn70xxp1;
	struct cvmx_usbdrdx_uahc_devten_s     cn73xx;
	struct cvmx_usbdrdx_uahc_devten_s     cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_devten cvmx_usbdrdx_uahc_devten_t;

/**
 * cvmx_usbdrd#_uahc_dgcmd
 *
 * This register enables software to program the core using a single generic command interface to
 * send link management packets and notifications. This register contains command, control, and
 * status fields relevant to the current generic command, while the USBDRD()_UAHC_DGCMDPAR
 * register provides the command parameter.
 *
 * This register can be reset by IOI reset or USBDRD()_UCTL_CTL[UAHC_RST] or
 * USBDRD()_UAHC_GCTL[CORESOFTRESET] or
 * USBDRD()_UAHC_USBCMD[HCRST] or USBDRD()_UAHC_USBCMD[LHCRST] or
 * USBDRD()_UAHC_DCTL[CSFTRST].
 */
union cvmx_usbdrdx_uahc_dgcmd {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_dgcmd_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t cmdstatus                    : 1;  /**< Command status.
                                                         0 = Indicates command success.
                                                         1 = CmdErr - Indicates that the device controller encountered an error
                                                            while processing the command. */
	uint32_t reserved_11_14               : 4;
	uint32_t cmdact                       : 1;  /**< Command active.
                                                         The software sets this bit to 1 to enable the device controller to execute the
                                                         generic command.
                                                         The device controller sets this bit to 0 after executing the command. */
	uint32_t reserved_9_9                 : 1;
	uint32_t cmdioc                       : 1;  /**< Command interrupt on complete.
                                                         When this bit is set, the device controller issues a generic command
                                                         completion event after executing the command. Note that this interrupt is
                                                         mapped to USBDRD()_UAHC_DCFG[INTRNUM].
                                                         Note: This field must not set to 1 if the USBDRD()_UAHC_DCTL[RS] field is 0. */
	uint32_t cmdtyp                       : 8;  /**< Specifies the type of command the software driver is requesting the core to
                                                         perform. See USBDRD_UAHC_DGCMD_CMDTYPE_E for encodings and usage. */
#else
	uint32_t cmdtyp                       : 8;
	uint32_t cmdioc                       : 1;
	uint32_t reserved_9_9                 : 1;
	uint32_t cmdact                       : 1;
	uint32_t reserved_11_14               : 4;
	uint32_t cmdstatus                    : 1;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_dgcmd_s      cn70xx;
	struct cvmx_usbdrdx_uahc_dgcmd_s      cn70xxp1;
	struct cvmx_usbdrdx_uahc_dgcmd_s      cn73xx;
	struct cvmx_usbdrdx_uahc_dgcmd_s      cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_dgcmd cvmx_usbdrdx_uahc_dgcmd_t;

/**
 * cvmx_usbdrd#_uahc_dgcmdpar
 *
 * This register indicates the device command parameter.
 * This must be programmed before or along with USBDRD()_UAHC_DGCMD.
 *
 * This register can be reset by IOI reset or USBDRD()_UCTL_CTL[UAHC_RST] or
 * USBDRD()_UAHC_GCTL[CORESOFTRESET] or
 * USBDRD()_UAHC_USBCMD[HCRST] or USBDRD()_UAHC_USBCMD[LHCRST] or
 * USBDRD()_UAHC_DCTL[CSFTRST].
 */
union cvmx_usbdrdx_uahc_dgcmdpar {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_dgcmdpar_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t param                        : 32; /**< Device generic command parameter.
                                                         Usage depends on which USBDRD()_UAHC_DGCMD[CMDTYPE] is used,
                                                         see usage notes in USBDRD_UAHC_DGCMD_CMDTYPE_E descriptions. */
#else
	uint32_t param                        : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_dgcmdpar_s   cn70xx;
	struct cvmx_usbdrdx_uahc_dgcmdpar_s   cn70xxp1;
	struct cvmx_usbdrdx_uahc_dgcmdpar_s   cn73xx;
	struct cvmx_usbdrdx_uahc_dgcmdpar_s   cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_dgcmdpar cvmx_usbdrdx_uahc_dgcmdpar_t;

/**
 * cvmx_usbdrd#_uahc_dnctrl
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.4.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_dnctrl {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_dnctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t n                            : 16; /**< Notification enable. */
#else
	uint32_t n                            : 16;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_dnctrl_s     cn70xx;
	struct cvmx_usbdrdx_uahc_dnctrl_s     cn70xxp1;
	struct cvmx_usbdrdx_uahc_dnctrl_s     cn73xx;
	struct cvmx_usbdrdx_uahc_dnctrl_s     cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_dnctrl cvmx_usbdrdx_uahc_dnctrl_t;

/**
 * cvmx_usbdrd#_uahc_dsts
 *
 * This register indicates the status of the device controller with respect to USB-related
 * events.
 *
 * This register can be reset by IOI reset or USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_dsts {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_dsts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t dcnrd                        : 1;  /**< Device controller not ready.
                                                         Will always read-as-zero. */
	uint32_t sre                          : 1;  /**< Save/restore error.
                                                         This bit is currently not supported. */
	uint32_t reserved_26_27               : 2;
	uint32_t rss                          : 1;  /**< Restore state status.
                                                         This bit is similar to the USBDRD()_USBSTS[RSS] in host mode.
                                                         When the controller has finished the restore process, it will complete the
                                                         command by setting RSS to 0.
                                                         Will always read-as-zero. */
	uint32_t sss                          : 1;  /**< Save state status.
                                                         This bit is similar to the USBDRD()_UAHC_USBSTS[SSS] in host mode.
                                                         When the controller has finished the save process, it will complete the
                                                         command by setting SSS to 0.
                                                         Will always read-as-zero. */
	uint32_t coreidle                     : 1;  /**< Core idle.
                                                         The bit indicates that the core finished transferring all RxFIFO data to
                                                         system memory, writing out all completed descriptors, and all event counts
                                                         are zero.
                                                         Note: While testing for Reset values, mask out the read value. This bit
                                                         represents the changing state of the core and does not hold a static value. */
	uint32_t devctrlhlt                   : 1;  /**< Device controller halted.
                                                         When 1, the core does not generate device events.
                                                         - This bit is set to 0 when the USBDRD()_UAHC_DCTL[RS] register is set to 1.
                                                         - The core sets this bit to 1 when, after software sets USBDRD()_UAHC_DCTL[RS] to 0,
                                                         the core is
                                                           idle and the lower layer finishes the disconnect process. */
	uint32_t usblnkst                     : 4;  /**< USB/link state.
                                                         In SuperSpeed mode, uses LTSSM State:
                                                            0x0 = U0.
                                                            0x1 = U1.
                                                            0x2 = U2.
                                                            0x3 = U3.
                                                            0x4 = SS_DIS.
                                                            0x5 = RX_DET.
                                                            0x6 = SS_INACT.
                                                            0x7 = POLL.
                                                            0x8 = RECOV.
                                                            0x9 = HRESET.
                                                            0xa = CMPLY.
                                                            0xb = LPBK.
                                                            0xf = Resume/Reset.
                                                            others: Reserved.
                                                         In High/Full/LowSpeed mode:
                                                            0x0 = On state.
                                                            0x2 = Sleep (L1) state.
                                                            0x3 = Suspend (L2) state.
                                                            0x4 = Disconnected state (Default state).
                                                            0x5 = Early Suspend state.
                                                            others: Reserved.
                                                         The link state resume/reset indicates that the core received a resume or USB
                                                         reset request from the host while the link was in hibernation. Software must
                                                         write 0x8 (recovery) to the USBDRD()_UAHC_DCTL[ULSTCHNGREQ] field to acknowledge
                                                         the resume/reset request. */
	uint32_t rxfifoempty                  : 1;  /**< RxFIFO Empty Indication. */
	uint32_t soffn                        : 14; /**< Frame/MicroFrame number of the received SOF.
                                                         When the core is operating at high-speed:
                                                           <16:6> = Frame number.
                                                           <5:3> = Microframe number.
                                                         When the core is operating at full-speed:
                                                           <16:14> = Not used, software can ignore these three bits.
                                                           <13:3> = Frame number. */
	uint32_t connectspd                   : 3;  /**< Connected speed.
                                                         Indicates the speed at which the controller core has come up after speed
                                                         detection through a chirp sequence.
                                                          0x0 = High-speed (PHY clock is running at 60 MHz).
                                                          0x1 = Full-speed (PHY clock is running at 60 MHz).
                                                          0x2 = Low-speed  (not supported).
                                                          0x3 = Full-speed (PHY clock is running at 48 MHz).
                                                          0x4 = SuperSpeed (PHY clock is running at 125 or 250 MHz). */
#else
	uint32_t connectspd                   : 3;
	uint32_t soffn                        : 14;
	uint32_t rxfifoempty                  : 1;
	uint32_t usblnkst                     : 4;
	uint32_t devctrlhlt                   : 1;
	uint32_t coreidle                     : 1;
	uint32_t sss                          : 1;
	uint32_t rss                          : 1;
	uint32_t reserved_26_27               : 2;
	uint32_t sre                          : 1;
	uint32_t dcnrd                        : 1;
	uint32_t reserved_30_31               : 2;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_dsts_s       cn70xx;
	struct cvmx_usbdrdx_uahc_dsts_s       cn70xxp1;
	struct cvmx_usbdrdx_uahc_dsts_s       cn73xx;
	struct cvmx_usbdrdx_uahc_dsts_s       cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_dsts cvmx_usbdrdx_uahc_dsts_t;

/**
 * cvmx_usbdrd#_uahc_erdp#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.5.2.3.3.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_erdpx {
	uint64_t u64;
	struct cvmx_usbdrdx_uahc_erdpx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t erdp                         : 60; /**< Event ring dequeue pointer bits <63:4>. */
	uint64_t ehb                          : 1;  /**< Event handler busy */
	uint64_t desi                         : 3;  /**< Dequeue ERST segment index. */
#else
	uint64_t desi                         : 3;
	uint64_t ehb                          : 1;
	uint64_t erdp                         : 60;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_erdpx_s      cn70xx;
	struct cvmx_usbdrdx_uahc_erdpx_s      cn70xxp1;
	struct cvmx_usbdrdx_uahc_erdpx_s      cn73xx;
	struct cvmx_usbdrdx_uahc_erdpx_s      cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_erdpx cvmx_usbdrdx_uahc_erdpx_t;

/**
 * cvmx_usbdrd#_uahc_erstba#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.5.2.3.2.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_erstbax {
	uint64_t u64;
	struct cvmx_usbdrdx_uahc_erstbax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t erstba                       : 58; /**< Event-ring segment-table base-address bits<63:6>. */
	uint64_t reserved_0_5                 : 6;
#else
	uint64_t reserved_0_5                 : 6;
	uint64_t erstba                       : 58;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_erstbax_s    cn70xx;
	struct cvmx_usbdrdx_uahc_erstbax_s    cn70xxp1;
	struct cvmx_usbdrdx_uahc_erstbax_s    cn73xx;
	struct cvmx_usbdrdx_uahc_erstbax_s    cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_erstbax cvmx_usbdrdx_uahc_erstbax_t;

/**
 * cvmx_usbdrd#_uahc_erstsz#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.5.2.3.1.
 *
 * This register can be reset by NCB reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_erstszx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_erstszx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t erstsz                       : 16; /**< Event-ring segment-table size. */
#else
	uint32_t erstsz                       : 16;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_erstszx_s    cn70xx;
	struct cvmx_usbdrdx_uahc_erstszx_s    cn70xxp1;
	struct cvmx_usbdrdx_uahc_erstszx_s    cn73xx;
	struct cvmx_usbdrdx_uahc_erstszx_s    cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_erstszx cvmx_usbdrdx_uahc_erstszx_t;

/**
 * cvmx_usbdrd#_uahc_gbuserraddr
 *
 * When the AXI master bus returns error response, the SoC bus error is generated. In the host
 * mode, the host_system_err port indicates this condition. In addition, it is also indicated in
 * USBDRD()_UAHC_USBSTS[HSE]. Due to the nature of AXI, it is possible that multiple AXI
 * transactions
 * are active at a time. The host controller does not keep track of the start address of all
 * outstanding transactions. Instead, it keeps track of the start address of the DMA transfer
 * associated with all active transactions. It is this address that is reported in
 * USBDRD()_UAHC_GBUSERRADDR when a bus error occurs. For example, if the host controller
 * initiates a DMA
 * transfer to write 1k of packet data starting at buffer address 0xABCD0000, and this DMA is
 * broken up into multiple 256B bursts on the AXI, then if a bus error occurs on any of these
 * associated AXI transfers, USBDRD()_UAHC_GBUSERRADDR reflects the DMA start address of
 * 0xABCD0000
 * regardless of which AXI transaction received the error.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gbuserraddr {
	uint64_t u64;
	struct cvmx_usbdrdx_uahc_gbuserraddr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t busaddr                      : 64; /**< Bus address. Contains the first bus address that encountered an SoC bus error. It is valid
                                                         when the USBDRD()_UAHC_GSTS[BUSERRADDRVLD] = 1. It can only be cleared by resetting the
                                                         core. */
#else
	uint64_t busaddr                      : 64;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gbuserraddr_s cn70xx;
	struct cvmx_usbdrdx_uahc_gbuserraddr_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_gbuserraddr_s cn73xx;
	struct cvmx_usbdrdx_uahc_gbuserraddr_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gbuserraddr cvmx_usbdrdx_uahc_gbuserraddr_t;

/**
 * cvmx_usbdrd#_uahc_gctl
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 *
 */
union cvmx_usbdrdx_uahc_gctl {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pwrdnscale                   : 13; /**< Power down scale. The USB3 suspend-clock input replaces pipe3_rx_pclk as a clock source to
                                                         a small part of the USB3 core that operates when the SuperSpeed PHY is in its lowest power
                                                         (P3) state, and therefore does not provide a clock. This field specifies how many suspend-
                                                         clock periods fit into a 16 kHz clock period. When performing the division, round up the
                                                         remainder.
                                                         For example, when using an 32-bit PHY and 25-MHz suspend clock, PWRDNSCALE = 25000 kHz/16
                                                         kHz = 1563 (rounded up).
                                                         The minimum suspend-clock frequency is 32 KHz, and maximum suspend-clock frequency is 125
                                                         MHz.
                                                         The LTSSM uses suspend clock for 12-ms and 100-ms timers during suspend mode. According to
                                                         the USB 3.0 specification, the accuracy on these timers is 0% to +50%. 12 ms + 0~+50%
                                                         accuracy = 18 ms (Range is 12 ms - 18 ms)
                                                         100 ms + 0~+50% accuracy = 150 ms (Range is 100 ms - 150 ms).
                                                         The suspend clock accuracy requirement is:
                                                         _ (12,000/62.5) * (GCTL[31:19]) * actual suspend_clk_period should be between 12,000 and
                                                         18,000
                                                         _ (100,000/62.5) * (GCTL[31:19]) * actual suspend_clk_period should be between 100,000 and
                                                         150,000
                                                         For example, if your suspend_clk frequency varies from 7.5 MHz to 10.5 MHz, then the value
                                                         needs to programmed is: power down scale = 10500/16 = 657 (rounded up; and fastest
                                                         frequency used). */
	uint32_t masterfiltbypass             : 1;  /**< Master filter bypass. Not relevant for Cavium's configuration. */
	uint32_t bypssetaddr                  : 1;  /**< Bypass SetAddress in device mode.
                                                         Always set to 0. */
	uint32_t u2rstecn                     : 1;  /**< If the SuperSpeed connection fails during POLL or LMP exchange, the device connects
                                                         at non-SuperSpeed mode. If this bit is set, then device attempts three more times to
                                                         connect at SuperSpeed, even if it previously failed to operate in SuperSpeed mode.
                                                         This bit is only applicable in device mode. */
	uint32_t frmscldwn                    : 2;  /**< Frame scale down. Scales down device view of a SOF/USOF/ITP duration.
                                                         For SuperSpeed/high-speed mode:
                                                         0x0 = Interval is 125 us.
                                                         0x1 = Interval is 62.5 us.
                                                         0x2 = Interval is 31.25 us.
                                                         0x3 = Interval is 15.625 us.
                                                         For full speed mode, the scale down value is multiplied by 8. */
	uint32_t prtcapdir                    : 2;  /**< 0x1 = for Host configurations.
                                                         0x2 = for Device configurations. */
	uint32_t coresoftreset                : 1;  /**< Core soft reset: 1 = soft reset to core, 0 = no soft reset.
                                                         Clears the interrupts and all the USBDRD()_UAHC_* CSRs except the
                                                         following registers: USBDRD()_UAHC_GCTL, USBDRD()_UAHC_GUCTL, USBDRD()_UAHC_GSTS,
                                                         USBDRD()_UAHC_GRLSID, USBDRD()_UAHC_GGPIO, USBDRD()_UAHC_GUID,
                                                         USBDRD()_UAHC_GUSB2PHYCFG(),
                                                         USBDRD()_UAHC_GUSB3PIPECTL().
                                                         When you reset PHYs (using USBDRD()_UAHC_GUSB2PHYCFG() or
                                                         USBDRD()_UAHC_GUSB3PIPECTL()), you must keep the core in reset state until PHY
                                                         clocks are stable. This controls the bus, RAM, and MAC domain resets. */
	uint32_t sofitpsync                   : 1;  /**< Synchronize ITP to reference clock. In host mode, if this bit is set to:
                                                         0 = The core keeps the UTMI/ULPI PHY on the first port in non-suspended state whenever
                                                         there is a SuperSpeed port that is not in Rx.Detect, SS.Disable, and U3 state.
                                                         1 = The core keeps the UTMI/ULPI PHY on the first port in non-suspended state whenever the
                                                         other non-SuperSpeed ports are not in suspended state.
                                                         This feature is useful because it saves power by suspending UTMI/ULPI when SuperSpeed only
                                                         is active and it helps resolve when the PHY does not transmit a host resume unless it is
                                                         placed in suspend state.
                                                         USBDRD()_UAHC_GUSB2PHYCFG()[SUSPHY] eventually decides to put the UTMI/ULPI PHY in to
                                                         suspend
                                                         state. In addition, when this bit is set to 1, the core generates ITP off of the REF_CLK-
                                                         based counter. Otherwise, ITP and SOF are generated off of UTMI/ULPI_CLK[0] based counter.
                                                         To program the reference clock period inside the core, refer to
                                                         USBDRD()_UAHC_GUCTL[REFCLKPER].
                                                         If you do not plan to ever use this feature or the
                                                         USBDRD()_UAHC_GFLADJ[GFLADJ_REFCLK_LPM_SEL]
                                                         feature, the minimum frequency for the ref_clk can be as low as 32 KHz. You can connect
                                                         the
                                                         SUSPEND_CLK (as low as 32 KHz) to REF_CLK.
                                                         If you plan to enable hardware-based LPM (PORTPMSC[HLE] = 1), this feature cannot be used.
                                                         Turn off this feature by setting this bit to zero and use the
                                                         USBDRD()_UAHC_GFLADJ[GFLADJ_REFCLK_LPM_SEL] feature.
                                                         If you set this bit to 1, the USBDRD()_UAHC_GUSB2PHYCFG()[U2_FREECLK_EXISTS] bit
                                                         must be set to 0. */
	uint32_t u1u2timerscale               : 1;  /**< Disable U1/U2 timer scaledown. If set to 1, along with SCALEDOWN = 0x1, disables the scale
                                                         down of U1/U2 inactive timer values.
                                                         This is for simulation mode only. */
	uint32_t debugattach                  : 1;  /**< Debug attach. When this bit is set:
                                                         * SuperSpeed link proceeds directly to the polling-link state (USBDRD()_UAHC_DCTL[RS] = 1)
                                                         without checking remote termination.
                                                         * Link LFPS polling timeout is infinite.
                                                         * Polling timeout during TS1 is infinite (in case link is waiting for TXEQ to finish). */
	uint32_t ramclksel                    : 2;  /**< RAM clock select. Always keep set to 0x0. */
	uint32_t scaledown                    : 2;  /**< Scale-down mode. When scale-down mode is enabled for simulation, the core uses scaled-down
                                                         timing values, resulting in faster simulations. When scale-down mode is disabled, actual
                                                         timing values are used. This is required for hardware operation.
                                                         High-speed/full-speed/low-speed modes:
                                                         0x0 = Disables all scale-downs. Actual timing values are used.
                                                         0x1 = Enables scale-down of all timing values. These include:
                                                         * Speed enumeration.
                                                         * HNP/SRP.
                                                         * Suspend and resume.
                                                         0x2 = N/A
                                                         0x3 = Enables bits <0> and <1> scale-down timing values.
                                                         SuperSpeed mode:
                                                         0x0 = Disables all scale-downs. Actual timing values are used.
                                                         0x1 = Enables scaled down SuperSpeed timing and repeat values including:
                                                         * Number of TxEq training sequences reduce to eight.
                                                         * LFPS polling burst time reduce to 100 ns.
                                                         * LFPS warm reset receive reduce to 30 us. */
	uint32_t disscramble                  : 1;  /**< Disable scrambling. Transmit request to link partner on next transition to recovery or polling. */
	uint32_t u2exit_lfps                  : 1;  /**< LFPS U2 exit.
                                                         0 = The link treats 248 ns LFPS as a valid U2 exit.
                                                         1 = The link waits for 8 us of LFPS before it detects a valid U2 exit.
                                                         This bit is added to improve interoperability with a third party host controller. This
                                                         host controller in U2 state while performing receiver detection generates an LFPS glitch
                                                         of about 4s duration. This causes the device to exit from U2 state because the LFPS filter
                                                         value is 248 ns. With the new functionality enabled, the device can stay in U2 while
                                                         ignoring this glitch from the host controller. */
	uint32_t reserved_1_1                 : 1;
	uint32_t dsblclkgtng                  : 1;  /**< Disable clock gating. When set to 1 and the core is in low power mode, internal clock
                                                         gating is disabled, which means the clocks are always running. This bit can be set to 1
                                                         after power-up reset. */
#else
	uint32_t dsblclkgtng                  : 1;
	uint32_t reserved_1_1                 : 1;
	uint32_t u2exit_lfps                  : 1;
	uint32_t disscramble                  : 1;
	uint32_t scaledown                    : 2;
	uint32_t ramclksel                    : 2;
	uint32_t debugattach                  : 1;
	uint32_t u1u2timerscale               : 1;
	uint32_t sofitpsync                   : 1;
	uint32_t coresoftreset                : 1;
	uint32_t prtcapdir                    : 2;
	uint32_t frmscldwn                    : 2;
	uint32_t u2rstecn                     : 1;
	uint32_t bypssetaddr                  : 1;
	uint32_t masterfiltbypass             : 1;
	uint32_t pwrdnscale                   : 13;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gctl_s       cn70xx;
	struct cvmx_usbdrdx_uahc_gctl_s       cn70xxp1;
	struct cvmx_usbdrdx_uahc_gctl_s       cn73xx;
	struct cvmx_usbdrdx_uahc_gctl_s       cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gctl cvmx_usbdrdx_uahc_gctl_t;

/**
 * cvmx_usbdrd#_uahc_gdbgbmu
 *
 * See description in USBDRD()_UAHC_GDBGFIFOSPACE.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gdbgbmu {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gdbgbmu_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t bmu_bcu_dbg                  : 24; /**< BMU_BCU debug information. */
	uint32_t bmu_dcu_dbg                  : 4;  /**< BMU_DCU debug information. */
	uint32_t bmu_ccu_dbg                  : 4;  /**< BMU_CCU debug information. */
#else
	uint32_t bmu_ccu_dbg                  : 4;
	uint32_t bmu_dcu_dbg                  : 4;
	uint32_t bmu_bcu_dbg                  : 24;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gdbgbmu_s    cn70xx;
	struct cvmx_usbdrdx_uahc_gdbgbmu_s    cn70xxp1;
	struct cvmx_usbdrdx_uahc_gdbgbmu_s    cn73xx;
	struct cvmx_usbdrdx_uahc_gdbgbmu_s    cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gdbgbmu cvmx_usbdrdx_uahc_gdbgbmu_t;

/**
 * cvmx_usbdrd#_uahc_gdbgepinfo
 *
 * See description in USBDRD()_UAHC_GDBGFIFOSPACE.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gdbgepinfo {
	uint64_t u64;
	struct cvmx_usbdrdx_uahc_gdbgepinfo_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t endpt_dbg                    : 64; /**< Endpoint debug information. */
#else
	uint64_t endpt_dbg                    : 64;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gdbgepinfo_s cn70xx;
	struct cvmx_usbdrdx_uahc_gdbgepinfo_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_gdbgepinfo_s cn73xx;
	struct cvmx_usbdrdx_uahc_gdbgepinfo_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gdbgepinfo cvmx_usbdrdx_uahc_gdbgepinfo_t;

/**
 * cvmx_usbdrd#_uahc_gdbgfifospace
 *
 * This register is for debug purposes. It provides debug information on the internal status and
 * state machines. Global debug registers have design-specific information, and are used by state
 * machines. Global debug registers have design-specific information, and are used for debugging
 * purposes. These registers are not intended to be used by the customer. If any debug assistance
 * is needed for the silicon, contact customer support with a dump of these registers.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gdbgfifospace {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gdbgfifospace_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t spaceavailable               : 16; /**< Space available in the selected FIFO. */
	uint32_t reserved_8_15                : 8;
	uint32_t select                       : 8;  /**< FIFO/queue select/port-select.
                                                         FIFO/queue select: <7:5> indicates the FIFO/queue type; <4:0> indicates the FIFO/queue
                                                         number.
                                                         For example, 0x21 refers to RxFIFO_1, and 0x5E refers to TxReqQ_30.
                                                         0x1F-0x0: TxFIFO_31 to TxFIFO_0.
                                                         0x3F-0x20: RxFIFO_31 to RxFIFO_0.
                                                         0x5F-0x40: TxReqQ_31 to TxReqQ_0.
                                                         0x7F-0x60: RxReqQ_31 to RxReqQ_0.
                                                         0x9F-0x80: RxInfoQ_31 to RxInfoQ_0.
                                                         0xA0: DescFetchQ.
                                                         0xA1: EventQ.
                                                         0xA2: ProtocolStatusQ.
                                                         Port-select: <3:0> selects the port-number when accessing USBDRD()_UAHC_GDBGLTSSM. */
#else
	uint32_t select                       : 8;
	uint32_t reserved_8_15                : 8;
	uint32_t spaceavailable               : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gdbgfifospace_s cn70xx;
	struct cvmx_usbdrdx_uahc_gdbgfifospace_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_gdbgfifospace_s cn73xx;
	struct cvmx_usbdrdx_uahc_gdbgfifospace_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gdbgfifospace cvmx_usbdrdx_uahc_gdbgfifospace_t;

/**
 * cvmx_usbdrd#_uahc_gdbglnmcc
 *
 * See description in USBDRD()_UAHC_GDBGFIFOSPACE.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gdbglnmcc {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gdbglnmcc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_9_31                : 23;
	uint32_t lnmcc_berc                   : 9;  /**< This field indicates the bit-error-rate information for the port selected in
                                                         USBDRD()_UAHC_GDBGFIFOSPACE[SELECT] (port-select).
                                                         This field is for debug purposes only. */
#else
	uint32_t lnmcc_berc                   : 9;
	uint32_t reserved_9_31                : 23;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gdbglnmcc_s  cn70xx;
	struct cvmx_usbdrdx_uahc_gdbglnmcc_s  cn70xxp1;
	struct cvmx_usbdrdx_uahc_gdbglnmcc_s  cn73xx;
	struct cvmx_usbdrdx_uahc_gdbglnmcc_s  cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gdbglnmcc cvmx_usbdrdx_uahc_gdbglnmcc_t;

/**
 * cvmx_usbdrd#_uahc_gdbglsp
 *
 * See description in USBDRD()_UAHC_GDBGFIFOSPACE.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gdbglsp {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gdbglsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lsp_dbg                      : 32; /**< LSP debug information. */
#else
	uint32_t lsp_dbg                      : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gdbglsp_s    cn70xx;
	struct cvmx_usbdrdx_uahc_gdbglsp_s    cn70xxp1;
	struct cvmx_usbdrdx_uahc_gdbglsp_s    cn73xx;
	struct cvmx_usbdrdx_uahc_gdbglsp_s    cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gdbglsp cvmx_usbdrdx_uahc_gdbglsp_t;

/**
 * cvmx_usbdrd#_uahc_gdbglspmux
 *
 * See description in USBDRD()_UAHC_GDBGFIFOSPACE.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gdbglspmux {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gdbglspmux_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t latraceportmuxselect         : 8;  /**< logic_analyzer_trace port multiplexer select. Only bits<21:16> are used. For details on
                                                         how the mux controls the debug traces, refer to the Verilog file.
                                                         A value of 0x3F drives 0s on the logic_analyzer_trace signal. If you plan to OR (instead
                                                         using a mux) this signal with other trace signals in your system to generate a common
                                                         trace signal, you can use this feature. */
	uint32_t endbc                        : 1;  /**< Enable debugging of the debug capability LSP. Use HOSTSELECT to select the DbC LSP debug
                                                         information presented in the GDBGLSP register. */
	uint32_t reserved_14_14               : 1;
	uint32_t hostselect                   : 14; /**< Host select. Selects the LSP debug information presented in USBDRD()_UAHC_GDBGLSP. */
#else
	uint32_t hostselect                   : 14;
	uint32_t reserved_14_14               : 1;
	uint32_t endbc                        : 1;
	uint32_t latraceportmuxselect         : 8;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gdbglspmux_s cn70xx;
	struct cvmx_usbdrdx_uahc_gdbglspmux_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_gdbglspmux_s cn73xx;
	struct cvmx_usbdrdx_uahc_gdbglspmux_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gdbglspmux cvmx_usbdrdx_uahc_gdbglspmux_t;

/**
 * cvmx_usbdrd#_uahc_gdbgltssm
 *
 * In multiport host configuration, the port number is defined by
 * USBDRD()_UAHC_GDBGFIFOSPACE[SELECT]<3:0>. Value of this register may change immediately after
 * reset.
 * See description in USBDRD()_UAHC_GDBGFIFOSPACE.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gdbgltssm {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gdbgltssm_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_27_31               : 5;
	uint32_t ltdbtimeout                  : 1;  /**< LTDB timeout. */
	uint32_t ltdblinkstate                : 4;  /**< LTDB link state. */
	uint32_t ltdbsubstate                 : 4;  /**< LTDB substate. */
	uint32_t debugpipestatus              : 18; /**< Debug PIPE status.
                                                         _ <17> Elastic buffer mode.
                                                         _ <16> TX elec idle.
                                                         _ <15> RX polarity.
                                                         _ <14> TX Detect RX/loopback.
                                                         _ <13:11> LTSSM PHY command state.
                                                         _ 0x0 = PHY_IDLE (PHY command state is in IDLE. No PHY request is pending.)
                                                         _ 0x1 = PHY_DET (Request to start receiver detection).
                                                         _ 0x2 = PHY_DET_3 (Wait for Phy_Status (receiver detection)).
                                                         _ 0x3 = PHY_PWR_DLY (delay Pipe3_PowerDown P0 -> P1/P2/P3 request).
                                                         _ 0x4 = PHY_PWR_A (delay for internal logic).
                                                         _ 0x5 = PHY_PWR_B (wait for Phy_Status(Power-state change request)).
                                                         _ <10:9> Power down.
                                                         _ <8> RxEq train.
                                                         _ <7:6> TX deemphasis.
                                                         _ <5:3> LTSSM clock state.
                                                         _ 0x0 = CLK_NORM (PHY is in non-P3 state and PCLK is running).
                                                         _ 0x1 = CLK_TO_P3 (P3 entry request to PHY).
                                                         _ 0x2 = CLK_WAIT1 (wait for Phy_Status (P3 request)).
                                                         _ 0x3 = CLK_P3 (PHY is in P3 and PCLK is not running).
                                                         _ 0x4 = CLK_TO_P0 (P3 exit request to PHY).
                                                         _ 0x5 = CLK_WAIT2 (Wait for Phy_Status (P3 exit request)).
                                                         _ <2> TX swing.
                                                         _ <1> RX termination.
                                                         _ <0> TX 1s/0s. */
#else
	uint32_t debugpipestatus              : 18;
	uint32_t ltdbsubstate                 : 4;
	uint32_t ltdblinkstate                : 4;
	uint32_t ltdbtimeout                  : 1;
	uint32_t reserved_27_31               : 5;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gdbgltssm_s  cn70xx;
	struct cvmx_usbdrdx_uahc_gdbgltssm_s  cn70xxp1;
	struct cvmx_usbdrdx_uahc_gdbgltssm_s  cn73xx;
	struct cvmx_usbdrdx_uahc_gdbgltssm_s  cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gdbgltssm cvmx_usbdrdx_uahc_gdbgltssm_t;

/**
 * cvmx_usbdrd#_uahc_gdmahlratio
 *
 * This register specifies the relative priority of the SuperSpeed FIFOs with respect to the
 * high-speed/full-speed/low-speed FIFOs. The DMA arbiter prioritizes the high-speed/full-speed
 * /low-speed round-robin arbiter group every DMA high-low priority ratio grants as indicated in
 * the register separately for TX and RX.
 *
 * To illustrate, consider that all FIFOs are requesting access simultaneously, and the ratio is
 * 4. SuperSpeed gets priority for four packets, high-speed/full-speed/low-speed gets priority
 * for one packet, SuperSpeed gets priority for four packets, high-speed/full-speed/low-speed
 * gets priority for one packet, and so on.
 *
 * If FIFOs from both speed groups are not requesting access simultaneously then:
 * * If SuperSpeed got grants four out of the last four times, then high-speed/full-speed/
 * low-speed get the priority on any future request.
 * * If high-speed/full-speed/low-speed got the grant last time, SuperSpeed gets the priority on
 * the next request.
 *
 * If there is a valid request on either SuperSpeed or high-speed/full-speed/low-speed, a grant
 * is always awarded; there is no idle.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gdmahlratio {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gdmahlratio_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_13_31               : 19;
	uint32_t rx_ratio                     : 5;  /**< Speed ratio for RX arbitration. */
	uint32_t reserved_5_7                 : 3;
	uint32_t tx_ratio                     : 5;  /**< Speed ratio for TX arbitration. */
#else
	uint32_t tx_ratio                     : 5;
	uint32_t reserved_5_7                 : 3;
	uint32_t rx_ratio                     : 5;
	uint32_t reserved_13_31               : 19;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gdmahlratio_s cn70xx;
	struct cvmx_usbdrdx_uahc_gdmahlratio_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_gdmahlratio_s cn73xx;
	struct cvmx_usbdrdx_uahc_gdmahlratio_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gdmahlratio cvmx_usbdrdx_uahc_gdmahlratio_t;

/**
 * cvmx_usbdrd#_uahc_gevntadr#
 *
 * This register holds the event buffer DMA address pointer. Software must initialize this
 * address once during power-on initialization. Software must not change the value of this
 * register after it is initialized.
 * Software must only use the GEVNTCOUNTn register for event processing. The lower n bits of the
 * address must be USBDRD()_UAHC_GEVNTSIZ()[EVNTSIZ]-aligned.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_gevntadrx {
	uint64_t u64;
	struct cvmx_usbdrdx_uahc_gevntadrx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t evntadr                      : 64; /**< Holds the start address of the external memory
                                                         for the Event Buffer. During operation, hardware does not update
                                                         this address. */
#else
	uint64_t evntadr                      : 64;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gevntadrx_s  cn70xx;
	struct cvmx_usbdrdx_uahc_gevntadrx_s  cn70xxp1;
	struct cvmx_usbdrdx_uahc_gevntadrx_s  cn73xx;
	struct cvmx_usbdrdx_uahc_gevntadrx_s  cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gevntadrx cvmx_usbdrdx_uahc_gevntadrx_t;

/**
 * cvmx_usbdrd#_uahc_gevntcount#
 *
 * This register holds the number of valid bytes in the event buffer. During initialization,
 * software must initialize the count by writing 0 to the event count field. Each time the
 * hardware writes a new event to the Event Buffer, it increments this count. Most events
 * are four bytes, but some events may span over multiple four byte entries. Whenever the
 * count is greater than zero, the hardware raises the corresponding interrupt
 * line (depending on the USBDRD()_UAHC_GEVNTSIZ()[EVNTINTMASK]). On an interrupt, software
 * processes one or more events out of the event buffer. Afterwards, software must write the
 * event count field with the number of bytes it processed.
 *
 * Clock crossing delays may result in the interrupt's continual assertion after software
 * acknowledges the last event. Therefore, when the interrupt line is asserted, software must
 * read the GEVNTCOUNT register and only process events if the GEVNTCOUNT is greater than 0.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_gevntcountx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gevntcountx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t evntcount                    : 16; /**< When read, returns the number of valid events in the event buffer (in bytes).
                                                         When written, hardware decrements the count by the value written.
                                                         The interrupt line remains high when count is not 0. */
#else
	uint32_t evntcount                    : 16;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gevntcountx_s cn70xx;
	struct cvmx_usbdrdx_uahc_gevntcountx_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_gevntcountx_s cn73xx;
	struct cvmx_usbdrdx_uahc_gevntcountx_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gevntcountx cvmx_usbdrdx_uahc_gevntcountx_t;

/**
 * cvmx_usbdrd#_uahc_gevntsiz#
 *
 * This register holds the event buffer size and the event interrupt mask bit. During power-on
 * initialization, software must initialize the size with the number of bytes allocated for
 * the event buffer. The event interrupt mask will mask the interrupt, but events are still
 * queued. After configuration, software must preserve the event buffer size value when
 * changing the event interrupt mask.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_gevntsizx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gevntsizx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t evntintmask                  : 1;  /**< When set to 1, this prevents the interrupt from being generated.
                                                         However, even when the mask is set, the events are queued. */
	uint32_t reserved_16_30               : 15;
	uint32_t evntsiz                      : 16; /**< Holds the size of the event buffer in bytes; must be a multiple of
                                                         four. This is programmed by software once during initialization.
                                                         The minimum size of the event buffer is 32 bytes. */
#else
	uint32_t evntsiz                      : 16;
	uint32_t reserved_16_30               : 15;
	uint32_t evntintmask                  : 1;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gevntsizx_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t evntintmask                  : 1;  /**< When set to 1, this prevents the interrupt from being generated.
                                                         However, even when the mask is set, the events are queued. */
	uint32_t reserved_30_16               : 15;
	uint32_t evntsiz                      : 16; /**< Holds the size of the event buffer in bytes; must be a multiple of
                                                         four. This is programmed by software once during initialization.
                                                         The minimum size of the event buffer is 32 bytes. */
#else
	uint32_t evntsiz                      : 16;
	uint32_t reserved_30_16               : 15;
	uint32_t evntintmask                  : 1;
#endif
	} cn70xx;
	struct cvmx_usbdrdx_uahc_gevntsizx_cn70xx cn70xxp1;
	struct cvmx_usbdrdx_uahc_gevntsizx_cn70xx cn73xx;
	struct cvmx_usbdrdx_uahc_gevntsizx_cn70xx cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gevntsizx cvmx_usbdrdx_uahc_gevntsizx_t;

/**
 * cvmx_usbdrd#_uahc_gfladj
 *
 * This register provides options for the software to control the core behavior with respect to
 * SOF (start of frame) and ITP (isochronous timestamp packet) timers and frame timer
 * functionality. It provides the option to override the sideband signal fladj_30mhz_reg. In
 * addition, it enables running SOF or ITP frame timer counters completely off of the REF_CLK.
 * This facilitates hardware LPM in host mode with the SOF or ITP counters being run off of the
 * REF_CLK signal.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gfladj {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gfladj_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t gfladj_refclk_240mhzdecr_pls1 : 1; /**< This field indicates that the decrement value that the controller applies for each REF_CLK
                                                         must be GFLADJ_REFCLK_240MHZ_DECR and GFLADJ_REFCLK_240MHZ_DECR +1 alternatively on each
                                                         REF_CLK. Set this bit to 1 only if [GFLADJ_REFCLK_LPM_SEL] is set to 1 and the fractional
                                                         component of 240/ref_frequency is greater than or equal to 0.5.
                                                         Example:
                                                         If the REF_CLK is 19.2 MHz then:
                                                         * USBDRD()_UAHC_GUCTL[REFCLKPER] = 52.
                                                         * [GFLADJ_REFCLK_240MHZ_DECR] = (240/19.2) = 12.5.
                                                         * [GFLADJ_REFCLK_240MHZDECR_PLS1] = 1.
                                                         If the REF_CLK is 24 MHz then:
                                                         * USBDRD()_UAHC_GUCTL[REFCLKPER] = 41.
                                                         * [GFLADJ_REFCLK_240MHZ_DECR] = (240/24) = 10.
                                                         * [GFLADJ_REFCLK_240MHZDECR_PLS1] = 0. */
	uint32_t gfladj_refclk_240mhz_decr    : 7;  /**< This field indicates the decrement value that the controller applies for each REF_CLK in
                                                         order to derive a frame timer in terms of a 240-MHz clock. This field must be programmed
                                                         to a nonzero value only if [GFLADJ_REFCLK_LPM_SEL] is set to 1.
                                                         The value is derived as follows:
                                                         _ [GFLADJ_REFCLK_240MHZ_DECR] = 240/ref_clk_frequency
                                                         Examples:
                                                         If the REF_CLK is 24 MHz then:
                                                         * USBDRD()_UAHC_GUCTL[REFCLKPER] = 41.
                                                         * [GFLADJ_REFCLK_240MHZ_DECR] = 240/24 = 10.
                                                         If the REF_CLK is 48 MHz then:
                                                         * USBDRD()_UAHC_GUCTL[REFCLKPER] = 20.
                                                         * [GFLADJ_REFCLK_240MHZ_DECR] = 240/48 = 5.
                                                         If the REF_CLK is 17 MHz then:
                                                         * USBDRD()_UAHC_GUCTL[REFCLKPER] = 58.
                                                         * [GFLADJ_REFCLK_240MHZ_DECR] = 240/17 = 14. */
	uint32_t gfladj_refclk_lpm_sel        : 1;  /**< This bit enables the functionality of running SOF/ITP counters on the REF_CLK.
                                                         This bit must not be set to 1 if USBDRD()_UAHC_GCTL[SOFITPSYNC] = 1. Similarly, if
                                                         [GFLADJ_REFCLK_LPM_SEL] = 1, USBDRD()_UAHC_GCTL[SOFITPSYNC] must not be set to 1.
                                                         When [GFLADJ_REFCLK_LPM_SEL] = 1 the overloading of the suspend control of the USB 2.0
                                                         first
                                                         port PHY (UTMI) with USB 3.0 port states is removed. Note that the REF_CLK frequencies
                                                         supported in this mode are 16/17/19.2/20/24/39.7/40 MHz. */
	uint32_t reserved_22_22               : 1;
	uint32_t gfladj_refclk_fladj          : 14; /**< This field indicates the frame length adjustment to be applied when SOF/ITP counter is
                                                         running off of the REF_CLK. This register value is used to adjust:.
                                                         * ITP interval when USBDRD()_UAHC_GCTL[SOFITPSYNC] = 1
                                                         * both SOF and ITP interval when [GFLADJ_REFCLK_LPM_SEL] = 1.
                                                         This field must be programmed to a nonzero value only if [GFLADJ_REFCLK_LPM_SEL] = 1 or
                                                         USBDRD()_UAHC_GCTL[SOFITPSYNC] = 1.
                                                         The value is derived as below:
                                                         _ FLADJ_REF_CLK_FLADJ = ((125000/ref_clk_period_integer) - (125000/ref_clk_period)) *
                                                         ref_clk_period
                                                         where,
                                                         * the ref_clk_period_integer is the integer value of the REF_CLK period got by truncating
                                                         the decimal (fractional) value that is programmed in USBDRD()_UAHC_GUCTL[REFCLKPER].
                                                         * the ref_clk_period is the REF_CLK period including the fractional value.
                                                         Examples:
                                                         If the REF_CLK is 24 MHz then:
                                                         * USBDRD()_UAHC_GUCTL[REFCLKPER] = 41.
                                                         * GLADJ_REFCLK_FLADJ = ((125000/41) -
                                                         (125000/41.6666)) * 41.6666 = 2032 (ignoring the fractional value).
                                                         If the REF_CLK is 48 MHz then:
                                                         * USBDRD()_UAHC_GUCTL[REFCLKPER] = 20.
                                                         * GLADJ_REFCLK_FLADJ = ((125000/20) -
                                                         (125000/20.8333)) * 20.8333 = 5208 (ignoring the fractional value). */
	uint32_t gfladj_30mhz_reg_sel         : 1;  /**< This field selects whether to use the input signal fladj_30mhz_reg or the [GFLADJ_30MHZ]
                                                         to
                                                         adjust the frame length for the SOF/ITP. When this bit is set to, 1, the controller uses
                                                         [GFLADJ_30MHZ] value 0x0, the controller uses the input signal fladj_30mhz_reg value. */
	uint32_t reserved_6_6                 : 1;
	uint32_t gfladj_30mhz                 : 6;  /**< This field indicates the value that is used for frame length adjustment instead of
                                                         considering from the sideband input signal fladj_30mhz_reg. This enables post-silicon
                                                         frame length adjustment in case the input signal fladj_30mhz_reg is connected to a wrong
                                                         value or is not valid. The controller uses this value if [GFLADJ_30MHZ_REG_SEL] = 1 and
                                                         the
                                                         SOF/ITP counters are running off of UTMI(ULPI) clock ([GFLADJ_REFCLK_LPM_SEL] = 0 and
                                                         USBDRD()_UAHC_GCTL[SOFITPSYNC] is 1 or 0). For details on how to set this value, refer to
                                                         section 5.2.4 Frame Length Adjustment Register (FLADJ) of the xHCI Specification. */
#else
	uint32_t gfladj_30mhz                 : 6;
	uint32_t reserved_6_6                 : 1;
	uint32_t gfladj_30mhz_reg_sel         : 1;
	uint32_t gfladj_refclk_fladj          : 14;
	uint32_t reserved_22_22               : 1;
	uint32_t gfladj_refclk_lpm_sel        : 1;
	uint32_t gfladj_refclk_240mhz_decr    : 7;
	uint32_t gfladj_refclk_240mhzdecr_pls1 : 1;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gfladj_s     cn70xx;
	struct cvmx_usbdrdx_uahc_gfladj_s     cn70xxp1;
	struct cvmx_usbdrdx_uahc_gfladj_s     cn73xx;
	struct cvmx_usbdrdx_uahc_gfladj_s     cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gfladj cvmx_usbdrdx_uahc_gfladj_t;

/**
 * cvmx_usbdrd#_uahc_ggpio
 *
 * The application can use this register for general purpose input and output ports or for
 * debugging.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_ggpio {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_ggpio_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t gpo                          : 16; /**< General purpose output. These outputs are not connected to anything. Can be used as scratch. */
	uint32_t gpi                          : 16; /**< General purpose input. These inputs are tied 0x0. */
#else
	uint32_t gpi                          : 16;
	uint32_t gpo                          : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_ggpio_s      cn70xx;
	struct cvmx_usbdrdx_uahc_ggpio_s      cn70xxp1;
	struct cvmx_usbdrdx_uahc_ggpio_s      cn73xx;
	struct cvmx_usbdrdx_uahc_ggpio_s      cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_ggpio cvmx_usbdrdx_uahc_ggpio_t;

/**
 * cvmx_usbdrd#_uahc_ghwparams0
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_usbdrdx_uahc_ghwparams0 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_ghwparams0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t awidth                       : 8;  /**< USB core bus-address width. */
	uint32_t sdwidth                      : 8;  /**< USB core bus slave-data width. */
	uint32_t mdwidth                      : 8;  /**< USB core bus master-data width. */
	uint32_t sbus_type                    : 2;  /**< USB core bus slave type: AXI. */
	uint32_t mbus_type                    : 3;  /**< USB core bus master type: AXI. */
	uint32_t mode                         : 3;  /**< Operation mode: 0x2: Dual-role device. */
#else
	uint32_t mode                         : 3;
	uint32_t mbus_type                    : 3;
	uint32_t sbus_type                    : 2;
	uint32_t mdwidth                      : 8;
	uint32_t sdwidth                      : 8;
	uint32_t awidth                       : 8;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_ghwparams0_s cn70xx;
	struct cvmx_usbdrdx_uahc_ghwparams0_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_ghwparams0_s cn73xx;
	struct cvmx_usbdrdx_uahc_ghwparams0_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_ghwparams0 cvmx_usbdrdx_uahc_ghwparams0_t;

/**
 * cvmx_usbdrd#_uahc_ghwparams1
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_usbdrdx_uahc_ghwparams1 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_ghwparams1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t en_dbc                       : 1;  /**< Enable debug capability. */
	uint32_t rm_opt_features              : 1;  /**< Remove optional features. */
	uint32_t sync_rst                     : 1;  /**< Synchronous reset coding. */
	uint32_t ram_bus_clks_sync            : 1;  /**< RAM_CLK and BUS_CLK are synchronous. */
	uint32_t mac_ram_clks_sync            : 1;  /**< MAC3_CLK and RAM_CLK are synchronous. */
	uint32_t mac_phy_clks_sync            : 1;  /**< MAC3_CLK and PHY_CLK are synchronous. */
	uint32_t en_pwropt                    : 2;  /**< Power optimization mode:
                                                         bit<0> = Clock-gating feature available.
                                                         bit<1> = Hibernation feature available. */
	uint32_t spram_typ                    : 1;  /**< SRAM type: one-port RAMs. */
	uint32_t num_rams                     : 2;  /**< Number of RAMs. */
	uint32_t device_num_int               : 6;  /**< Number of event buffers (and interrupts) in device-mode. */
	uint32_t aspacewidth                  : 3;  /**< Native interface address-space port width. */
	uint32_t reqinfowidth                 : 3;  /**< Native interface request/response-info port width. */
	uint32_t datainfowidth                : 3;  /**< Native interface data-info port width. */
	uint32_t burstwidth_m1                : 3;  /**< Width minus one of AXI length field. */
	uint32_t idwidth_m1                   : 3;  /**< Width minus one of AXI ID field. */
#else
	uint32_t idwidth_m1                   : 3;
	uint32_t burstwidth_m1                : 3;
	uint32_t datainfowidth                : 3;
	uint32_t reqinfowidth                 : 3;
	uint32_t aspacewidth                  : 3;
	uint32_t device_num_int               : 6;
	uint32_t num_rams                     : 2;
	uint32_t spram_typ                    : 1;
	uint32_t en_pwropt                    : 2;
	uint32_t mac_phy_clks_sync            : 1;
	uint32_t mac_ram_clks_sync            : 1;
	uint32_t ram_bus_clks_sync            : 1;
	uint32_t sync_rst                     : 1;
	uint32_t rm_opt_features              : 1;
	uint32_t en_dbc                       : 1;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_ghwparams1_s cn70xx;
	struct cvmx_usbdrdx_uahc_ghwparams1_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_ghwparams1_s cn73xx;
	struct cvmx_usbdrdx_uahc_ghwparams1_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_ghwparams1 cvmx_usbdrdx_uahc_ghwparams1_t;

/**
 * cvmx_usbdrd#_uahc_ghwparams2
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_usbdrdx_uahc_ghwparams2 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_ghwparams2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t userid                       : 32; /**< User ID. */
#else
	uint32_t userid                       : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_ghwparams2_s cn70xx;
	struct cvmx_usbdrdx_uahc_ghwparams2_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_ghwparams2_s cn73xx;
	struct cvmx_usbdrdx_uahc_ghwparams2_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_ghwparams2 cvmx_usbdrdx_uahc_ghwparams2_t;

/**
 * cvmx_usbdrd#_uahc_ghwparams3
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_usbdrdx_uahc_ghwparams3 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_ghwparams3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t cache_total_xfer_resources   : 8;  /**< Maximum number of transfer resources in the core. */
	uint32_t num_in_eps                   : 5;  /**< Maximum number of device-mode IN endpoints active. */
	uint32_t num_eps                      : 6;  /**< Number of device-mode single-directional endpoints. */
	uint32_t ulpi_carkit                  : 1;  /**< ULPI carkit is not supported. */
	uint32_t vendor_ctl_interface         : 1;  /**< UTMI+ PHY vendor control interface enabled. */
	uint32_t reserved_8_9                 : 2;
	uint32_t hsphy_dwidth                 : 2;  /**< Data width of the UTMI+ PHY interface: 0x2 = 8-or-16 bits. */
	uint32_t fsphy_interface              : 2;  /**< USB 1.1 full-speed serial transceiver interface. */
	uint32_t hsphy_interface              : 2;  /**< High-speed PHY interface: 0x1 = UTMI+. */
	uint32_t ssphy_interface              : 2;  /**< SuperSpeed PHY interface: 0x1 = PIPE3. */
#else
	uint32_t ssphy_interface              : 2;
	uint32_t hsphy_interface              : 2;
	uint32_t fsphy_interface              : 2;
	uint32_t hsphy_dwidth                 : 2;
	uint32_t reserved_8_9                 : 2;
	uint32_t vendor_ctl_interface         : 1;
	uint32_t ulpi_carkit                  : 1;
	uint32_t num_eps                      : 6;
	uint32_t num_in_eps                   : 5;
	uint32_t cache_total_xfer_resources   : 8;
	uint32_t reserved_31_31               : 1;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_ghwparams3_s cn70xx;
	struct cvmx_usbdrdx_uahc_ghwparams3_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_ghwparams3_s cn73xx;
	struct cvmx_usbdrdx_uahc_ghwparams3_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_ghwparams3 cvmx_usbdrdx_uahc_ghwparams3_t;

/**
 * cvmx_usbdrd#_uahc_ghwparams4
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_usbdrdx_uahc_ghwparams4 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_ghwparams4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t bmu_lsp_depth                : 4;  /**< Depth of the BMU-LSP status buffer. */
	uint32_t bmu_ptl_depth_m1             : 4;  /**< Depth of the BMU-PTL source/sink buffers minus 1. */
	uint32_t en_isoc_supt                 : 1;  /**< Isochronous support enabled. */
	uint32_t reserved_22_22               : 1;
	uint32_t ext_buff_control             : 1;  /**< Enables device external buffer control sideband controls. */
	uint32_t num_ss_usb_instances         : 4;  /**< Number of SuperSpeed bus instances. */
	uint32_t hiber_scratchbufs            : 4;  /**< Number of hibernation scratchpad buffers. */
	uint32_t reserved_6_12                : 7;
	uint32_t cache_trbs_per_transfer      : 6;  /**< Number of TRBs per transfer that can be cached. */
#else
	uint32_t cache_trbs_per_transfer      : 6;
	uint32_t reserved_6_12                : 7;
	uint32_t hiber_scratchbufs            : 4;
	uint32_t num_ss_usb_instances         : 4;
	uint32_t ext_buff_control             : 1;
	uint32_t reserved_22_22               : 1;
	uint32_t en_isoc_supt                 : 1;
	uint32_t bmu_ptl_depth_m1             : 4;
	uint32_t bmu_lsp_depth                : 4;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_ghwparams4_s cn70xx;
	struct cvmx_usbdrdx_uahc_ghwparams4_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_ghwparams4_s cn73xx;
	struct cvmx_usbdrdx_uahc_ghwparams4_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_ghwparams4 cvmx_usbdrdx_uahc_ghwparams4_t;

/**
 * cvmx_usbdrd#_uahc_ghwparams5
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_usbdrdx_uahc_ghwparams5 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_ghwparams5_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_28_31               : 4;
	uint32_t dfq_fifo_depth               : 6;  /**< Size of the BMU descriptor fetch-request queue. */
	uint32_t dwq_fifo_depth               : 6;  /**< Size of the BMU descriptor write queue. */
	uint32_t txq_fifo_depth               : 6;  /**< Size of the BMU TX request queue. */
	uint32_t rxq_fifo_depth               : 6;  /**< Size of the BMU RX request queue. */
	uint32_t bmu_busgm_depth              : 4;  /**< Depth of the BMU-BUSGM source/sink buffers. */
#else
	uint32_t bmu_busgm_depth              : 4;
	uint32_t rxq_fifo_depth               : 6;
	uint32_t txq_fifo_depth               : 6;
	uint32_t dwq_fifo_depth               : 6;
	uint32_t dfq_fifo_depth               : 6;
	uint32_t reserved_28_31               : 4;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_ghwparams5_s cn70xx;
	struct cvmx_usbdrdx_uahc_ghwparams5_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_ghwparams5_s cn73xx;
	struct cvmx_usbdrdx_uahc_ghwparams5_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_ghwparams5 cvmx_usbdrdx_uahc_ghwparams5_t;

/**
 * cvmx_usbdrd#_uahc_ghwparams6
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_usbdrdx_uahc_ghwparams6 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_ghwparams6_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ram0_depth                   : 16; /**< RAM0 depth. */
	uint32_t en_bus_filters               : 1;  /**< VBus filters support. */
	uint32_t en_bc                        : 1;  /**< Enable battery charging support. */
	uint32_t en_otg_ss                    : 1;  /**< Enable OTG SuperSpeed support. */
	uint32_t en_adp                       : 1;  /**< Enable ADP support. */
	uint32_t hnp_support                  : 1;  /**< HNP support. */
	uint32_t srp_support                  : 1;  /**< SRP support. */
	uint32_t reserved_8_9                 : 2;
	uint32_t en_fpga                      : 1;  /**< Enable FPGA implementation. */
	uint32_t en_dbg_ports                 : 1;  /**< Enable debug ports for FGPA. */
	uint32_t psq_fifo_depth               : 6;  /**< Size of the BMU protocol status queue. */
#else
	uint32_t psq_fifo_depth               : 6;
	uint32_t en_dbg_ports                 : 1;
	uint32_t en_fpga                      : 1;
	uint32_t reserved_8_9                 : 2;
	uint32_t srp_support                  : 1;
	uint32_t hnp_support                  : 1;
	uint32_t en_adp                       : 1;
	uint32_t en_otg_ss                    : 1;
	uint32_t en_bc                        : 1;
	uint32_t en_bus_filters               : 1;
	uint32_t ram0_depth                   : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_ghwparams6_s cn70xx;
	struct cvmx_usbdrdx_uahc_ghwparams6_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_ghwparams6_s cn73xx;
	struct cvmx_usbdrdx_uahc_ghwparams6_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_ghwparams6 cvmx_usbdrdx_uahc_ghwparams6_t;

/**
 * cvmx_usbdrd#_uahc_ghwparams7
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_usbdrdx_uahc_ghwparams7 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_ghwparams7_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ram2_depth                   : 16; /**< RAM2 depth. */
	uint32_t ram1_depth                   : 16; /**< RAM1 depth. */
#else
	uint32_t ram1_depth                   : 16;
	uint32_t ram2_depth                   : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_ghwparams7_s cn70xx;
	struct cvmx_usbdrdx_uahc_ghwparams7_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_ghwparams7_s cn73xx;
	struct cvmx_usbdrdx_uahc_ghwparams7_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_ghwparams7 cvmx_usbdrdx_uahc_ghwparams7_t;

/**
 * cvmx_usbdrd#_uahc_ghwparams8
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_usbdrdx_uahc_ghwparams8 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_ghwparams8_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dcache_depth_info            : 32; /**< Dcache depth. */
#else
	uint32_t dcache_depth_info            : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_ghwparams8_s cn70xx;
	struct cvmx_usbdrdx_uahc_ghwparams8_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_ghwparams8_s cn73xx;
	struct cvmx_usbdrdx_uahc_ghwparams8_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_ghwparams8 cvmx_usbdrdx_uahc_ghwparams8_t;

/**
 * cvmx_usbdrd#_uahc_gpmsts
 *
 * This debug register gives information on which event caused the hibernation exit. These
 * registers are for debug purposes. They provide debug information on the internal status and
 * state machines. Global debug registers have design-specific information, and are used by for
 * debugging purposes. These registers are not intended to be used by the customer. If any debug
 * assistance is needed for the silicon, contact customer support with a dump of these registers.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gpmsts {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gpmsts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t portsel                      : 4;  /**< This field selects the port number. Always 0x0. */
	uint32_t reserved_17_27               : 11;
	uint32_t u3wakeup                     : 5;  /**< This field gives the USB 3.0 port wakeup conditions.
                                                         bit<12> = Overcurrent detected.
                                                         bit<13> = Resume detected.
                                                         bit<14> = Connect detected.
                                                         bit<15> = Disconnect detected.
                                                         bit<16> = Last connection state. */
	uint32_t reserved_10_11               : 2;
	uint32_t u2wakeup                     : 10; /**< This field indicates the USB 2.0 port wakeup conditions.
                                                         bit<0> = Overcurrent detected.
                                                         bit<1> = Resume detected.
                                                         bit<2> = Connect detected.
                                                         bit<3> = Disconnect detected.
                                                         bit<4> = Last connection state.
                                                         bit<5> = ID change detected.
                                                         bit<6> = SRP request detected.
                                                         bit<7> = ULPI interrupt detected.
                                                         bit<8> = USB reset detected.
                                                         bit<9> = Resume detected changed. */
#else
	uint32_t u2wakeup                     : 10;
	uint32_t reserved_10_11               : 2;
	uint32_t u3wakeup                     : 5;
	uint32_t reserved_17_27               : 11;
	uint32_t portsel                      : 4;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gpmsts_s     cn70xx;
	struct cvmx_usbdrdx_uahc_gpmsts_s     cn70xxp1;
	struct cvmx_usbdrdx_uahc_gpmsts_s     cn73xx;
	struct cvmx_usbdrdx_uahc_gpmsts_s     cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gpmsts cvmx_usbdrdx_uahc_gpmsts_t;

/**
 * cvmx_usbdrd#_uahc_gprtbimap
 *
 * This register specifies the SuperSpeed USB instance number to which each USB 3.0 port is
 * connected. By default, USB 3.0 ports are evenly distributed among all SuperSpeed USB
 * instances. Software can program this register to specify how USB 3.0 ports are connected to
 * SuperSpeed USB instances. The UAHC only implements one SuperSpeed bus-instance, so this
 * register should always be 0.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gprtbimap {
	uint64_t u64;
	struct cvmx_usbdrdx_uahc_gprtbimap_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t binum1                       : 4;  /**< SuperSpeed USB instance number for port 1. */
#else
	uint64_t binum1                       : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gprtbimap_s  cn70xx;
	struct cvmx_usbdrdx_uahc_gprtbimap_s  cn70xxp1;
	struct cvmx_usbdrdx_uahc_gprtbimap_s  cn73xx;
	struct cvmx_usbdrdx_uahc_gprtbimap_s  cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gprtbimap cvmx_usbdrdx_uahc_gprtbimap_t;

/**
 * cvmx_usbdrd#_uahc_gprtbimap_fs
 *
 * This register specifies the full-speed/low-speed USB instance number to which each USB 1.1
 * port is connected. By default, USB 1.1 ports are evenly distributed among all full-speed/
 * low-speed USB instances. Software can program this register to specify how USB 1.1 ports are
 * connected to full-speed/low-speed USB instances. The UAHC only implements one full-speed/
 * low-speed bus-instance, so this register should always be 0x0.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gprtbimap_fs {
	uint64_t u64;
	struct cvmx_usbdrdx_uahc_gprtbimap_fs_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t binum1                       : 4;  /**< Full-speed USB instance number for port 1. */
#else
	uint64_t binum1                       : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gprtbimap_fs_s cn70xx;
	struct cvmx_usbdrdx_uahc_gprtbimap_fs_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_gprtbimap_fs_s cn73xx;
	struct cvmx_usbdrdx_uahc_gprtbimap_fs_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gprtbimap_fs cvmx_usbdrdx_uahc_gprtbimap_fs_t;

/**
 * cvmx_usbdrd#_uahc_gprtbimap_hs
 *
 * This register specifies the high-speed USB instance number to which each USB 2.0 port is
 * connected. By default, USB 2.0 ports are evenly distributed among all high-speed USB
 * instances. Software can program this register to specify how USB 2.0 ports are connected to
 * high-speed USB instances. The UAHC only implements one high-speed bus-instance, so this
 * register should always be 0.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gprtbimap_hs {
	uint64_t u64;
	struct cvmx_usbdrdx_uahc_gprtbimap_hs_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t binum1                       : 4;  /**< High-speed USB instance number for port 1. */
#else
	uint64_t binum1                       : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gprtbimap_hs_s cn70xx;
	struct cvmx_usbdrdx_uahc_gprtbimap_hs_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_gprtbimap_hs_s cn73xx;
	struct cvmx_usbdrdx_uahc_gprtbimap_hs_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gprtbimap_hs cvmx_usbdrdx_uahc_gprtbimap_hs_t;

/**
 * cvmx_usbdrd#_uahc_grlsid
 *
 * This is a read-only register that contains the release number of the core.
 *
 */
union cvmx_usbdrdx_uahc_grlsid {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_grlsid_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t releaseid                    : 32; /**< Software can use this register to configure release-specific features in the driver. */
#else
	uint32_t releaseid                    : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_grlsid_s     cn70xx;
	struct cvmx_usbdrdx_uahc_grlsid_s     cn70xxp1;
	struct cvmx_usbdrdx_uahc_grlsid_s     cn73xx;
	struct cvmx_usbdrdx_uahc_grlsid_s     cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_grlsid cvmx_usbdrdx_uahc_grlsid_t;

/**
 * cvmx_usbdrd#_uahc_grxfifoprihst
 *
 * This register specifies the relative DMA priority level among the host RXFIFOs (one per USB
 * bus instance) within the associated speed group (SuperSpeed or high-speed/full-speed/
 * low-speed). When multiple RXFIFOs compete for DMA service at a given time, the RXDMA arbiter
 * grants access on a packet-basis in the following manner:
 *
 * Among the FIFOs in the same speed group (SuperSpeed or high-speed/full-speed/low-speed):
 * * High-priority RXFIFOs are granted access using round-robin arbitration.
 * * Low-priority RXFIFOs are granted access using round-robin arbitration only after high-
 * priority
 * RXFIFOs have no further processing to do (i.e., either the RXQs are empty or the corresponding
 * RXFIFOs do not have the required data).
 *
 * The RX DMA arbiter prioritizes the SuperSpeed group or high-speed/full-speed/low-speed group
 * according to the ratio programmed in
 * USBDRD()_UAHC_GDMAHLRATIO.
 *
 * For scatter-gather packets, the arbiter grants successive DMA requests to the same FIFO until
 * the entire packet is completed. The register size corresponds to the number of configured USB
 * bus instances; for example, in the default configuration, there are three USB bus instances (one
 * SuperSpeed, one high-speed, and one full-speed/low-speed).
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_grxfifoprihst {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_grxfifoprihst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_3_31                : 29;
	uint32_t rx_priority                  : 3;  /**< Each register bit[n] controls the priority (1 = high, 0 = low) of RXFIFO[n] within a speed group. */
#else
	uint32_t rx_priority                  : 3;
	uint32_t reserved_3_31                : 29;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_grxfifoprihst_s cn70xx;
	struct cvmx_usbdrdx_uahc_grxfifoprihst_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_grxfifoprihst_s cn73xx;
	struct cvmx_usbdrdx_uahc_grxfifoprihst_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_grxfifoprihst cvmx_usbdrdx_uahc_grxfifoprihst_t;

/**
 * cvmx_usbdrd#_uahc_grxfifosiz#
 *
 * The application can program the internal RAM start address/depth of the each RxFIFO as shown
 * below. It is recommended that software use the default value. In Host mode, per-port registers
 * are implemented. One register per FIFO.
 *
 * Reset values = 0:[0x0000_0084] 1:[0x0084_0104] 2:[0x0188_0180].
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_grxfifosizx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_grxfifosizx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t rxfstaddr                    : 16; /**< RxFIFOn RAM start address. This field contains the memory start address for RxFIFOn. The
                                                         reset value is derived from configuration parameters. */
	uint32_t rxfdep                       : 16; /**< RxFIFOn depth. This value is in terms of RX RAM Data width.
                                                         minimum value = 0x20, maximum value = 0x4000. */
#else
	uint32_t rxfdep                       : 16;
	uint32_t rxfstaddr                    : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_grxfifosizx_s cn70xx;
	struct cvmx_usbdrdx_uahc_grxfifosizx_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_grxfifosizx_s cn73xx;
	struct cvmx_usbdrdx_uahc_grxfifosizx_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_grxfifosizx cvmx_usbdrdx_uahc_grxfifosizx_t;

/**
 * cvmx_usbdrd#_uahc_grxthrcfg
 *
 * In a normal case, an RX burst starts as soon as 1-packet space is available. This works well
 * as long as the system bus is faster than the USB3.0 bus (a 1024-bytes packet takes ~2.2 us on
 * the USB bus in SuperSpeed mode). If the system bus latency is larger than 2.2 us to access a
 * 1024-byte packet, then starting a burst on 1-packet condition leads to an early abort of the
 * burst causing unnecessary performance reduction. This register allows the configuration of
 * threshold and burst size control. This feature is enabled by USBRXPKTCNTSEL.
 *
 * Receive Path:
 * * The RX threshold is controlled by USBRXPKTCNT and the RX burst size is controlled by
 * USBMAXRXBURSTSIZE.
 * * Selecting optimal RX FIFO size, RX threshold, and RX burst size avoids RX burst aborts due
 * to overrun if the system bus is slower than USB. Once in a while overrun is OK, and there is
 * no functional issue.
 * * Some devices do not support terminating ACK retry. With these devices, host cannot set ACK=0
 * and Retry=0 and do retry later and you have to retry immediately. For such devices, minimize
 * retry due to underrun. Setting threshold and burst size guarantees this.
 * A larger RX threshold affects the performance since the scheduler is idle during this time.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_grxthrcfg {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_grxthrcfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t usbrxpktcntsel               : 1;  /**< USB receive-packet-count enable. Enables/disables the USB reception multipacket
                                                         thresholding:
                                                         0 = the core can only start reception on the USB when the RX FIFO has space for at least
                                                         one packet.
                                                         1 = the core can only start reception on the USB when the RX FIFO has space for at least
                                                         USBRXPKTCNT amount of packets.
                                                         This mode is only used for SuperSpeed.
                                                         In device mode, setting this bit to 1 also enables the functionality of reporting
                                                         NUMP in the ACK TP based on the RX FIFO space instead of reporting a fixed NUMP derived
                                                         from USBDRD()_UAHC_DCFG[NUMP]. */
	uint32_t reserved_28_28               : 1;
	uint32_t usbrxpktcnt                  : 4;  /**< USB receive-packet count.
                                                         In host-mode, specifies space (in number of packets) that must be available in
                                                         the RX FIFO before the core can start the corresponding USB RX transaction (burst).
                                                         In device mode, specifies the space (in number of packets) that must be available in
                                                         the RX FIFO before the core can send ERDY for a flow-controlled endpoint.
                                                         This field is only valid when [USBRXPKTCNTSEL] = 1. The valid values are from 0x1 to 0xF.
                                                         This field must be <= [USBMAXRXBURSTSIZE]. */
	uint32_t usbmaxrxburstsize            : 5;  /**< USB maximum receive-burst size. In host-mode, specifies the maximum bulk IN burst the core
                                                         should do. When the system bus is slower than the USB, RX FIFO can overrun during a long
                                                         burst.
                                                         Program a smaller value to this field to limit the RX burst size that the core can do. It
                                                         only applies to SuperSpeed Bulk, Isochronous, and Interrupt IN endpoints in the host mode.
                                                         In device mode, specified the NUMP value that will be sent in ERDy for an OUT endpoint.
                                                         This field is only valid when [USBRXPKTCNTSEL] = 1. The valid values are from 0x1 to 0x10. */
	uint32_t reserved_0_18                : 19;
#else
	uint32_t reserved_0_18                : 19;
	uint32_t usbmaxrxburstsize            : 5;
	uint32_t usbrxpktcnt                  : 4;
	uint32_t reserved_28_28               : 1;
	uint32_t usbrxpktcntsel               : 1;
	uint32_t reserved_30_31               : 2;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_grxthrcfg_s  cn70xx;
	struct cvmx_usbdrdx_uahc_grxthrcfg_s  cn70xxp1;
	struct cvmx_usbdrdx_uahc_grxthrcfg_s  cn73xx;
	struct cvmx_usbdrdx_uahc_grxthrcfg_s  cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_grxthrcfg cvmx_usbdrdx_uahc_grxthrcfg_t;

/**
 * cvmx_usbdrd#_uahc_gsbuscfg0
 *
 * This register can be used to configure the core after power-on or a change in mode of
 * operation. This register mainly contains AXI system-related configuration parameters. Do not
 * change this register after the initial programming. The application must program this register
 * before starting any transactions on AXI. When [INCRBRSTENA] is enabled, it has the highest
 * priority over other burst lengths. The core always performs the largest burst when enabled.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gsbuscfg0 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gsbuscfg0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t datrdreqinfo                 : 4;  /**< AXI-cache for data-read operations. Always set to 0x0. */
	uint32_t desrdreqinfo                 : 4;  /**< AXI-cache for descriptor-read operations. Always set to 0x0. */
	uint32_t datwrreqinfo                 : 4;  /**< AXI-cache for data-write operations. Always set to 0x0. */
	uint32_t deswrreqinfo                 : 4;  /**< AXI-cache for descriptor-write operations. Always set to 0x0. */
	uint32_t reserved_12_15               : 4;
	uint32_t datbigend                    : 1;  /**< Data access is big-endian. Keep this set to 0 (little-endian) and use the
                                                         USBDRD()_UCTL_SHIM_CFG[DMA_ENDIAN_MODE] setting instead. */
	uint32_t descbigend                   : 1;  /**< Descriptor access is big-endian. Keep this set to 0 (little-endian) and use the
                                                         USBDRD()_UCTL_SHIM_CFG[DMA_ENDIAN_MODE] setting instead. */
	uint32_t reserved_8_9                 : 2;
	uint32_t incr256brstena               : 1;  /**< INCR256 burst-type enable. Always set to 0. */
	uint32_t incr128brstena               : 1;  /**< INCR128 burst-type enable. Always set to 0. */
	uint32_t incr64brstena                : 1;  /**< INCR64 burst-type enable. Always set to 0. */
	uint32_t incr32brstena                : 1;  /**< INCR32 burst-type enable. Always set to 0. */
	uint32_t incr16brstena                : 1;  /**< INCR16 burst-type enable. Allows the AXI master to generate INCR 16-beat bursts. */
	uint32_t incr8brstena                 : 1;  /**< INCR8 burst-type enable. Allows the AXI master to generate INCR eight-beat bursts. */
	uint32_t incr4brstena                 : 1;  /**< INCR4 burst-type enable. Allows the AXI master to generate INCR four-beat bursts. */
	uint32_t incrbrstena                  : 1;  /**< Undefined-length INCR burst-type enable.
                                                         This bit determines the set of burst lengths to be utilized by the master interface. It
                                                         works in conjunction with the GSBUSCFG0[7:1] enables (INCR*BRSTENA).
                                                         If disabled, the AXI master will use only the burst lengths
                                                         1, 4, 8, 16 (assuming the INCR*BRSTENA are set to their reset values).
                                                         If enabled, the AXI master uses any length less than or equal to the largest-enabled burst
                                                         length based on the INCR*BRSTENA fields. */
#else
	uint32_t incrbrstena                  : 1;
	uint32_t incr4brstena                 : 1;
	uint32_t incr8brstena                 : 1;
	uint32_t incr16brstena                : 1;
	uint32_t incr32brstena                : 1;
	uint32_t incr64brstena                : 1;
	uint32_t incr128brstena               : 1;
	uint32_t incr256brstena               : 1;
	uint32_t reserved_8_9                 : 2;
	uint32_t descbigend                   : 1;
	uint32_t datbigend                    : 1;
	uint32_t reserved_12_15               : 4;
	uint32_t deswrreqinfo                 : 4;
	uint32_t datwrreqinfo                 : 4;
	uint32_t desrdreqinfo                 : 4;
	uint32_t datrdreqinfo                 : 4;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gsbuscfg0_s  cn70xx;
	struct cvmx_usbdrdx_uahc_gsbuscfg0_s  cn70xxp1;
	struct cvmx_usbdrdx_uahc_gsbuscfg0_s  cn73xx;
	struct cvmx_usbdrdx_uahc_gsbuscfg0_s  cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gsbuscfg0 cvmx_usbdrdx_uahc_gsbuscfg0_t;

/**
 * cvmx_usbdrd#_uahc_gsbuscfg1
 *
 * This register can be used to configure the core after power-on or a change in mode of
 * operation. This register mainly contains AXI system-related configuration parameters. Do not
 * change this register after the initial programming. The application must program this register
 * before starting any transactions on AXI.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gsbuscfg1 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gsbuscfg1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_13_31               : 19;
	uint32_t en1kpage                     : 1;  /**< 1K page-boundary enable.
                                                         0 = Break transfers at the 4K page boundary (default).
                                                         1 = Break transfers at the 1K page boundary. */
	uint32_t pipetranslimit               : 4;  /**< AXI pipelined transfers burst-request limit. Controls the number of outstanding pipelined
                                                         transfers requests the AXI master will push to the AXI slave. Once the AXI master reaches
                                                         this limit, it does not make more requests on the AXI ARADDR and AWADDR buses until the
                                                         associated data phases complete. This field is encoded as follows:
                                                         0x0 = 1 request. 0x8 = 9 requests.
                                                         0x1 = 2 requests. 0x9 = 10 requests.
                                                         0x2 = 3 requests. 0xA = 11 requests.
                                                         0x3 = 4 requests. 0xB = 12 requests.
                                                         0x4 = 5 requests. 0xC = 13 requests.
                                                         0x5 = 6 requests. 0xD = 14 requests.
                                                         0x6 = 7 requests. 0xE = 15 requests.
                                                         0x7 = 8 requests. 0xF = 16 requests. */
	uint32_t reserved_0_7                 : 8;
#else
	uint32_t reserved_0_7                 : 8;
	uint32_t pipetranslimit               : 4;
	uint32_t en1kpage                     : 1;
	uint32_t reserved_13_31               : 19;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gsbuscfg1_s  cn70xx;
	struct cvmx_usbdrdx_uahc_gsbuscfg1_s  cn70xxp1;
	struct cvmx_usbdrdx_uahc_gsbuscfg1_s  cn73xx;
	struct cvmx_usbdrdx_uahc_gsbuscfg1_s  cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gsbuscfg1 cvmx_usbdrdx_uahc_gsbuscfg1_t;

/**
 * cvmx_usbdrd#_uahc_gsts
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 *
 */
union cvmx_usbdrdx_uahc_gsts {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gsts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t cbelt                        : 12; /**< Current BELT value. In host mode, indicates the minimum value of all received device BELT
                                                         values and the BELT value that is set by the set latency tolerance value command. */
	uint32_t reserved_8_19                : 12;
	uint32_t host_ip                      : 1;  /**< Host interrupt pending. Indicates that there is a pending interrupt pertaining to xHC in
                                                         the host-event queue. */
	uint32_t device_ip                    : 1;  /**< Device interrupt pending. Indicates that there is a pending interrupt pertaining to
                                                         peripheral (device) operation in the Device event queue. */
	uint32_t csrtimeout                   : 1;  /**< CSR timeout. When set to 1, indicates that software performed a write or read operation to
                                                         a core register that could not be completed within 0xFFFF controller-clock cycles. */
	uint32_t buserraddrvld                : 1;  /**< Bus-error address valid. Indicates that USBDRD()_UAHC_GBUSERRADDR is valid and reports the
                                                         first bus address that encounters a bus error. */
	uint32_t reserved_2_3                 : 2;
	uint32_t curmod                       : 2;  /**< Current mode of operation. 0x0 for device, 0x1 for host. */
#else
	uint32_t curmod                       : 2;
	uint32_t reserved_2_3                 : 2;
	uint32_t buserraddrvld                : 1;
	uint32_t csrtimeout                   : 1;
	uint32_t device_ip                    : 1;
	uint32_t host_ip                      : 1;
	uint32_t reserved_8_19                : 12;
	uint32_t cbelt                        : 12;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gsts_s       cn70xx;
	struct cvmx_usbdrdx_uahc_gsts_s       cn70xxp1;
	struct cvmx_usbdrdx_uahc_gsts_s       cn73xx;
	struct cvmx_usbdrdx_uahc_gsts_s       cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gsts cvmx_usbdrdx_uahc_gsts_t;

/**
 * cvmx_usbdrd#_uahc_gtxfifopridev
 *
 * This register specifies the relative DMA priority level among the Device TXFIFOs (one per IN
 * endpoint).
 * Each register bit[n] controls the priority (1: high, 0: low) of each TXFIFO[n]. When multiple
 * TXFIFOs compete for DMA service at a given time (i.e., multiple TXQs contain TX DMA requests
 * and their corresponding TXFIFOs have space available), the TX DMA arbiter grants access on a
 * packet-basis in the following manner:
 *
 *   1. High-priority TXFIFOs are granted access using round-robin arbitration
 *
 *   2. Low-priority TXFIFOs are granted access using round-robin arbitration only after the
 *      high-priority TXFIFOs have no further processing to do (i.e., either the TXQs are empty
 *      or the corresponding TXFIFOs are full).
 *
 * For scatter-gather packets, the arbiter grants successive DMA requests to the same FIFO until
 * the entire packet is completed.
 *
 * When configuring periodic IN endpoints, software must set register bit[n]=1, where n is the
 * TXFIFO assignment. This ensures that the DMA for isochronous or interrupt IN endpoints are
 * prioritized over bulk or control IN endpoints.
 *
 * This register is present only when the core is configured to operate in the device mode
 * (includes DRD and OTG modes). The register size corresponds to the number of Device IN
 * endpoints.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET].
 */
union cvmx_usbdrdx_uahc_gtxfifopridev {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gtxfifopridev_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_4_31                : 28;
	uint32_t tx_priority                  : 4;  /**< Each register bit[n] controls the priority (1: high, 0: low) of TXFIFO[n] within a speed
                                                         group. */
#else
	uint32_t tx_priority                  : 4;
	uint32_t reserved_4_31                : 28;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gtxfifopridev_s cn70xx;
	struct cvmx_usbdrdx_uahc_gtxfifopridev_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_gtxfifopridev_s cn73xx;
	struct cvmx_usbdrdx_uahc_gtxfifopridev_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gtxfifopridev cvmx_usbdrdx_uahc_gtxfifopridev_t;

/**
 * cvmx_usbdrd#_uahc_gtxfifoprihst
 *
 * This register specifies the relative DMA priority level among the Host TXFIFOs (one per USB
 * bus instance) within the associated speed group (SuperSpeed or HighSpeed/FullSpeed/LowSpeed).
 * When multiple TXFIFOs compete for DMA service at a given time, the TXDMA arbiter grants access
 * on a packet-basis in the following manner:
 *
 *   1. Among the FIFOs in the same speed group (SuperSpeed or HighSpeed/FullSpeed/LowSpeed):
 *
 * _   a. High-priority TXFIFOs are granted access using round-robin arbitration
 *
 * _   b. Low-priority TXFIFOs are granted access using round-robin arbitration only after the
 *        high priority TXFIFOs have no further processing to do (i.e., either the TXQs are empty
 *        or thecorresponding TXFIFOs are full).
 *
 *   2. The TX DMA arbiter prioritizes the SuperSpeed group or HighSpeed/FullSpeed/LowSpeed group
 *      according to the ratio programmed in the USBDRD()_UAHC_GDMAHLRATIO register.
 *
 * For scatter-gather packets, the arbiter grants successive DMA requests to the same FIFO until
 * the entire packet is completed.
 * The register size corresponds to the number of configured USB bus instances; for example, in
 * the default configuration, there are 3 USB bus instances (1 SuperSpeed, 1 HighSpeed, and 1
 * FullSpeed/LowSpeed).
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET].
 */
union cvmx_usbdrdx_uahc_gtxfifoprihst {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gtxfifoprihst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_3_31                : 29;
	uint32_t tx_priority                  : 3;  /**< Each register bit n controls the priority (1: high, 0: low) of TX FIFO<n> within a speed
                                                         group. */
#else
	uint32_t tx_priority                  : 3;
	uint32_t reserved_3_31                : 29;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gtxfifoprihst_s cn70xx;
	struct cvmx_usbdrdx_uahc_gtxfifoprihst_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_gtxfifoprihst_s cn73xx;
	struct cvmx_usbdrdx_uahc_gtxfifoprihst_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gtxfifoprihst cvmx_usbdrdx_uahc_gtxfifoprihst_t;

/**
 * cvmx_usbdrd#_uahc_gtxfifosiz#
 *
 * This register holds the internal RAM start address/depth of each TxFIFO implemented. Unless
 * packet size/buffer size for each endpoint is different and application-specific, it is
 * recommended that the software use the default value. One register per FIFO. One register per
 * FIFO.
 *
 * Reset values = 0:[0x0000_0082] 1:[0x0082_0103] 2:[0x0185_0205].
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gtxfifosizx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gtxfifosizx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t txfstaddr                    : 16; /**< Transmit FIFOn RAM start address. Contains the memory start address for TxFIFOn. The reset
                                                         is value derived from configuration parameters. */
	uint32_t txfdep                       : 16; /**< TxFIFOn depth. This value is in terms of TX RAM data width.
                                                         minimum value = 0x20, maximum value = 0x8000. */
#else
	uint32_t txfdep                       : 16;
	uint32_t txfstaddr                    : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gtxfifosizx_s cn70xx;
	struct cvmx_usbdrdx_uahc_gtxfifosizx_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_gtxfifosizx_s cn73xx;
	struct cvmx_usbdrdx_uahc_gtxfifosizx_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gtxfifosizx cvmx_usbdrdx_uahc_gtxfifosizx_t;

/**
 * cvmx_usbdrd#_uahc_gtxthrcfg
 *
 * In a normal case, a TX burst starts as soon as one packet is prefetched. This works well as
 * long as the system bus is faster than the USB3.0 bus (a 1024-bytes packet takes ~2.2 us on the
 * USB bus in SuperSpeed mode). If the system bus latency is larger than 2.2 us to access a
 * 1024-byte packet, then starting a burst on 1-packet condition leads to an early abort of the
 * burst causing unnecessary performance reduction. This register allows the configuration of
 * threshold and burst size control. This feature is enabled by [USBTXPKTCNTSEL].
 *
 * Transmit path:
 * * The TX threshold is controlled by [USBTXPKTCNT], and the TX burst size is controlled by
 * [USBMAXTXBURSTSIZE].
 * * Selecting optimal TX FIFO size, TX threshold, and TX burst size avoids TX burst aborts due
 * to an underrun if the system bus is slower than USB. Once in a while an underrun is OK, and
 * there is no functional issue.
 * * A larger threshold affects the performance, since the scheduler is idle during this time.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gtxthrcfg {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gtxthrcfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t usbtxpktcntsel               : 1;  /**< USB transmit packet-count enable. Enables/disables the USB transmission multipacket
                                                         thresholding:
                                                         0 = USB transmission multipacket thresholding is disabled, the core can only start
                                                         transmission on the USB after the entire packet has been fetched into the corresponding
                                                         TXFIFO.
                                                         1 = USB transmission multipacket thresholding is enabled. The core can only start
                                                         transmission on the USB after USBTXPKTCNT amount of packets for the USB transaction
                                                         (burst) are already in the corresponding TXFIFO. This mode is only valid in host mode.
                                                         This mode is only used for SuperSpeed. */
	uint32_t reserved_28_28               : 1;
	uint32_t usbtxpktcnt                  : 4;  /**< USB transmit-packet count. Specifies the number of packets that must be in the TXFIFO
                                                         before the core can start transmission for the corresponding USB transaction (burst). This
                                                         field is only valid when [USBTXPKTCNTSEL] = 1. Valid values are from 0x1 to 0xF.
                                                         This field must be <= [USBMAXTXBURSTSIZE]. */
	uint32_t usbmaxtxburstsize            : 8;  /**< USB maximum TX burst size. When [USBTXPKTCNTSEL] = 1, this field specifies the
                                                         maximum bulk OUT burst the core should do. When the system bus is slower than
                                                         the USB, TX FIFO can underrun during a long burst. Program a smaller value to
                                                         this field to limit the TX burst size that the core can do. It only applies to
                                                         SuperSpeed bulk, isochronous, and interrupt OUT endpoints in the host
                                                         mode. Valid values are from 0x1 to 0x10. */
	uint32_t reserved_0_15                : 16;
#else
	uint32_t reserved_0_15                : 16;
	uint32_t usbmaxtxburstsize            : 8;
	uint32_t usbtxpktcnt                  : 4;
	uint32_t reserved_28_28               : 1;
	uint32_t usbtxpktcntsel               : 1;
	uint32_t reserved_30_31               : 2;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gtxthrcfg_s  cn70xx;
	struct cvmx_usbdrdx_uahc_gtxthrcfg_s  cn70xxp1;
	struct cvmx_usbdrdx_uahc_gtxthrcfg_s  cn73xx;
	struct cvmx_usbdrdx_uahc_gtxthrcfg_s  cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gtxthrcfg cvmx_usbdrdx_uahc_gtxthrcfg_t;

/**
 * cvmx_usbdrd#_uahc_guctl
 *
 * This register provides a few options for the software to control the core behavior in the host
 * mode. Most of the options are used to improve host inter-operability with different devices.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_guctl {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_guctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t refclkper                    : 10; /**< Reference-clock period. Indicates (in terms of ns) the period of REF_CLK. The default
                                                         value is set to 0x8
                                                         (8 ns/125 MHz). This field must be updated during power on initialization if
                                                         USBDRD()_UAHC_GCTL[SOFITPSYNC] = 1 or USBDRD()_UAHC_GFLADJ [GFLADJ_REFCLK_LPM_SEL] = 1.
                                                         The
                                                         programmable maximum value 62 ns, and the minimum value is 8 ns. You use a reference clock
                                                         with a period that is a integer multiple, so that ITP can meet the jitter margin of 32 ns.
                                                         The allowable REF_CLK frequencies whose period is not integer multiples are
                                                         16/17/19.2/24/39.7 MHz.
                                                         This field should not be set to 0x0 at any time. If you do not plan to use this feature,
                                                         then you need to set this field to 0x8, the default value. */
	uint32_t noextrdl                     : 1;  /**< No extra delay between SOF and the first packet.
                                                         Some high-speed devices misbehave when the host sends a packet immediately after an SOF.
                                                         However, adding an extra delay between an SOF and the first packet can reduce the USB data
                                                         rate and performance.
                                                         This bit is used to control whether the host should wait for 2 us before it sends the
                                                         first packet after a SOF, or not. You can set this bit to 1 to improve the performance if
                                                         those problematic devices are not a concern in your host environment.
                                                         0 = host waits for 2 us after an SOF before it sends the first USB packet.
                                                         1 = host does not wait after an SOF before it sends the first USB packet. */
	uint32_t psqextrressp                 : 3;  /**< PSQ extra reserved space. This is a debug feature, and is not intended for normal usage.
                                                         This parameter specifies how much additional space in the PSQ (protocol-status queue) must
                                                         be reserved before the U3PTL initiates a new USB transaction and burst beats. */
	uint32_t sprsctrltransen              : 1;  /**< Sparse control transaction enable. Some devices are slow in responding to control
                                                         transfers. Scheduling multiple transactions in one microframe/frame can cause these
                                                         devices to misbehave. If this bit is set to 1, the host controller schedules transactions
                                                         for a control transfer in different microframes/frames. */
	uint32_t resbwhseps                   : 1;  /**< Reserving 85% bandwidth for high-speed periodic EPs. By default, host controller reserves
                                                         80% of the bandwidth for periodic EPs. If this bit is set, the bandwidth is relaxed to 85%
                                                         to accommodate two high-speed, high-bandwidth ISOC EPs.
                                                         USB 2.0 required 80% bandwidth allocated for ISOC traffic. If two high bandwidth ISOC
                                                         devices (HD webcams) are connected, and if each requires 1024-bytes * 3 packets per
                                                         microframe, then the bandwidth required is around 82%. If this bit is set to 1, it is
                                                         possible to connect two webcams of 1024 bytes * 3 payload per microframe each. Otherwise,
                                                         you may have to reduce the resolution of the webcams. */
	uint32_t cmdevaddr                    : 1;  /**< Compliance mode for device address. When set to 1, slot ID can have different value than
                                                         device address if max_slot_enabled < 128.
                                                         0 = Device address is equal to slot ID.
                                                         1 = Increment device address on each address device command.
                                                         The xHCI compliance requires this bit to be set to 1. The 0 mode is for debug purpose
                                                         only. This allows you to easily identify a device connected to a port in the Lecroy or
                                                         Eliisys trace during hardware debug.
                                                         This bit is used in host mode only. */
	uint32_t reserved_14_14               : 1;
	uint32_t enoverlapchk                 : 1;  /**< Enable check for LFPS overlap during remote Ux Exit. If this bit is set to:
                                                         0 = When the link exists U1/U2/U3 because of a remote exit, it does not look for an LFPS
                                                         overlap.
                                                         1 = The SuperSpeed link, when exiting U1/U2/U3, waits for either the remote link LFPS or
                                                         TS1/TS2 training symbols before it confirms that the LFPS handshake is complete. This is
                                                         done to handle the case where the LFPS glitch causes the link to start exiting from the
                                                         low power state. Looking for the LFPS overlap makes sure that the link partner also sees
                                                         the LFPS. */
	uint32_t extcapsupten                 : 1;  /**< External extended capability support enable. If disabled, a read
                                                         USBDRD()_UAHC_SUPTPRT3_DW0
                                                         [NEXTCAPPTR] returns 0 in the next capability pointer field. This indicates there are no
                                                         more capabilities. If enabled, a read to USBDRD()_UAHC_SUPTPRT3_DW0[NEXTCAPPTR] returns 4
                                                         in
                                                         the
                                                         next capability pointer field.
                                                         Always set to 0x0. */
	uint32_t insrtextrfsbodi              : 1;  /**< Insert extra delay between full-speed bulk OUT transactions. Some full-speed devices are
                                                         slow to receive bulk OUT data and can get stuck when there are consecutive bulk OUT
                                                         transactions with short inter-transaction delays. This bit is used to control whether the
                                                         host inserts extra delay between consecutive bulk OUT transactions to a full-speed
                                                         endpoint.
                                                         0 = Host does not insert extra delay.
                                                         Setting this bit to 1 reduces the bulk OUT transfer performance for most of the full-speed
                                                         devices.
                                                         1 = Host inserts about 12 us extra delay between consecutive bulk OUT transactions to an
                                                         full-speed endpoint to work around the device issue. */
	uint32_t dtct                         : 2;  /**< Device timeout coarse tuning. This field determines how long the host waits for a response
                                                         from device before considering a timeout.
                                                         The core first checks the [DTCT] value. If it is 0, then the timeout value is defined by
                                                         the
                                                         [DTFT]. If it is nonzero, then it uses the following timeout values:
                                                         0x0 = 0 us; use [DTFT] value instead.
                                                         0x1 = 500 us.
                                                         0x2 = 1.5 ms.
                                                         0x3 = 6.5 ms. */
	uint32_t dtft                         : 9;  /**< Device timeout fine tuning. This field determines how long the host waits for a response
                                                         from a device before considering a timeout. For [DTFT] to take effect, [DTCT] must be set
                                                         to
                                                         0x0.
                                                         The [DTFT] value specifies the number of 125 MHz clock cycles * 256 to count before
                                                         considering a device timeout. For the 125 MHz clock cycles (8 ns period), this is
                                                         calculated as follows:
                                                         _ [DTFT value] * 256 * 8 (ns)
                                                         0x2 = 2 * 256 * 8 -> 4 us.
                                                         0x5 = 5 * 256 * 8 -> 10 us.
                                                         0xA = 10 * 256 * 8 -> 20 us.
                                                         0x10 = 16 * 256 * 8 -> 32 us.
                                                         0x19 = 25 * 256 * 8 -> 51 us.
                                                         0x31 = 49 * 256 * 8 -> 100 us.
                                                         0x62 = 98 * 256 * 8 -> 200 us. */
#else
	uint32_t dtft                         : 9;
	uint32_t dtct                         : 2;
	uint32_t insrtextrfsbodi              : 1;
	uint32_t extcapsupten                 : 1;
	uint32_t enoverlapchk                 : 1;
	uint32_t reserved_14_14               : 1;
	uint32_t cmdevaddr                    : 1;
	uint32_t resbwhseps                   : 1;
	uint32_t sprsctrltransen              : 1;
	uint32_t psqextrressp                 : 3;
	uint32_t noextrdl                     : 1;
	uint32_t refclkper                    : 10;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_guctl_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t refclkper                    : 10; /**< Reference-clock period. Indicates (in terms of ns) the period of REF_CLK. The default
                                                         value is set to 0x8
                                                         (8 ns/125 MHz). This field must be updated during power on initialization if
                                                         USBDRD()_UAHC_GCTL[SOFITPSYNC] = 1 or USBDRD()_UAHC_GFLADJ [GFLADJ_REFCLK_LPM_SEL] = 1.
                                                         The
                                                         programmable maximum value 62 ns, and the minimum value is 8 ns. You use a reference clock
                                                         with a period that is a integer multiple, so that ITP can meet the jitter margin of 32 ns.
                                                         The allowable REF_CLK frequencies whose period is not integer multiples are
                                                         16/17/19.2/24/39.7 MHz.
                                                         This field should not be set to 0x0 at any time. If you do not plan to use this feature,
                                                         then you need to set this field to 0x8, the default value. */
	uint32_t noextrdl                     : 1;  /**< No extra delay between SOF and the first packet.
                                                         Some high-speed devices misbehave when the host sends a packet immediately after an SOF.
                                                         However, adding an extra delay between an SOF and the first packet can reduce the USB data
                                                         rate and performance.
                                                         This bit is used to control whether the host should wait for 2 us before it sends the
                                                         first packet after a SOF, or not. You can set this bit to 1 to improve the performance if
                                                         those problematic devices are not a concern in your host environment.
                                                         0 = host waits for 2 us after an SOF before it sends the first USB packet.
                                                         1 = host does not wait after an SOF before it sends the first USB packet. */
	uint32_t psqextrressp                 : 3;  /**< PSQ extra reserved space. This is a debug feature, and is not intended for normal usage.
                                                         This parameter specifies how much additional space in the PSQ (protocol-status queue) must
                                                         be reserved before the U3PTL initiates a new USB transaction and burst beats. */
	uint32_t sprsctrltransen              : 1;  /**< Sparse control transaction enable. Some devices are slow in responding to control
                                                         transfers. Scheduling multiple transactions in one microframe/frame can cause these
                                                         devices to misbehave. If this bit is set to 1, the host controller schedules transactions
                                                         for a control transfer in different microframes/frames. */
	uint32_t resbwhseps                   : 1;  /**< Reserving 85% bandwidth for high-speed periodic EPs. By default, host controller reserves
                                                         80% of the bandwidth for periodic EPs. If this bit is set, the bandwidth is relaxed to 85%
                                                         to accommodate two high-speed, high-bandwidth ISOC EPs.
                                                         USB 2.0 required 80% bandwidth allocated for ISOC traffic. If two high bandwidth ISOC
                                                         devices (HD webcams) are connected, and if each requires 1024-bytes * 3 packets per
                                                         microframe, then the bandwidth required is around 82%. If this bit is set to 1, it is
                                                         possible to connect two webcams of 1024 bytes * 3 payload per microframe each. Otherwise,
                                                         you may have to reduce the resolution of the webcams. */
	uint32_t cmdevaddr                    : 1;  /**< Compliance mode for device address. When set to 1, slot ID can have different value than
                                                         device address if max_slot_enabled < 128.
                                                         0 = Device address is equal to slot ID.
                                                         1 = Increment device address on each address device command.
                                                         The xHCI compliance requires this bit to be set to 1. The 0 mode is for debug purpose
                                                         only. This allows you to easily identify a device connected to a port in the Lecroy or
                                                         Eliisys trace during hardware debug.
                                                         This bit is used in host mode only. */
	uint32_t usbhstinautoretryen          : 1;  /**< Host IN auto-retry enable. When set, this field enables the auto-retry feature. For IN
                                                         transfers (non-isochronous) that encounter data packets with CRC errors or internal
                                                         overrun scenarios, the auto-retry feature causes the host core to reply to the device with
                                                         a non-terminating retry ACK (i.e. an ACK transaction packet with Retry = 1 and NumP != 0).
                                                         If the auto-retry feature is disabled (default), the core responds with a terminating
                                                         retry ACK (i.e. an ACK transaction packet with Retry = 1 and NumP = 0). */
	uint32_t enoverlapchk                 : 1;  /**< Enable check for LFPS overlap during remote Ux Exit. If this bit is set to:
                                                         0 = When the link exists U1/U2/U3 because of a remote exit, it does not look for an LFPS
                                                         overlap.
                                                         1 = The SuperSpeed link, when exiting U1/U2/U3, waits for either the remote link LFPS or
                                                         TS1/TS2 training symbols before it confirms that the LFPS handshake is complete. This is
                                                         done to handle the case where the LFPS glitch causes the link to start exiting from the
                                                         low power state. Looking for the LFPS overlap makes sure that the link partner also sees
                                                         the LFPS. */
	uint32_t extcapsupten                 : 1;  /**< External extended capability support enable. If disabled, a read
                                                         USBDRD()_UAHC_SUPTPRT3_DW0
                                                         [NEXTCAPPTR] returns 0 in the next capability pointer field. This indicates there are no
                                                         more capabilities. If enabled, a read to USBDRD()_UAHC_SUPTPRT3_DW0[NEXTCAPPTR] returns 4
                                                         in
                                                         the
                                                         next capability pointer field.
                                                         Always set to 0x0. */
	uint32_t insrtextrfsbodi              : 1;  /**< Insert extra delay between full-speed bulk OUT transactions. Some full-speed devices are
                                                         slow to receive bulk OUT data and can get stuck when there are consecutive bulk OUT
                                                         transactions with short inter-transaction delays. This bit is used to control whether the
                                                         host inserts extra delay between consecutive bulk OUT transactions to a full-speed
                                                         endpoint.
                                                         0 = Host does not insert extra delay.
                                                         Setting this bit to 1 reduces the bulk OUT transfer performance for most of the full-speed
                                                         devices.
                                                         1 = Host inserts about 12 us extra delay between consecutive bulk OUT transactions to an
                                                         full-speed endpoint to work around the device issue. */
	uint32_t dtct                         : 2;  /**< Device timeout coarse tuning. This field determines how long the host waits for a response
                                                         from device before considering a timeout.
                                                         The core first checks the DTCT value. If it is 0, then the timeout value is defined by the
                                                         DTFT. If it is non-zero, then it uses the following timeout values:
                                                         0x0 = 0 us; use DTFT value instead.
                                                         0x1 = 500 us.
                                                         0x2 = 1.5 ms.
                                                         0x3 = 6.5 ms. */
	uint32_t dtft                         : 9;  /**< Device timeout fine tuning. This field determines how long the host waits for a response
                                                         from a device before considering a timeout. For [DTFT] to take effect, [DTCT] must be set
                                                         to
                                                         0x0.
                                                         The [DTFT] value specifies the number of 125 MHz clock cycles * 256 to count before
                                                         considering a device timeout. For the 125 MHz clock cycles (8 ns period), this is
                                                         calculated as follows:
                                                         _ [DTFT value] * 256 * 8 (ns)
                                                         0x2 = 2 * 256 * 8 -> 4 us.
                                                         0x5 = 5 * 256 * 8 -> 10 us.
                                                         0xA = 10 * 256 * 8 -> 20 us.
                                                         0x10 = 16 * 256 * 8 -> 32 us.
                                                         0x19 = 25 * 256 * 8 -> 51 us.
                                                         0x31 = 49 * 256 * 8 -> 100 us.
                                                         0x62 = 98 * 256 * 8 -> 200 us. */
#else
	uint32_t dtft                         : 9;
	uint32_t dtct                         : 2;
	uint32_t insrtextrfsbodi              : 1;
	uint32_t extcapsupten                 : 1;
	uint32_t enoverlapchk                 : 1;
	uint32_t usbhstinautoretryen          : 1;
	uint32_t cmdevaddr                    : 1;
	uint32_t resbwhseps                   : 1;
	uint32_t sprsctrltransen              : 1;
	uint32_t psqextrressp                 : 3;
	uint32_t noextrdl                     : 1;
	uint32_t refclkper                    : 10;
#endif
	} cn70xx;
	struct cvmx_usbdrdx_uahc_guctl_cn70xx cn70xxp1;
	struct cvmx_usbdrdx_uahc_guctl_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t refclkper                    : 10; /**< Reference-clock period. Indicates (in terms of ns) the period of REF_CLK. The default
                                                         value is set to 0x8
                                                         (8 ns/125 MHz). This field must be updated during power on initialization if
                                                         USBDRD()_UAHC_GCTL[SOFITPSYNC] = 1 or USBDRD()_UAHC_GFLADJ [GFLADJ_REFCLK_LPM_SEL] = 1.
                                                         The
                                                         programmable maximum value 62 ns, and the minimum value is 8 ns. You use a reference clock
                                                         with a period that is a integer multiple, so that ITP can meet the jitter margin of 32 ns.
                                                         The allowable REF_CLK frequencies whose period is not integer multiples are
                                                         16/17/19.2/24/39.7 MHz.
                                                         This field should not be set to 0x0 at any time. If you do not plan to use this feature,
                                                         then you need to set this field to 0x8, the default value. */
	uint32_t noextrdl                     : 1;  /**< No extra delay between SOF and the first packet.
                                                         Some high-speed devices misbehave when the host sends a packet immediately after an SOF.
                                                         However, adding an extra delay between an SOF and the first packet can reduce the USB data
                                                         rate and performance.
                                                         This bit is used to control whether the host should wait for 2 us before it sends the
                                                         first packet after a SOF, or not. You can set this bit to 1 to improve the performance if
                                                         those problematic devices are not a concern in your host environment.
                                                         0 = host waits for 2 us after an SOF before it sends the first USB packet.
                                                         1 = host does not wait after an SOF before it sends the first USB packet. */
	uint32_t psqextrressp                 : 3;  /**< PSQ extra reserved space. This is a debug feature, and is not intended for normal usage.
                                                         This parameter specifies how much additional space in the PSQ (protocol-status queue) must
                                                         be reserved before the U3PTL initiates a new USB transaction and burst beats. */
	uint32_t sprsctrltransen              : 1;  /**< Sparse control transaction enable. Some devices are slow in responding to control
                                                         transfers. Scheduling multiple transactions in one microframe/frame can cause these
                                                         devices to misbehave. If this bit is set to 1, the host controller schedules transactions
                                                         for a control transfer in different microframes/frames. */
	uint32_t resbwhseps                   : 1;  /**< Reserving 85% bandwidth for high-speed periodic EPs. By default, host controller reserves
                                                         80% of the bandwidth for periodic EPs. If this bit is set, the bandwidth is relaxed to 85%
                                                         to accommodate two high-speed, high-bandwidth ISOC EPs.
                                                         USB 2.0 required 80% bandwidth allocated for ISOC traffic. If two high bandwidth ISOC
                                                         devices (HD webcams) are connected, and if each requires 1024-bytes * 3 packets per
                                                         microframe, then the bandwidth required is around 82%. If this bit is set to 1, it is
                                                         possible to connect two webcams of 1024 bytes * 3 payload per microframe each. Otherwise,
                                                         you may have to reduce the resolution of the webcams. */
	uint32_t cmdevaddr                    : 1;  /**< Compliance mode for device address. When set to 1, slot ID can have different value than
                                                         device address if max_slot_enabled < 128.
                                                         0 = Device address is equal to slot ID.
                                                         1 = Increment device address on each address device command.
                                                         The xHCI compliance requires this bit to be set to 1. The 0 mode is for debug purpose
                                                         only. This allows you to easily identify a device connected to a port in the Lecroy or
                                                         Eliisys trace during hardware debug.
                                                         This bit is used in host mode only. */
	uint32_t usbdrdstinautoretryen        : 1;  /**< Host IN auto-retry enable. When set, this field enables the auto-retry feature. For IN
                                                         transfers (non-isochronous) that encounter data packets with CRC errors or internal
                                                         overrun scenarios, the auto-retry feature causes the host core to reply to the device with
                                                         a non-terminating retry ACK (i.e. an ACK transaction packet with Retry = 1 and NumP != 0).
                                                         If the auto-retry feature is disabled (default), the core responds with a terminating
                                                         retry ACK (i.e. an ACK transaction packet with Retry = 1 and NumP = 0). */
	uint32_t enoverlapchk                 : 1;  /**< Enable check for LFPS overlap during remote Ux Exit. If this bit is set to:
                                                         0 = When the link exists U1/U2/U3 because of a remote exit, it does not look for an LFPS
                                                         overlap.
                                                         1 = The SuperSpeed link, when exiting U1/U2/U3, waits for either the remote link LFPS or
                                                         TS1/TS2 training symbols before it confirms that the LFPS handshake is complete. This is
                                                         done to handle the case where the LFPS glitch causes the link to start exiting from the
                                                         low power state. Looking for the LFPS overlap makes sure that the link partner also sees
                                                         the LFPS. */
	uint32_t extcapsupten                 : 1;  /**< External extended capability support enable. If disabled, a read
                                                         USBDRD()_UAHC_SUPTPRT3_DW0
                                                         [NEXTCAPPTR] returns 0 in the next capability pointer field. This indicates there are no
                                                         more capabilities. If enabled, a read to USBDRD()_UAHC_SUPTPRT3_DW0[NEXTCAPPTR] returns 4
                                                         in
                                                         the
                                                         next capability pointer field.
                                                         Always set to 0x0. */
	uint32_t insrtextrfsbodi              : 1;  /**< Insert extra delay between full-speed bulk OUT transactions. Some full-speed devices are
                                                         slow to receive bulk OUT data and can get stuck when there are consecutive bulk OUT
                                                         transactions with short inter-transaction delays. This bit is used to control whether the
                                                         host inserts extra delay between consecutive bulk OUT transactions to a full-speed
                                                         endpoint.
                                                         0 = Host does not insert extra delay.
                                                         Setting this bit to 1 reduces the bulk OUT transfer performance for most of the full-speed
                                                         devices.
                                                         1 = Host inserts about 12 us extra delay between consecutive bulk OUT transactions to an
                                                         full-speed endpoint to work around the device issue. */
	uint32_t dtct                         : 2;  /**< Device timeout coarse tuning. This field determines how long the host waits for a response
                                                         from device before considering a timeout.
                                                         The core first checks the [DTCT] value. If it is 0, then the timeout value is defined by
                                                         the
                                                         [DTFT]. If it is nonzero, then it uses the following timeout values:
                                                         0x0 = 0 us; use [DTFT] value instead.
                                                         0x1 = 500 us.
                                                         0x2 = 1.5 ms.
                                                         0x3 = 6.5 ms. */
	uint32_t dtft                         : 9;  /**< Device timeout fine tuning. This field determines how long the host waits for a response
                                                         from a device before considering a timeout. For [DTFT] to take effect, [DTCT] must be set
                                                         to
                                                         0x0.
                                                         The [DTFT] value specifies the number of 125 MHz clock cycles * 256 to count before
                                                         considering a device timeout. For the 125 MHz clock cycles (8 ns period), this is
                                                         calculated as follows:
                                                         _ [DTFT value] * 256 * 8 (ns)
                                                         0x2 = 2 * 256 * 8 -> 4 us.
                                                         0x5 = 5 * 256 * 8 -> 10 us.
                                                         0xA = 10 * 256 * 8 -> 20 us.
                                                         0x10 = 16 * 256 * 8 -> 32 us.
                                                         0x19 = 25 * 256 * 8 -> 51 us.
                                                         0x31 = 49 * 256 * 8 -> 100 us.
                                                         0x62 = 98 * 256 * 8 -> 200 us. */
#else
	uint32_t dtft                         : 9;
	uint32_t dtct                         : 2;
	uint32_t insrtextrfsbodi              : 1;
	uint32_t extcapsupten                 : 1;
	uint32_t enoverlapchk                 : 1;
	uint32_t usbdrdstinautoretryen        : 1;
	uint32_t cmdevaddr                    : 1;
	uint32_t resbwhseps                   : 1;
	uint32_t sprsctrltransen              : 1;
	uint32_t psqextrressp                 : 3;
	uint32_t noextrdl                     : 1;
	uint32_t refclkper                    : 10;
#endif
	} cn73xx;
	struct cvmx_usbdrdx_uahc_guctl_cn73xx cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_guctl cvmx_usbdrdx_uahc_guctl_t;

/**
 * cvmx_usbdrd#_uahc_guctl1
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 *
 */
union cvmx_usbdrdx_uahc_guctl1 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_guctl1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_2_31                : 30;
	uint32_t ovrld_l1_susp_com            : 1;  /**< Always set to 0. */
	uint32_t loa_filter_en                : 1;  /**< If this bit is set, the USB 2.0 port babble is checked at least three consecutive times
                                                         before the port is disabled. This prevents false triggering of the babble condition when
                                                         using low quality cables.
                                                         This bit is only valid in host mode. */
#else
	uint32_t loa_filter_en                : 1;
	uint32_t ovrld_l1_susp_com            : 1;
	uint32_t reserved_2_31                : 30;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_guctl1_s     cn70xx;
	struct cvmx_usbdrdx_uahc_guctl1_s     cn70xxp1;
	struct cvmx_usbdrdx_uahc_guctl1_s     cn73xx;
	struct cvmx_usbdrdx_uahc_guctl1_s     cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_guctl1 cvmx_usbdrdx_uahc_guctl1_t;

/**
 * cvmx_usbdrd#_uahc_guid
 *
 * This is a read/write register containing the User ID. The power-on value for this register is
 * specified as the user identification register. This register can be used in the following
 * ways:
 * * To store the version or revision of your system.
 * * To store hardware configurations that are outside of the core.
 * * As a scratch register.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_guid {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_guid_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t userid                       : 32; /**< User ID. Application-programmable ID field. */
#else
	uint32_t userid                       : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_guid_s       cn70xx;
	struct cvmx_usbdrdx_uahc_guid_s       cn70xxp1;
	struct cvmx_usbdrdx_uahc_guid_s       cn73xx;
	struct cvmx_usbdrdx_uahc_guid_s       cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_guid cvmx_usbdrdx_uahc_guid_t;

/**
 * cvmx_usbdrd#_uahc_gusb2i2cctl#
 *
 * This register is reserved for future use.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UAHC_RST].
 */
union cvmx_usbdrdx_uahc_gusb2i2cctlx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gusb2i2cctlx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gusb2i2cctlx_s cn70xx;
	struct cvmx_usbdrdx_uahc_gusb2i2cctlx_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_gusb2i2cctlx_s cn73xx;
	struct cvmx_usbdrdx_uahc_gusb2i2cctlx_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gusb2i2cctlx cvmx_usbdrdx_uahc_gusb2i2cctlx_t;

/**
 * cvmx_usbdrd#_uahc_gusb2phycfg#
 *
 * This register is used to configure the core after power-on. It contains USB 2.0 and USB 2.0
 * PHY-related configuration parameters. The application must program this register before
 * starting any transactions on either the SoC bus or the USB. Per-port registers are
 * implemented.
 *
 * Do not make changes to this register after the initial programming.
 */
union cvmx_usbdrdx_uahc_gusb2phycfgx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gusb2phycfgx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t physoftrst                   : 1;  /**< PHY soft reset. Causes the usb2phy_reset signal to be asserted to reset a UTMI PHY. */
	uint32_t u2_freeclk_exists            : 1;  /**< Specifies whether your USB 2.0 PHY provides a free-running PHY clock, which is active when
                                                         the clock control input is active. If your USB 2.0 PHY provides a free-running PHY clock,
                                                         it must be connected to the utmi_clk[0] input. The remaining utmi_clk[n] must be connected
                                                         to the respective port clocks. The core uses the Port-0 clock for generating the internal
                                                         mac2 clock.
                                                         0 = USB 2.0 free clock does not exist.
                                                         1 = USB 2.0 free clock exists.
                                                         This field must be set to zero if you enable ITP generation based on the REF_CLK
                                                         counter, USBDRD()_UAHC_GCTL[SOFITPSYNC] = 1, or USBDRD()_UAHC_GFLADJ
                                                         [GFLADJ_REFCLK_LPM_SEL] =
                                                         1. */
	uint32_t ulpi_lpm_with_opmode_chk     : 1;  /**< Support the LPM over ULPI without NOPID token to the ULPI PHY. Always 0x0. */
	uint32_t reserved_19_28               : 10;
	uint32_t ulpiextvbusindicator         : 1;  /**< Reserved (unused in this configuration). */
	uint32_t ulpiextvbusdrv               : 1;  /**< Reserved (unused in this configuration). */
	uint32_t ulpiclksusm                  : 1;  /**< Reserved (unused in this configuration). */
	uint32_t ulpiautores                  : 1;  /**< Reserved (unused in this configuration). */
	uint32_t reserved_14_14               : 1;
	uint32_t usbtrdtim                    : 4;  /**< USB 2.0 turnaround time. Sets the turnaround time in PHY clock cycles. Specifies the
                                                         response time for a MAC request to the packet FIFO controller (PFC) to fetch data from the
                                                         DFIFO (SPRAM).
                                                         USB turnaround time is a critical certification criteria when using long cables and five
                                                         hub levels.
                                                         When the MAC interface is 8-bit UTMI+/ULPI, the required values for this field is 0x9. */
	uint32_t xcvrdly                      : 1;  /**< Transceiver delay.
                                                         Enables a delay between the assertion of the UTMI transceiver select signal (for
                                                         HighSpeed) and the assertion of the TxValid signal during a HighSpeed chirp.
                                                         When this bit is set to 1, a delay of approximately 2.5 us is introduced from
                                                         the time when the transceiver select is set to 0x0, to the time when the TxValid
                                                         is driven to 0 for sending the chirp-K. This delay is required for some UTMI PHYs.
                                                         This bit is only valid in device mode. */
	uint32_t enblslpm                     : 1;  /**< Enable utmi_sleep_n and utmi_l1_suspend_n. The application uses this field to control
                                                         utmi_sleep_n and utmi_l1_suspend_n assertion to the PHY in the L1 state.
                                                         0 = utmi_sleep_n and utmi_l1_suspend_n assertion from the core is not transferred to the
                                                         external PHY.
                                                         1 = utmi_sleep_n and utmi_l1_suspend_n assertion from the core is transferred to the
                                                         external PHY.
                                                         When hardware LPM is enabled, this bit should be set high for Port0. */
	uint32_t physel                       : 1;  /**< USB 2.0 high-speed PHY or USB 1.1 full-speed serial transceiver select. */
	uint32_t susphy                       : 1;  /**< Suspend USB2.0 high-speed/full-speed/low-speed PHY. When set, USB2.0 PHY enters suspend
                                                         mode if suspend conditions are valid. */
	uint32_t fsintf                       : 1;  /**< Full-speed serial-interface select. Always reads as 0x0. */
	uint32_t ulpi_utmi_sel                : 1;  /**< ULPI or UTMI+ select. Always reads as 0x0, indicating UTMI+. */
	uint32_t phyif                        : 1;  /**< PHY interface width: 1 = 16-bit, 0 = 8-bit.
                                                         All the enabled 2.0 ports should have the same clock frequency as Port0 clock frequency
                                                         (utmi_clk[0]).
                                                         The UTMI 8-bit and 16-bit modes cannot be used together for different ports at the same
                                                         time (i.e., all the ports should be in 8-bit mode, or all of them should be in 16-bit
                                                         mode). */
	uint32_t toutcal                      : 3;  /**< High-speed/full-speed timeout calibration.
                                                         The number of PHY clock cycles, as indicated by the application in this field, is
                                                         multiplied by a bit-time factor; this factor is added to the high-speed/full-speed
                                                         interpacket timeout duration in the core to account for additional delays introduced by
                                                         the PHY. This might be required, since the delay introduced by the PHY in generating the
                                                         linestate condition can vary among PHYs.
                                                         The USB standard timeout value for high-speed operation is 736 to 816 (inclusive) bit
                                                         times. The USB standard timeout value for full-speed operation is 16 to 18 (inclusive) bit
                                                         times. The application must program this field based on the speed of connection.
                                                         The number of bit times added per PHY clock are:
                                                         * High-speed operation:
                                                         - one 30-MHz PHY clock = 16 bit times.
                                                         - one 60-MHz PHY clock = 8 bit times.
                                                         * Full-speed operation:
                                                         - one 30-MHz PHY clock = 0.4 bit times.
                                                         - one 60-MHz PHY clock = 0.2 bit times.
                                                         - one 48-MHz PHY clock = 0.25 bit times. */
#else
	uint32_t toutcal                      : 3;
	uint32_t phyif                        : 1;
	uint32_t ulpi_utmi_sel                : 1;
	uint32_t fsintf                       : 1;
	uint32_t susphy                       : 1;
	uint32_t physel                       : 1;
	uint32_t enblslpm                     : 1;
	uint32_t xcvrdly                      : 1;
	uint32_t usbtrdtim                    : 4;
	uint32_t reserved_14_14               : 1;
	uint32_t ulpiautores                  : 1;
	uint32_t ulpiclksusm                  : 1;
	uint32_t ulpiextvbusdrv               : 1;
	uint32_t ulpiextvbusindicator         : 1;
	uint32_t reserved_19_28               : 10;
	uint32_t ulpi_lpm_with_opmode_chk     : 1;
	uint32_t u2_freeclk_exists            : 1;
	uint32_t physoftrst                   : 1;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gusb2phycfgx_s cn70xx;
	struct cvmx_usbdrdx_uahc_gusb2phycfgx_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_gusb2phycfgx_s cn73xx;
	struct cvmx_usbdrdx_uahc_gusb2phycfgx_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gusb2phycfgx cvmx_usbdrdx_uahc_gusb2phycfgx_t;

/**
 * cvmx_usbdrd#_uahc_gusb3pipectl#
 *
 * This register is used to configure the core after power-on. It contains USB 3.0 and USB 3.0
 * PHY-related configuration parameters. The application must program this register before
 * starting any transactions on either the SoC bus or the USB. Per-port registers are
 * implemented.
 *
 * Do not make changes to this register after the initial programming.
 */
union cvmx_usbdrdx_uahc_gusb3pipectlx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_gusb3pipectlx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t physoftrst                   : 1;  /**< USB3 PHY soft reset (PHYSoftRst). When set to 1, initiates a PHY soft reset. After setting
                                                         this bit to 1, the software needs to clear this bit. */
	uint32_t hstprtcmpl                   : 1;  /**< Host port compliance. Setting this bit to 1 enables placing the SuperSpeed port link into
                                                         a compliance state, which allows testing of the PIPE PHY compliance patterns without
                                                         having to have a test fixture on the USB 3.0 cable. By default, this bit should be set to
                                                         0.
                                                         In compliance-lab testing, the SuperSpeed port link enters compliance after failing the
                                                         first polling sequence after power on. Set this bit to 0 when you run compliance tests.
                                                         The sequence for using this functionality is as follows:
                                                         * Disconnect any plugged-in devices.
                                                         * Set USBDRD()_UAHC_USBCMD[HCRST] = 1 or power-on-chip reset.
                                                         * Set USBDRD()_UAHC_PORTSC()[PP] = 0.
                                                         * Set HSTPRTCMPL = 1. This places the link into compliance state.
                                                         To advance the compliance pattern, follow this sequence (toggle HSTPRTCMPL):
                                                         * Set HSTPRTCMPL = 0.
                                                         * Set HSTPRTCMPL = 1. This advances the link to the next compliance pattern.
                                                         To exit from the compliance state, set USBDRD()_UAHC_USBCMD[HCRST] = 1 or power-on-chip
                                                         reset. */
	uint32_t u2ssinactp3ok                : 1;  /**< P3 OK for U2/SS.Inactive:
                                                         0 = During link state U2/SS.Inactive, put PHY in P2 (default).
                                                         1 = During link state U2/SS.Inactive, put PHY in P3. */
	uint32_t disrxdetp3                   : 1;  /**< Disables receiver detection in P3. If PHY is in P3 and the core needs to perform receiver
                                                         detection:
                                                         0 = Core performs receiver detection in P3 (default).
                                                         1 = Core changes the PHY power state to P2 and then performs receiver detection. After
                                                         receiver detection, core changes PHY power state to P3. */
	uint32_t ux_exit_in_px                : 1;  /**< UX exit in Px:
                                                         0 = Core does U1/U2/U3 exit in PHY power state P0 (default behavior).
                                                         1 = Core does U1/U2/U3 exit in PHY power state P1/P2/P3 respectively.
                                                         This bit is added for SuperSpeed PHY workaround where SuperSpeed PHY injects a glitch on
                                                         pipe3_RxElecIdle while receiving Ux exit LFPS, and pipe3_PowerDown change is in progress. */
	uint32_t ping_enchance_en             : 1;  /**< Ping enhancement enable. When set to 1, the downstream-port U1-ping-receive timeout
                                                         becomes 500 ms instead of 300 ms. Minimum Ping.LFPS receive duration is 8 ns (one mac3_clk
                                                         cycle). This field is valid for the downstream port only. */
	uint32_t u1u2exitfail_to_recov        : 1;  /**< U1U2exit fail to recovery. When set to 1, and U1/U2 LFPS handshake fails, the LTSSM
                                                         transitions from U1/U2 to recovery instead of SS.inactive.
                                                         If recovery fails, then the LTSSM can enter SS.Inactive. This is an enhancement only. It
                                                         prevents interoperability issue if the remote link does not do the proper handshake. */
	uint32_t request_p1p2p3               : 1;  /**< Always request P1/P2/P3 for U1/U2/U3.
                                                         0 = if immediate Ux exit (remotely initiated, or locally initiated) happens, the core does
                                                         not request P1/P2/P3 power state change.
                                                         1 = the core always requests PHY power change from P0 to P1/P2/P3 during U0 to U1/U2/U3
                                                         transition. */
	uint32_t startrxdetu3rxdet            : 1;  /**< Start receiver detection in U3/Rx.Detect.
                                                         If DISRXDETU3RXDET is set to 1 during reset, and the link is in U3 or Rx.Detect state, the
                                                         core starts receiver detection on rising edge of this bit.
                                                         This bit is valid for downstream ports only, and this feature must not be enabled for
                                                         normal operation. */
	uint32_t disrxdetu3rxdet              : 1;  /**< Disable receiver detection in U3/Rx.Detect. When set to 1, the core does not do receiver
                                                         detection in U3 or Rx.Detect state. If STARTRXDETU3RXDET is set to 1 during reset,
                                                         receiver detection starts manually.
                                                         This bit is valid for downstream ports only, and this feature must not be enabled for
                                                         normal operation. */
	uint32_t delaypx                      : 3;  /**< Delay P1P2P3. Delay P0 to P1/P2/P3 request when entering U1/U2/U3 until (DELAYPX * 8)
                                                         8B10B error occurs, or Pipe3_RxValid drops to 0.
                                                         DELAYPXTRANSENTERUX must reset to 1 to enable this functionality. */
	uint32_t delaypxtransenterux          : 1;  /**< Delay PHY power change from P0 to P1/P2/P3 when link state changing from U0 to U1/U2/U3
                                                         respectively.
                                                         0 = when entering U1/U2/U3, transition to P1/P2/P3 without checking for Pipe3_RxElecIlde
                                                         and pipe3_RxValid.
                                                         1 = when entering U1/U2/U3, delay the transition to P1/P2/P3 until the pipe3 signals,
                                                         Pipe3_RxElecIlde is 1 and pipe3_RxValid is 0. */
	uint32_t suspend_en                   : 1;  /**< Suspend USB3.0 SuperSpeed PHY (Suspend_en). When set to 1, and if suspend conditions are
                                                         valid, the USB 3.0 PHY enters suspend mode. */
	uint32_t datwidth                     : 2;  /**< PIPE data width.
                                                         0x0 = 32 bits.
                                                         0x1 = 16 bits.
                                                         0x2 = 8 bits.
                                                         0x3 = reserved.
                                                         One clock cycle after reset, these bits receive the value seen on the pipe3_DataBusWidth.
                                                         This will always be 0x0. */
	uint32_t abortrxdetinu2               : 1;  /**< Abort RX Detect in U2. When set to 1, and the link state is U2, the core aborts receiver
                                                         detection if it receives U2 exit LFPS from the remote link partner.
                                                         This bit is for downstream port only. */
	uint32_t skiprxdet                    : 1;  /**< Skip RX detect. When set to 1, the core skips RX detection if pipe3_RxElecIdle is low.
                                                         Skip is defined as waiting for the appropriate timeout, then repeating the operation. */
	uint32_t lfpsp0algn                   : 1;  /**< LFPS P0 align. When set to 1:
                                                         * the core deasserts LFPS transmission on the clock edge that it requests Phy power state
                                                         0 when exiting U1, U2, or U3 low power states. Otherwise, LFPS transmission is asserted
                                                         one clock earlier.
                                                         * the core requests symbol transmission two pipe3_rx_pclks periods after the PHY asserts
                                                         PhyStatus as a result of the PHY switching from P1 or P2 state to P0 state.
                                                         For USB 3.0 host, this is not required. */
	uint32_t p3p2tranok                   : 1;  /**< P3 P2 transitions OK.
                                                         0 = P0 is always entered as an intermediate state during transitions between P2 and P3, as
                                                         defined in the PIPE3 specification.
                                                         1 = the core transitions directly from Phy power state P2 to P3 or from state P3 to P2.
                                                         According to PIPE3 specification, any direct transition between P3 and P2 is illegal. */
	uint32_t p3exsigp2                    : 1;  /**< P3 exit signal in P2. When set to 1, the core always changes the PHY power state to P2,
                                                         before attempting a U3 exit handshake. */
	uint32_t lfpsfilt                     : 1;  /**< LFPS filter. When set to 1, filter LFPS reception with pipe3_RxValid in PHY power state
                                                         P0, ignore LFPS reception from the PHY unless both pipe3_Rxelecidle and pipe3_RxValid are
                                                         deasserted. */
	uint32_t rxdet2polllfpsctrl           : 1;  /**< RX_DETECT to polling.
                                                         0 = Enables a 400 us delay to start polling LFPS after RX_DETECT. This allows VCM offset
                                                         to settle to a proper level.
                                                         1 = Disables the 400 us delay to start polling LFPS after RX_DETECT. */
	uint32_t reserved_7_7                 : 1;
	uint32_t txswing                      : 1;  /**< TX swing. Refer to the PIPE3 specification. */
	uint32_t txmargin                     : 3;  /**< TX margin. Refer to the PIPE3 specification, table 5-3. */
	uint32_t txdeemphasis                 : 2;  /**< TX deemphasis. The value driven to the PHY is controlled by the LTSSM during USB3
                                                         compliance mode. Refer to the PIPE3 specification, table 5-3.
                                                         Use the following values for the appropriate level of de-emphasis (From pipe3 spec):
                                                         0x0 =   -6 dB de-emphasis, use USBDRD()_UCTL_PORT()_CFG_SS[PCS_TX_DEEMPH_6DB].
                                                         0x1 = -3.5 dB de-emphasis, use USBDRD()_UCTL_PORT()_CFG_SS[PCS_TX_DEEMPH_3P5DB].
                                                         0x2 =     No de-emphasis.
                                                         0x3 =     Reserved. */
	uint32_t elasticbuffermode            : 1;  /**< Elastic buffer mode. Refer to the PIPE3 specification, table 5-3. */
#else
	uint32_t elasticbuffermode            : 1;
	uint32_t txdeemphasis                 : 2;
	uint32_t txmargin                     : 3;
	uint32_t txswing                      : 1;
	uint32_t reserved_7_7                 : 1;
	uint32_t rxdet2polllfpsctrl           : 1;
	uint32_t lfpsfilt                     : 1;
	uint32_t p3exsigp2                    : 1;
	uint32_t p3p2tranok                   : 1;
	uint32_t lfpsp0algn                   : 1;
	uint32_t skiprxdet                    : 1;
	uint32_t abortrxdetinu2               : 1;
	uint32_t datwidth                     : 2;
	uint32_t suspend_en                   : 1;
	uint32_t delaypxtransenterux          : 1;
	uint32_t delaypx                      : 3;
	uint32_t disrxdetu3rxdet              : 1;
	uint32_t startrxdetu3rxdet            : 1;
	uint32_t request_p1p2p3               : 1;
	uint32_t u1u2exitfail_to_recov        : 1;
	uint32_t ping_enchance_en             : 1;
	uint32_t ux_exit_in_px                : 1;
	uint32_t disrxdetp3                   : 1;
	uint32_t u2ssinactp3ok                : 1;
	uint32_t hstprtcmpl                   : 1;
	uint32_t physoftrst                   : 1;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_gusb3pipectlx_s cn70xx;
	struct cvmx_usbdrdx_uahc_gusb3pipectlx_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_gusb3pipectlx_s cn73xx;
	struct cvmx_usbdrdx_uahc_gusb3pipectlx_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_gusb3pipectlx cvmx_usbdrdx_uahc_gusb3pipectlx_t;

/**
 * cvmx_usbdrd#_uahc_hccparams
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.3.6.
 *
 */
union cvmx_usbdrdx_uahc_hccparams {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_hccparams_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t xecp                         : 16; /**< xHCI extended capabilities pointer. */
	uint32_t maxpsasize                   : 4;  /**< Maximum primary-stream-array size. */
	uint32_t reserved_9_11                : 3;
	uint32_t pae                          : 1;  /**< Parse all event data. */
	uint32_t nss                          : 1;  /**< No secondary SID support. */
	uint32_t ltc                          : 1;  /**< Latency tolerance messaging capability. */
	uint32_t lhrc                         : 1;  /**< Light HC reset capability. */
	uint32_t pind                         : 1;  /**< Port indicators. */
	uint32_t ppc                          : 1;  /**< Port power control. Value is based on USBDRD()_UCTL_HOST_CFG[PPC_EN]. */
	uint32_t csz                          : 1;  /**< Context size. */
	uint32_t bnc                          : 1;  /**< BW negotiation capability. */
	uint32_t ac64                         : 1;  /**< 64-bit addressing capability. */
#else
	uint32_t ac64                         : 1;
	uint32_t bnc                          : 1;
	uint32_t csz                          : 1;
	uint32_t ppc                          : 1;
	uint32_t pind                         : 1;
	uint32_t lhrc                         : 1;
	uint32_t ltc                          : 1;
	uint32_t nss                          : 1;
	uint32_t pae                          : 1;
	uint32_t reserved_9_11                : 3;
	uint32_t maxpsasize                   : 4;
	uint32_t xecp                         : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_hccparams_s  cn70xx;
	struct cvmx_usbdrdx_uahc_hccparams_s  cn70xxp1;
	struct cvmx_usbdrdx_uahc_hccparams_s  cn73xx;
	struct cvmx_usbdrdx_uahc_hccparams_s  cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_hccparams cvmx_usbdrdx_uahc_hccparams_t;

/**
 * cvmx_usbdrd#_uahc_hcsparams1
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.3.3.
 *
 */
union cvmx_usbdrdx_uahc_hcsparams1 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_hcsparams1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t maxports                     : 8;  /**< Maximum number of ports. */
	uint32_t reserved_19_23               : 5;
	uint32_t maxintrs                     : 11; /**< Maximum number of interrupters. */
	uint32_t maxslots                     : 8;  /**< Maximum number of device slots. */
#else
	uint32_t maxslots                     : 8;
	uint32_t maxintrs                     : 11;
	uint32_t reserved_19_23               : 5;
	uint32_t maxports                     : 8;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_hcsparams1_s cn70xx;
	struct cvmx_usbdrdx_uahc_hcsparams1_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_hcsparams1_s cn73xx;
	struct cvmx_usbdrdx_uahc_hcsparams1_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_hcsparams1 cvmx_usbdrdx_uahc_hcsparams1_t;

/**
 * cvmx_usbdrd#_uahc_hcsparams2
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.3.4.
 *
 */
union cvmx_usbdrdx_uahc_hcsparams2 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_hcsparams2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t maxscratchpadbufs_l          : 5;  /**< Maximum number of scratchpad buffers[4:0]. */
	uint32_t reserved_26_26               : 1;
	uint32_t maxscratchpadbufs_h          : 5;  /**< Maximum number of scratchpad buffers[9:5]. */
	uint32_t reserved_8_20                : 13;
	uint32_t erst_max                     : 4;  /**< Event ring segment table maximum. */
	uint32_t ist                          : 4;  /**< Isochronous scheduling threshold. */
#else
	uint32_t ist                          : 4;
	uint32_t erst_max                     : 4;
	uint32_t reserved_8_20                : 13;
	uint32_t maxscratchpadbufs_h          : 5;
	uint32_t reserved_26_26               : 1;
	uint32_t maxscratchpadbufs_l          : 5;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_hcsparams2_s cn70xx;
	struct cvmx_usbdrdx_uahc_hcsparams2_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_hcsparams2_s cn73xx;
	struct cvmx_usbdrdx_uahc_hcsparams2_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_hcsparams2 cvmx_usbdrdx_uahc_hcsparams2_t;

/**
 * cvmx_usbdrd#_uahc_hcsparams3
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.3.5.
 *
 */
union cvmx_usbdrdx_uahc_hcsparams3 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_hcsparams3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t u2_device_exit_latency       : 16; /**< U2 device exit latency. */
	uint32_t reserved_8_15                : 8;
	uint32_t u1_device_exit_latency       : 8;  /**< U1 device exit latency. */
#else
	uint32_t u1_device_exit_latency       : 8;
	uint32_t reserved_8_15                : 8;
	uint32_t u2_device_exit_latency       : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_hcsparams3_s cn70xx;
	struct cvmx_usbdrdx_uahc_hcsparams3_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_hcsparams3_s cn73xx;
	struct cvmx_usbdrdx_uahc_hcsparams3_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_hcsparams3 cvmx_usbdrdx_uahc_hcsparams3_t;

/**
 * cvmx_usbdrd#_uahc_iman#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.5.2.1.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_imanx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_imanx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_2_31                : 30;
	uint32_t ie                           : 1;  /**< Interrupt enable. */
	uint32_t ip                           : 1;  /**< Interrupt pending. */
#else
	uint32_t ip                           : 1;
	uint32_t ie                           : 1;
	uint32_t reserved_2_31                : 30;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_imanx_s      cn70xx;
	struct cvmx_usbdrdx_uahc_imanx_s      cn70xxp1;
	struct cvmx_usbdrdx_uahc_imanx_s      cn73xx;
	struct cvmx_usbdrdx_uahc_imanx_s      cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_imanx cvmx_usbdrdx_uahc_imanx_t;

/**
 * cvmx_usbdrd#_uahc_imod#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.5.2.2.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_imodx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_imodx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t imodc                        : 16; /**< Interrupt moderation counter. */
	uint32_t imodi                        : 16; /**< Interrupt moderation interval. */
#else
	uint32_t imodi                        : 16;
	uint32_t imodc                        : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_imodx_s      cn70xx;
	struct cvmx_usbdrdx_uahc_imodx_s      cn70xxp1;
	struct cvmx_usbdrdx_uahc_imodx_s      cn73xx;
	struct cvmx_usbdrdx_uahc_imodx_s      cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_imodx cvmx_usbdrdx_uahc_imodx_t;

/**
 * cvmx_usbdrd#_uahc_mfindex
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.5.1.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_mfindex {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_mfindex_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_14_31               : 18;
	uint32_t mfindex                      : 14; /**< Microframe index. */
#else
	uint32_t mfindex                      : 14;
	uint32_t reserved_14_31               : 18;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_mfindex_s    cn70xx;
	struct cvmx_usbdrdx_uahc_mfindex_s    cn70xxp1;
	struct cvmx_usbdrdx_uahc_mfindex_s    cn73xx;
	struct cvmx_usbdrdx_uahc_mfindex_s    cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_mfindex cvmx_usbdrdx_uahc_mfindex_t;

/**
 * cvmx_usbdrd#_uahc_pagesize
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.3.
 *
 */
union cvmx_usbdrdx_uahc_pagesize {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_pagesize_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t pagesize                     : 16; /**< Page size. */
#else
	uint32_t pagesize                     : 16;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_pagesize_s   cn70xx;
	struct cvmx_usbdrdx_uahc_pagesize_s   cn70xxp1;
	struct cvmx_usbdrdx_uahc_pagesize_s   cn73xx;
	struct cvmx_usbdrdx_uahc_pagesize_s   cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_pagesize cvmx_usbdrdx_uahc_pagesize_t;

/**
 * cvmx_usbdrd#_uahc_porthlpmc_20#
 *
 * For information on this register, refer to the xHCI Specification, v1.1, section 5.4.11.2.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_porthlpmc_20x {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_porthlpmc_20x_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_14_31               : 18;
	uint32_t hirdd                        : 4;  /**< See section 5.4.11.2 of the XHCI Spec 1.1.
                                                         If USBDRD()_UAHC_SUPTPRT2_DW2[BLC] = 0, then HIRD timing is applied to this field.
                                                         If USBDRD()_UAHC_SUPTPRT2_DW2[BLC] = 1, then BESL timing is applied to this field. */
	uint32_t l1_timeout                   : 8;  /**< Timeout value for the L1 inactivity timer (LPM timer). This field is set to 0x0 by the
                                                         assertion of PR to 1. Refer to section 4.23.5.1.1.1 (in XHCI spec 1.1) for more
                                                         information on L1 Timeout operation.
                                                         The following are permissible values:
                                                         0x0 =  128 us. (default).
                                                         0x1 =  256 us.
                                                         0x2 =  512 us.
                                                         0x3 =  768 us.
                                                         _ ...
                                                         0xFF =  65280 us. */
	uint32_t hirdm                        : 2;  /**< Host-initiated resume-duration mode. */
#else
	uint32_t hirdm                        : 2;
	uint32_t l1_timeout                   : 8;
	uint32_t hirdd                        : 4;
	uint32_t reserved_14_31               : 18;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_porthlpmc_20x_s cn70xx;
	struct cvmx_usbdrdx_uahc_porthlpmc_20x_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_porthlpmc_20x_s cn73xx;
	struct cvmx_usbdrdx_uahc_porthlpmc_20x_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_porthlpmc_20x cvmx_usbdrdx_uahc_porthlpmc_20x_t;

/**
 * cvmx_usbdrd#_uahc_porthlpmc_ss#
 *
 * The USB3 port hardware LPM control register is reserved and shall be treated as RsvdP by
 * software. See xHCI specification v1.1 section 5.4.11.1.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST].
 */
union cvmx_usbdrdx_uahc_porthlpmc_ssx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_porthlpmc_ssx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_porthlpmc_ssx_s cn70xx;
	struct cvmx_usbdrdx_uahc_porthlpmc_ssx_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_porthlpmc_ssx_s cn73xx;
	struct cvmx_usbdrdx_uahc_porthlpmc_ssx_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_porthlpmc_ssx cvmx_usbdrdx_uahc_porthlpmc_ssx_t;

/**
 * cvmx_usbdrd#_uahc_portli_20#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.10.
 *
 */
union cvmx_usbdrdx_uahc_portli_20x {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_portli_20x_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_portli_20x_s cn70xx;
	struct cvmx_usbdrdx_uahc_portli_20x_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_portli_20x_s cn73xx;
	struct cvmx_usbdrdx_uahc_portli_20x_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_portli_20x cvmx_usbdrdx_uahc_portli_20x_t;

/**
 * cvmx_usbdrd#_uahc_portli_ss#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.10.
 *
 */
union cvmx_usbdrdx_uahc_portli_ssx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_portli_ssx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t linkerrorcount               : 16; /**< Link error count. */
#else
	uint32_t linkerrorcount               : 16;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_portli_ssx_s cn70xx;
	struct cvmx_usbdrdx_uahc_portli_ssx_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_portli_ssx_s cn73xx;
	struct cvmx_usbdrdx_uahc_portli_ssx_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_portli_ssx cvmx_usbdrdx_uahc_portli_ssx_t;

/**
 * cvmx_usbdrd#_uahc_portpmsc_20#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.9.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST].
 */
union cvmx_usbdrdx_uahc_portpmsc_20x {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_portpmsc_20x_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t port_test_control            : 4;  /**< Port test control. */
	uint32_t reserved_17_27               : 11;
	uint32_t hle                          : 1;  /**< Hardware LPM enable. */
	uint32_t l1_device_slot               : 8;  /**< L1 device slot. */
	uint32_t hird                         : 4;  /**< Host-initiated resume duration. */
	uint32_t rwe                          : 1;  /**< Remove wake enable. */
	uint32_t l1s                          : 3;  /**< L1 status. */
#else
	uint32_t l1s                          : 3;
	uint32_t rwe                          : 1;
	uint32_t hird                         : 4;
	uint32_t l1_device_slot               : 8;
	uint32_t hle                          : 1;
	uint32_t reserved_17_27               : 11;
	uint32_t port_test_control            : 4;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_portpmsc_20x_s cn70xx;
	struct cvmx_usbdrdx_uahc_portpmsc_20x_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_portpmsc_20x_s cn73xx;
	struct cvmx_usbdrdx_uahc_portpmsc_20x_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_portpmsc_20x cvmx_usbdrdx_uahc_portpmsc_20x_t;

/**
 * cvmx_usbdrd#_uahc_portpmsc_ss#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.9.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST].
 */
union cvmx_usbdrdx_uahc_portpmsc_ssx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_portpmsc_ssx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_17_31               : 15;
	uint32_t fla                          : 1;  /**< Force link PM accept. */
	uint32_t u2_timeout                   : 8;  /**< U2 timeout. */
	uint32_t u1_timeout                   : 8;  /**< U1 timeout. */
#else
	uint32_t u1_timeout                   : 8;
	uint32_t u2_timeout                   : 8;
	uint32_t fla                          : 1;
	uint32_t reserved_17_31               : 15;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_portpmsc_ssx_s cn70xx;
	struct cvmx_usbdrdx_uahc_portpmsc_ssx_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_portpmsc_ssx_s cn73xx;
	struct cvmx_usbdrdx_uahc_portpmsc_ssx_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_portpmsc_ssx cvmx_usbdrdx_uahc_portpmsc_ssx_t;

/**
 * cvmx_usbdrd#_uahc_portsc#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.8. Port 1
 * is USB3.0 SuperSpeed link, Port 0 is USB2.0 high-speed/full-speed/low-speed link.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST].
 */
union cvmx_usbdrdx_uahc_portscx {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_portscx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t wpr                          : 1;  /**< Warm port reset. */
	uint32_t dr                           : 1;  /**< Device removable. */
	uint32_t reserved_28_29               : 2;
	uint32_t woe                          : 1;  /**< Wake on overcurrent enable. */
	uint32_t wde                          : 1;  /**< Wake on disconnect enable. */
	uint32_t wce                          : 1;  /**< Wake on connect enable. */
	uint32_t cas                          : 1;  /**< Cold attach status. */
	uint32_t cec                          : 1;  /**< Port configuration error change. */
	uint32_t plc                          : 1;  /**< Port link state change. */
	uint32_t prc                          : 1;  /**< Port reset change. */
	uint32_t occ                          : 1;  /**< Overcurrent change. */
	uint32_t wrc                          : 1;  /**< Warm port reset change. */
	uint32_t pec                          : 1;  /**< Port enabled/disabled change. */
	uint32_t csc                          : 1;  /**< Connect status change. */
	uint32_t lws                          : 1;  /**< Port link state write strobe. */
	uint32_t pic                          : 2;  /**< Port indicator control. */
	uint32_t portspeed                    : 4;  /**< Port speed. */
	uint32_t pp                           : 1;  /**< Port power. */
	uint32_t pls                          : 4;  /**< Port link state. */
	uint32_t pr                           : 1;  /**< Port reset. */
	uint32_t oca                          : 1;  /**< Overcurrent active. */
	uint32_t reserved_2_2                 : 1;
	uint32_t ped                          : 1;  /**< Port enabled/disabled. */
	uint32_t ccs                          : 1;  /**< Current connect status. */
#else
	uint32_t ccs                          : 1;
	uint32_t ped                          : 1;
	uint32_t reserved_2_2                 : 1;
	uint32_t oca                          : 1;
	uint32_t pr                           : 1;
	uint32_t pls                          : 4;
	uint32_t pp                           : 1;
	uint32_t portspeed                    : 4;
	uint32_t pic                          : 2;
	uint32_t lws                          : 1;
	uint32_t csc                          : 1;
	uint32_t pec                          : 1;
	uint32_t wrc                          : 1;
	uint32_t occ                          : 1;
	uint32_t prc                          : 1;
	uint32_t plc                          : 1;
	uint32_t cec                          : 1;
	uint32_t cas                          : 1;
	uint32_t wce                          : 1;
	uint32_t wde                          : 1;
	uint32_t woe                          : 1;
	uint32_t reserved_28_29               : 2;
	uint32_t dr                           : 1;
	uint32_t wpr                          : 1;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_portscx_s    cn70xx;
	struct cvmx_usbdrdx_uahc_portscx_s    cn70xxp1;
	struct cvmx_usbdrdx_uahc_portscx_s    cn73xx;
	struct cvmx_usbdrdx_uahc_portscx_s    cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_portscx cvmx_usbdrdx_uahc_portscx_t;

/**
 * cvmx_usbdrd#_uahc_rtsoff
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.3.8.
 *
 */
union cvmx_usbdrdx_uahc_rtsoff {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_rtsoff_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t rtsoff                       : 27; /**< Runtime register-space offset. */
	uint32_t reserved_0_4                 : 5;
#else
	uint32_t reserved_0_4                 : 5;
	uint32_t rtsoff                       : 27;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_rtsoff_s     cn70xx;
	struct cvmx_usbdrdx_uahc_rtsoff_s     cn70xxp1;
	struct cvmx_usbdrdx_uahc_rtsoff_s     cn73xx;
	struct cvmx_usbdrdx_uahc_rtsoff_s     cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_rtsoff cvmx_usbdrdx_uahc_rtsoff_t;

/**
 * cvmx_usbdrd#_uahc_suptprt2_dw0
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.2.
 *
 */
union cvmx_usbdrdx_uahc_suptprt2_dw0 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_suptprt2_dw0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t majorrev                     : 8;  /**< Major revision. */
	uint32_t minorrev                     : 8;  /**< Minor revision. */
	uint32_t nextcapptr                   : 8;  /**< Next capability pointer. */
	uint32_t capid                        : 8;  /**< Capability ID = supported protocol. */
#else
	uint32_t capid                        : 8;
	uint32_t nextcapptr                   : 8;
	uint32_t minorrev                     : 8;
	uint32_t majorrev                     : 8;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_suptprt2_dw0_s cn70xx;
	struct cvmx_usbdrdx_uahc_suptprt2_dw0_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_suptprt2_dw0_s cn73xx;
	struct cvmx_usbdrdx_uahc_suptprt2_dw0_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_suptprt2_dw0 cvmx_usbdrdx_uahc_suptprt2_dw0_t;

/**
 * cvmx_usbdrd#_uahc_suptprt2_dw1
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.2.
 *
 */
union cvmx_usbdrdx_uahc_suptprt2_dw1 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_suptprt2_dw1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t name                         : 32; /**< Name string: 'USB'. */
#else
	uint32_t name                         : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_suptprt2_dw1_s cn70xx;
	struct cvmx_usbdrdx_uahc_suptprt2_dw1_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_suptprt2_dw1_s cn73xx;
	struct cvmx_usbdrdx_uahc_suptprt2_dw1_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_suptprt2_dw1 cvmx_usbdrdx_uahc_suptprt2_dw1_t;

/**
 * cvmx_usbdrd#_uahc_suptprt2_dw2
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.2.
 *
 */
union cvmx_usbdrdx_uahc_suptprt2_dw2 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_suptprt2_dw2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t psic                         : 4;  /**< Protocol speed ID count. */
	uint32_t reserved_21_27               : 7;
	uint32_t blc                          : 1;  /**< BESL LPM capability. */
	uint32_t hlc                          : 1;  /**< Hardware LMP capability. */
	uint32_t ihi                          : 1;  /**< Integrated hub implemented. */
	uint32_t hso                          : 1;  /**< High-speed only. */
	uint32_t reserved_16_16               : 1;
	uint32_t compatprtcnt                 : 8;  /**< Compatible port count. */
	uint32_t compatprtoff                 : 8;  /**< Compatible port offset. */
#else
	uint32_t compatprtoff                 : 8;
	uint32_t compatprtcnt                 : 8;
	uint32_t reserved_16_16               : 1;
	uint32_t hso                          : 1;
	uint32_t ihi                          : 1;
	uint32_t hlc                          : 1;
	uint32_t blc                          : 1;
	uint32_t reserved_21_27               : 7;
	uint32_t psic                         : 4;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_suptprt2_dw2_s cn70xx;
	struct cvmx_usbdrdx_uahc_suptprt2_dw2_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_suptprt2_dw2_s cn73xx;
	struct cvmx_usbdrdx_uahc_suptprt2_dw2_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_suptprt2_dw2 cvmx_usbdrdx_uahc_suptprt2_dw2_t;

/**
 * cvmx_usbdrd#_uahc_suptprt2_dw3
 *
 * For information on this register, refer to the xHCI Specification, v1.1, section 7.2.
 *
 */
union cvmx_usbdrdx_uahc_suptprt2_dw3 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_suptprt2_dw3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_5_31                : 27;
	uint32_t protslottype                 : 5;  /**< Protocol slot type. */
#else
	uint32_t protslottype                 : 5;
	uint32_t reserved_5_31                : 27;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_suptprt2_dw3_s cn70xx;
	struct cvmx_usbdrdx_uahc_suptprt2_dw3_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_suptprt2_dw3_s cn73xx;
	struct cvmx_usbdrdx_uahc_suptprt2_dw3_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_suptprt2_dw3 cvmx_usbdrdx_uahc_suptprt2_dw3_t;

/**
 * cvmx_usbdrd#_uahc_suptprt3_dw0
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.2.
 *
 */
union cvmx_usbdrdx_uahc_suptprt3_dw0 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_suptprt3_dw0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t majorrev                     : 8;  /**< Major revision. */
	uint32_t minorrev                     : 8;  /**< Minor revision. */
	uint32_t nextcapptr                   : 8;  /**< Next capability pointer. Value depends on USBDRD()_UAHC_GUCTL[EXTCAPSUPTEN]. If
                                                         EXTCAPSUPTEN
                                                         =
                                                         0, value is 0x0. If EXTCAPSUPTEN = 1, value is 0x4. */
	uint32_t capid                        : 8;  /**< Capability ID = supported protocol. */
#else
	uint32_t capid                        : 8;
	uint32_t nextcapptr                   : 8;
	uint32_t minorrev                     : 8;
	uint32_t majorrev                     : 8;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_suptprt3_dw0_s cn70xx;
	struct cvmx_usbdrdx_uahc_suptprt3_dw0_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_suptprt3_dw0_s cn73xx;
	struct cvmx_usbdrdx_uahc_suptprt3_dw0_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_suptprt3_dw0 cvmx_usbdrdx_uahc_suptprt3_dw0_t;

/**
 * cvmx_usbdrd#_uahc_suptprt3_dw1
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.2.
 *
 */
union cvmx_usbdrdx_uahc_suptprt3_dw1 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_suptprt3_dw1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t name                         : 32; /**< Name string: 'USB'. */
#else
	uint32_t name                         : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_suptprt3_dw1_s cn70xx;
	struct cvmx_usbdrdx_uahc_suptprt3_dw1_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_suptprt3_dw1_s cn73xx;
	struct cvmx_usbdrdx_uahc_suptprt3_dw1_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_suptprt3_dw1 cvmx_usbdrdx_uahc_suptprt3_dw1_t;

/**
 * cvmx_usbdrd#_uahc_suptprt3_dw2
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.2.
 *
 */
union cvmx_usbdrdx_uahc_suptprt3_dw2 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_suptprt3_dw2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t psic                         : 4;  /**< Protocol speed ID count. */
	uint32_t reserved_16_27               : 12;
	uint32_t compatprtcnt                 : 8;  /**< Compatible port count. */
	uint32_t compatprtoff                 : 8;  /**< Compatible port offset. */
#else
	uint32_t compatprtoff                 : 8;
	uint32_t compatprtcnt                 : 8;
	uint32_t reserved_16_27               : 12;
	uint32_t psic                         : 4;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_suptprt3_dw2_s cn70xx;
	struct cvmx_usbdrdx_uahc_suptprt3_dw2_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_suptprt3_dw2_s cn73xx;
	struct cvmx_usbdrdx_uahc_suptprt3_dw2_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_suptprt3_dw2 cvmx_usbdrdx_uahc_suptprt3_dw2_t;

/**
 * cvmx_usbdrd#_uahc_suptprt3_dw3
 *
 * For information on this register, refer to the xHCI Specification, v1.1, section 7.2.
 *
 */
union cvmx_usbdrdx_uahc_suptprt3_dw3 {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_suptprt3_dw3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_5_31                : 27;
	uint32_t protslottype                 : 5;  /**< Protocol slot type. */
#else
	uint32_t protslottype                 : 5;
	uint32_t reserved_5_31                : 27;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_suptprt3_dw3_s cn70xx;
	struct cvmx_usbdrdx_uahc_suptprt3_dw3_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_suptprt3_dw3_s cn73xx;
	struct cvmx_usbdrdx_uahc_suptprt3_dw3_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_suptprt3_dw3 cvmx_usbdrdx_uahc_suptprt3_dw3_t;

/**
 * cvmx_usbdrd#_uahc_usbcmd
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.1.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_usbcmd {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_usbcmd_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_12_31               : 20;
	uint32_t eu3s                         : 1;  /**< Enable U3 MFINDEX stop. */
	uint32_t ewe                          : 1;  /**< Enable wrap event. */
	uint32_t crs                          : 1;  /**< Controller restore state. */
	uint32_t css                          : 1;  /**< Controller save state. */
	uint32_t lhcrst                       : 1;  /**< Light host controller reset. */
	uint32_t reserved_4_6                 : 3;
	uint32_t hsee                         : 1;  /**< Host system error enable. */
	uint32_t inte                         : 1;  /**< Interrupter enable. */
	uint32_t hcrst                        : 1;  /**< Host controller reset. */
	uint32_t r_s                          : 1;  /**< Run/stop. */
#else
	uint32_t r_s                          : 1;
	uint32_t hcrst                        : 1;
	uint32_t inte                         : 1;
	uint32_t hsee                         : 1;
	uint32_t reserved_4_6                 : 3;
	uint32_t lhcrst                       : 1;
	uint32_t css                          : 1;
	uint32_t crs                          : 1;
	uint32_t ewe                          : 1;
	uint32_t eu3s                         : 1;
	uint32_t reserved_12_31               : 20;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_usbcmd_s     cn70xx;
	struct cvmx_usbdrdx_uahc_usbcmd_s     cn70xxp1;
	struct cvmx_usbdrdx_uahc_usbcmd_s     cn73xx;
	struct cvmx_usbdrdx_uahc_usbcmd_s     cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_usbcmd cvmx_usbdrdx_uahc_usbcmd_t;

/**
 * cvmx_usbdrd#_uahc_usblegctlsts
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.1.2. Note
 * that the SMI interrupts are not connected to anything in a CNXXXX configuration.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_usblegctlsts {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_usblegctlsts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t smi_on_bar                   : 1;  /**< System management interrupt on BAR. Never generated. */
	uint32_t smi_on_pci_command           : 1;  /**< System management interrupt on PCI command. Never generated. */
	uint32_t smi_on_os_ownership          : 1;  /**< System management interrupt on OS ownership change. This bit is set to 1 whenever
                                                         USBDRD()_UAHC_USBLEGSUP[HC_OS_OWNED_SEMAPHORES] transitions. */
	uint32_t reserved_21_28               : 8;
	uint32_t smi_on_hostsystemerr         : 1;  /**< System-management interrupt on host-system error. Shadow bit of USBDRD()_UAHC_USBSTS[HSE].
                                                         Refer to
                                                         xHCI Section 5.4.2 for definition and effects of the events associated with this bit being
                                                         set to 1.
                                                         To clear this bit to a 0, system software must write a 1 to USBDRD()_UAHC_USBSTS[HSE]. */
	uint32_t reserved_17_19               : 3;
	uint32_t smi_on_event_interrupt       : 1;  /**< System-management interrupt on event interrupt. Shadow bit of USBDRD()_UAHC_USBSTS[EINT].
                                                         Refer to
                                                         xHCI Section 5.4.2 for definition. This bit automatically clears when [EINT] clears and
                                                         sets when [EINT] sets. */
	uint32_t smi_on_bar_en                : 1;  /**< System-management interrupt on BAR enable. */
	uint32_t smi_on_pci_command_en        : 1;  /**< System-management interrupt on PCI command enable. */
	uint32_t smi_on_os_ownership_en       : 1;  /**< System-management interrupt on OS ownership enable. */
	uint32_t reserved_5_12                : 8;
	uint32_t smi_on_hostsystemerr_en      : 1;  /**< System-management interrupt on host-system error enable */
	uint32_t reserved_1_3                 : 3;
	uint32_t usb_smi_en                   : 1;  /**< USB system-management interrupt enable. */
#else
	uint32_t usb_smi_en                   : 1;
	uint32_t reserved_1_3                 : 3;
	uint32_t smi_on_hostsystemerr_en      : 1;
	uint32_t reserved_5_12                : 8;
	uint32_t smi_on_os_ownership_en       : 1;
	uint32_t smi_on_pci_command_en        : 1;
	uint32_t smi_on_bar_en                : 1;
	uint32_t smi_on_event_interrupt       : 1;
	uint32_t reserved_17_19               : 3;
	uint32_t smi_on_hostsystemerr         : 1;
	uint32_t reserved_21_28               : 8;
	uint32_t smi_on_os_ownership          : 1;
	uint32_t smi_on_pci_command           : 1;
	uint32_t smi_on_bar                   : 1;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_usblegctlsts_s cn70xx;
	struct cvmx_usbdrdx_uahc_usblegctlsts_s cn70xxp1;
	struct cvmx_usbdrdx_uahc_usblegctlsts_s cn73xx;
	struct cvmx_usbdrdx_uahc_usblegctlsts_s cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_usblegctlsts cvmx_usbdrdx_uahc_usblegctlsts_t;

/**
 * cvmx_usbdrd#_uahc_usblegsup
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.1.1.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_usblegsup {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_usblegsup_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_25_31               : 7;
	uint32_t hc_os_owned_semaphores       : 1;  /**< HC OS-owned semaphore. */
	uint32_t reserved_17_23               : 7;
	uint32_t hc_bios_owned_semaphores     : 1;  /**< HC BIOS-owned semaphore. */
	uint32_t nextcapptr                   : 8;  /**< Next xHCI extended-capability pointer. */
	uint32_t capid                        : 8;  /**< Capability ID = USB legacy support. */
#else
	uint32_t capid                        : 8;
	uint32_t nextcapptr                   : 8;
	uint32_t hc_bios_owned_semaphores     : 1;
	uint32_t reserved_17_23               : 7;
	uint32_t hc_os_owned_semaphores       : 1;
	uint32_t reserved_25_31               : 7;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_usblegsup_s  cn70xx;
	struct cvmx_usbdrdx_uahc_usblegsup_s  cn70xxp1;
	struct cvmx_usbdrdx_uahc_usblegsup_s  cn73xx;
	struct cvmx_usbdrdx_uahc_usblegsup_s  cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_usblegsup cvmx_usbdrdx_uahc_usblegsup_t;

/**
 * cvmx_usbdrd#_uahc_usbsts
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.2.
 *
 * This register can be reset by IOI reset,
 * or USBDRD()_UCTL_CTL[UAHC_RST],
 * or USBDRD()_UAHC_GCTL[CORESOFTRESET],
 * or USBDRD()_UAHC_USBCMD[HCRST], or USBDRD()_UAHC_USBCMD[LHCRST].
 */
union cvmx_usbdrdx_uahc_usbsts {
	uint32_t u32;
	struct cvmx_usbdrdx_uahc_usbsts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_13_31               : 19;
	uint32_t hce                          : 1;  /**< Host controller error. */
	uint32_t cnr                          : 1;  /**< Controller not ready. */
	uint32_t sre                          : 1;  /**< Save/restore error. */
	uint32_t rss                          : 1;  /**< Restore state status. */
	uint32_t sss                          : 1;  /**< Save state status. */
	uint32_t reserved_5_7                 : 3;
	uint32_t pcd                          : 1;  /**< Port change detect. */
	uint32_t eint                         : 1;  /**< Event interrupt. */
	uint32_t hse                          : 1;  /**< Host system error. The typical software response to an HSE is to reset the core. */
	uint32_t reserved_1_1                 : 1;
	uint32_t hch                          : 1;  /**< HC halted. */
#else
	uint32_t hch                          : 1;
	uint32_t reserved_1_1                 : 1;
	uint32_t hse                          : 1;
	uint32_t eint                         : 1;
	uint32_t pcd                          : 1;
	uint32_t reserved_5_7                 : 3;
	uint32_t sss                          : 1;
	uint32_t rss                          : 1;
	uint32_t sre                          : 1;
	uint32_t cnr                          : 1;
	uint32_t hce                          : 1;
	uint32_t reserved_13_31               : 19;
#endif
	} s;
	struct cvmx_usbdrdx_uahc_usbsts_s     cn70xx;
	struct cvmx_usbdrdx_uahc_usbsts_s     cn70xxp1;
	struct cvmx_usbdrdx_uahc_usbsts_s     cn73xx;
	struct cvmx_usbdrdx_uahc_usbsts_s     cnf75xx;
};
typedef union cvmx_usbdrdx_uahc_usbsts cvmx_usbdrdx_uahc_usbsts_t;

/**
 * cvmx_usbdrd#_uctl_bist_status
 *
 * This register indicates the results from the built-in self-test (BIST) runs of USBDRD
 * memories.
 * A 0 indicates pass or never run, a 1 indicates fail. This register can be reset by IOI reset.
 */
union cvmx_usbdrdx_uctl_bist_status {
	uint64_t u64;
	struct cvmx_usbdrdx_uctl_bist_status_s {
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
	} s;
	struct cvmx_usbdrdx_uctl_bist_status_s cn70xx;
	struct cvmx_usbdrdx_uctl_bist_status_s cn70xxp1;
	struct cvmx_usbdrdx_uctl_bist_status_s cn73xx;
	struct cvmx_usbdrdx_uctl_bist_status_s cnf75xx;
};
typedef union cvmx_usbdrdx_uctl_bist_status cvmx_usbdrdx_uctl_bist_status_t;

/**
 * cvmx_usbdrd#_uctl_ctl
 *
 * This register controls clocks, resets, power, and BIST.
 *
 * This register can be reset by IOI reset.
 */
union cvmx_usbdrdx_uctl_ctl {
	uint64_t u64;
	struct cvmx_usbdrdx_uctl_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t clear_bist                   : 1;  /**< BIST fast-clear mode select. A BIST run with this bit set clears all entries in USBDRD
                                                         RAMs
                                                         to 0x0.
                                                         There are two major modes of BIST: full and clear. Full BIST is run by the BIST state
                                                         machine when CLEAR_BIST is deasserted during BIST. Clear BIST is run if CLEAR_BIST is
                                                         asserted during BIST.
                                                         To avoid race conditions, software must first perform a CSR write operation that puts the
                                                         CLEAR_BIST setting into the correct state and then perform another CSR write operation to
                                                         set the BIST trigger (keeping the CLEAR_BIST state constant).
                                                         CLEAR BIST completion is indicated by USBDRD()_UCTL_BIST_STATUS. A BIST clear operation
                                                         takes almost 2,000 controller-clock cycles for the largest RAM. */
	uint64_t start_bist                   : 1;  /**< Rising edge starts BIST on the memories in USBDRD.
                                                         To run BIST, the controller clock must be both configured and enabled, and should be
                                                         configured to the maximum available frequency given the available coprocessor clock and
                                                         dividers.
                                                         Also, the UCTL, UAHC, and UPHY should be held in software- initiated reset (using
                                                         UPHY_RST, UAHC_RST, UCTL_RST) until BIST is complete.
                                                         BIST defect status can be checked after FULL BIST completion, both of which are indicated
                                                         in USBDRD()_UCTL_BIST_STATUS. The full BIST run takes almost 80,000 controller-clock
                                                         cycles
                                                         for
                                                         the largest RAM. */
	uint64_t ref_clk_sel                  : 2;  /**< Reference clock select. Choose reference-clock source for the SuperSpeed and high-speed
                                                         PLL blocks.
                                                         0x0 = Reference clock sources for both PLLs come from the USB pads.
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
                                                         A value of 0x0 means this feature is disabled.
                                                         The legal values are:
                                                           If REF_CLK_SEL = 0x0 or 0x1, then:
                                                             0x0 is the only legal value.
                                                           If REF_CLK_SEL = 0x2 or 0x3, then:
                                                             0x0:   if DLMC_REF_CLK* is another supported frequency (see list in
                                                                    MPLL_MULTIPLIER description).
                                                         All other values are reserved.
                                                         This value may only be changed during [UPHY_RST].
                                                         Note: If REF_CLK_SEL = 0x2 or 0x3, then MPLL_MULTPLIER, REF_CLK_DIV2, and SSC_REF_CLK_SEL
                                                         must all be programmed to the same frequency setting. */
	uint64_t mpll_multiplier              : 7;  /**< Multiplies the reference clock to a frequency suitable for intended operating speed. The
                                                         legal values are:
                                                           If REF_CLK_SEL = 0x0 or 0x1, then:
                                                             0x19 = 100  MHz on DLMC_REF_CLK*
                                                           If REF_CLK_SEL = 0x2 or 0x3, then:
                                                             0x32 =  50  MHz on DLMC_REF_CLK*
                                                             0x19 =  100 MHz on DLMC_REF_CLK*
                                                             0x28 =  125 MHz on DLMC_REF_CLK*
                                                         All other values are reserved.
                                                         This value may only be changed during [UPHY_RST].
                                                         Note: If REF_CLK_SEL = 0x2 or 0x3, then MPLL_MULTPLIER, REF_CLK_DIV2, and SSC_REF_CLK_SEL
                                                         must all be programmed to the same frequency setting. */
	uint64_t ref_ssp_en                   : 1;  /**< Enables reference clock to the prescaler for SuperSpeed function. This should always be
                                                         enabled since this output clock is used to drive the UAHC suspend-mode clock during
                                                         low-power states.
                                                         This value can be changed only during UPHY_RST or during low-power states.
                                                         The reference clock must be running and stable before UPHY_RST is deasserted and before
                                                         REF_SSP_EN is asserted. */
	uint64_t ref_clk_div2                 : 1;  /**< Divides the reference clock by 2 before feeding it into the REF_CLK_FSEL divider.
                                                         The legal values are:
                                                           If REF_CLK_SEL = 0x0 or 0x1, then:
                                                             all DLMC_REF_CLK* frequencies: 0x0 is the only legal value.
                                                           If REF_CLK_SEL = 0x2 or 0x3, then:
                                                             0x1: if DLMC_REF_CLK* is 125MHz.
                                                             0x0: if DLMC_REF_CLK* is another supported frequency (see list in
                                                                  MPLL_MULTIPLIER description).
                                                         This value can be changed only during UPHY_RST.
                                                         Note: If REF_CLK_SEL = 0x2 or 0x3, then MPLL_MULTPLIER, REF_CLK_DIV2, and SSC_REF_CLK_SEL
                                                         must all be programmed to the same frequency setting. */
	uint64_t ref_clk_fsel                 : 6;  /**< Selects the reference clock frequency for the SuperSpeed and high-speed PLL blocks.
                                                         The legal values are:
                                                           If REF_CLK_SEL = 0x0 or 0x1, then:
                                                             0x27 = 100  MHz on DLMC_REF_CLK*
                                                           If REF_CLK_SEL = 0x2 or 0x3, then:
                                                             0x07 is the only legal value.
                                                         All other values are reserved.
                                                         This value may only be changed during [UPHY_RST].
                                                         Note: When REF_CLK_SEL = 0x2 or 0x3, the MPLL_MULTIPLIER, REF_CLK_DIV2, and
                                                         SSC_REF_CLK_SEL settings are used to configure the SuperSpeed reference clock
                                                         multiplier. */
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
                                                         The hclk frequency must be at or below 300 MHz.
                                                         The hclk frequency must be at or above 150 MHz for full-rate USB3
                                                         operation.
                                                         The hclk frequency must be at or above 125 MHz for any USB3
                                                         functionality.
                                                         If [DRD_MODE] = DEVICE, the hclk frequency must be at or above 125 MHz for
                                                         correct USB2 functionality.
                                                         If [DRD_MODE] = HOST, the hclk frequency must be at or above 90 MHz
                                                         for full-rate USB2 operation.
                                                         If [DRD_MODE] = HOST, the hclk frequency must be at or above 62.5 MHz
                                                         for any USB2 operation.
                                                         This field can be changed only when [H_CLKDIV_RST] = 1. */
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
                                                         USBDRD()_UAHC_HCSPARAMS1[MAXPORTS] is not affected by this signal.
                                                         This is a strap signal; it should be modified only when [UPHY_RST] is asserted. */
	uint64_t reserved_17_17               : 1;
	uint64_t usb2_port_disable            : 1;  /**< Disables USB2 (high-speed/full-speed/low-speed) portion of this PHY. When set to 1, this
                                                         signal stops reporting connect/disconnect events on the port and keeps the port in
                                                         disabled state. This could be used for security reasons where hardware can disable a port
                                                         regardless of whether xHCI driver enables a port or not.
                                                         USBDRD()_UAHC_HCSPARAMS1[MAXPORTS] is not affected by this signal.
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
	uint64_t drd_mode                     : 1;  /**< Switches between host or device mode for USBDRD.
                                                         1 - Device
                                                         0 - Host */
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
	uint64_t drd_mode                     : 1;
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
	struct cvmx_usbdrdx_uctl_ctl_s        cn70xx;
	struct cvmx_usbdrdx_uctl_ctl_s        cn70xxp1;
	struct cvmx_usbdrdx_uctl_ctl_s        cn73xx;
	struct cvmx_usbdrdx_uctl_ctl_s        cnf75xx;
};
typedef union cvmx_usbdrdx_uctl_ctl cvmx_usbdrdx_uctl_ctl_t;

/**
 * cvmx_usbdrd#_uctl_ecc
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
 * This register is accessible only when USBDRD()_UCTL_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UCTL_RST].
 */
union cvmx_usbdrdx_uctl_ecc {
	uint64_t u64;
	struct cvmx_usbdrdx_uctl_ecc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t ecc_err_source               : 4;  /**< Source of ECC error, see USBDRD_UCTL_ECC_ERR_SOURCE_E. */
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
	struct cvmx_usbdrdx_uctl_ecc_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t ecc_err_source               : 4;  /**< Source of ECC error, see USBDRD_UCTL_ECC_ERR_SOURCE_E. */
	uint64_t ecc_err_syndrome             : 8;  /**< Syndrome bits of the ECC error. */
	uint64_t ecc_err_address              : 16; /**< RAM address of the ECC error. */
	uint64_t reserved_9_31                : 23;
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
	uint64_t reserved_9_31                : 23;
	uint64_t ecc_err_address              : 16;
	uint64_t ecc_err_syndrome             : 8;
	uint64_t ecc_err_source               : 4;
	uint64_t reserved_60_63               : 4;
#endif
	} cn70xx;
	struct cvmx_usbdrdx_uctl_ecc_cn70xx   cn70xxp1;
	struct cvmx_usbdrdx_uctl_ecc_s        cn73xx;
	struct cvmx_usbdrdx_uctl_ecc_s        cnf75xx;
};
typedef union cvmx_usbdrdx_uctl_ecc cvmx_usbdrdx_uctl_ecc_t;

/**
 * cvmx_usbdrd#_uctl_host_cfg
 *
 * This register allows configuration of various host controller (UAHC) features. Most of these
 * are strap signals and should be modified only while the controller is not running.
 *
 * This register is accessible only when USBDRD()_UCTL_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UCTL_RST].
 */
union cvmx_usbdrdx_uctl_host_cfg {
	uint64_t u64;
	struct cvmx_usbdrdx_uctl_host_cfg_s {
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
                                                         0 = USBDRD()_UAHC_HCCPARAMS[PPC] report port-power-control feature is unavailable.
                                                         1 = USBDRD()_UAHC_HCCPARAMS[PPC] reports port-power-control feature is available. PPC
                                                         output
                                                         from UAHC is taken to the GPIO signals and sense-converted based on PPC_ACTIVE_HIGH_EN.
                                                         The MIO GPIO multiplexer must be programmed accordingly.
                                                         This is a strap signal; it should only be modified when either the UCTL_CTL[UAHC] or
                                                         UAHC_GCTL[CORESOFTRESET] is asserted. */
	uint64_t ppc_active_high_en           : 1;  /**< Port power control sense selection. The active-high port-power-control output to off-chip
                                                         source is converted to match the off-chip sense.
                                                         0 = Port-power control to off-chip source is active-low.
                                                         1 = Port-power control to off-chip source is active-high.
                                                         This is a strap signal; it should only be modified when either the UCTL_CTL[UAHC] or
                                                         UAHC_GCTL[CORESOFTRESET] is asserted. */
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
	struct cvmx_usbdrdx_uctl_host_cfg_s   cn70xx;
	struct cvmx_usbdrdx_uctl_host_cfg_s   cn70xxp1;
	struct cvmx_usbdrdx_uctl_host_cfg_s   cn73xx;
	struct cvmx_usbdrdx_uctl_host_cfg_s   cnf75xx;
};
typedef union cvmx_usbdrdx_uctl_host_cfg cvmx_usbdrdx_uctl_host_cfg_t;

/**
 * cvmx_usbdrd#_uctl_intstat
 *
 * This register provides a summary of different bits of RSL interrupts. DBEs are detected and
 * SBE are corrected. For debugging output for ECC DBEs/SBEs, see USBDRD()_UCTL_ECC. This
 * register
 * can
 * be reset by IOI reset.
 */
union cvmx_usbdrdx_uctl_intstat {
	uint64_t u64;
	struct cvmx_usbdrdx_uctl_intstat_s {
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
                                                         USBDRD()_UCTL_SHIM_CFG[XM_BAD_DMA_*]. Received a DMA request from UAHC that violates
                                                         the assumptions made by the AXI-to-IOI shim. Such scenarios include: illegal length/size
                                                         combinations and address out-of-bounds.
                                                         For more information on exact failures, see the description in
                                                         USBDRD()_UCTL_SHIM_CFG[XM_BAD_DMA_TYPE]. The hardware does not translate the request
                                                         correctly
                                                         and results may violate IOI protocols. */
	uint64_t xs_ncb_oob                   : 1;  /**< Detected out-of-bound register access to UAHC over IOI. The UAHC defines 1MB of register
                                                         space, starting at offset 0x0. Any accesses outside of this register space cause this bit
                                                         to be set to 1. Error information is logged in USBDRD()_UCTL_SHIM_CFG[XS_NCB_OOB_*]. */
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
	struct cvmx_usbdrdx_uctl_intstat_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_22_63               : 42;
	uint64_t ram2_dbe                     : 1;  /**< Detected double-bit error on the UAHC RxFIFO RAMs (RAM2). */
	uint64_t ram2_sbe                     : 1;  /**< Detected single-bit error on the UAHC RxFIFO RAMs (RAM2). */
	uint64_t ram1_dbe                     : 1;  /**< Detected double-bit error on the UAHC TxFIFO RAMs (RAM1). */
	uint64_t ram1_sbe                     : 1;  /**< Detected single-bit error on the UAHC TxFIFO RAMs (RAM1). */
	uint64_t ram0_dbe                     : 1;  /**< Detected double-bit error on the UAHC Desc/Reg Cache (RAM0). */
	uint64_t ram0_sbe                     : 1;  /**< Detected single-bit error on the UAHC Desc/Reg Cache (RAM0). */
	uint64_t reserved_3_15                : 13;
	uint64_t xm_bad_dma                   : 1;  /**< Detected bad DMA access from UAHC to IOI. Error information is logged in
                                                         USBDRD()_UCTL_SHIM_CFG[XM_BAD_DMA_*]. Received a DMA request from UAHC that violates
                                                         the assumptions made by the AXI-to-IOI shim. Such scenarios include: illegal length/size
                                                         combinations and address out-of-bounds.
                                                         For more information on exact failures, see the description in
                                                         USBDRD()_UCTL_SHIM_CFG[XM_BAD_DMA_TYPE]. The hardware does not translate the request
                                                         correctly
                                                         and results may violate IOI protocols. */
	uint64_t xs_ncb_oob                   : 1;  /**< Detected out-of-bound register access to UAHC over IOI. The UAHC defines 1MB of register
                                                         space, starting at offset 0x0. Any accesses outside of this register space cause this bit
                                                         to be set to 1. Error information is logged in USBDRD()_UCTL_SHIM_CFG[XS_NCB_OOB_*]. */
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
	uint64_t reserved_22_63               : 42;
#endif
	} cn70xx;
	struct cvmx_usbdrdx_uctl_intstat_cn70xx cn70xxp1;
	struct cvmx_usbdrdx_uctl_intstat_s    cn73xx;
	struct cvmx_usbdrdx_uctl_intstat_s    cnf75xx;
};
typedef union cvmx_usbdrdx_uctl_intstat cvmx_usbdrdx_uctl_intstat_t;

/**
 * cvmx_usbdrd#_uctl_port#_cfg_hs
 *
 * This register controls configuration and test controls for the high-speed port 0 PHY.
 *
 * This register is accessible only when USBDRD()_UCTL_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UCTL_RST].
 */
union cvmx_usbdrdx_uctl_portx_cfg_hs {
	uint64_t u64;
	struct cvmx_usbdrdx_uctl_portx_cfg_hs_s {
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
                                                         0 = 2*, long preemphasis current duration (design default)
                                                         1 = 1*, short preemphasis current duration
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
	uint64_t reserved_7_31                : 25;
	uint64_t otgtune                      : 3;  /**< "VBUS Valid Threshold Adjustment
                                                         Function: This bus adjusts the voltage level for the VBUS<\#>
                                                         valid threshold. To enable tuning at the board level, connect this
                                                         bus to a register.
                                                         Note: A positive binary bit setting change results in a +3%
                                                         incremental change in threshold voltage level, while a negative
                                                         binary bit setting change results in a -3% incremental change
                                                         in threshold voltage level. " */
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
	uint64_t otgtune                      : 3;
	uint64_t reserved_7_31                : 25;
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
	struct cvmx_usbdrdx_uctl_portx_cfg_hs_s cn70xx;
	struct cvmx_usbdrdx_uctl_portx_cfg_hs_s cn70xxp1;
	struct cvmx_usbdrdx_uctl_portx_cfg_hs_s cn73xx;
	struct cvmx_usbdrdx_uctl_portx_cfg_hs_s cnf75xx;
};
typedef union cvmx_usbdrdx_uctl_portx_cfg_hs cvmx_usbdrdx_uctl_portx_cfg_hs_t;

/**
 * cvmx_usbdrd#_uctl_port#_cfg_ss
 *
 * This register controls configuration and test controls for the SS port 0 PHY.
 *
 * This register is accessible only when USBDRD()_UCTL_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UCTL_RST].
 */
union cvmx_usbdrdx_uctl_portx_cfg_ss {
	uint64_t u64;
	struct cvmx_usbdrdx_uctl_portx_cfg_ss_s {
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
                                                         If the USBDRD()_UCTL_CTL[REF_CLK_DIV2] is used, then
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
                                                         USBDRD()_UAHC_GUSB3PIPECTL()[TXDEEMPHASIS] is set to
                                                         0x1 (according to the PIPE3 specification). The values for transmit deemphasis are derived
                                                         from the following equation:
                                                         _ TX deemphasis (db) = 20 * log_base_10((128 - 2 * pcs_tx_deemph)/128)
                                                         In general, the parameter controls are static signals to be set prior to taking the PHY
                                                         out of reset. However, you can dynamically change these values on-the-fly for test
                                                         purposes. In this case, changes to the transmitter to reflect the current value occur only
                                                         after USBDRD()_UAHC_GUSB3PIPECTL()[TXDEEMPHASIS] changes. */
	uint64_t pcs_tx_deemph_6db            : 6;  /**< Fine-tune transmitter driver deemphasis when set to 6 db.
                                                         This static value sets the TX driver deemphasis value when
                                                         USBDRD()_UAHC_GUSB3PIPECTL()[TXDEEMPHASIS]  is set to
                                                         0x0 (according to the PIPE3 specification). This bus is provided for completeness and as a
                                                         second potential launch amplitude. The values for transmit deemphasis are derived from the
                                                         following equation:
                                                         _ TX deemphasis (db) = 20 * log_base_10((128 - 2 * pcs_tx_deemph)/128)
                                                         In general, the parameter controls are static signals to be set prior to taking the PHY
                                                         out of reset. However, you can dynamically change these values on-the-fly for test
                                                         purposes. In this case, changes to the transmitter to reflect the current value occur only
                                                         after USBDRD()_UAHC_GUSB3PIPECTL()[TXDEEMPHASIS] changes. */
	uint64_t pcs_tx_swing_full            : 7;  /**< Launch amplitude of the transmitter. Sets the launch amplitude of the transmitter. The
                                                         values for transmit amplitude are derived from the following equation:
                                                         TX amplitude (V) = vptx * ((pcs_tx_swing_full + 1)/128)
                                                         In general, the parameter controls are static signals to be set prior to taking the PHY
                                                         out of reset. However, you can dynamically change these values on-the-fly for test
                                                         purposes. In this case, changes to the transmitter to reflect the current value occur only
                                                         after USBDRD()_UAHC_GUSB3PIPECTL()[TXDEEMPHASIS] changes. */
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
	struct cvmx_usbdrdx_uctl_portx_cfg_ss_s cn70xx;
	struct cvmx_usbdrdx_uctl_portx_cfg_ss_s cn70xxp1;
	struct cvmx_usbdrdx_uctl_portx_cfg_ss_s cn73xx;
	struct cvmx_usbdrdx_uctl_portx_cfg_ss_s cnf75xx;
};
typedef union cvmx_usbdrdx_uctl_portx_cfg_ss cvmx_usbdrdx_uctl_portx_cfg_ss_t;

/**
 * cvmx_usbdrd#_uctl_port#_cr_dbg_cfg
 *
 * This register allows indirect access to the configuration and test controls for the port 0
 * PHY.
 *
 * This register is accessible only when USBDRD()_UCTL_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UCTL_RST].
 */
union cvmx_usbdrdx_uctl_portx_cr_dbg_cfg {
	uint64_t u64;
	struct cvmx_usbdrdx_uctl_portx_cr_dbg_cfg_s {
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
	struct cvmx_usbdrdx_uctl_portx_cr_dbg_cfg_s cn70xx;
	struct cvmx_usbdrdx_uctl_portx_cr_dbg_cfg_s cn70xxp1;
	struct cvmx_usbdrdx_uctl_portx_cr_dbg_cfg_s cn73xx;
	struct cvmx_usbdrdx_uctl_portx_cr_dbg_cfg_s cnf75xx;
};
typedef union cvmx_usbdrdx_uctl_portx_cr_dbg_cfg cvmx_usbdrdx_uctl_portx_cr_dbg_cfg_t;

/**
 * cvmx_usbdrd#_uctl_port#_cr_dbg_status
 *
 * This register allows indirect access to the configuration and test controls for the port 0
 * PHY.
 *
 * This register is accessible only when USBDRD()_UCTL_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UCTL_RST].
 */
union cvmx_usbdrdx_uctl_portx_cr_dbg_status {
	uint64_t u64;
	struct cvmx_usbdrdx_uctl_portx_cr_dbg_status_s {
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
	struct cvmx_usbdrdx_uctl_portx_cr_dbg_status_s cn70xx;
	struct cvmx_usbdrdx_uctl_portx_cr_dbg_status_s cn70xxp1;
	struct cvmx_usbdrdx_uctl_portx_cr_dbg_status_s cn73xx;
	struct cvmx_usbdrdx_uctl_portx_cr_dbg_status_s cnf75xx;
};
typedef union cvmx_usbdrdx_uctl_portx_cr_dbg_status cvmx_usbdrdx_uctl_portx_cr_dbg_status_t;

/**
 * cvmx_usbdrd#_uctl_shim_cfg
 *
 * This register allows configuration of various shim (UCTL) features. The fields XS_NCB_OOB_*
 * are captured when there are no outstanding OOB errors indicated in INTSTAT and a new OOB error
 * arrives. The fields XS_BAD_DMA_* are captured when there are no outstanding DMA errors
 * indicated in INTSTAT and a new DMA error arrives.
 *
 * This register is accessible only when USBDRD()_UCTL_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UCTL_RST].
 */
union cvmx_usbdrdx_uctl_shim_cfg {
	uint64_t u64;
	struct cvmx_usbdrdx_uctl_shim_cfg_s {
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
                                                         0 = read error log, 1 = write error log */
	uint64_t reserved_44_46               : 3;
	uint64_t xm_bad_dma_type              : 4;  /**< ErrType error log for bad DMA access from UAHC. Encodes the type of error encountered
                                                         (error largest encoded value has priority). See USBDRD_UCTL_XM_BAD_DMA_TYPE_E. */
	uint64_t reserved_14_39               : 26;
	uint64_t dma_read_cmd                 : 2;  /**< Selects the IOI read command used by DMA accesses. See USBDRD_UCTL_DMA_READ_CMD_E. */
	uint64_t reserved_11_11               : 1;
	uint64_t dma_write_cmd                : 1;  /**< Selects the NCB write command used by DMA accesses. See UCTL_DMA_WRITE_CMD_E. */
	uint64_t dma_endian_mode              : 2;  /**< Selects the endian format for DMA accesses to the L2C. See USBDRD_UCTL_ENDIAN_MODE_E. */
	uint64_t reserved_2_7                 : 6;
	uint64_t csr_endian_mode              : 2;  /**< Selects the endian format for IOI CSR accesses to the UAHC. Note that when UAHC CSRs are
                                                         accessed via RSL, they are returned as big-endian. See USBDRD_UCTL_ENDIAN_MODE_E. */
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
	struct cvmx_usbdrdx_uctl_shim_cfg_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t xs_ncb_oob_wrn               : 1;  /**< Read/write error log for out-of-bound UAHC register access.
                                                         0 = read, 1 = write. */
	uint64_t reserved_60_62               : 3;
	uint64_t xs_ncb_oob_osrc              : 12; /**< SRCID error log for out-of-bound UAHC register access. The IOI outbound SRCID for the OOB
                                                         error.
                                                         <59:58> = chipID.
                                                         <57> = Request source: 0 = core, 1 = NCB-device.
                                                         <56:51> = Core/NCB-device number. Note that for NCB devices, <56> is always 0.
                                                         <50:48> = SubID. */
	uint64_t xm_bad_dma_wrn               : 1;  /**< Read/write error log for bad DMA access from UAHC.
                                                         0 = read error log, 1 = write error log */
	uint64_t reserved_44_46               : 3;
	uint64_t xm_bad_dma_type              : 4;  /**< ErrType error log for bad DMA access from UAHC. Encodes the type of error encountered
                                                         (error largest encoded value has priority). See USBDRD_UCTL_XM_BAD_DMA_TYPE_E. */
	uint64_t reserved_13_39               : 27;
	uint64_t dma_read_cmd                 : 1;  /**< Selects the IOI read command used by DMA accesses. See USBDRD_UCTL_DMA_READ_CMD_E. */
	uint64_t reserved_10_11               : 2;
	uint64_t dma_endian_mode              : 2;  /**< Selects the endian format for DMA accesses to the L2C. See USBDRD_UCTL_ENDIAN_MODE_E. */
	uint64_t reserved_2_7                 : 6;
	uint64_t csr_endian_mode              : 2;  /**< Selects the endian format for IOI CSR accesses to the UAHC. Note that when UAHC CSRs are
                                                         accessed via RSL, they are returned as big-endian. See USBDRD_UCTL_ENDIAN_MODE_E. */
#else
	uint64_t csr_endian_mode              : 2;
	uint64_t reserved_2_7                 : 6;
	uint64_t dma_endian_mode              : 2;
	uint64_t reserved_10_11               : 2;
	uint64_t dma_read_cmd                 : 1;
	uint64_t reserved_13_39               : 27;
	uint64_t xm_bad_dma_type              : 4;
	uint64_t reserved_44_46               : 3;
	uint64_t xm_bad_dma_wrn               : 1;
	uint64_t xs_ncb_oob_osrc              : 12;
	uint64_t reserved_60_62               : 3;
	uint64_t xs_ncb_oob_wrn               : 1;
#endif
	} cn70xx;
	struct cvmx_usbdrdx_uctl_shim_cfg_cn70xx cn70xxp1;
	struct cvmx_usbdrdx_uctl_shim_cfg_s   cn73xx;
	struct cvmx_usbdrdx_uctl_shim_cfg_s   cnf75xx;
};
typedef union cvmx_usbdrdx_uctl_shim_cfg cvmx_usbdrdx_uctl_shim_cfg_t;

/**
 * cvmx_usbdrd#_uctl_spare0
 *
 * Accessible by: always
 * This register can be reset by IOI reset.
 * This register is spare.
 */
union cvmx_usbdrdx_uctl_spare0 {
	uint64_t u64;
	struct cvmx_usbdrdx_uctl_spare0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_usbdrdx_uctl_spare0_s     cn70xx;
	struct cvmx_usbdrdx_uctl_spare0_s     cn70xxp1;
};
typedef union cvmx_usbdrdx_uctl_spare0 cvmx_usbdrdx_uctl_spare0_t;

/**
 * cvmx_usbdrd#_uctl_spare0_eco
 *
 * This register is a spare register. This register can be reset by IOI reset.
 *
 */
union cvmx_usbdrdx_uctl_spare0_eco {
	uint64_t u64;
	struct cvmx_usbdrdx_uctl_spare0_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t eco_rw                       : 32; /**< N/A */
#else
	uint64_t eco_rw                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uctl_spare0_eco_s cn73xx;
	struct cvmx_usbdrdx_uctl_spare0_eco_s cnf75xx;
};
typedef union cvmx_usbdrdx_uctl_spare0_eco cvmx_usbdrdx_uctl_spare0_eco_t;

/**
 * cvmx_usbdrd#_uctl_spare1
 *
 * Accessible by: only when H_CLKDIV_EN.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UCTL_RST].
 */
union cvmx_usbdrdx_uctl_spare1 {
	uint64_t u64;
	struct cvmx_usbdrdx_uctl_spare1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_usbdrdx_uctl_spare1_s     cn70xx;
	struct cvmx_usbdrdx_uctl_spare1_s     cn70xxp1;
};
typedef union cvmx_usbdrdx_uctl_spare1 cvmx_usbdrdx_uctl_spare1_t;

/**
 * cvmx_usbdrd#_uctl_spare1_eco
 *
 * This register is accessible only when USBDRD()_UCTL_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or with USBDRD()_UCTL_CTL[UCTL_RST].
 */
union cvmx_usbdrdx_uctl_spare1_eco {
	uint64_t u64;
	struct cvmx_usbdrdx_uctl_spare1_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t eco_rw                       : 32; /**< N/A */
#else
	uint64_t eco_rw                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_usbdrdx_uctl_spare1_eco_s cn73xx;
	struct cvmx_usbdrdx_uctl_spare1_eco_s cnf75xx;
};
typedef union cvmx_usbdrdx_uctl_spare1_eco cvmx_usbdrdx_uctl_spare1_eco_t;

#endif
