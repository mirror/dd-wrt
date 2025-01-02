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
 * cvmx-uahcx-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon uahcx.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_UAHCX_DEFS_H__
#define __CVMX_UAHCX_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_CAPLENGTH(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_CAPLENGTH(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000000ull);
}
#else
#define CVMX_UAHCX_CAPLENGTH(offset) (CVMX_ADD_IO_SEG(0x0001680000000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_CONFIG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_CONFIG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000058ull);
}
#else
#define CVMX_UAHCX_CONFIG(offset) (CVMX_ADD_IO_SEG(0x0001680000000058ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_CRCR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_CRCR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000038ull);
}
#else
#define CVMX_UAHCX_CRCR(offset) (CVMX_ADD_IO_SEG(0x0001680000000038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_DBOFF(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_DBOFF(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000014ull);
}
#else
#define CVMX_UAHCX_DBOFF(offset) (CVMX_ADD_IO_SEG(0x0001680000000014ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_DBX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 64)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 64)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_DBX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000480ull) + (((offset) & 127) + ((block_id) & 0) * 0x0ull) * 4;
}
#else
#define CVMX_UAHCX_DBX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000480ull) + (((offset) & 127) + ((block_id) & 0) * 0x0ull) * 4)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_DCBAAP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_DCBAAP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000050ull);
}
#else
#define CVMX_UAHCX_DCBAAP(offset) (CVMX_ADD_IO_SEG(0x0001680000000050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_DNCTRL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_DNCTRL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000034ull);
}
#else
#define CVMX_UAHCX_DNCTRL(offset) (CVMX_ADD_IO_SEG(0x0001680000000034ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_ASYNCLISTADDR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_ASYNCLISTADDR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000028ull);
}
#else
#define CVMX_UAHCX_EHCI_ASYNCLISTADDR(offset) (CVMX_ADD_IO_SEG(0x00016F0000000028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_CONFIGFLAG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_CONFIGFLAG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000050ull);
}
#else
#define CVMX_UAHCX_EHCI_CONFIGFLAG(offset) (CVMX_ADD_IO_SEG(0x00016F0000000050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_CTRLDSSEGMENT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_CTRLDSSEGMENT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000020ull);
}
#else
#define CVMX_UAHCX_EHCI_CTRLDSSEGMENT(offset) (CVMX_ADD_IO_SEG(0x00016F0000000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_FRINDEX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_FRINDEX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F000000001Cull);
}
#else
#define CVMX_UAHCX_EHCI_FRINDEX(offset) (CVMX_ADD_IO_SEG(0x00016F000000001Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_HCCAPBASE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_HCCAPBASE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000000ull);
}
#else
#define CVMX_UAHCX_EHCI_HCCAPBASE(offset) (CVMX_ADD_IO_SEG(0x00016F0000000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_HCCPARAMS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_HCCPARAMS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000008ull);
}
#else
#define CVMX_UAHCX_EHCI_HCCPARAMS(offset) (CVMX_ADD_IO_SEG(0x00016F0000000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_HCSPARAMS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_HCSPARAMS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000004ull);
}
#else
#define CVMX_UAHCX_EHCI_HCSPARAMS(offset) (CVMX_ADD_IO_SEG(0x00016F0000000004ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_INSNREG00(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_INSNREG00(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000090ull);
}
#else
#define CVMX_UAHCX_EHCI_INSNREG00(offset) (CVMX_ADD_IO_SEG(0x00016F0000000090ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_INSNREG03(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_INSNREG03(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F000000009Cull);
}
#else
#define CVMX_UAHCX_EHCI_INSNREG03(offset) (CVMX_ADD_IO_SEG(0x00016F000000009Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_INSNREG04(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_INSNREG04(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F00000000A0ull);
}
#else
#define CVMX_UAHCX_EHCI_INSNREG04(offset) (CVMX_ADD_IO_SEG(0x00016F00000000A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_INSNREG06(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_INSNREG06(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F00000000E8ull);
}
#else
#define CVMX_UAHCX_EHCI_INSNREG06(offset) (CVMX_ADD_IO_SEG(0x00016F00000000E8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_INSNREG07(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_INSNREG07(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F00000000ECull);
}
#else
#define CVMX_UAHCX_EHCI_INSNREG07(offset) (CVMX_ADD_IO_SEG(0x00016F00000000ECull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_PERIODICLISTBASE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_PERIODICLISTBASE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000024ull);
}
#else
#define CVMX_UAHCX_EHCI_PERIODICLISTBASE(offset) (CVMX_ADD_IO_SEG(0x00016F0000000024ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_PORTSCX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((((offset >= 1) && (offset <= 2))) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((((offset >= 1) && (offset <= 2))) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((((offset >= 1) && (offset <= 2))) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((((offset >= 1) && (offset <= 2))) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((((offset >= 1) && (offset <= 2))) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_EHCI_PORTSCX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00016F0000000050ull) + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 4;
}
#else
#define CVMX_UAHCX_EHCI_PORTSCX(offset, block_id) (CVMX_ADD_IO_SEG(0x00016F0000000050ull) + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 4)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_USBCMD(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_USBCMD(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000010ull);
}
#else
#define CVMX_UAHCX_EHCI_USBCMD(offset) (CVMX_ADD_IO_SEG(0x00016F0000000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_USBINTR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_USBINTR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000018ull);
}
#else
#define CVMX_UAHCX_EHCI_USBINTR(offset) (CVMX_ADD_IO_SEG(0x00016F0000000018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_EHCI_USBSTS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_EHCI_USBSTS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000014ull);
}
#else
#define CVMX_UAHCX_EHCI_USBSTS(offset) (CVMX_ADD_IO_SEG(0x00016F0000000014ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_ERDPX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_ERDPX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000478ull);
}
#else
#define CVMX_UAHCX_ERDPX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000478ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_ERSTBAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_ERSTBAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000470ull);
}
#else
#define CVMX_UAHCX_ERSTBAX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000470ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_ERSTSZX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_ERSTSZX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000468ull);
}
#else
#define CVMX_UAHCX_ERSTSZX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000468ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GBUSERRADDR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GBUSERRADDR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C130ull);
}
#else
#define CVMX_UAHCX_GBUSERRADDR(offset) (CVMX_ADD_IO_SEG(0x000168000000C130ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GCTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GCTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C110ull);
}
#else
#define CVMX_UAHCX_GCTL(offset) (CVMX_ADD_IO_SEG(0x000168000000C110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GDBGBMU(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GDBGBMU(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C16Cull);
}
#else
#define CVMX_UAHCX_GDBGBMU(offset) (CVMX_ADD_IO_SEG(0x000168000000C16Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GDBGEPINFO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GDBGEPINFO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C178ull);
}
#else
#define CVMX_UAHCX_GDBGEPINFO(offset) (CVMX_ADD_IO_SEG(0x000168000000C178ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GDBGFIFOSPACE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GDBGFIFOSPACE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C160ull);
}
#else
#define CVMX_UAHCX_GDBGFIFOSPACE(offset) (CVMX_ADD_IO_SEG(0x000168000000C160ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GDBGLNMCC(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GDBGLNMCC(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C168ull);
}
#else
#define CVMX_UAHCX_GDBGLNMCC(offset) (CVMX_ADD_IO_SEG(0x000168000000C168ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GDBGLSP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GDBGLSP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C174ull);
}
#else
#define CVMX_UAHCX_GDBGLSP(offset) (CVMX_ADD_IO_SEG(0x000168000000C174ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GDBGLSPMUX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GDBGLSPMUX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C170ull);
}
#else
#define CVMX_UAHCX_GDBGLSPMUX(offset) (CVMX_ADD_IO_SEG(0x000168000000C170ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GDBGLTSSM(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GDBGLTSSM(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C164ull);
}
#else
#define CVMX_UAHCX_GDBGLTSSM(offset) (CVMX_ADD_IO_SEG(0x000168000000C164ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GDMAHLRATIO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GDMAHLRATIO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C624ull);
}
#else
#define CVMX_UAHCX_GDMAHLRATIO(offset) (CVMX_ADD_IO_SEG(0x000168000000C624ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GFLADJ(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GFLADJ(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C630ull);
}
#else
#define CVMX_UAHCX_GFLADJ(offset) (CVMX_ADD_IO_SEG(0x000168000000C630ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GGPIO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GGPIO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C124ull);
}
#else
#define CVMX_UAHCX_GGPIO(offset) (CVMX_ADD_IO_SEG(0x000168000000C124ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GHWPARAMS0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GHWPARAMS0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C140ull);
}
#else
#define CVMX_UAHCX_GHWPARAMS0(offset) (CVMX_ADD_IO_SEG(0x000168000000C140ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GHWPARAMS1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GHWPARAMS1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C144ull);
}
#else
#define CVMX_UAHCX_GHWPARAMS1(offset) (CVMX_ADD_IO_SEG(0x000168000000C144ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GHWPARAMS2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GHWPARAMS2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C148ull);
}
#else
#define CVMX_UAHCX_GHWPARAMS2(offset) (CVMX_ADD_IO_SEG(0x000168000000C148ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GHWPARAMS3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GHWPARAMS3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C14Cull);
}
#else
#define CVMX_UAHCX_GHWPARAMS3(offset) (CVMX_ADD_IO_SEG(0x000168000000C14Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GHWPARAMS4(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GHWPARAMS4(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C150ull);
}
#else
#define CVMX_UAHCX_GHWPARAMS4(offset) (CVMX_ADD_IO_SEG(0x000168000000C150ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GHWPARAMS5(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GHWPARAMS5(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C154ull);
}
#else
#define CVMX_UAHCX_GHWPARAMS5(offset) (CVMX_ADD_IO_SEG(0x000168000000C154ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GHWPARAMS6(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GHWPARAMS6(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C158ull);
}
#else
#define CVMX_UAHCX_GHWPARAMS6(offset) (CVMX_ADD_IO_SEG(0x000168000000C158ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GHWPARAMS7(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GHWPARAMS7(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C15Cull);
}
#else
#define CVMX_UAHCX_GHWPARAMS7(offset) (CVMX_ADD_IO_SEG(0x000168000000C15Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GHWPARAMS8(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GHWPARAMS8(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C600ull);
}
#else
#define CVMX_UAHCX_GHWPARAMS8(offset) (CVMX_ADD_IO_SEG(0x000168000000C600ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GPMSTS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GPMSTS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C114ull);
}
#else
#define CVMX_UAHCX_GPMSTS(offset) (CVMX_ADD_IO_SEG(0x000168000000C114ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GPRTBIMAP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GPRTBIMAP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C138ull);
}
#else
#define CVMX_UAHCX_GPRTBIMAP(offset) (CVMX_ADD_IO_SEG(0x000168000000C138ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GPRTBIMAP_FS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GPRTBIMAP_FS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C188ull);
}
#else
#define CVMX_UAHCX_GPRTBIMAP_FS(offset) (CVMX_ADD_IO_SEG(0x000168000000C188ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GPRTBIMAP_HS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GPRTBIMAP_HS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C180ull);
}
#else
#define CVMX_UAHCX_GPRTBIMAP_HS(offset) (CVMX_ADD_IO_SEG(0x000168000000C180ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GRLSID(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GRLSID(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C120ull);
}
#else
#define CVMX_UAHCX_GRLSID(offset) (CVMX_ADD_IO_SEG(0x000168000000C120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GRXFIFOPRIHST(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GRXFIFOPRIHST(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C61Cull);
}
#else
#define CVMX_UAHCX_GRXFIFOPRIHST(offset) (CVMX_ADD_IO_SEG(0x000168000000C61Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GRXFIFOSIZX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 2)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 2)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_GRXFIFOSIZX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C380ull) + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 4;
}
#else
#define CVMX_UAHCX_GRXFIFOSIZX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C380ull) + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 4)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GRXTHRCFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GRXTHRCFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C10Cull);
}
#else
#define CVMX_UAHCX_GRXTHRCFG(offset) (CVMX_ADD_IO_SEG(0x000168000000C10Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GSBUSCFG0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GSBUSCFG0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C100ull);
}
#else
#define CVMX_UAHCX_GSBUSCFG0(offset) (CVMX_ADD_IO_SEG(0x000168000000C100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GSBUSCFG1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GSBUSCFG1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C104ull);
}
#else
#define CVMX_UAHCX_GSBUSCFG1(offset) (CVMX_ADD_IO_SEG(0x000168000000C104ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GSTS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GSTS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C118ull);
}
#else
#define CVMX_UAHCX_GSTS(offset) (CVMX_ADD_IO_SEG(0x000168000000C118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GTXFIFOPRIHST(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GTXFIFOPRIHST(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C618ull);
}
#else
#define CVMX_UAHCX_GTXFIFOPRIHST(offset) (CVMX_ADD_IO_SEG(0x000168000000C618ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GTXFIFOSIZX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 2)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 2)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_GTXFIFOSIZX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C300ull) + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 4;
}
#else
#define CVMX_UAHCX_GTXFIFOSIZX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C300ull) + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 4)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GTXTHRCFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GTXTHRCFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C108ull);
}
#else
#define CVMX_UAHCX_GTXTHRCFG(offset) (CVMX_ADD_IO_SEG(0x000168000000C108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GUCTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GUCTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C12Cull);
}
#else
#define CVMX_UAHCX_GUCTL(offset) (CVMX_ADD_IO_SEG(0x000168000000C12Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GUCTL1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GUCTL1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C11Cull);
}
#else
#define CVMX_UAHCX_GUCTL1(offset) (CVMX_ADD_IO_SEG(0x000168000000C11Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GUID(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_GUID(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000C128ull);
}
#else
#define CVMX_UAHCX_GUID(offset) (CVMX_ADD_IO_SEG(0x000168000000C128ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GUSB2I2CCTLX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_GUSB2I2CCTLX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C240ull);
}
#else
#define CVMX_UAHCX_GUSB2I2CCTLX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C240ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GUSB2PHYCFGX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_GUSB2PHYCFGX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C200ull);
}
#else
#define CVMX_UAHCX_GUSB2PHYCFGX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C200ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_GUSB3PIPECTLX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_GUSB3PIPECTLX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000C2C0ull);
}
#else
#define CVMX_UAHCX_GUSB3PIPECTLX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000C2C0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_HCCPARAMS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_HCCPARAMS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000010ull);
}
#else
#define CVMX_UAHCX_HCCPARAMS(offset) (CVMX_ADD_IO_SEG(0x0001680000000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_HCSPARAMS1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_HCSPARAMS1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000004ull);
}
#else
#define CVMX_UAHCX_HCSPARAMS1(offset) (CVMX_ADD_IO_SEG(0x0001680000000004ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_HCSPARAMS2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_HCSPARAMS2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000008ull);
}
#else
#define CVMX_UAHCX_HCSPARAMS2(offset) (CVMX_ADD_IO_SEG(0x0001680000000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_HCSPARAMS3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_HCSPARAMS3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000000Cull);
}
#else
#define CVMX_UAHCX_HCSPARAMS3(offset) (CVMX_ADD_IO_SEG(0x000168000000000Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_IMANX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_IMANX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000460ull);
}
#else
#define CVMX_UAHCX_IMANX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000460ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_IMODX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_IMODX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000464ull);
}
#else
#define CVMX_UAHCX_IMODX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000464ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_MFINDEX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_MFINDEX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000440ull);
}
#else
#define CVMX_UAHCX_MFINDEX(offset) (CVMX_ADD_IO_SEG(0x0001680000000440ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCBULKCURRENTED(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCBULKCURRENTED(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F000000042Cull);
}
#else
#define CVMX_UAHCX_OHCI0_HCBULKCURRENTED(offset) (CVMX_ADD_IO_SEG(0x00016F000000042Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCBULKHEADED(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCBULKHEADED(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000428ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCBULKHEADED(offset) (CVMX_ADD_IO_SEG(0x00016F0000000428ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCCOMMANDSTATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCCOMMANDSTATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000408ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCCOMMANDSTATUS(offset) (CVMX_ADD_IO_SEG(0x00016F0000000408ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCCONTROL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCCONTROL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000404ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCCONTROL(offset) (CVMX_ADD_IO_SEG(0x00016F0000000404ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCCONTROLCURRENTED(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCCONTROLCURRENTED(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000424ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCCONTROLCURRENTED(offset) (CVMX_ADD_IO_SEG(0x00016F0000000424ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCCONTROLHEADED(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCCONTROLHEADED(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000420ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCCONTROLHEADED(offset) (CVMX_ADD_IO_SEG(0x00016F0000000420ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCDONEHEAD(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCDONEHEAD(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000430ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCDONEHEAD(offset) (CVMX_ADD_IO_SEG(0x00016F0000000430ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCFMINTERVAL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCFMINTERVAL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000434ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCFMINTERVAL(offset) (CVMX_ADD_IO_SEG(0x00016F0000000434ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCFMNUMBER(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCFMNUMBER(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F000000043Cull);
}
#else
#define CVMX_UAHCX_OHCI0_HCFMNUMBER(offset) (CVMX_ADD_IO_SEG(0x00016F000000043Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCFMREMAINING(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCFMREMAINING(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000438ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCFMREMAINING(offset) (CVMX_ADD_IO_SEG(0x00016F0000000438ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCHCCA(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCHCCA(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000418ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCHCCA(offset) (CVMX_ADD_IO_SEG(0x00016F0000000418ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCINTERRUPTDISABLE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCINTERRUPTDISABLE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000414ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCINTERRUPTDISABLE(offset) (CVMX_ADD_IO_SEG(0x00016F0000000414ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCINTERRUPTENABLE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCINTERRUPTENABLE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000410ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCINTERRUPTENABLE(offset) (CVMX_ADD_IO_SEG(0x00016F0000000410ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCINTERRUPTSTATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCINTERRUPTSTATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F000000040Cull);
}
#else
#define CVMX_UAHCX_OHCI0_HCINTERRUPTSTATUS(offset) (CVMX_ADD_IO_SEG(0x00016F000000040Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCLSTHRESHOLD(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCLSTHRESHOLD(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000444ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCLSTHRESHOLD(offset) (CVMX_ADD_IO_SEG(0x00016F0000000444ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCPERIODCURRENTED(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCPERIODCURRENTED(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F000000041Cull);
}
#else
#define CVMX_UAHCX_OHCI0_HCPERIODCURRENTED(offset) (CVMX_ADD_IO_SEG(0x00016F000000041Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCPERIODICSTART(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCPERIODICSTART(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000440ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCPERIODICSTART(offset) (CVMX_ADD_IO_SEG(0x00016F0000000440ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCREVISION(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCREVISION(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000400ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCREVISION(offset) (CVMX_ADD_IO_SEG(0x00016F0000000400ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCRHDESCRIPTORA(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCRHDESCRIPTORA(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000448ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCRHDESCRIPTORA(offset) (CVMX_ADD_IO_SEG(0x00016F0000000448ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCRHDESCRIPTORB(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCRHDESCRIPTORB(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F000000044Cull);
}
#else
#define CVMX_UAHCX_OHCI0_HCRHDESCRIPTORB(offset) (CVMX_ADD_IO_SEG(0x00016F000000044Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCRHPORTSTATUSX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((((offset >= 1) && (offset <= 2))) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((((offset >= 1) && (offset <= 2))) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((((offset >= 1) && (offset <= 2))) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((((offset >= 1) && (offset <= 2))) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((((offset >= 1) && (offset <= 2))) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCRHPORTSTATUSX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00016F0000000450ull) + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 4;
}
#else
#define CVMX_UAHCX_OHCI0_HCRHPORTSTATUSX(offset, block_id) (CVMX_ADD_IO_SEG(0x00016F0000000450ull) + (((offset) & 3) + ((block_id) & 0) * 0x0ull) * 4)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_HCRHSTATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_HCRHSTATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000450ull);
}
#else
#define CVMX_UAHCX_OHCI0_HCRHSTATUS(offset) (CVMX_ADD_IO_SEG(0x00016F0000000450ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_INSNREG06(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_INSNREG06(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F0000000498ull);
}
#else
#define CVMX_UAHCX_OHCI0_INSNREG06(offset) (CVMX_ADD_IO_SEG(0x00016F0000000498ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_OHCI0_INSNREG07(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_OHCI0_INSNREG07(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016F000000049Cull);
}
#else
#define CVMX_UAHCX_OHCI0_INSNREG07(offset) (CVMX_ADD_IO_SEG(0x00016F000000049Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_PAGESIZE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_PAGESIZE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000028ull);
}
#else
#define CVMX_UAHCX_PAGESIZE(offset) (CVMX_ADD_IO_SEG(0x0001680000000028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_PORTHLPMC_20X(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_PORTHLPMC_20X(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000042Cull);
}
#else
#define CVMX_UAHCX_PORTHLPMC_20X(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000042Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_PORTHLPMC_SSX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_PORTHLPMC_SSX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000168000000043Cull);
}
#else
#define CVMX_UAHCX_PORTHLPMC_SSX(offset, block_id) (CVMX_ADD_IO_SEG(0x000168000000043Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_PORTLI_20X(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_PORTLI_20X(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000428ull);
}
#else
#define CVMX_UAHCX_PORTLI_20X(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000428ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_PORTLI_SSX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_PORTLI_SSX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000438ull);
}
#else
#define CVMX_UAHCX_PORTLI_SSX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000438ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_PORTPMSC_20X(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_PORTPMSC_20X(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000424ull);
}
#else
#define CVMX_UAHCX_PORTPMSC_20X(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000424ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_PORTPMSC_SSX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_PORTPMSC_SSX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000434ull);
}
#else
#define CVMX_UAHCX_PORTPMSC_SSX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000434ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_PORTSCX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UAHCX_PORTSCX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001680000000420ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 16;
}
#else
#define CVMX_UAHCX_PORTSCX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001680000000420ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_RTSOFF(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_RTSOFF(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000018ull);
}
#else
#define CVMX_UAHCX_RTSOFF(offset) (CVMX_ADD_IO_SEG(0x0001680000000018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_SUPTPRT2_DW0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_SUPTPRT2_DW0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000890ull);
}
#else
#define CVMX_UAHCX_SUPTPRT2_DW0(offset) (CVMX_ADD_IO_SEG(0x0001680000000890ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_SUPTPRT2_DW1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_SUPTPRT2_DW1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000894ull);
}
#else
#define CVMX_UAHCX_SUPTPRT2_DW1(offset) (CVMX_ADD_IO_SEG(0x0001680000000894ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_SUPTPRT2_DW2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_SUPTPRT2_DW2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000898ull);
}
#else
#define CVMX_UAHCX_SUPTPRT2_DW2(offset) (CVMX_ADD_IO_SEG(0x0001680000000898ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_SUPTPRT2_DW3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_SUPTPRT2_DW3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000168000000089Cull);
}
#else
#define CVMX_UAHCX_SUPTPRT2_DW3(offset) (CVMX_ADD_IO_SEG(0x000168000000089Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_SUPTPRT3_DW0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_SUPTPRT3_DW0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016800000008A0ull);
}
#else
#define CVMX_UAHCX_SUPTPRT3_DW0(offset) (CVMX_ADD_IO_SEG(0x00016800000008A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_SUPTPRT3_DW1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_SUPTPRT3_DW1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016800000008A4ull);
}
#else
#define CVMX_UAHCX_SUPTPRT3_DW1(offset) (CVMX_ADD_IO_SEG(0x00016800000008A4ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_SUPTPRT3_DW2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_SUPTPRT3_DW2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016800000008A8ull);
}
#else
#define CVMX_UAHCX_SUPTPRT3_DW2(offset) (CVMX_ADD_IO_SEG(0x00016800000008A8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_SUPTPRT3_DW3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_SUPTPRT3_DW3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016800000008ACull);
}
#else
#define CVMX_UAHCX_SUPTPRT3_DW3(offset) (CVMX_ADD_IO_SEG(0x00016800000008ACull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_USBCMD(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_USBCMD(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000020ull);
}
#else
#define CVMX_UAHCX_USBCMD(offset) (CVMX_ADD_IO_SEG(0x0001680000000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_USBLEGCTLSTS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_USBLEGCTLSTS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000884ull);
}
#else
#define CVMX_UAHCX_USBLEGCTLSTS(offset) (CVMX_ADD_IO_SEG(0x0001680000000884ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_USBLEGSUP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_USBLEGSUP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000880ull);
}
#else
#define CVMX_UAHCX_USBLEGSUP(offset) (CVMX_ADD_IO_SEG(0x0001680000000880ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UAHCX_USBSTS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UAHCX_USBSTS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001680000000024ull);
}
#else
#define CVMX_UAHCX_USBSTS(offset) (CVMX_ADD_IO_SEG(0x0001680000000024ull))
#endif

/**
 * cvmx_uahc#_caplength
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.3.1.
 *
 */
union cvmx_uahcx_caplength {
	uint32_t u32;
	struct cvmx_uahcx_caplength_s {
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
	struct cvmx_uahcx_caplength_s         cn78xx;
	struct cvmx_uahcx_caplength_s         cn78xxp1;
};
typedef union cvmx_uahcx_caplength cvmx_uahcx_caplength_t;

/**
 * cvmx_uahc#_config
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.7.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_config {
	uint32_t u32;
	struct cvmx_uahcx_config_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_8_31                : 24;
	uint32_t maxslotsen                   : 8;  /**< Maximum device slots enabled. */
#else
	uint32_t maxslotsen                   : 8;
	uint32_t reserved_8_31                : 24;
#endif
	} s;
	struct cvmx_uahcx_config_s            cn78xx;
	struct cvmx_uahcx_config_s            cn78xxp1;
};
typedef union cvmx_uahcx_config cvmx_uahcx_config_t;

/**
 * cvmx_uahc#_crcr
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.5.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_crcr {
	uint64_t u64;
	struct cvmx_uahcx_crcr_s {
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
	struct cvmx_uahcx_crcr_s              cn78xx;
	struct cvmx_uahcx_crcr_s              cn78xxp1;
};
typedef union cvmx_uahcx_crcr cvmx_uahcx_crcr_t;

/**
 * cvmx_uahc#_db#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.6.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_dbx {
	uint32_t u32;
	struct cvmx_uahcx_dbx_s {
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
	struct cvmx_uahcx_dbx_s               cn78xx;
	struct cvmx_uahcx_dbx_s               cn78xxp1;
};
typedef union cvmx_uahcx_dbx cvmx_uahcx_dbx_t;

/**
 * cvmx_uahc#_dboff
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.3.7.
 *
 */
union cvmx_uahcx_dboff {
	uint32_t u32;
	struct cvmx_uahcx_dboff_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dboff                        : 30; /**< Doorbell array offset. */
	uint32_t reserved_0_1                 : 2;
#else
	uint32_t reserved_0_1                 : 2;
	uint32_t dboff                        : 30;
#endif
	} s;
	struct cvmx_uahcx_dboff_s             cn78xx;
	struct cvmx_uahcx_dboff_s             cn78xxp1;
};
typedef union cvmx_uahcx_dboff cvmx_uahcx_dboff_t;

/**
 * cvmx_uahc#_dcbaap
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.6.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_dcbaap {
	uint64_t u64;
	struct cvmx_uahcx_dcbaap_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dcbaap                       : 58; /**< Device context base address array pointer. */
	uint64_t reserved_0_5                 : 6;
#else
	uint64_t reserved_0_5                 : 6;
	uint64_t dcbaap                       : 58;
#endif
	} s;
	struct cvmx_uahcx_dcbaap_s            cn78xx;
	struct cvmx_uahcx_dcbaap_s            cn78xxp1;
};
typedef union cvmx_uahcx_dcbaap cvmx_uahcx_dcbaap_t;

/**
 * cvmx_uahc#_dnctrl
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.4.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_dnctrl {
	uint32_t u32;
	struct cvmx_uahcx_dnctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t n                            : 16; /**< Notification enable. */
#else
	uint32_t n                            : 16;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_uahcx_dnctrl_s            cn78xx;
	struct cvmx_uahcx_dnctrl_s            cn78xxp1;
};
typedef union cvmx_uahcx_dnctrl cvmx_uahcx_dnctrl_t;

/**
 * cvmx_uahc#_ehci_asynclistaddr
 *
 * ASYNCLISTADDR = Current Asynchronous List Address Register
 *
 * This 32-bit register contains the address of the next asynchronous queue head to be executed. If the host
 * controller is in 64-bit mode (as indicated by a one in 64-bit Addressing Capability field in the
 * HCCPARAMS register), then the most significant 32 bits of every control data structure address comes from
 * the CTRLDSSEGMENT register (See Section 2.3.5). Bits [4:0] of this register cannot be modified by system
 * software and will always return a zero when read. The memory structure referenced by this physical memory
 * pointer is assumed to be 32-byte (cache line) aligned.
 */
union cvmx_uahcx_ehci_asynclistaddr {
	uint32_t u32;
	struct cvmx_uahcx_ehci_asynclistaddr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lpl                          : 27; /**< Link Pointer Low (LPL). These bits correspond to memory address signals [31:5],
                                                         respectively. This field may only reference a Queue Head (QH). */
	uint32_t reserved_0_4                 : 5;
#else
	uint32_t reserved_0_4                 : 5;
	uint32_t lpl                          : 27;
#endif
	} s;
	struct cvmx_uahcx_ehci_asynclistaddr_s cn61xx;
	struct cvmx_uahcx_ehci_asynclistaddr_s cn63xx;
	struct cvmx_uahcx_ehci_asynclistaddr_s cn63xxp1;
	struct cvmx_uahcx_ehci_asynclistaddr_s cn66xx;
	struct cvmx_uahcx_ehci_asynclistaddr_s cn68xx;
	struct cvmx_uahcx_ehci_asynclistaddr_s cn68xxp1;
	struct cvmx_uahcx_ehci_asynclistaddr_s cnf71xx;
};
typedef union cvmx_uahcx_ehci_asynclistaddr cvmx_uahcx_ehci_asynclistaddr_t;

/**
 * cvmx_uahc#_ehci_configflag
 *
 * CONFIGFLAG = Configure Flag Register
 * This register is in the auxiliary power well. It is only reset by hardware when the auxiliary power is initially
 * applied or in response to a host controller reset.
 */
union cvmx_uahcx_ehci_configflag {
	uint32_t u32;
	struct cvmx_uahcx_ehci_configflag_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_1_31                : 31;
	uint32_t cf                           : 1;  /**< Configure Flag (CF) .Host software sets this bit as the last action in
                                                         its process of configuring the Host Controller (see Section 4.1). This bit controls the
                                                         default port-routing control logic. Bit values and side-effects are listed below.
                                                          0b: Port routing control logic default-routes each port to an implementation
                                                              dependent classic host controller.
                                                          1b: Port routing control logic default-routes all ports to this host controller. */
#else
	uint32_t cf                           : 1;
	uint32_t reserved_1_31                : 31;
#endif
	} s;
	struct cvmx_uahcx_ehci_configflag_s   cn61xx;
	struct cvmx_uahcx_ehci_configflag_s   cn63xx;
	struct cvmx_uahcx_ehci_configflag_s   cn63xxp1;
	struct cvmx_uahcx_ehci_configflag_s   cn66xx;
	struct cvmx_uahcx_ehci_configflag_s   cn68xx;
	struct cvmx_uahcx_ehci_configflag_s   cn68xxp1;
	struct cvmx_uahcx_ehci_configflag_s   cnf71xx;
};
typedef union cvmx_uahcx_ehci_configflag cvmx_uahcx_ehci_configflag_t;

/**
 * cvmx_uahc#_ehci_ctrldssegment
 *
 * CTRLDSSEGMENT = Control Data Structure Segment Register
 *
 * This 32-bit register corresponds to the most significant address bits [63:32] for all EHCI data structures. If
 * the 64-bit Addressing Capability field in HCCPARAMS is a zero, then this register is not used. Software
 * cannot write to it and a read from this register will return zeros.
 *
 * If the 64-bit Addressing Capability field in HCCPARAMS is a one, then this register is used with the link
 * pointers to construct 64-bit addresses to EHCI control data structures. This register is concatenated with the
 * link pointer from either the PERIODICLISTBASE, ASYNCLISTADDR, or any control data structure link
 * field to construct a 64-bit address.
 *
 * This register allows the host software to locate all control data structures within the same 4 Gigabyte
 * memory segment.
 */
union cvmx_uahcx_ehci_ctrldssegment {
	uint32_t u32;
	struct cvmx_uahcx_ehci_ctrldssegment_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ctrldsseg                    : 32; /**< Control Data Strucute Semgent Address Bit [63:32] */
#else
	uint32_t ctrldsseg                    : 32;
#endif
	} s;
	struct cvmx_uahcx_ehci_ctrldssegment_s cn61xx;
	struct cvmx_uahcx_ehci_ctrldssegment_s cn63xx;
	struct cvmx_uahcx_ehci_ctrldssegment_s cn63xxp1;
	struct cvmx_uahcx_ehci_ctrldssegment_s cn66xx;
	struct cvmx_uahcx_ehci_ctrldssegment_s cn68xx;
	struct cvmx_uahcx_ehci_ctrldssegment_s cn68xxp1;
	struct cvmx_uahcx_ehci_ctrldssegment_s cnf71xx;
};
typedef union cvmx_uahcx_ehci_ctrldssegment cvmx_uahcx_ehci_ctrldssegment_t;

/**
 * cvmx_uahc#_ehci_frindex
 *
 * FRINDEX = Frame Index Register
 * This register is used by the host controller to index into the periodic frame list. The register updates every
 * 125 microseconds (once each micro-frame). Bits [N:3] are used to select a particular entry in the Periodic
 * Frame List during periodic schedule execution. The number of bits used for the index depends on the size of
 * the frame list as set by system software in the Frame List Size field in the USBCMD register.
 * This register cannot be written unless the Host Controller is in the Halted state as indicated by the
 * HCHalted bit. A write to this register while the Run/Stop bit is set to a one (USBCMD register) produces
 * undefined results. Writes to this register also affect the SOF value.
 */
union cvmx_uahcx_ehci_frindex {
	uint32_t u32;
	struct cvmx_uahcx_ehci_frindex_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_14_31               : 18;
	uint32_t fi                           : 14; /**< Frame Index. The value in this register increments at the end of each time frame (e.g.
                                                         micro-frame). Bits [N:3] are used for the Frame List current index. This means that each
                                                         location of the frame list is accessed 8 times (frames or micro-frames) before moving to
                                                         the next index. The following illustrates values of N based on the value of the Frame List
                                                         Size field in the USBCMD register.
                                                         USBCMD[Frame List Size] Number Elements N
                                                            00b (1024) 12
                                                            01b (512) 11
                                                            10b (256) 10
                                                            11b Reserved */
#else
	uint32_t fi                           : 14;
	uint32_t reserved_14_31               : 18;
#endif
	} s;
	struct cvmx_uahcx_ehci_frindex_s      cn61xx;
	struct cvmx_uahcx_ehci_frindex_s      cn63xx;
	struct cvmx_uahcx_ehci_frindex_s      cn63xxp1;
	struct cvmx_uahcx_ehci_frindex_s      cn66xx;
	struct cvmx_uahcx_ehci_frindex_s      cn68xx;
	struct cvmx_uahcx_ehci_frindex_s      cn68xxp1;
	struct cvmx_uahcx_ehci_frindex_s      cnf71xx;
};
typedef union cvmx_uahcx_ehci_frindex cvmx_uahcx_ehci_frindex_t;

/**
 * cvmx_uahc#_ehci_hccapbase
 *
 * HCCAPBASE = Host Controller BASE Capability Register
 *
 */
union cvmx_uahcx_ehci_hccapbase {
	uint32_t u32;
	struct cvmx_uahcx_ehci_hccapbase_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t hciversion                   : 16; /**< Host Controller Interface Version Number */
	uint32_t reserved_8_15                : 8;
	uint32_t caplength                    : 8;  /**< Capabitlity Registers Length */
#else
	uint32_t caplength                    : 8;
	uint32_t reserved_8_15                : 8;
	uint32_t hciversion                   : 16;
#endif
	} s;
	struct cvmx_uahcx_ehci_hccapbase_s    cn61xx;
	struct cvmx_uahcx_ehci_hccapbase_s    cn63xx;
	struct cvmx_uahcx_ehci_hccapbase_s    cn63xxp1;
	struct cvmx_uahcx_ehci_hccapbase_s    cn66xx;
	struct cvmx_uahcx_ehci_hccapbase_s    cn68xx;
	struct cvmx_uahcx_ehci_hccapbase_s    cn68xxp1;
	struct cvmx_uahcx_ehci_hccapbase_s    cnf71xx;
};
typedef union cvmx_uahcx_ehci_hccapbase cvmx_uahcx_ehci_hccapbase_t;

/**
 * cvmx_uahc#_ehci_hccparams
 *
 * HCCPARAMS = Host Controller Capability Parameters
 * Multiple Mode control (time-base bit functionality), addressing capability
 */
union cvmx_uahcx_ehci_hccparams {
	uint32_t u32;
	struct cvmx_uahcx_ehci_hccparams_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t eecp                         : 8;  /**< EHCI Extended Capabilities Pointer. Default = Implementation Dependent.
                                                         This optional field indicates the existence of a capabilities list. A value of 00h indicates
                                                         no extended capabilities are implemented. A non-zero value in this register indicates the
                                                         offset in PCI configuration space of the first EHCI extended capability. The pointer value
                                                         must be 40h or greater if implemented to maintain the consistency of the PCI header
                                                         defined for this class of device. */
	uint32_t ist                          : 4;  /**< Isochronous Scheduling Threshold. Default = implementation dependent. This field
                                                         indicates, relative to the current position of the executing host controller, where software
                                                         can reliably update the isochronous schedule. When bit [7] is zero, the value of the least
                                                         significant 3 bits indicates the number of micro-frames a host controller can hold a set of
                                                         isochronous data structures (one or more) before flushing the state. When bit [7] is a
                                                         one, then host software assumes the host controller may cache an isochronous data
                                                         structure for an entire frame. Refer to Section 4.7.2.1 for details on how software uses
                                                         this information for scheduling isochronous transfers. */
	uint32_t reserved_3_3                 : 1;
	uint32_t aspc                         : 1;  /**< Asynchronous Schedule Park Capability. Default = Implementation dependent. If this
                                                         bit is set to a one, then the host controller supports the park feature for high-speed
                                                         queue heads in the Asynchronous Schedule. The feature can be disabled or enabled
                                                         and set to a specific level by using the Asynchronous Schedule Park Mode Enable and
                                                         Asynchronous Schedule Park Mode Count fields in the USBCMD register. */
	uint32_t pflf                         : 1;  /**< Programmable Frame List Flag. Default = Implementation dependent. If this bit is set
                                                         to a zero, then system software must use a frame list length of 1024 elements with this
                                                         host controller. The USBCMD register Frame List Size field is a read-only register and
                                                         should be set to zero.
                                                         If set to a one, then system software can specify and use a smaller frame list and
                                                         configure the host controller via the USBCMD register Frame List Size field. The frame
                                                         list must always be aligned on a 4K page boundary. This requirement ensures that the
                                                         frame list is always physically contiguous. */
	uint32_t ac64                         : 1;  /**< 64-bit Addressing Capability1 . This field documents the addressing range capability of
                                                          this implementation. The value of this field determines whether software should use the
                                                          data structures defined in Section 3 (32-bit) or those defined in Appendix B (64-bit).
                                                          Values for this field have the following interpretation:
                                                         - 0: data structures using 32-bit address memory pointers
                                                         - 1: data structures using 64-bit address memory pointers */
#else
	uint32_t ac64                         : 1;
	uint32_t pflf                         : 1;
	uint32_t aspc                         : 1;
	uint32_t reserved_3_3                 : 1;
	uint32_t ist                          : 4;
	uint32_t eecp                         : 8;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_uahcx_ehci_hccparams_s    cn61xx;
	struct cvmx_uahcx_ehci_hccparams_s    cn63xx;
	struct cvmx_uahcx_ehci_hccparams_s    cn63xxp1;
	struct cvmx_uahcx_ehci_hccparams_s    cn66xx;
	struct cvmx_uahcx_ehci_hccparams_s    cn68xx;
	struct cvmx_uahcx_ehci_hccparams_s    cn68xxp1;
	struct cvmx_uahcx_ehci_hccparams_s    cnf71xx;
};
typedef union cvmx_uahcx_ehci_hccparams cvmx_uahcx_ehci_hccparams_t;

/**
 * cvmx_uahc#_ehci_hcsparams
 *
 * HCSPARAMS = Host Controller Structural Parameters
 * This is a set of fields that are structural parameters: Number of downstream ports, etc.
 */
union cvmx_uahcx_ehci_hcsparams {
	uint32_t u32;
	struct cvmx_uahcx_ehci_hcsparams_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t dpn                          : 4;  /**< Debug Port Number. Optional. This register identifies which of the host controller ports
                                                         is the debug port. The value is the port number (one-based) of the debug port. A nonzero
                                                         value in this field indicates the presence of a debug port. The value in this register
                                                         must not be greater than N_PORTS (see below). */
	uint32_t reserved_17_19               : 3;
	uint32_t p_indicator                  : 1;  /**< Port Indicator. This bit indicates whether the ports support port
                                                         indicator control. When this bit is a one, the port status and control
                                                         registers include a read/writeable field for controlling the state of
                                                         the port indicator. */
	uint32_t n_cc                         : 4;  /**< Number of Companion Controller. This field indicates the number of
                                                         companion controllers associated with this USB 2.0 host controller.
                                                         A zero in this field indicates there are no companion host controllers.
                                                         Port-ownership hand-off is not supported. Only high-speed devices are
                                                         supported on the host controller root ports.
                                                         A value larger than zero in this field indicates there are companion USB 1.1 host
                                                         controller(s). Port-ownership hand-offs are supported. High, Full-and Low-speed
                                                         devices are supported on the host controller root ports. */
	uint32_t n_pcc                        : 4;  /**< Number of Ports per Companion Controller (N_PCC). This field indicates
                                                         the number of ports supported per companion host controller. It is used to
                                                         indicate the port routing  configuration to system software. */
	uint32_t prr                          : 1;  /**< Port Routing Rules. This field indicates the method used by this implementation for
                                                         how all ports are mapped to companion controllers. The value of this field has
                                                         the following interpretation:
                                                         0 The first N_PCC ports are routed to the lowest numbered function
                                                           companion host controller, the next N_PCC port are routed to the next
                                                           lowest function companion controller, and so on.
                                                         1 The port routing is explicitly enumerated by the first N_PORTS elements
                                                           of the HCSP-PORTROUTE array. */
	uint32_t reserved_5_6                 : 2;
	uint32_t ppc                          : 1;  /**< Port Power Control. This field indicates whether the host controller
                                                         implementation includes port power control. A one in this bit indicates the ports have
                                                         port power switches. A zero in this bit indicates the port do not have port power
                                                         switches. The value of this field affects the functionality of the Port Power field
                                                         in each port status and control register (see Section 2.3.8). */
	uint32_t n_ports                      : 4;  /**< This field specifies the number of physical downstream ports implemented
                                                         on this host controller. The value of this field determines how many port registers are
                                                         addressable in the Operational Register Space (see Table 2-8). Valid values are in the
                                                         range of 1H to FH. A zero in this field is undefined. */
#else
	uint32_t n_ports                      : 4;
	uint32_t ppc                          : 1;
	uint32_t reserved_5_6                 : 2;
	uint32_t prr                          : 1;
	uint32_t n_pcc                        : 4;
	uint32_t n_cc                         : 4;
	uint32_t p_indicator                  : 1;
	uint32_t reserved_17_19               : 3;
	uint32_t dpn                          : 4;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_uahcx_ehci_hcsparams_s    cn61xx;
	struct cvmx_uahcx_ehci_hcsparams_s    cn63xx;
	struct cvmx_uahcx_ehci_hcsparams_s    cn63xxp1;
	struct cvmx_uahcx_ehci_hcsparams_s    cn66xx;
	struct cvmx_uahcx_ehci_hcsparams_s    cn68xx;
	struct cvmx_uahcx_ehci_hcsparams_s    cn68xxp1;
	struct cvmx_uahcx_ehci_hcsparams_s    cnf71xx;
};
typedef union cvmx_uahcx_ehci_hcsparams cvmx_uahcx_ehci_hcsparams_t;

/**
 * cvmx_uahc#_ehci_insnreg00
 *
 * EHCI_INSNREG00 = EHCI Programmable Microframe Base Value Register (Synopsys Speicific)
 * This register allows you to change the microframe length value (default is microframe SOF = 125 s) to reduce the simulation time.
 */
union cvmx_uahcx_ehci_insnreg00 {
	uint32_t u32;
	struct cvmx_uahcx_ehci_insnreg00_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_14_31               : 18;
	uint32_t mfmc                         : 13; /**< For byte interface (8-bits), <13:1> is used as the 1-microframe counter.
                                                         For word interface (16_bits> <12:1> is used as the 1-microframe counter with word
                                                           interface (16-bits). */
	uint32_t en                           : 1;  /**< Writing 1b1 enables this register.
                                                         Note: Do not enable this register for the gate-level netlist */
#else
	uint32_t en                           : 1;
	uint32_t mfmc                         : 13;
	uint32_t reserved_14_31               : 18;
#endif
	} s;
	struct cvmx_uahcx_ehci_insnreg00_s    cn61xx;
	struct cvmx_uahcx_ehci_insnreg00_s    cn63xx;
	struct cvmx_uahcx_ehci_insnreg00_s    cn63xxp1;
	struct cvmx_uahcx_ehci_insnreg00_s    cn66xx;
	struct cvmx_uahcx_ehci_insnreg00_s    cn68xx;
	struct cvmx_uahcx_ehci_insnreg00_s    cn68xxp1;
	struct cvmx_uahcx_ehci_insnreg00_s    cnf71xx;
};
typedef union cvmx_uahcx_ehci_insnreg00 cvmx_uahcx_ehci_insnreg00_t;

/**
 * cvmx_uahc#_ehci_insnreg03
 *
 * EHCI_INSNREG03 = EHCI Timing Adjust Register (Synopsys Speicific)
 * This register allows you to change the timing of Phy Tx turnaround delay etc.
 */
union cvmx_uahcx_ehci_insnreg03 {
	uint32_t u32;
	struct cvmx_uahcx_ehci_insnreg03_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_13_31               : 19;
	uint32_t txtx_tadao                   : 3;  /**< Tx-Tx turnaround Delay Add on. This field specifies the extra delays in phy_clks to
                                                         be added to the "Transmit to Transmit turnaround delay" value maintained in the core.
                                                         The default value of this register field is 0. This default value of 0 is sufficient
                                                         for most PHYs. But for some PHYs which puts wait states during the token packet, it
                                                         may be required to program a value greater than 0 to meet the transmit to transmit
                                                         minimum turnaround time. The recommendation to use the default value of 0 and change
                                                         it only if there is an issue with minimum transmit-to- transmit turnaround time. This
                                                         value should be programmed during core initialization and should not be changed afterwards. */
	uint32_t reserved_9_9                 : 1;
	uint32_t ta_off                       : 8;  /**< Time-Available Offset. This value indicates the additional number of bytes to be
                                                         accommodated for the time-available calculation. The USB traffic on the bus can be started
                                                         only when sufficient time is available to complete the packet within the EOF1 point. Refer
                                                         to the USB 2.0 specification for details of the EOF1 point. This time-available
                                                         calculation is done in the hardware, and can be further offset by programming a value in
                                                         this location.
                                                         Note: Time-available calculation is added for future flexibility. The application is not
                                                         required to program this field by default. */
	uint32_t reserved_0_0                 : 1;
#else
	uint32_t reserved_0_0                 : 1;
	uint32_t ta_off                       : 8;
	uint32_t reserved_9_9                 : 1;
	uint32_t txtx_tadao                   : 3;
	uint32_t reserved_13_31               : 19;
#endif
	} s;
	struct cvmx_uahcx_ehci_insnreg03_s    cn61xx;
	struct cvmx_uahcx_ehci_insnreg03_s    cn63xx;
	struct cvmx_uahcx_ehci_insnreg03_s    cn63xxp1;
	struct cvmx_uahcx_ehci_insnreg03_s    cn66xx;
	struct cvmx_uahcx_ehci_insnreg03_s    cn68xx;
	struct cvmx_uahcx_ehci_insnreg03_s    cn68xxp1;
	struct cvmx_uahcx_ehci_insnreg03_s    cnf71xx;
};
typedef union cvmx_uahcx_ehci_insnreg03 cvmx_uahcx_ehci_insnreg03_t;

/**
 * cvmx_uahc#_ehci_insnreg04
 *
 * EHCI_INSNREG04 = EHCI Debug Register (Synopsys Speicific)
 * This register is used only for debug purposes.
 */
union cvmx_uahcx_ehci_insnreg04 {
	uint32_t u32;
	struct cvmx_uahcx_ehci_insnreg04_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_6_31                : 26;
	uint32_t auto_dis                     : 1;  /**< Automatic feature disable.
                                                          1'b0: 0 by default, the automatic feature is enabled. The Suspend signal is deasserted
                                                                (logic level 1'b1) when run/stop is reset by software, but the hchalted bit is not
                                                                yet set.
                                                          1'b1: Disables the automatic feature, which takes all ports out of suspend when software
                                                                clears the run/stop bit. This is for backward compatibility.
                                                         This bit has an added functionality in release 2.80a and later. For systems where the host
                                                         is halted without waking up all ports out of suspend, the port can become stuck because
                                                         the PHYCLK is not running when the halt is programmed. To avoid this, the DWC H20AHB host
                                                         core automatically pulls ports out of suspend when the host is halted by software. This bit
                                                         is used to disable this automatic function. */
	uint32_t nakrf_dis                    : 1;  /**< NAK Reload Fix Disable.
                                                         1b0: NAK reload fix enabled.
                                                         1b1: NAK reload fix disabled. (Incorrect NAK reload transition at the end of a microframe
                                                              for backward compatibility with Release 2.40c. For more information see the USB 2.0
                                                              Host-AHB Release Notes. */
	uint32_t reserved_3_3                 : 1;
	uint32_t pesd                         : 1;  /**< Scales down port enumeration time.
                                                          1'b1: scale down enabled
                                                          1'b0:  scale downd disabled
                                                         This is for simulation only. */
	uint32_t hcp_fw                       : 1;  /**< HCCPARAMS Field Writeable.
                                                         1'b1: The HCCPARAMS register's bits 17, 15:4, and 2:0 become writable.
                                                         1'b0: The HCCPARAMS register's bits 17, 15:4, and 2:0 are not writable. */
	uint32_t hcp_rw                       : 1;  /**< HCCPARAMS Reigster Writeable.
                                                         1'b1: The HCCPARAMS register becomes writable.
                                                         1'b0: The HCCPARAMS register is not writable. */
#else
	uint32_t hcp_rw                       : 1;
	uint32_t hcp_fw                       : 1;
	uint32_t pesd                         : 1;
	uint32_t reserved_3_3                 : 1;
	uint32_t nakrf_dis                    : 1;
	uint32_t auto_dis                     : 1;
	uint32_t reserved_6_31                : 26;
#endif
	} s;
	struct cvmx_uahcx_ehci_insnreg04_s    cn61xx;
	struct cvmx_uahcx_ehci_insnreg04_s    cn63xx;
	struct cvmx_uahcx_ehci_insnreg04_s    cn63xxp1;
	struct cvmx_uahcx_ehci_insnreg04_s    cn66xx;
	struct cvmx_uahcx_ehci_insnreg04_s    cn68xx;
	struct cvmx_uahcx_ehci_insnreg04_s    cn68xxp1;
	struct cvmx_uahcx_ehci_insnreg04_s    cnf71xx;
};
typedef union cvmx_uahcx_ehci_insnreg04 cvmx_uahcx_ehci_insnreg04_t;

/**
 * cvmx_uahc#_ehci_insnreg06
 *
 * EHCI_INSNREG06 = EHCI  AHB Error Status Register (Synopsys Speicific)
 * This register contains AHB Error Status.
 */
union cvmx_uahcx_ehci_insnreg06 {
	uint32_t u32;
	struct cvmx_uahcx_ehci_insnreg06_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t vld                          : 1;  /**< AHB Error Captured. Indicator that an AHB error was encountered and values were captured.
                                                         To clear this field the application must write a 0 to it. */
	uint32_t reserved_0_30                : 31;
#else
	uint32_t reserved_0_30                : 31;
	uint32_t vld                          : 1;
#endif
	} s;
	struct cvmx_uahcx_ehci_insnreg06_s    cn61xx;
	struct cvmx_uahcx_ehci_insnreg06_s    cn63xx;
	struct cvmx_uahcx_ehci_insnreg06_s    cn63xxp1;
	struct cvmx_uahcx_ehci_insnreg06_s    cn66xx;
	struct cvmx_uahcx_ehci_insnreg06_s    cn68xx;
	struct cvmx_uahcx_ehci_insnreg06_s    cn68xxp1;
	struct cvmx_uahcx_ehci_insnreg06_s    cnf71xx;
};
typedef union cvmx_uahcx_ehci_insnreg06 cvmx_uahcx_ehci_insnreg06_t;

/**
 * cvmx_uahc#_ehci_insnreg07
 *
 * EHCI_INSNREG07 = EHCI  AHB Error Address Register (Synopsys Speicific)
 * This register contains AHB Error Status.
 */
union cvmx_uahcx_ehci_insnreg07 {
	uint32_t u32;
	struct cvmx_uahcx_ehci_insnreg07_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t err_addr                     : 32; /**< AHB Master Error Address. AHB address of the control phase at which the AHB error occurred */
#else
	uint32_t err_addr                     : 32;
#endif
	} s;
	struct cvmx_uahcx_ehci_insnreg07_s    cn61xx;
	struct cvmx_uahcx_ehci_insnreg07_s    cn63xx;
	struct cvmx_uahcx_ehci_insnreg07_s    cn63xxp1;
	struct cvmx_uahcx_ehci_insnreg07_s    cn66xx;
	struct cvmx_uahcx_ehci_insnreg07_s    cn68xx;
	struct cvmx_uahcx_ehci_insnreg07_s    cn68xxp1;
	struct cvmx_uahcx_ehci_insnreg07_s    cnf71xx;
};
typedef union cvmx_uahcx_ehci_insnreg07 cvmx_uahcx_ehci_insnreg07_t;

/**
 * cvmx_uahc#_ehci_periodiclistbase
 *
 * PERIODICLISTBASE = Periodic Frame List Base Address Register
 *
 * This 32-bit register contains the beginning address of the Periodic Frame List in the system memory. If the
 * host controller is in 64-bit mode (as indicated by a one in the 64-bit Addressing Capability field in the
 * HCCSPARAMS register), then the most significant 32 bits of every control data structure address comes
 * from the CTRLDSSEGMENT register (see Section 2.3.5). System software loads this register prior to
 * starting the schedule execution by the Host Controller (see 4.1). The memory structure referenced by this
 * physical memory pointer is assumed to be 4-Kbyte aligned. The contents of this register are combined with
 * the Frame Index Register (FRINDEX) to enable the Host Controller to step through the Periodic Frame List
 * in sequence.
 */
union cvmx_uahcx_ehci_periodiclistbase {
	uint32_t u32;
	struct cvmx_uahcx_ehci_periodiclistbase_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t baddr                        : 20; /**< Base Address (Low). These bits correspond to memory address signals [31:12],respectively. */
	uint32_t reserved_0_11                : 12;
#else
	uint32_t reserved_0_11                : 12;
	uint32_t baddr                        : 20;
#endif
	} s;
	struct cvmx_uahcx_ehci_periodiclistbase_s cn61xx;
	struct cvmx_uahcx_ehci_periodiclistbase_s cn63xx;
	struct cvmx_uahcx_ehci_periodiclistbase_s cn63xxp1;
	struct cvmx_uahcx_ehci_periodiclistbase_s cn66xx;
	struct cvmx_uahcx_ehci_periodiclistbase_s cn68xx;
	struct cvmx_uahcx_ehci_periodiclistbase_s cn68xxp1;
	struct cvmx_uahcx_ehci_periodiclistbase_s cnf71xx;
};
typedef union cvmx_uahcx_ehci_periodiclistbase cvmx_uahcx_ehci_periodiclistbase_t;

/**
 * cvmx_uahc#_ehci_portsc#
 *
 * PORTSCX = Port X Status and Control Register
 * Default: 00002000h (w/PPC set to one); 00003000h (w/PPC set to a zero)
 */
union cvmx_uahcx_ehci_portscx {
	uint32_t u32;
	struct cvmx_uahcx_ehci_portscx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_23_31               : 9;
	uint32_t wkoc_e                       : 1;  /**< Wake on Over-current Enable.Writing this bit to a
                                                         one enables the port to be sensitive to over-current conditions as wake-up events.
                                                         This field is zero if Port Power is zero. */
	uint32_t wkdscnnt_e                   : 1;  /**< Wake on Disconnect Enable. Writing this bit to a one enables the port to be
                                                         sensitive to device disconnects as wake-up events.
                                                         This field is zero if Port Power is zero. */
	uint32_t wkcnnt_e                     : 1;  /**< Wake on Connect Enable. Writing this bit to a one enables the port to be
                                                         sensitive to device connects as wake-up events.
                                                         This field is zero if Port Power is zero. */
	uint32_t ptc                          : 4;  /**< Port Test Control. When this field is zero, the port is NOT
                                                         operating in a test mode. A non-zero value indicates that it is operating
                                                         in test mode and the specific test mode is indicated by the specific value.
                                                         The encoding of the test mode bits are (0110b - 1111b are reserved):
                                                         Bits Test Mode
                                                          0000b Test mode not enabled
                                                          0001b Test J_STATE
                                                          0010b Test K_STATE
                                                          0011b Test SE0_NAK
                                                          0100b Test Packet
                                                          0101b Test FORCE_ENABLE */
	uint32_t pic                          : 2;  /**< Port Indicator Control. Writing to these bits has no effect if the
                                                         P_INDICATOR bit in the HCSPARAMS register is a zero. If P_INDICATOR bit is a one,
                                                         then the bit encodings are:
                                                         Bit Value Meaning
                                                          00b Port indicators are off
                                                          01b Amber
                                                          10b Green
                                                          11b Undefined
                                                         This field is zero if Port Power is zero. */
	uint32_t po                           : 1;  /**< Port Owner.This bit unconditionally goes to a 0b when the
                                                         Configured bit in the CONFIGFLAG register makes a 0b to 1b transition. This bit
                                                         unconditionally goes to 1b whenever the Configured bit is zero.
                                                         System software uses this field to release ownership of the port to a selected host
                                                         controller (in the event that the attached device is not a high-speed device). Software
                                                         writes a one to this bit when the attached device is not a high-speed device. A one in
                                                         this bit means that a companion host controller owns and controls the port. */
	uint32_t pp                           : 1;  /**< Port Power. The function of this bit depends on the value of the Port
                                                         Power Control (PPC) field in the HCSPARAMS register. The behavior is as follows:
                                                         PPC PP    Operation
                                                          0b 1b    RO  - Host controller does not have port power control switches.
                                                                         Each port is hard-wired to power.
                                                          1b 1b/0b R/W - Host controller has port power control switches. This bit
                                                                         represents the current setting of the switch (0 = off, 1 = on). When
                                                                         power is not available on a port (i.e. PP equals a 0), the port is
                                                                         nonfunctional  and will not report attaches, detaches, etc.
                                                         When an over-current condition is detected on a powered port and PPC is a one, the PP
                                                         bit in each affected port may be transitioned by the host controller from a 1 to 0
                                                         (removing power from the port). */
	uint32_t lsts                         : 2;  /**< Line Status.These bits reflect the current logical levels of the D+ (bit 11) and D(bit 10)
                                                          signal lines. These bits are used for detection of low-speed USB devices prior to
                                                          the port reset and enable sequence. This field is valid only when the port enable bit is
                                                          zero and the current connect status bit is set to a one.
                                                          The encoding of the bits are:
                                                           Bits[11:10] USB State   Interpretation
                                                           00b         SE0         Not Low-speed device, perform EHCI reset
                                                           10b         J-state     Not Low-speed device, perform EHCI reset
                                                           01b         K-state     Low-speed device, release ownership of port
                                                           11b         Undefined   Not Low-speed device, perform EHCI reset.
                                                         This value of this field is undefined if Port Power is zero. */
	uint32_t reserved_9_9                 : 1;
	uint32_t prst                         : 1;  /**< Port Reset.1=Port is in Reset. 0=Port is not in Reset. Default = 0. When
                                                         software writes a one to this bit (from a zero), the bus reset sequence as defined in the
                                                         USB Specification Revision 2.0 is started. Software writes a zero to this bit to terminate
                                                         the bus reset sequence. Software must keep this bit at a one long enough to ensure the
                                                         reset sequence, as specified in the USB Specification Revision 2.0, completes. Note:
                                                         when software writes this bit to a one, it must also write a zero to the Port Enable bit.
                                                         Note that when software writes a zero to this bit there may be a delay before the bit
                                                         status changes to a zero. The bit status will not read as a zero until after the reset has
                                                         completed. If the port is in high-speed mode after reset is complete, the host controller
                                                         will automatically enable this port (e.g. set the Port Enable bit to a one). A host controller
                                                         must terminate the reset and stabilize the state of the port within 2 milliseconds of
                                                         software transitioning this bit from a one to a zero. For example: if the port detects that
                                                         the attached device is high-speed during reset, then the host controller must have the
                                                         port in the enabled state within 2ms of software writing this bit to a zero.
                                                         The HCHalted bit in the USBSTS register should be a zero before software attempts to
                                                         use this bit. The host controller may hold Port Reset asserted to a one when the
                                                         HCHalted bit is a one.
                                                         This field is zero if Port Power is zero. */
	uint32_t spd                          : 1;  /**< Suspend. 1=Port in suspend state. 0=Port not in suspend state. Default = 0. Port
                                                         Enabled Bit and Suspend bit of this register define the port states as follows:
                                                         Bits [Port Enabled, Suspend]     Port State
                                                                      0X                  Disable
                                                                      10                  Enable
                                                                      11                  Suspend
                                                         When in suspend state, downstream propagation of data is blocked on this port, except
                                                         for port reset. The blocking occurs at the end of the current transaction, if a transaction
                                                         was in progress when this bit was written to 1. In the suspend state, the port is sensitive
                                                         to resume detection. Note that the bit status does not change until the port is
                                                         suspended and that there may be a delay in suspending a port if there is a transaction
                                                         currently in progress on the USB.
                                                         A write of zero to this bit is ignored by the host controller. The host controller will
                                                         unconditionally set this bit to a zero when:
                                                         . Software sets the Force Port Resume bit to a zero (from a one).
                                                         . Software sets the Port Reset bit to a one (from a zero).
                                                         If host software sets this bit to a one when the port is not enabled (i.e. Port enabled bit is
                                                         a zero) the results are undefined.
                                                         This field is zero if Port Power is zero. */
	uint32_t fpr                          : 1;  /**< Force Port Resume.
                                                         1= Resume detected/driven on port. 0=No resume (Kstate)
                                                         detected/driven on port. Default = 0. This functionality defined for manipulating
                                                         this bit depends on the value of the Suspend bit. For example, if the port is not
                                                         suspended (Suspend and Enabled bits are a one) and software transitions this bit to a
                                                         one, then the effects on the bus are undefined.
                                                         Software sets this bit to a 1 to drive resume signaling. The Host Controller sets this bit to
                                                         a 1 if a J-to-K transition is detected while the port is in the Suspend state. When this bit
                                                         transitions to a one because a J-to-K transition is detected, the Port Change Detect bit in
                                                         the USBSTS register is also set to a one. If software sets this bit to a one, the host
                                                         controller must not set the Port Change Detect bit.
                                                         Note that when the EHCI controller owns the port, the resume sequence follows the
                                                         defined sequence documented in the USB Specification Revision 2.0. The resume
                                                         signaling (Full-speed 'K') is driven on the port as long as this bit remains a one. Software
                                                         must appropriately time the Resume and set this bit to a zero when the appropriate
                                                         amount of time has elapsed. Writing a zero (from one) causes the port to return to high-
                                                         speed mode (forcing the bus below the port into a high-speed idle). This bit will remain a
                                                         one until the port has switched to the high-speed idle. The host controller must complete
                                                         this transition within 2 milliseconds of software setting this bit to a zero.
                                                         This field is zero if Port Power is zero. */
	uint32_t occ                          : 1;  /**< Over-current Change. 1=This bit gets set to a one when there is a change to Over-current Active.
                                                         Software clears this bit by writing a one to this bit position. */
	uint32_t oca                          : 1;  /**< Over-current Active. 1=This port currently has an over-current condition. 0=This port does not
                                                         have an over-current condition. This bit will automatically transition from a one to a zero when
                                                         the over current condition is removed. */
	uint32_t pedc                         : 1;  /**< Port Enable/Disable Change. 1=Port enabled/disabled status has changed.
                                                         0=No change. Default = 0. For the root hub, this bit gets set to a one only when a port is
                                                               disabled due to the appropriate conditions existing at the EOF2 point (See Chapter 11 of
                                                         the USB Specification for the definition of a Port Error). Software clears this bit by writing
                                                         a 1 to it.
                                                         This field is zero if Port Power is zero. */
	uint32_t ped                          : 1;  /**< Port Enabled/Disabled. 1=Enable. 0=Disable. Ports can only be
                                                         enabled by the host controller as a part of the reset and enable. Software cannot enable
                                                         a port by writing a one to this field. The host controller will only set this bit to a one when
                                                         the reset sequence determines that the attached device is a high-speed device.
                                                         Ports can be disabled by either a fault condition (disconnect event or other fault
                                                         condition) or by host software. Note that the bit status does not change until the port
                                                         state actually changes. There may be a delay in disabling or enabling a port due to other
                                                         host controller and bus events. See Section 4.2 for full details on port reset and enable.
                                                         When the port is disabled (0b) downstream propagation of data is blocked on this port,
                                                         except for reset.
                                                         This field is zero if Port Power is zero. */
	uint32_t csc                          : 1;  /**< Connect Status Change. 1=Change in Current Connect Status. 0=No change. Indicates a change
                                                         has occurred in the port's Current Connect Status. The host controller sets this bit for all
                                                         changes to the port device connect status, even if system software has not cleared an existing
                                                         connect status change. For example, the insertion status changes twice before system software
                                                         has cleared the changed condition, hub hardware will be setting an already-set bit
                                                         (i.e., the bit will remain set). Software sets this bit to 0 by writing a 1 to it.
                                                         This field is zero if Port Power is zero. */
	uint32_t ccs                          : 1;  /**< Current Connect Status. 1=Device is present on port. 0=No device is present.
                                                         This value reflects the current state of the port, and may not correspond
                                                         directly to the event that caused the Connect Status Change bit (Bit 1) to be set.
                                                         This field is zero if Port Power is zero. */
#else
	uint32_t ccs                          : 1;
	uint32_t csc                          : 1;
	uint32_t ped                          : 1;
	uint32_t pedc                         : 1;
	uint32_t oca                          : 1;
	uint32_t occ                          : 1;
	uint32_t fpr                          : 1;
	uint32_t spd                          : 1;
	uint32_t prst                         : 1;
	uint32_t reserved_9_9                 : 1;
	uint32_t lsts                         : 2;
	uint32_t pp                           : 1;
	uint32_t po                           : 1;
	uint32_t pic                          : 2;
	uint32_t ptc                          : 4;
	uint32_t wkcnnt_e                     : 1;
	uint32_t wkdscnnt_e                   : 1;
	uint32_t wkoc_e                       : 1;
	uint32_t reserved_23_31               : 9;
#endif
	} s;
	struct cvmx_uahcx_ehci_portscx_s      cn61xx;
	struct cvmx_uahcx_ehci_portscx_s      cn63xx;
	struct cvmx_uahcx_ehci_portscx_s      cn63xxp1;
	struct cvmx_uahcx_ehci_portscx_s      cn66xx;
	struct cvmx_uahcx_ehci_portscx_s      cn68xx;
	struct cvmx_uahcx_ehci_portscx_s      cn68xxp1;
	struct cvmx_uahcx_ehci_portscx_s      cnf71xx;
};
typedef union cvmx_uahcx_ehci_portscx cvmx_uahcx_ehci_portscx_t;

/**
 * cvmx_uahc#_ehci_usbcmd
 *
 * USBCMD = USB Command Register
 * The Command Register indicates the command to be executed by the serial bus host controller. Writing to the register causes a command to be executed.
 */
union cvmx_uahcx_ehci_usbcmd {
	uint32_t u32;
	struct cvmx_uahcx_ehci_usbcmd_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t itc                          : 8;  /**< Interrupt Threshold Control. This field is used by system software
                                                         to select the maximum rate at which the host controller will issue interrupts. The only
                                                         valid values are defined below. If software writes an invalid value to this register, the
                                                         results are undefined. Value Maximum Interrupt Interval
                                                           00h Reserved
                                                           01h 1 micro-frame
                                                           02h 2 micro-frames
                                                           04h 4 micro-frames
                                                           08h 8 micro-frames (default, equates to 1 ms)
                                                           10h 16 micro-frames (2 ms)
                                                           20h 32 micro-frames (4 ms)
                                                           40h 64 micro-frames (8 ms) */
	uint32_t reserved_12_15               : 4;
	uint32_t aspm_en                      : 1;  /**< Asynchronous Schedule Park Mode Enable. */
	uint32_t reserved_10_10               : 1;
	uint32_t aspmc                        : 2;  /**< Asynchronous Schedule Park Mode Count. */
	uint32_t lhcr                         : 1;  /**< Light Host Controller Reset */
	uint32_t iaa_db                       : 1;  /**< Interrupt on Async Advance Doorbell.This bit is used as a doorbell by
                                                         software to tell the host controller to issue an interrupt the next time it advances
                                                         asynchronous schedule. Software must write a 1 to this bit to ring the doorbell.
                                                         When the host controller has evicted all appropriate cached schedule state, it sets the
                                                         Interrupt on Async Advance status bit in the USBSTS register. If the Interrupt on Async
                                                         Advance Enable bit in the USBINTR register is a one then the host controller will assert
                                                         an interrupt at the next interrupt threshold. */
	uint32_t as_en                        : 1;  /**< Asynchronous Schedule Enable .This bit controls whether the host
                                                         controller skips processing the Asynchronous Schedule. Values mean:
                                                          - 0: Do not process the Asynchronous Schedule
                                                          - 1: Use the ASYNCLISTADDR register to access the Asynchronous Schedule. */
	uint32_t ps_en                        : 1;  /**< Periodic Schedule Enable. This bit controls whether the host
                                                         controller skips processing the Periodic Schedule. Values mean:
                                                            - 0: Do not process the Periodic Schedule
                                                            - 1: Use the PERIODICLISTBASE register to access the Periodic Schedule. */
	uint32_t fls                          : 2;  /**< Frame List Size. This field is R/W only if Programmable
                                                         Frame List Flag in the HCCPARAMS registers is set to a one. This field specifies the
                                                         size of the frame list. The size the frame list controls which bits in the Frame Index
                                                         Register should be used for the Frame List Current index. Values mean:
                                                              00b: 1024 elements (4096 bytes) Default value
                                                              01b: 512 elements  (2048 bytes)
                                                              10b: 256 elements  (1024 bytes) - for resource-constrained environments
                                                              11b: Reserved */
	uint32_t hcreset                      : 1;  /**< Host Controller Reset (HCRESET). This control bit is used by software to reset
                                                         the host controller. The effects of this on Root Hub registers are similar to a Chip
                                                         Hardware Reset. When software writes a one to this bit, the Host Controller resets
                                                         its internal pipelines, timers, counters, state machines, etc. to their initial
                                                         value. Any transaction currently in progress on USB is immediately terminated.
                                                         A USB reset is not driven on downstream ports.
                                                         This bit is set to zero by the Host Controller when the reset process is complete. Software can not
                                                         terminate the reset process early by writing zero to this register.
                                                         Software should not set this bit to a one when the HCHalted bit in the USBSTS register is a zero.
                                                         Attempting to reset an activtely running host controller will result in undefined behavior. */
	uint32_t rs                           : 1;  /**< Run/Stop (RS).
                                                           1=Run. 0=Stop.
                                                         When set to a 1, the Host Controller proceeds with execution of the schedule.
                                                         The Host Controller continues execution as long as this bit is set to a 1.
                                                         When this bit is set to 0, the Host Controller completes the current and any
                                                         actively pipelined transactions on the USB and then halts. The Host
                                                         Controller must halt within 16 micro-frames after software clears the Run bit. The HC
                                                         Halted bit in the status register indicates when the Host Controller has finished its
                                                         pending pipelined transactions and has entered the stopped state. Software must not
                                                         write a one to this field unless the host controller is in the Halted state (i.e. HCHalted in
                                                         the USBSTS register is a one). Doing so will yield undefined results. */
#else
	uint32_t rs                           : 1;
	uint32_t hcreset                      : 1;
	uint32_t fls                          : 2;
	uint32_t ps_en                        : 1;
	uint32_t as_en                        : 1;
	uint32_t iaa_db                       : 1;
	uint32_t lhcr                         : 1;
	uint32_t aspmc                        : 2;
	uint32_t reserved_10_10               : 1;
	uint32_t aspm_en                      : 1;
	uint32_t reserved_12_15               : 4;
	uint32_t itc                          : 8;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_uahcx_ehci_usbcmd_s       cn61xx;
	struct cvmx_uahcx_ehci_usbcmd_s       cn63xx;
	struct cvmx_uahcx_ehci_usbcmd_s       cn63xxp1;
	struct cvmx_uahcx_ehci_usbcmd_s       cn66xx;
	struct cvmx_uahcx_ehci_usbcmd_s       cn68xx;
	struct cvmx_uahcx_ehci_usbcmd_s       cn68xxp1;
	struct cvmx_uahcx_ehci_usbcmd_s       cnf71xx;
};
typedef union cvmx_uahcx_ehci_usbcmd cvmx_uahcx_ehci_usbcmd_t;

/**
 * cvmx_uahc#_ehci_usbintr
 *
 * USBINTR = USB Interrupt Enable Register
 * This register enables and disables reporting of the corresponding interrupt to the software. When a bit is set
 * and the corresponding interrupt is active, an interrupt is generated to the host. Interrupt sources that are
 * disabled in this register still appear in the USBSTS to allow the software to poll for events.
 * Each interrupt enable bit description indicates whether it is dependent on the interrupt threshold mechanism.
 * Note: for all enable register bits, 1= Enabled, 0= Disabled
 */
union cvmx_uahcx_ehci_usbintr {
	uint32_t u32;
	struct cvmx_uahcx_ehci_usbintr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_6_31                : 26;
	uint32_t ioaa_en                      : 1;  /**< Interrupt on Async Advance Enable When this bit is a one, and the Interrupt on
                                                         Async Advance bit in the USBSTS register is a one, the host controller will issue an
                                                         interrupt at the next interrupt threshold. The interrupt is acknowledged by software
                                                         clearing the Interrupt on Async Advance bit. */
	uint32_t hserr_en                     : 1;  /**< Host System Error Enable When this bit is a one, and the Host System
                                                         Error Status bit in the USBSTS register is a one, the host controller will issue an
                                                         interrupt. The interrupt is acknowledged by software clearing the Host System Error bit. */
	uint32_t flro_en                      : 1;  /**< Frame List Rollover Enable. When this bit is a one, and the Frame List
                                                         Rollover bit in the USBSTS register is a one, the host controller will issue an
                                                         interrupt. The interrupt is acknowledged by software clearing the Frame List Rollover bit. */
	uint32_t pci_en                       : 1;  /**< Port Change Interrupt Enable. When this bit is a one, and the Port Change Detect bit in
                                                         the USBSTS register is a one, the host controller will issue an interrupt.
                                                         The interrupt is acknowledged by software clearing the Port Change Detect bit. */
	uint32_t usberrint_en                 : 1;  /**< USB Error Interrupt Enable. When this bit is a one, and the USBERRINT
                                                         bit in the USBSTS register is a one, the host controller will issue an interrupt at the next
                                                         interrupt threshold. The interrupt is acknowledged by software clearing the USBERRINT bit. */
	uint32_t usbint_en                    : 1;  /**< USB Interrupt Enable. When this bit is a one, and the USBINT bit in the USBSTS register
                                                         is a one, the host controller will issue an interrupt at the next interrupt threshold.
                                                         The interrupt is acknowledged by software clearing the USBINT bit. */
#else
	uint32_t usbint_en                    : 1;
	uint32_t usberrint_en                 : 1;
	uint32_t pci_en                       : 1;
	uint32_t flro_en                      : 1;
	uint32_t hserr_en                     : 1;
	uint32_t ioaa_en                      : 1;
	uint32_t reserved_6_31                : 26;
#endif
	} s;
	struct cvmx_uahcx_ehci_usbintr_s      cn61xx;
	struct cvmx_uahcx_ehci_usbintr_s      cn63xx;
	struct cvmx_uahcx_ehci_usbintr_s      cn63xxp1;
	struct cvmx_uahcx_ehci_usbintr_s      cn66xx;
	struct cvmx_uahcx_ehci_usbintr_s      cn68xx;
	struct cvmx_uahcx_ehci_usbintr_s      cn68xxp1;
	struct cvmx_uahcx_ehci_usbintr_s      cnf71xx;
};
typedef union cvmx_uahcx_ehci_usbintr cvmx_uahcx_ehci_usbintr_t;

/**
 * cvmx_uahc#_ehci_usbsts
 *
 * USBSTS = USB Status Register
 * This register indicates pending interrupts and various states of the Host Controller. The status resulting from
 * a transaction on the serial bus is not indicated in this register. Software sets a bit to 0 in this register by
 * writing a 1 to it.
 */
union cvmx_uahcx_ehci_usbsts {
	uint32_t u32;
	struct cvmx_uahcx_ehci_usbsts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t ass                          : 1;  /**< Asynchronous Schedule Status. The bit reports the current real
                                                         status of the Asynchronous Schedule. If this bit is a zero then the status of the
                                                         Asynchronous Schedule is disabled. If this bit is a one then the status of the
                                                         Asynchronous Schedule is enabled. The Host Controller is not required to immediately
                                                         disable or enable the Asynchronous Schedule when software transitions the
                                                         Asynchronous Schedule Enable bit in the USBCMD register. When this bit and the
                                                         Asynchronous Schedule Enable bit are the same value, the Asynchronous Schedule is
                                                         either enabled (1) or disabled (0). */
	uint32_t pss                          : 1;  /**< Periodic Schedule Status. The bit reports the current real status of
                                                         the Periodic Schedule. If this bit is a zero then the status of the Periodic
                                                         Schedule is disabled. If this bit is a one then the status of the Periodic Schedule
                                                         is enabled. The Host Controller is not required to immediately disable or enable the
                                                         Periodic Schedule when software transitions the Periodic Schedule Enable bit in
                                                         the USBCMD register. When this bit and the Periodic Schedule Enable bit are the
                                                         same value, the Periodic Schedule is either enabled (1) or disabled (0). */
	uint32_t reclm                        : 1;  /**< Reclamation.This is a read-only status bit, which is used to detect an
                                                         empty asynchronous schedule. */
	uint32_t hchtd                        : 1;  /**< HCHalted. This bit is a zero whenever the Run/Stop bit is a one. The
                                                         Host Controller sets this bit to one after it has stopped executing as a result of the
                                                         Run/Stop bit being set to 0, either by software or by the Host Controller hardware (e.g.
                                                         internal error). */
	uint32_t reserved_6_11                : 6;
	uint32_t ioaa                         : 1;  /**< Interrupt on Async Advance. System software can force the host
                                                         controller to issue an interrupt the next time the host controller advances the
                                                         asynchronous schedule by writing a one to the Interrupt on Async Advance Doorbell bit
                                                         in the USBCMD register. This status bit indicates the assertion of that interrupt source. */
	uint32_t hsyserr                      : 1;  /**< Host System Error. The Host Controller sets this bit to 1 when a serious error
                                                         occurs during a host system access involving the Host Controller module. */
	uint32_t flro                         : 1;  /**< Frame List Rollover. The Host Controller sets this bit to a one when the
                                                         Frame List Index rolls over from its maximum value to zero. The exact value at
                                                         which the rollover occurs depends on the frame list size. For example, if
                                                         the frame list size (as programmed in the Frame List Size field of the USBCMD register)
                                                         is 1024, the Frame Index Register rolls over every time FRINDEX[13] toggles. Similarly,
                                                         if the size is 512, the Host Controller sets this bit to a one every time FRINDEX[12]
                                                         toggles. */
	uint32_t pcd                          : 1;  /**< Port Change Detect. The Host Controller sets this bit to a one when any port
                                                         for which the Port Owner bit is set to zero (see Section 2.3.9) has a change bit transition
                                                         from a zero to a one or a Force Port Resume bit transition from a zero to a one as a
                                                         result of a J-K transition detected on a suspended port. This bit will also be set as a
                                                         result of the Connect Status Change being set to a one after system software has
                                                         relinquished ownership of a connected port by writing a one to a port's Port Owner bit. */
	uint32_t usberrint                    : 1;  /**< USB Error Interrupt. The Host Controller sets this bit to 1 when completion of a USB
                                                         transaction results in an error condition (e.g., error counter underflow). If the TD on
                                                         which the error interrupt occurred also had its IOC bit set, both this bit and USBINT
                                                         bit are set. */
	uint32_t usbint                       : 1;  /**< USB Interrupt. The Host Controller sets this bit to 1 on the completion of a USB
                                                         transaction, which results in the retirement of a Transfer Descriptor that had its
                                                         IOC bit set. The Host Controller also sets this bit to 1 when a short packet is
                                                         detected (actual number of bytes received was less than the expected number of bytes). */
#else
	uint32_t usbint                       : 1;
	uint32_t usberrint                    : 1;
	uint32_t pcd                          : 1;
	uint32_t flro                         : 1;
	uint32_t hsyserr                      : 1;
	uint32_t ioaa                         : 1;
	uint32_t reserved_6_11                : 6;
	uint32_t hchtd                        : 1;
	uint32_t reclm                        : 1;
	uint32_t pss                          : 1;
	uint32_t ass                          : 1;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_uahcx_ehci_usbsts_s       cn61xx;
	struct cvmx_uahcx_ehci_usbsts_s       cn63xx;
	struct cvmx_uahcx_ehci_usbsts_s       cn63xxp1;
	struct cvmx_uahcx_ehci_usbsts_s       cn66xx;
	struct cvmx_uahcx_ehci_usbsts_s       cn68xx;
	struct cvmx_uahcx_ehci_usbsts_s       cn68xxp1;
	struct cvmx_uahcx_ehci_usbsts_s       cnf71xx;
};
typedef union cvmx_uahcx_ehci_usbsts cvmx_uahcx_ehci_usbsts_t;

/**
 * cvmx_uahc#_erdp#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.5.2.3.3.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_erdpx {
	uint64_t u64;
	struct cvmx_uahcx_erdpx_s {
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
	struct cvmx_uahcx_erdpx_s             cn78xx;
	struct cvmx_uahcx_erdpx_s             cn78xxp1;
};
typedef union cvmx_uahcx_erdpx cvmx_uahcx_erdpx_t;

/**
 * cvmx_uahc#_erstba#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.5.2.3.2.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_erstbax {
	uint64_t u64;
	struct cvmx_uahcx_erstbax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t erstba                       : 58; /**< Event-ring segment-table base-address bits<63:6>. */
	uint64_t reserved_0_5                 : 6;
#else
	uint64_t reserved_0_5                 : 6;
	uint64_t erstba                       : 58;
#endif
	} s;
	struct cvmx_uahcx_erstbax_s           cn78xx;
	struct cvmx_uahcx_erstbax_s           cn78xxp1;
};
typedef union cvmx_uahcx_erstbax cvmx_uahcx_erstbax_t;

/**
 * cvmx_uahc#_erstsz#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.5.2.3.1.
 *
 * This register can be reset by NCB reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_erstszx {
	uint32_t u32;
	struct cvmx_uahcx_erstszx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t erstsz                       : 16; /**< Event-ring segment-table size. */
#else
	uint32_t erstsz                       : 16;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_uahcx_erstszx_s           cn78xx;
	struct cvmx_uahcx_erstszx_s           cn78xxp1;
};
typedef union cvmx_uahcx_erstszx cvmx_uahcx_erstszx_t;

/**
 * cvmx_uahc#_gbuserraddr
 *
 * When the AXI master bus returns error response, the SoC bus error is generated. In the host
 * mode, the host_system_err port indicates this condition. In addition, it is also indicated in
 * UAHC()_USBSTS[HSE]. Due to the nature of AXI, it is possible that multiple AXI transactions
 * are active at a time. The host controller does not keep track of the start address of all
 * outstanding transactions. Instead, it keeps track of the start address of the DMA transfer
 * associated with all active transactions. It is this address that is reported in
 * UAHC()_GBUSERRADDR when a bus error occurs. For example, if the host controller initiates a
 * DMA
 * transfer to write 1 k of packet data starting at buffer address 0xABCD0000, and this DMA is
 * broken up into multiple 256 B bursts on the AXI, then if a bus error occurs on any of these
 * associated AXI transfers, UAHC()_GBUSERRADDR reflects the DMA start address of 0xABCD0000
 * regardless of which AXI transaction received the error.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gbuserraddr {
	uint64_t u64;
	struct cvmx_uahcx_gbuserraddr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t busaddr                      : 64; /**< Bus address. Contains the first bus address that encountered an SoC bus error. It is valid
                                                         when the UAHC()_GSTS[BUSERRADDRVLD] = 1. It can only be cleared by resetting the core. */
#else
	uint64_t busaddr                      : 64;
#endif
	} s;
	struct cvmx_uahcx_gbuserraddr_s       cn78xx;
	struct cvmx_uahcx_gbuserraddr_s       cn78xxp1;
};
typedef union cvmx_uahcx_gbuserraddr cvmx_uahcx_gbuserraddr_t;

/**
 * cvmx_uahc#_gctl
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 *
 */
union cvmx_uahcx_gctl {
	uint32_t u32;
	struct cvmx_uahcx_gctl_s {
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
	uint32_t reserved_16_17               : 2;
	uint32_t frmscldwn                    : 2;  /**< Frame scale down. Scales down device view of a SOF/USOF/ITP duration.
                                                         For SuperSpeed/high-speed mode:
                                                         0x0 = Interval is 125 us.
                                                         0x1 = Interval is 62.5 us.
                                                         0x2 = Interval is 31.25 us.
                                                         0x3 = Interval is 15.625 us.
                                                         For full speed mode, the scale down value is multiplied by 8. */
	uint32_t prtcapdir                    : 2;  /**< Port capability direction. Always keep set to 0x1. */
	uint32_t coresoftreset                : 1;  /**< Core soft reset: 1 = soft reset to core, 0 = no soft reset.
                                                         Clears the interrupts and all the UAHC()_* CSRs except the
                                                         following registers: UAHC()_GCTL, UAHC()_GUCTL, UAHC()_GSTS,
                                                         UAHC()_GRLSID, UAHC()_GGPIO, UAHC()_GUID, UAHC()_GUSB2PHYCFG(),
                                                         UAHC()_GUSB3PIPECTL().
                                                         When you reset PHYs (using UAHC()_GUSB2PHYCFG() or UAHC()_GUSB3PIPECTL()), you must keep
                                                         the
                                                         core in reset state until PHY clocks are stable. This controls the bus, RAM, and MAC
                                                         domain resets. */
	uint32_t sofitpsync                   : 1;  /**< Synchronize ITP to reference clock. In host mode, if this bit is set to:
                                                         0 = The core keeps the UTMI/ULPI PHY on the first port in non-suspended state whenever
                                                         there is a SuperSpeed port that is not in Rx.Detect, SS.Disable, and U3 state.
                                                         1 = The core keeps the UTMI/ULPI PHY on the first port in non-suspended state whenever the
                                                         other non-SuperSpeed ports are not in suspended state.
                                                         This feature is useful because it saves power by suspending UTMI/ULPI when SuperSpeed only
                                                         is active and it helps resolve when the PHY does not transmit a host resume unless it is
                                                         placed in suspend state.
                                                         UAHC()_GUSB2PHYCFG()[SUSPHY] eventually decides to put the UTMI/ULPI PHY in to suspend
                                                         state. In addition, when this bit is set to 1, the core generates ITP off of the REF_CLK-
                                                         based counter. Otherwise, ITP and SOF are generated off of UTMI/ULPI_CLK[0] based counter.
                                                         To program the reference clock period inside the core, refer to UAHC()_GUCTL[REFCLKPER].
                                                         If you do not plan to ever use this feature or the UAHC()_GFLADJ[GFLADJ_REFCLK_LPM_SEL]
                                                         feature, the minimum frequency for the ref_clk can be as low as 32 KHz. You can connect
                                                         the
                                                         SUSPEND_CLK (as low as 32 KHz) to REF_CLK.
                                                         If you plan to enable hardware-based LPM (PORTPMSC[HLE] = 1), this feature cannot be used.
                                                         Turn off this feature by setting this bit to zero and use the
                                                         UAHC()_GFLADJ[GFLADJ_REFCLK_LPM_SEL] feature.
                                                         If you set this bit to 1, the UAHC()_GUSB2PHYCFG() [U2_FREECLK_EXISTS] bit must be set to
                                                         0. */
	uint32_t u1u2timerscale               : 1;  /**< Disable U1/U2 timer scaledown. If set to 1, along with SCALEDOWN = 0x1, disables the scale
                                                         down of U1/U2 inactive timer values.
                                                         This is for simulation mode only. */
	uint32_t debugattach                  : 1;  /**< Debug attach. When this bit is set:
                                                         * SuperSpeed link proceeds directly to the polling-link state (UAHC()_DCTL[RS] = 1)
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
                                                         0x2 = N/A.
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
	uint32_t reserved_16_17               : 2;
	uint32_t masterfiltbypass             : 1;
	uint32_t pwrdnscale                   : 13;
#endif
	} s;
	struct cvmx_uahcx_gctl_s              cn78xx;
	struct cvmx_uahcx_gctl_s              cn78xxp1;
};
typedef union cvmx_uahcx_gctl cvmx_uahcx_gctl_t;

/**
 * cvmx_uahc#_gdbgbmu
 *
 * See description in UAHC()_GDBGFIFOSPACE.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gdbgbmu {
	uint32_t u32;
	struct cvmx_uahcx_gdbgbmu_s {
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
	struct cvmx_uahcx_gdbgbmu_s           cn78xx;
	struct cvmx_uahcx_gdbgbmu_s           cn78xxp1;
};
typedef union cvmx_uahcx_gdbgbmu cvmx_uahcx_gdbgbmu_t;

/**
 * cvmx_uahc#_gdbgepinfo
 *
 * See description in UAHC()_GDBGFIFOSPACE.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gdbgepinfo {
	uint64_t u64;
	struct cvmx_uahcx_gdbgepinfo_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t endpt_dbg                    : 64; /**< Endpoint debug information. */
#else
	uint64_t endpt_dbg                    : 64;
#endif
	} s;
	struct cvmx_uahcx_gdbgepinfo_s        cn78xx;
	struct cvmx_uahcx_gdbgepinfo_s        cn78xxp1;
};
typedef union cvmx_uahcx_gdbgepinfo cvmx_uahcx_gdbgepinfo_t;

/**
 * cvmx_uahc#_gdbgfifospace
 *
 * This register is for debug purposes. It provides debug information on the internal status and
 * state machines. Global debug registers have design-specific information, and are used by state
 * machines. Global debug registers have design-specific information, and are used for debugging
 * purposes. These registers are not intended to be used by the customer. If any debug assistance
 * is needed for the silicon, contact customer support with a dump of these registers.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gdbgfifospace {
	uint32_t u32;
	struct cvmx_uahcx_gdbgfifospace_s {
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
                                                         Port-select: <3:0> selects the port-number when accessing UAHC()_GDBGLTSSM. */
#else
	uint32_t select                       : 8;
	uint32_t reserved_8_15                : 8;
	uint32_t spaceavailable               : 16;
#endif
	} s;
	struct cvmx_uahcx_gdbgfifospace_s     cn78xx;
	struct cvmx_uahcx_gdbgfifospace_s     cn78xxp1;
};
typedef union cvmx_uahcx_gdbgfifospace cvmx_uahcx_gdbgfifospace_t;

/**
 * cvmx_uahc#_gdbglnmcc
 *
 * See description in UAHC()_GDBGFIFOSPACE.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gdbglnmcc {
	uint32_t u32;
	struct cvmx_uahcx_gdbglnmcc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_9_31                : 23;
	uint32_t lnmcc_berc                   : 9;  /**< This field indicates the bit-error-rate information for the port selected in
                                                         UAHC()_GDBGFIFOSPACE[SELECT] (port-select).
                                                         This field is for debug purposes only. */
#else
	uint32_t lnmcc_berc                   : 9;
	uint32_t reserved_9_31                : 23;
#endif
	} s;
	struct cvmx_uahcx_gdbglnmcc_s         cn78xx;
	struct cvmx_uahcx_gdbglnmcc_s         cn78xxp1;
};
typedef union cvmx_uahcx_gdbglnmcc cvmx_uahcx_gdbglnmcc_t;

/**
 * cvmx_uahc#_gdbglsp
 *
 * See description in UAHC()_GDBGFIFOSPACE.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gdbglsp {
	uint32_t u32;
	struct cvmx_uahcx_gdbglsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lsp_dbg                      : 32; /**< LSP debug information. */
#else
	uint32_t lsp_dbg                      : 32;
#endif
	} s;
	struct cvmx_uahcx_gdbglsp_s           cn78xx;
	struct cvmx_uahcx_gdbglsp_s           cn78xxp1;
};
typedef union cvmx_uahcx_gdbglsp cvmx_uahcx_gdbglsp_t;

/**
 * cvmx_uahc#_gdbglspmux
 *
 * See description in UAHC()_GDBGFIFOSPACE.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gdbglspmux {
	uint32_t u32;
	struct cvmx_uahcx_gdbglspmux_s {
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
	uint32_t hostselect                   : 14; /**< Host select. Selects the LSP debug information presented in UAHC()_GDBGLSP. */
#else
	uint32_t hostselect                   : 14;
	uint32_t reserved_14_14               : 1;
	uint32_t endbc                        : 1;
	uint32_t latraceportmuxselect         : 8;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_uahcx_gdbglspmux_s        cn78xx;
	struct cvmx_uahcx_gdbglspmux_s        cn78xxp1;
};
typedef union cvmx_uahcx_gdbglspmux cvmx_uahcx_gdbglspmux_t;

/**
 * cvmx_uahc#_gdbgltssm
 *
 * In multiport host configuration, the port number is defined by
 * UAHC()_GDBGFIFOSPACE[SELECT]<3:0>. Value of this register may change immediately after reset.
 * See description in UAHC()_GDBGFIFOSPACE.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gdbgltssm {
	uint32_t u32;
	struct cvmx_uahcx_gdbgltssm_s {
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
                                                         _ 0x0 = PHY_IDLE (PHY command state is in IDLE. No PHY request is pending).
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
	struct cvmx_uahcx_gdbgltssm_s         cn78xx;
	struct cvmx_uahcx_gdbgltssm_s         cn78xxp1;
};
typedef union cvmx_uahcx_gdbgltssm cvmx_uahcx_gdbgltssm_t;

/**
 * cvmx_uahc#_gdmahlratio
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
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gdmahlratio {
	uint32_t u32;
	struct cvmx_uahcx_gdmahlratio_s {
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
	struct cvmx_uahcx_gdmahlratio_s       cn78xx;
	struct cvmx_uahcx_gdmahlratio_s       cn78xxp1;
};
typedef union cvmx_uahcx_gdmahlratio cvmx_uahcx_gdmahlratio_t;

/**
 * cvmx_uahc#_gfladj
 *
 * This register provides options for the software to control the core behavior with respect to
 * SOF (start of frame) and ITP (isochronous timestamp packet) timers and frame timer
 * functionality. It provides the option to override the sideband signal fladj_30mhz_reg. In
 * addition, it enables running SOF or ITP frame timer counters completely off of the REF_CLK.
 * This facilitates hardware LPM in host mode with the SOF or ITP counters being run off of the
 * REF_CLK signal.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gfladj {
	uint32_t u32;
	struct cvmx_uahcx_gfladj_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t gfladj_refclk_240mhzdecr_pls1 : 1; /**< This field indicates that the decrement value that the controller applies for each REF_CLK
                                                         must be GFLADJ_REFCLK_240MHZ_DECR and GFLADJ_REFCLK_240MHZ_DECR +1 alternatively on each
                                                         REF_CLK. Set this bit to 1 only if [GFLADJ_REFCLK_LPM_SEL] is set to 1 and the fractional
                                                         component of 240/ref_frequency is greater than or equal to 0.5.
                                                         Example:
                                                         If the REF_CLK is 19.2 MHz then:
                                                         * UAHC()_GUCTL[REFCLKPER] = 52.
                                                         * [GFLADJ_REFCLK_240MHZ_DECR] = (240/19.2) = 12.5.
                                                         * [GFLADJ_REFCLK_240MHZDECR_PLS1] = 1.
                                                         If the REF_CLK is 24 MHz then:
                                                         * UAHC()_GUCTL[REFCLKPER] = 41.
                                                         * [GFLADJ_REFCLK_240MHZ_DECR] = (240/24) = 10.
                                                         * [GFLADJ_REFCLK_240MHZDECR_PLS1] = 0. */
	uint32_t gfladj_refclk_240mhz_decr    : 7;  /**< This field indicates the decrement value that the controller applies for each REF_CLK in
                                                         order to derive a frame timer in terms of a 240-MHz clock. This field must be programmed
                                                         to a nonzero value only if [GFLADJ_REFCLK_LPM_SEL] is set to 1.
                                                         The value is derived as follows:
                                                         _ [GFLADJ_REFCLK_240MHZ_DECR] = 240/ref_clk_frequency
                                                         Examples:
                                                         If the REF_CLK is 24 MHz then:
                                                         * UAHC()_GUCTL[REFCLKPER] = 41.
                                                         * [GFLADJ_REFCLK_240MHZ_DECR] = 240/24 = 10.
                                                         If the REF_CLK is 48 MHz then:
                                                         * UAHC()_GUCTL[REFCLKPER] = 20.
                                                         * [GFLADJ_REFCLK_240MHZ_DECR] = 240/48 = 5.
                                                         If the REF_CLK is 17 MHz then:
                                                         * UAHC()_GUCTL[REFCLKPER] = 58.
                                                         * [GFLADJ_REFCLK_240MHZ_DECR] = 240/17 = 14. */
	uint32_t gfladj_refclk_lpm_sel        : 1;  /**< This bit enables the functionality of running SOF/ITP counters on the REF_CLK.
                                                         This bit must not be set to 1 if UAHC()_GCTL[SOFITPSYNC] = 1. Similarly, if
                                                         GFLADJ_REFCLK_LPM_SEL = 1, UAHC()_GCTL[SOFITPSYNC] must not be set to 1.
                                                         When GFLADJ_REFCLK_LPM_SEL = 1 the overloading of the suspend control of the USB 2.0 first
                                                         port PHY (UTMI) with USB 3.0 port states is removed. Note that the REF_CLK frequencies
                                                         supported in this mode are 16/17/19.2/20/24/39.7/40 MHz. */
	uint32_t reserved_22_22               : 1;
	uint32_t gfladj_refclk_fladj          : 14; /**< This field indicates the frame length adjustment to be applied when SOF/ITP counter is
                                                         running off of the REF_CLK. This register value is used to adjust:.
                                                         * ITP interval when UAHC()_GCTL[SOFITPSYNC] = 1
                                                         * both SOF and ITP interval when [GFLADJ_REFCLK_LPM_SEL] = 1.
                                                         This field must be programmed to a nonzero value only if [GFLADJ_REFCLK_LPM_SEL] = 1 or
                                                         UAHC()_GCTL[SOFITPSYNC] = 1.
                                                         The value is derived as below:
                                                         _ FLADJ_REF_CLK_FLADJ = ((125000/ref_clk_period_integer) - (125000/ref_clk_period)) *
                                                         ref_clk_period
                                                         where,
                                                         * the ref_clk_period_integer is the integer value of the REF_CLK period got by truncating
                                                         the decimal (fractional) value that is programmed in UAHC()_GUCTL[REFCLKPER].
                                                         * the ref_clk_period is the REF_CLK period including the fractional value.
                                                         Examples:
                                                         If the REF_CLK is 24 MHz then:
                                                         * UAHC()_GUCTL[REFCLKPER] = 41.
                                                         * GLADJ_REFCLK_FLADJ = ((125000/41) -
                                                         (125000/41.6666)) * 41.6666 = 2032 (ignoring the fractional value).
                                                         If the REF_CLK is 48 MHz then:
                                                         * UAHC()_GUCTL[REFCLKPER] = 20.
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
                                                         UAHC()_GCTL[SOFITPSYNC] is 1 or 0). For details on how to set this value, refer to
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
	struct cvmx_uahcx_gfladj_s            cn78xx;
	struct cvmx_uahcx_gfladj_s            cn78xxp1;
};
typedef union cvmx_uahcx_gfladj cvmx_uahcx_gfladj_t;

/**
 * cvmx_uahc#_ggpio
 *
 * The application can use this register for general purpose input and output ports or for
 * debugging.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_ggpio {
	uint32_t u32;
	struct cvmx_uahcx_ggpio_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t gpo                          : 16; /**< General purpose output. These outputs are not connected to anything. Can be used as scratch. */
	uint32_t gpi                          : 16; /**< General purpose input. These inputs are tied 0x0. */
#else
	uint32_t gpi                          : 16;
	uint32_t gpo                          : 16;
#endif
	} s;
	struct cvmx_uahcx_ggpio_s             cn78xx;
	struct cvmx_uahcx_ggpio_s             cn78xxp1;
};
typedef union cvmx_uahcx_ggpio cvmx_uahcx_ggpio_t;

/**
 * cvmx_uahc#_ghwparams0
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_uahcx_ghwparams0 {
	uint32_t u32;
	struct cvmx_uahcx_ghwparams0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t awidth                       : 8;  /**< USB core bus-address width. */
	uint32_t sdwidth                      : 8;  /**< USB core bus slave-data width. */
	uint32_t mdwidth                      : 8;  /**< USB core bus master-data width. */
	uint32_t sbus_type                    : 2;  /**< USB core bus slave type: AXI. */
	uint32_t mbus_type                    : 3;  /**< USB core bus master type: AXI. */
	uint32_t mode                         : 3;  /**< Operation mode: 0x1: host-only. */
#else
	uint32_t mode                         : 3;
	uint32_t mbus_type                    : 3;
	uint32_t sbus_type                    : 2;
	uint32_t mdwidth                      : 8;
	uint32_t sdwidth                      : 8;
	uint32_t awidth                       : 8;
#endif
	} s;
	struct cvmx_uahcx_ghwparams0_s        cn78xx;
	struct cvmx_uahcx_ghwparams0_s        cn78xxp1;
};
typedef union cvmx_uahcx_ghwparams0 cvmx_uahcx_ghwparams0_t;

/**
 * cvmx_uahc#_ghwparams1
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_uahcx_ghwparams1 {
	uint32_t u32;
	struct cvmx_uahcx_ghwparams1_s {
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
	uint32_t device_num_int               : 6;  /**< Number of event buffers (and interrupts) in device-mode (unsupported). */
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
	struct cvmx_uahcx_ghwparams1_s        cn78xx;
	struct cvmx_uahcx_ghwparams1_s        cn78xxp1;
};
typedef union cvmx_uahcx_ghwparams1 cvmx_uahcx_ghwparams1_t;

/**
 * cvmx_uahc#_ghwparams2
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_uahcx_ghwparams2 {
	uint32_t u32;
	struct cvmx_uahcx_ghwparams2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t userid                       : 32; /**< User ID. */
#else
	uint32_t userid                       : 32;
#endif
	} s;
	struct cvmx_uahcx_ghwparams2_s        cn78xx;
	struct cvmx_uahcx_ghwparams2_s        cn78xxp1;
};
typedef union cvmx_uahcx_ghwparams2 cvmx_uahcx_ghwparams2_t;

/**
 * cvmx_uahc#_ghwparams3
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_uahcx_ghwparams3 {
	uint32_t u32;
	struct cvmx_uahcx_ghwparams3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t cache_total_xfer_resources   : 8;  /**< Maximum number of transfer resources in the core. */
	uint32_t num_in_eps                   : 5;  /**< Maximum number of device-mode (unsupported) IN endpoints active. */
	uint32_t num_eps                      : 6;  /**< Number of device-mode (unsupported) single-directional endpoints. */
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
	struct cvmx_uahcx_ghwparams3_s        cn78xx;
	struct cvmx_uahcx_ghwparams3_s        cn78xxp1;
};
typedef union cvmx_uahcx_ghwparams3 cvmx_uahcx_ghwparams3_t;

/**
 * cvmx_uahc#_ghwparams4
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_uahcx_ghwparams4 {
	uint32_t u32;
	struct cvmx_uahcx_ghwparams4_s {
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
	struct cvmx_uahcx_ghwparams4_s        cn78xx;
	struct cvmx_uahcx_ghwparams4_s        cn78xxp1;
};
typedef union cvmx_uahcx_ghwparams4 cvmx_uahcx_ghwparams4_t;

/**
 * cvmx_uahc#_ghwparams5
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_uahcx_ghwparams5 {
	uint32_t u32;
	struct cvmx_uahcx_ghwparams5_s {
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
	struct cvmx_uahcx_ghwparams5_s        cn78xx;
	struct cvmx_uahcx_ghwparams5_s        cn78xxp1;
};
typedef union cvmx_uahcx_ghwparams5 cvmx_uahcx_ghwparams5_t;

/**
 * cvmx_uahc#_ghwparams6
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_uahcx_ghwparams6 {
	uint32_t u32;
	struct cvmx_uahcx_ghwparams6_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ram0_depth                   : 16; /**< RAM0 depth. */
	uint32_t en_bus_filters               : 1;  /**< Enable VBus filters support. */
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
	struct cvmx_uahcx_ghwparams6_s        cn78xx;
	struct cvmx_uahcx_ghwparams6_s        cn78xxp1;
};
typedef union cvmx_uahcx_ghwparams6 cvmx_uahcx_ghwparams6_t;

/**
 * cvmx_uahc#_ghwparams7
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_uahcx_ghwparams7 {
	uint32_t u32;
	struct cvmx_uahcx_ghwparams7_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ram2_depth                   : 16; /**< RAM2 depth. */
	uint32_t ram1_depth                   : 16; /**< RAM1 depth. */
#else
	uint32_t ram1_depth                   : 16;
	uint32_t ram2_depth                   : 16;
#endif
	} s;
	struct cvmx_uahcx_ghwparams7_s        cn78xx;
	struct cvmx_uahcx_ghwparams7_s        cn78xxp1;
};
typedef union cvmx_uahcx_ghwparams7 cvmx_uahcx_ghwparams7_t;

/**
 * cvmx_uahc#_ghwparams8
 *
 * This register contains the hardware configuration options selected at compile-time.
 *
 */
union cvmx_uahcx_ghwparams8 {
	uint32_t u32;
	struct cvmx_uahcx_ghwparams8_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dcache_depth_info            : 32; /**< Dcache depth. */
#else
	uint32_t dcache_depth_info            : 32;
#endif
	} s;
	struct cvmx_uahcx_ghwparams8_s        cn78xx;
	struct cvmx_uahcx_ghwparams8_s        cn78xxp1;
};
typedef union cvmx_uahcx_ghwparams8 cvmx_uahcx_ghwparams8_t;

/**
 * cvmx_uahc#_gpmsts
 *
 * This debug register gives information on which event caused the hibernation exit. These
 * registers are for debug purposes. They provide debug information on the internal status and
 * state machines. Global debug registers have design-specific information, and are used by for
 * debugging purposes. These registers are not intended to be used by the customer. If any debug
 * assistance is needed for the silicon, contact customer support with a dump of these registers.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gpmsts {
	uint32_t u32;
	struct cvmx_uahcx_gpmsts_s {
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
	struct cvmx_uahcx_gpmsts_s            cn78xx;
	struct cvmx_uahcx_gpmsts_s            cn78xxp1;
};
typedef union cvmx_uahcx_gpmsts cvmx_uahcx_gpmsts_t;

/**
 * cvmx_uahc#_gprtbimap
 *
 * This register specifies the SuperSpeed USB instance number to which each USB 3.0 port is
 * connected. By default, USB 3.0 ports are evenly distributed among all SuperSpeed USB
 * instances. Software can program this register to specify how USB 3.0 ports are connected to
 * SuperSpeed USB instances. The UAHC only implements one SuperSpeed bus-instance, so this
 * register should always be 0.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gprtbimap {
	uint64_t u64;
	struct cvmx_uahcx_gprtbimap_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t binum1                       : 4;  /**< SuperSpeed USB instance number for port 1. */
#else
	uint64_t binum1                       : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_uahcx_gprtbimap_s         cn78xx;
	struct cvmx_uahcx_gprtbimap_s         cn78xxp1;
};
typedef union cvmx_uahcx_gprtbimap cvmx_uahcx_gprtbimap_t;

/**
 * cvmx_uahc#_gprtbimap_fs
 *
 * This register specifies the full-speed/low-speed USB instance number to which each USB 1.1
 * port is connected. By default, USB 1.1 ports are evenly distributed among all full-speed/
 * low-speed USB instances. Software can program this register to specify how USB 1.1 ports are
 * connected to full-speed/low-speed USB instances. The UAHC only implements one full-speed/
 * low-speed bus-instance, so this register should always be 0x0.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gprtbimap_fs {
	uint64_t u64;
	struct cvmx_uahcx_gprtbimap_fs_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t binum1                       : 4;  /**< Full-speed USB instance number for port 1. */
#else
	uint64_t binum1                       : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_uahcx_gprtbimap_fs_s      cn78xx;
	struct cvmx_uahcx_gprtbimap_fs_s      cn78xxp1;
};
typedef union cvmx_uahcx_gprtbimap_fs cvmx_uahcx_gprtbimap_fs_t;

/**
 * cvmx_uahc#_gprtbimap_hs
 *
 * This register specifies the high-speed USB instance number to which each USB 2.0 port is
 * connected. By default, USB 2.0 ports are evenly distributed among all high-speed USB
 * instances. Software can program this register to specify how USB 2.0 ports are connected to
 * high-speed USB instances. The UAHC only implements one high-speed bus-instance, so this
 * register should always be 0.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gprtbimap_hs {
	uint64_t u64;
	struct cvmx_uahcx_gprtbimap_hs_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t binum1                       : 4;  /**< High-speed USB instance number for port 1. */
#else
	uint64_t binum1                       : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_uahcx_gprtbimap_hs_s      cn78xx;
	struct cvmx_uahcx_gprtbimap_hs_s      cn78xxp1;
};
typedef union cvmx_uahcx_gprtbimap_hs cvmx_uahcx_gprtbimap_hs_t;

/**
 * cvmx_uahc#_grlsid
 *
 * This is a read-only register that contains the release number of the core.
 *
 */
union cvmx_uahcx_grlsid {
	uint32_t u32;
	struct cvmx_uahcx_grlsid_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t releaseid                    : 32; /**< Software can use this register to configure release-specific features in the driver. */
#else
	uint32_t releaseid                    : 32;
#endif
	} s;
	struct cvmx_uahcx_grlsid_s            cn78xx;
	struct cvmx_uahcx_grlsid_s            cn78xxp1;
};
typedef union cvmx_uahcx_grlsid cvmx_uahcx_grlsid_t;

/**
 * cvmx_uahc#_grxfifoprihst
 *
 * This register specifies the relative DMA priority level among the host RXFIFOs (one per USB
 * bus instance) within the associated speed group (SuperSpeed or high-speed/full-speed
 * /low-speed). When multiple RXFIFOs compete for DMA service at a given time, the RXDMA arbiter
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
 * UAHC()_GDMAHLRATIO.
 *
 * For scatter-gather packets, the arbiter grants successive DMA requests to the same FIFO until
 * the entire packet is completed. The register size corresponds to the number of configured USB
 * bus instances; for example, in the default configuration, there are 3 USB bus instances (1
 * SuperSpeed, 1 high-speed, and 1 full-speed/low-speed).
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_grxfifoprihst {
	uint32_t u32;
	struct cvmx_uahcx_grxfifoprihst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_3_31                : 29;
	uint32_t rx_priority                  : 3;  /**< Each register bit[n] controls the priority (1 = high, 0 = low) of RXFIFO[n] within a speed group. */
#else
	uint32_t rx_priority                  : 3;
	uint32_t reserved_3_31                : 29;
#endif
	} s;
	struct cvmx_uahcx_grxfifoprihst_s     cn78xx;
	struct cvmx_uahcx_grxfifoprihst_s     cn78xxp1;
};
typedef union cvmx_uahcx_grxfifoprihst cvmx_uahcx_grxfifoprihst_t;

/**
 * cvmx_uahc#_grxfifosiz#
 *
 * The application can program the internal RAM start address/depth of the each RxFIFO as shown
 * below. It is recommended that software use the default value. In Host mode, per-port registers
 * are implemented. One register per FIFO.
 *
 * Reset values = 0:[0x0000_0084] 1:[0x0084_0104] 2:[0x0188_0180].
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_grxfifosizx {
	uint32_t u32;
	struct cvmx_uahcx_grxfifosizx_s {
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
	struct cvmx_uahcx_grxfifosizx_s       cn78xx;
	struct cvmx_uahcx_grxfifosizx_s       cn78xxp1;
};
typedef union cvmx_uahcx_grxfifosizx cvmx_uahcx_grxfifosizx_t;

/**
 * cvmx_uahc#_grxthrcfg
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
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_grxthrcfg {
	uint32_t u32;
	struct cvmx_uahcx_grxthrcfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t usbrxpktcntsel               : 1;  /**< USB receive-packet-count enable. Enables/disables the USB reception multipacket
                                                         thresholding:
                                                         0 = the core can only start reception on the USB when the RX FIFO has space for at least
                                                         one packet.
                                                         1 = the core can only start reception on the USB when the RX FIFO has space for at least
                                                         USBRXPKTCNT amount of packets.
                                                         This mode is only used for SuperSpeed. */
	uint32_t reserved_28_28               : 1;
	uint32_t usbrxpktcnt                  : 4;  /**< USB receive-packet count. In host-mode, specifies space (in number of packets) that must
                                                         be available in the RX FIFO before the core can start the corresponding USB RX transaction
                                                         (burst).
                                                         This field is only valid when [USBRXPKTCNTSEL] = 1. The valid values are from 0x1 to 0xF.
                                                         This field must be <= [USBMAXRXBURSTSIZE]. */
	uint32_t usbmaxrxburstsize            : 5;  /**< USB maximum receive-burst size. In host-mode, specifies the maximum bulk IN burst the core
                                                         should do. When the system bus is slower than the USB, RX FIFO can overrun during a long
                                                         burst.
                                                         Program a smaller value to this field to limit the RX burst size that the core can do. It
                                                         only applies to SuperSpeed Bulk, Isochronous, and Interrupt IN endpoints in the host mode.
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
	struct cvmx_uahcx_grxthrcfg_s         cn78xx;
	struct cvmx_uahcx_grxthrcfg_s         cn78xxp1;
};
typedef union cvmx_uahcx_grxthrcfg cvmx_uahcx_grxthrcfg_t;

/**
 * cvmx_uahc#_gsbuscfg0
 *
 * This register can be used to configure the core after power-on or a change in mode of
 * operation. This register mainly contains AXI system-related configuration parameters. Do not
 * change this register after the initial programming. The application must program this register
 * before starting any transactions on AXI. When [INCRBRSTENA] is enabled, it has the highest
 * priority over other burst lengths. The core always performs the largest burst when enabled.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gsbuscfg0 {
	uint32_t u32;
	struct cvmx_uahcx_gsbuscfg0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t datrdreqinfo                 : 4;  /**< AXI-cache for data-read operations. Always set to 0x0. */
	uint32_t desrdreqinfo                 : 4;  /**< AXI-cache for descriptor-read operations. Always set to 0x0. */
	uint32_t datwrreqinfo                 : 4;  /**< AXI-cache for data-write operations. Always set to 0x0. */
	uint32_t deswrreqinfo                 : 4;  /**< AXI-cache for descriptor-write operations. Always set to 0x0. */
	uint32_t reserved_12_15               : 4;
	uint32_t datbigend                    : 1;  /**< Data access is big-endian. Keep this set to 0 (little-endian) and use the
                                                         UCTL()_SHIM_CFG[DMA_ENDIAN_MODE] setting instead. */
	uint32_t descbigend                   : 1;  /**< Descriptor access is big-endian. Keep this set to 0 (little-endian) and use the
                                                         UCTL()_SHIM_CFG[DMA_ENDIAN_MODE] setting instead. */
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
	struct cvmx_uahcx_gsbuscfg0_s         cn78xx;
	struct cvmx_uahcx_gsbuscfg0_s         cn78xxp1;
};
typedef union cvmx_uahcx_gsbuscfg0 cvmx_uahcx_gsbuscfg0_t;

/**
 * cvmx_uahc#_gsbuscfg1
 *
 * This register can be used to configure the core after power-on or a change in mode of
 * operation. This register mainly contains AXI system-related configuration parameters. Do not
 * change this register after the initial programming. The application must program this register
 * before starting any transactions on AXI.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gsbuscfg1 {
	uint32_t u32;
	struct cvmx_uahcx_gsbuscfg1_s {
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
	struct cvmx_uahcx_gsbuscfg1_s         cn78xx;
	struct cvmx_uahcx_gsbuscfg1_s         cn78xxp1;
};
typedef union cvmx_uahcx_gsbuscfg1 cvmx_uahcx_gsbuscfg1_t;

/**
 * cvmx_uahc#_gsts
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 *
 */
union cvmx_uahcx_gsts {
	uint32_t u32;
	struct cvmx_uahcx_gsts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t cbelt                        : 12; /**< Current BELT value. In host mode, indicates the minimum value of all received device BELT
                                                         values and the BELT value that is set by the set latency tolerance value command. */
	uint32_t reserved_8_19                : 12;
	uint32_t host_ip                      : 1;  /**< Host interrupt pending. Indicates that there is a pending interrupt pertaining to xHC in
                                                         the host-event queue. */
	uint32_t reserved_6_6                 : 1;
	uint32_t csrtimeout                   : 1;  /**< CSR timeout. When set to 1, indicates that software performed a write or read operation to
                                                         a core register that could not be completed within 0xFFFF controller-clock cycles. */
	uint32_t buserraddrvld                : 1;  /**< Bus-error address valid. Indicates that UAHC()_GBUSERRADDR is valid and reports the
                                                         first bus address that encounters a bus error. */
	uint32_t reserved_2_3                 : 2;
	uint32_t curmod                       : 2;  /**< Current mode of operation. Always 0x1. */
#else
	uint32_t curmod                       : 2;
	uint32_t reserved_2_3                 : 2;
	uint32_t buserraddrvld                : 1;
	uint32_t csrtimeout                   : 1;
	uint32_t reserved_6_6                 : 1;
	uint32_t host_ip                      : 1;
	uint32_t reserved_8_19                : 12;
	uint32_t cbelt                        : 12;
#endif
	} s;
	struct cvmx_uahcx_gsts_s              cn78xx;
	struct cvmx_uahcx_gsts_s              cn78xxp1;
};
typedef union cvmx_uahcx_gsts cvmx_uahcx_gsts_t;

/**
 * cvmx_uahc#_gtxfifoprihst
 *
 * This register specifies the relative DMA priority level among the host TXFIFOs (one per USB
 * bus instance) within the associated speed group (SuperSpeed or high-speed/full-speed
 * /low-speed). When multiple TXFIFOs compete for DMA service at a given time, the TXDMA arbiter
 * grants access on a packet-basis in the following manner:
 *
 * Among the FIFOs in the same speed group (SuperSpeed or high-speed/full-speed/low-speed):
 *
 * * High-priority TXFIFOs are granted access using round-robin arbitration.
 * * Low-priority TXFIFOs are granted access using round-robin arbitration only after high-
 * priority
 * TXFIFOs have no further processing to do (i.e., either the TXQs are empty or the corresponding
 * TXFIFOs do not have the required data).
 *
 * The TX DMA arbiter prioritizes the SuperSpeed group or high-speed/full-speed/low-speed group
 * according to the ratio programmed in
 * UAHC()_GDMAHLRATIO.
 *
 * For scatter-gather packets, the arbiter grants successive DMA requests to the same FIFO until
 * the entire packet is completed. The register size corresponds to the number of configured USB
 * bus instances; for example, in the default configuration, there are 3 USB bus instances (1
 * SuperSpeed, 1 high-speed, and 1 full-speed/low-speed).
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gtxfifoprihst {
	uint32_t u32;
	struct cvmx_uahcx_gtxfifoprihst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_3_31                : 29;
	uint32_t tx_priority                  : 3;  /**< Each register bit n controls the priority (1: high, 0: low) of TX FIFO<n> within a speed
                                                         group. */
#else
	uint32_t tx_priority                  : 3;
	uint32_t reserved_3_31                : 29;
#endif
	} s;
	struct cvmx_uahcx_gtxfifoprihst_s     cn78xx;
	struct cvmx_uahcx_gtxfifoprihst_s     cn78xxp1;
};
typedef union cvmx_uahcx_gtxfifoprihst cvmx_uahcx_gtxfifoprihst_t;

/**
 * cvmx_uahc#_gtxfifosiz#
 *
 * This register holds the internal RAM start address/depth of each TxFIFO implemented. Unless
 * packet size/buffer size for each endpoint is different and application-specific, it is
 * recommended that the software use the default value. One register per FIFO. One register per
 * FIFO.
 *
 * Reset values = 0:[0x0000_0082] 1:[0x0082_0103] 2:[0x0185_0205].
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gtxfifosizx {
	uint32_t u32;
	struct cvmx_uahcx_gtxfifosizx_s {
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
	struct cvmx_uahcx_gtxfifosizx_s       cn78xx;
	struct cvmx_uahcx_gtxfifosizx_s       cn78xxp1;
};
typedef union cvmx_uahcx_gtxfifosizx cvmx_uahcx_gtxfifosizx_t;

/**
 * cvmx_uahc#_gtxthrcfg
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
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gtxthrcfg {
	uint32_t u32;
	struct cvmx_uahcx_gtxthrcfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t usbtxpktcntsel               : 1;  /**< USB transmit packet-count enable. Enables/disables the USB transmission multipacket
                                                         thresholding:
                                                         0 = USB transmission multipacket thresholding is disabled, the core can only start
                                                         transmission on the USB after the entire packet has been fetched into the corresponding
                                                         TXFIFO.
                                                         1 = USB transmission multipacket thresholding is enabled. The core can only start
                                                         transmission on the USB after USBTXPKTCNT amount of packets for the USB transaction
                                                         (burst) are already in the corresponding TXFIFO.
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
	struct cvmx_uahcx_gtxthrcfg_s         cn78xx;
	struct cvmx_uahcx_gtxthrcfg_s         cn78xxp1;
};
typedef union cvmx_uahcx_gtxthrcfg cvmx_uahcx_gtxthrcfg_t;

/**
 * cvmx_uahc#_guctl
 *
 * This register provides a few options for the software to control the core behavior in the host
 * mode. Most of the options are used to improve host inter-operability with different devices.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_guctl {
	uint32_t u32;
	struct cvmx_uahcx_guctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t refclkper                    : 10; /**< Reference-clock period. Indicates (in terms of ns) the period of REF_CLK. The default
                                                         value is set to 0x8
                                                         (8 ns/125 MHz). This field must be updated during power on initialization if
                                                         UAHC()_GCTL[SOFITPSYNC] = 1 or UAHC()_GFLADJ [GFLADJ_REFCLK_LPM_SEL] = 1. The
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
                                                         Eliisys trace during hardware debug. */
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
	uint32_t extcapsupten                 : 1;  /**< External extended capability support enable. If disabled, a read UAHC()_SUPTPRT3_DW0
                                                         [NEXTCAPPTR] returns 0 in the next capability pointer field. This indicates there are no
                                                         more capabilities. If enabled, a read to UAHC()_SUPTPRT3_DW0[NEXTCAPPTR] returns 4 in the
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
	uint32_t usbhstinautoretryen          : 1;
	uint32_t cmdevaddr                    : 1;
	uint32_t resbwhseps                   : 1;
	uint32_t sprsctrltransen              : 1;
	uint32_t psqextrressp                 : 3;
	uint32_t noextrdl                     : 1;
	uint32_t refclkper                    : 10;
#endif
	} s;
	struct cvmx_uahcx_guctl_s             cn78xx;
	struct cvmx_uahcx_guctl_s             cn78xxp1;
};
typedef union cvmx_uahcx_guctl cvmx_uahcx_guctl_t;

/**
 * cvmx_uahc#_guctl1
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 *
 */
union cvmx_uahcx_guctl1 {
	uint32_t u32;
	struct cvmx_uahcx_guctl1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_2_31                : 30;
	uint32_t ovrld_l1_susp_com            : 1;  /**< Always set to 0. */
	uint32_t loa_filter_en                : 1;  /**< If this bit is set, the USB 2.0 port babble is checked at least three consecutive times
                                                         before the port is disabled. This prevents false triggering of the babble condition when
                                                         using low quality cables. */
#else
	uint32_t loa_filter_en                : 1;
	uint32_t ovrld_l1_susp_com            : 1;
	uint32_t reserved_2_31                : 30;
#endif
	} s;
	struct cvmx_uahcx_guctl1_s            cn78xx;
	struct cvmx_uahcx_guctl1_s            cn78xxp1;
};
typedef union cvmx_uahcx_guctl1 cvmx_uahcx_guctl1_t;

/**
 * cvmx_uahc#_guid
 *
 * This is a read/write register containing the User ID. The power-on value for this register is
 * specified as the user identification register. This register can be used in the following
 * ways:
 * * To store the version or revision of your system.
 * * To store hardware configurations that are outside of the core.
 * * As a scratch register.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_guid {
	uint32_t u32;
	struct cvmx_uahcx_guid_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t userid                       : 32; /**< User ID. Application-programmable ID field. */
#else
	uint32_t userid                       : 32;
#endif
	} s;
	struct cvmx_uahcx_guid_s              cn78xx;
	struct cvmx_uahcx_guid_s              cn78xxp1;
};
typedef union cvmx_uahcx_guid cvmx_uahcx_guid_t;

/**
 * cvmx_uahc#_gusb2i2cctl#
 *
 * This register is reserved for future use.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UAHC_RST].
 */
union cvmx_uahcx_gusb2i2cctlx {
	uint32_t u32;
	struct cvmx_uahcx_gusb2i2cctlx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_uahcx_gusb2i2cctlx_s      cn78xx;
	struct cvmx_uahcx_gusb2i2cctlx_s      cn78xxp1;
};
typedef union cvmx_uahcx_gusb2i2cctlx cvmx_uahcx_gusb2i2cctlx_t;

/**
 * cvmx_uahc#_gusb2phycfg#
 *
 * This register is used to configure the core after power-on. It contains USB 2.0 and USB 2.0
 * PHY-related configuration parameters. The application must program this register before
 * starting any transactions on either the SoC bus or the USB. Per-port registers are
 * implemented.
 *
 * Do not make changes to this register after the initial programming.
 */
union cvmx_uahcx_gusb2phycfgx {
	uint32_t u32;
	struct cvmx_uahcx_gusb2phycfgx_s {
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
                                                         counter, UAHC()_GCTL[SOFITPSYNC] = 1, or UAHC()_GFLADJ [GFLADJ_REFCLK_LPM_SEL] = 1. */
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
	uint32_t reserved_9_9                 : 1;
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
                                                         _ one 30-MHz PHY clock = 16 bit times.
                                                         _ one 60-MHz PHY clock = 8 bit times.
                                                         * Full-speed operation:
                                                         _ one 30-MHz PHY clock = 0.4 bit times.
                                                         _ one 60-MHz PHY clock = 0.2 bit times.
                                                         _ one 48-MHz PHY clock = 0.25 bit times. */
#else
	uint32_t toutcal                      : 3;
	uint32_t phyif                        : 1;
	uint32_t ulpi_utmi_sel                : 1;
	uint32_t fsintf                       : 1;
	uint32_t susphy                       : 1;
	uint32_t physel                       : 1;
	uint32_t enblslpm                     : 1;
	uint32_t reserved_9_9                 : 1;
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
	struct cvmx_uahcx_gusb2phycfgx_s      cn78xx;
	struct cvmx_uahcx_gusb2phycfgx_s      cn78xxp1;
};
typedef union cvmx_uahcx_gusb2phycfgx cvmx_uahcx_gusb2phycfgx_t;

/**
 * cvmx_uahc#_gusb3pipectl#
 *
 * This register is used to configure the core after power-on. It contains USB 3.0 and USB 3.0
 * PHY-related configuration parameters. The application must program this register before
 * starting any transactions on either the SoC bus or the USB. Per-port registers are
 * implemented.
 *
 * Do not make changes to this register after the initial programming.
 */
union cvmx_uahcx_gusb3pipectlx {
	uint32_t u32;
	struct cvmx_uahcx_gusb3pipectlx_s {
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
                                                         * Set UAHC()_USBCMD[HCRST] = 1 or power-on-chip reset.
                                                         * Set UAHC()_PORTSC()[PP] = 0.
                                                         * Set HSTPRTCMPL = 1. This places the link into compliance state.
                                                         To advance the compliance pattern, follow this sequence (toggle HSTPRTCMPL):
                                                         * Set HSTPRTCMPL = 0.
                                                         * Set HSTPRTCMPL = 1. This advances the link to the next compliance pattern.
                                                         To exit from the compliance state, set UAHC()_USBCMD[HCRST] = 1 or power-on-chip reset. */
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
	uint32_t rxdet2polllfpsctrl           : 1;  /**< RX_DETECT to Polling.
                                                         0 = Enables a 400 us delay to start polling LFPS after RX_DETECT. This allows VCM offset
                                                         to settle to a proper level.
                                                         1 = Disables the 400 us delay to start polling LFPS after RX_DETECT. */
	uint32_t reserved_7_7                 : 1;
	uint32_t txswing                      : 1;  /**< TX swing. Refer to the PIPE3 specification. */
	uint32_t txmargin                     : 3;  /**< TX margin. Refer to the PIPE3 specification, table 5-3. */
	uint32_t txdeemphasis                 : 2;  /**< TX deemphasis. The value driven to the PHY is controlled by the LTSSM during USB3
                                                         compliance mode. Refer to the PIPE3 specification, table 5-3.
                                                         Use the following values for the appropriate level of de-emphasis (From pipe3 spec):
                                                         0x0 =   -6 dB de-emphasis, use UCTL()_PORT()_CFG_SS[PCS_TX_DEEMPH_6DB].
                                                         0x1 = -3.5 dB de-emphasis, use UCTL()_PORT()_CFG_SS[PCS_TX_DEEMPH_3P5DB].
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
	struct cvmx_uahcx_gusb3pipectlx_s     cn78xx;
	struct cvmx_uahcx_gusb3pipectlx_s     cn78xxp1;
};
typedef union cvmx_uahcx_gusb3pipectlx cvmx_uahcx_gusb3pipectlx_t;

/**
 * cvmx_uahc#_hccparams
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.3.6.
 *
 */
union cvmx_uahcx_hccparams {
	uint32_t u32;
	struct cvmx_uahcx_hccparams_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t xecp                         : 16; /**< xHCI extended capabilities pointer. */
	uint32_t maxpsasize                   : 4;  /**< Maximum primary-stream-array size. */
	uint32_t reserved_9_11                : 3;
	uint32_t pae                          : 1;  /**< Parse all event data. */
	uint32_t nss                          : 1;  /**< No secondary SID support. */
	uint32_t ltc                          : 1;  /**< Latency tolerance messaging capability. */
	uint32_t lhrc                         : 1;  /**< Light HC reset capability. */
	uint32_t pind                         : 1;  /**< Port indicators. */
	uint32_t ppc                          : 1;  /**< Port power control. Value is based on UCTL()_HOST_CFG[PPC_EN]. */
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
	struct cvmx_uahcx_hccparams_s         cn78xx;
	struct cvmx_uahcx_hccparams_s         cn78xxp1;
};
typedef union cvmx_uahcx_hccparams cvmx_uahcx_hccparams_t;

/**
 * cvmx_uahc#_hcsparams1
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.3.3.
 *
 */
union cvmx_uahcx_hcsparams1 {
	uint32_t u32;
	struct cvmx_uahcx_hcsparams1_s {
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
	struct cvmx_uahcx_hcsparams1_s        cn78xx;
	struct cvmx_uahcx_hcsparams1_s        cn78xxp1;
};
typedef union cvmx_uahcx_hcsparams1 cvmx_uahcx_hcsparams1_t;

/**
 * cvmx_uahc#_hcsparams2
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.3.4.
 *
 */
union cvmx_uahcx_hcsparams2 {
	uint32_t u32;
	struct cvmx_uahcx_hcsparams2_s {
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
	struct cvmx_uahcx_hcsparams2_s        cn78xx;
	struct cvmx_uahcx_hcsparams2_s        cn78xxp1;
};
typedef union cvmx_uahcx_hcsparams2 cvmx_uahcx_hcsparams2_t;

/**
 * cvmx_uahc#_hcsparams3
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.3.5.
 *
 */
union cvmx_uahcx_hcsparams3 {
	uint32_t u32;
	struct cvmx_uahcx_hcsparams3_s {
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
	struct cvmx_uahcx_hcsparams3_s        cn78xx;
	struct cvmx_uahcx_hcsparams3_s        cn78xxp1;
};
typedef union cvmx_uahcx_hcsparams3 cvmx_uahcx_hcsparams3_t;

/**
 * cvmx_uahc#_iman#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.5.2.1.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_imanx {
	uint32_t u32;
	struct cvmx_uahcx_imanx_s {
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
	struct cvmx_uahcx_imanx_s             cn78xx;
	struct cvmx_uahcx_imanx_s             cn78xxp1;
};
typedef union cvmx_uahcx_imanx cvmx_uahcx_imanx_t;

/**
 * cvmx_uahc#_imod#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.5.2.2.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_imodx {
	uint32_t u32;
	struct cvmx_uahcx_imodx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t imodc                        : 16; /**< Interrupt moderation counter. */
	uint32_t imodi                        : 16; /**< Interrupt moderation interval. */
#else
	uint32_t imodi                        : 16;
	uint32_t imodc                        : 16;
#endif
	} s;
	struct cvmx_uahcx_imodx_s             cn78xx;
	struct cvmx_uahcx_imodx_s             cn78xxp1;
};
typedef union cvmx_uahcx_imodx cvmx_uahcx_imodx_t;

/**
 * cvmx_uahc#_mfindex
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.5.1.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_mfindex {
	uint32_t u32;
	struct cvmx_uahcx_mfindex_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_14_31               : 18;
	uint32_t mfindex                      : 14; /**< Microframe index. */
#else
	uint32_t mfindex                      : 14;
	uint32_t reserved_14_31               : 18;
#endif
	} s;
	struct cvmx_uahcx_mfindex_s           cn78xx;
	struct cvmx_uahcx_mfindex_s           cn78xxp1;
};
typedef union cvmx_uahcx_mfindex cvmx_uahcx_mfindex_t;

/**
 * cvmx_uahc#_ohci0_hcbulkcurrented
 *
 * HCBULKCURRENTED = Host Controller Bulk Current ED Register
 *
 * The HcBulkCurrentED register contains the physical address of the current endpoint of the Bulk list. As the Bulk list will be served in a round-robin
 * fashion, the endpoints will be ordered according to their insertion to the list.
 */
union cvmx_uahcx_ohci0_hcbulkcurrented {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcbulkcurrented_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t bced                         : 28; /**< BulkCurrentED. This is advanced to the next ED after the HC has served the
                                                         present one. HC continues processing the list from where it left off in the
                                                         last Frame. When it reaches the end of the Bulk list, HC checks the
                                                         ControlListFilled of HcControl. If set, it copies the content of HcBulkHeadED
                                                         to HcBulkCurrentED and clears the bit. If it is not set, it does nothing.
                                                         HCD is only allowed to modify this register when the BulkListEnable of
                                                         HcControl is cleared. When set, the HCD only reads the instantaneous value of
                                                         this register. This is initially set to zero to indicate the end of the Bulk
                                                         list. */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t bced                         : 28;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcbulkcurrented_s cn61xx;
	struct cvmx_uahcx_ohci0_hcbulkcurrented_s cn63xx;
	struct cvmx_uahcx_ohci0_hcbulkcurrented_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hcbulkcurrented_s cn66xx;
	struct cvmx_uahcx_ohci0_hcbulkcurrented_s cn68xx;
	struct cvmx_uahcx_ohci0_hcbulkcurrented_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hcbulkcurrented_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcbulkcurrented cvmx_uahcx_ohci0_hcbulkcurrented_t;

/**
 * cvmx_uahc#_ohci0_hcbulkheaded
 *
 * HCBULKHEADED = Host Controller Bulk Head ED Register
 *
 * The HcBulkHeadED register contains the physical address of the first Endpoint Descriptor of the Bulk list.
 */
union cvmx_uahcx_ohci0_hcbulkheaded {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcbulkheaded_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t bhed                         : 28; /**< BulkHeadED. HC traverses the Bulk list starting with the HcBulkHeadED
                                                         pointer. The content is loaded from HCCA during the initialization of HC. */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t bhed                         : 28;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcbulkheaded_s cn61xx;
	struct cvmx_uahcx_ohci0_hcbulkheaded_s cn63xx;
	struct cvmx_uahcx_ohci0_hcbulkheaded_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hcbulkheaded_s cn66xx;
	struct cvmx_uahcx_ohci0_hcbulkheaded_s cn68xx;
	struct cvmx_uahcx_ohci0_hcbulkheaded_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hcbulkheaded_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcbulkheaded cvmx_uahcx_ohci0_hcbulkheaded_t;

/**
 * cvmx_uahc#_ohci0_hccommandstatus
 *
 * HCCOMMANDSTATUS = Host Controller Command Status Register
 *
 * The HcCommandStatus register is used by the Host Controller to receive commands issued by the Host Controller Driver, as well as reflecting the
 * current status of the Host Controller. To the Host Controller Driver, it appears to be a "write to set" register. The Host Controller must ensure
 * that bits written as '1' become set in the register while bits written as '0' remain unchanged in the register. The Host Controller Driver
 * may issue multiple distinct commands to the Host Controller without concern for corrupting previously issued commands. The Host Controller Driver
 * has normal read access to all bits.
 * The SchedulingOverrunCount field indicates the number of frames with which the Host Controller has detected the scheduling overrun error. This
 * occurs when the Periodic list does not complete before EOF. When a scheduling overrun error is detected, the Host Controller increments the counter
 * and sets the SchedulingOverrun field in the HcInterruptStatus register.
 */
union cvmx_uahcx_ohci0_hccommandstatus {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hccommandstatus_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_18_31               : 14;
	uint32_t soc                          : 2;  /**< SchedulingOverrunCount. These bits are incremented on each scheduling overrun
                                                         error. It is initialized to 00b and wraps around at 11b. This will be
                                                         incremented when a scheduling overrun is detected even if SchedulingOverrun
                                                         in HcInterruptStatus has already been set. This is used by HCD to monitor
                                                         any persistent scheduling problems. */
	uint32_t reserved_4_15                : 12;
	uint32_t ocr                          : 1;  /**< OwnershipChangeRequest. This bit is set by an OS HCD to request a change of
                                                         control of the HC. When set HC will set the OwnershipChange field in
                                                         HcInterruptStatus. After the changeover, this bit is cleared and remains so
                                                         until the next request from OS HCD. */
	uint32_t blf                          : 1;  /**< BulkListFilled This bit is used to indicate whether there are any TDs on the
                                                         Bulk list. It is set by HCD whenever it adds a TD to an ED in the Bulk list.
                                                         When HC begins to process the head of the Bulk list, it checks BF. As long
                                                         as BulkListFilled is 0, HC will not start processing the Bulk list. If
                                                         BulkListFilled is 1, HC will start processing the Bulk list and will set BF
                                                         to 0. If HC finds a TD on the list, then HC will set BulkListFilled to 1
                                                         causing the Bulk list processing to continue. If no TD is found on the Bulk
                                                         list,and if HCD does not set BulkListFilled, then BulkListFilled will still
                                                         be 0 when HC completes processing the Bulk list and Bulk list processing will
                                                         stop. */
	uint32_t clf                          : 1;  /**< ControlListFilled. This bit is used to indicate whether there are any TDs
                                                         on the Control list. It is set by HCD whenever it adds a TD to an ED in the
                                                         Control list. When HC begins to process the head of the Control list, it
                                                         checks CLF. As long as ControlListFilled is 0, HC will not start processing
                                                         the Control list. If CF is 1, HC will start processing the Control list and
                                                         will set ControlListFilled to 0. If HC finds a TD on the list, then HC will
                                                         set ControlListFilled to 1 causing the Control list processing to continue.
                                                         If no TD is found on the Control list, and if the HCD does not set
                                                         ControlListFilled, then ControlListFilled will still be 0 when HC completes
                                                         processing the Control list and Control list processing will stop. */
	uint32_t hcr                          : 1;  /**< HostControllerReset. This bit is set by HCD to initiate a software reset of
                                                         HC. Regardless of the functional state of HC, it moves to the USBSUSPEND
                                                         state in which most of the operational registers are reset except those
                                                         stated otherwise; e.g., the InterruptRouting field of HcControl, and no
                                                         Host bus accesses are allowed. This bit is cleared by HC upon the
                                                         completion of the reset operation. The reset operation must be completed
                                                         within 10 ms. This bit, when set, should not cause a reset to the Root Hub
                                                         and no subsequent reset signaling should be asserted to its downstream ports. */
#else
	uint32_t hcr                          : 1;
	uint32_t clf                          : 1;
	uint32_t blf                          : 1;
	uint32_t ocr                          : 1;
	uint32_t reserved_4_15                : 12;
	uint32_t soc                          : 2;
	uint32_t reserved_18_31               : 14;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hccommandstatus_s cn61xx;
	struct cvmx_uahcx_ohci0_hccommandstatus_s cn63xx;
	struct cvmx_uahcx_ohci0_hccommandstatus_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hccommandstatus_s cn66xx;
	struct cvmx_uahcx_ohci0_hccommandstatus_s cn68xx;
	struct cvmx_uahcx_ohci0_hccommandstatus_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hccommandstatus_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hccommandstatus cvmx_uahcx_ohci0_hccommandstatus_t;

/**
 * cvmx_uahc#_ohci0_hccontrol
 *
 * HCCONTROL = Host Controller Control Register
 *
 * The HcControl register defines the operating modes for the Host Controller. Most of the fields in this register are modified only by the Host Controller
 * Driver, except HostControllerFunctionalState and RemoteWakeupConnected.
 */
union cvmx_uahcx_ohci0_hccontrol {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hccontrol_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_11_31               : 21;
	uint32_t rwe                          : 1;  /**< RemoteWakeupEnable. This bit is used by HCD to enable or disable the remote wakeup
                                                         feature upon the detection of upstream resume signaling. When this bit is set and
                                                         the ResumeDetected bit in HcInterruptStatus is set, a remote wakeup is signaled
                                                         to the host system. Setting this bit has no impact on the generation of hardware
                                                         interrupt. */
	uint32_t rwc                          : 1;  /**< RemoteWakeupConnected.This bit indicates whether HC supports remote wakeup signaling.
                                                         If remote wakeup is supported and used by the system it is the responsibility of
                                                         system firmware to set this bit during POST. HC clears the bit upon a hardware reset
                                                         but does not alter it upon a software reset. Remote wakeup signaling of the host
                                                         system is host-bus-specific and is not described in this specification. */
	uint32_t ir                           : 1;  /**< InterruptRouting
                                                         This bit determines the routing of interrupts generated by events registered in
                                                         HcInterruptStatus. If clear, all interrupts are routed to the normal host bus
                                                         interrupt mechanism. If set, interrupts are routed to the System Management
                                                         Interrupt. HCD clears this bit upon a hardware reset, but it does not alter
                                                         this bit upon a software reset. HCD uses this bit as a tag to indicate the
                                                         ownership of HC. */
	uint32_t hcfs                         : 2;  /**< HostControllerFunctionalState for USB
                                                          00b: USBRESET
                                                          01b: USBRESUME
                                                          10b: USBOPERATIONAL
                                                          11b: USBSUSPEND
                                                         A transition to USBOPERATIONAL from another state causes SOF generation to begin
                                                         1 ms later. HCD may determine whether HC has begun sending SOFs by reading the
                                                         StartofFrame field of HcInterruptStatus.
                                                         This field may be changed by HC only when in the USBSUSPEND state. HC may move from
                                                         the USBSUSPEND state to the USBRESUME state after detecting the resume signaling
                                                         from a downstream port.
                                                         HC enters USBSUSPEND after a software reset, whereas it enters USBRESET after a
                                                         hardware reset. The latter also resets the Root Hub and asserts subsequent reset
                                                         signaling to downstream ports. */
	uint32_t ble                          : 1;  /**< BulkListEnable. This bit is set to enable the processing of the Bulk list in the
                                                         next Frame. If cleared by HCD, processing of the Bulk list does not occur after
                                                         the next SOF. HC checks this bit whenever it determines to process the list. When
                                                         disabled, HCD may modify the list. If HcBulkCurrentED is pointing to an ED to be
                                                         removed, HCD must advance the pointer by updating HcBulkCurrentED before re-enabling
                                                         processing of the list. */
	uint32_t cle                          : 1;  /**< ControlListEnable. This bit is set to enable the processing of the Control list in
                                                         the next Frame. If cleared by HCD, processing of the Control list does not occur
                                                         after the next SOF. HC must check this bit whenever it determines to process the
                                                         list. When disabled, HCD may modify the list. If HcControlCurrentED is pointing to
                                                         an ED to be removed, HCD must advance the pointer by updating HcControlCurrentED
                                                         before re-enabling processing of the list. */
	uint32_t ie                           : 1;  /**< IsochronousEnable This bit is used by HCD to enable/disable processing of
                                                         isochronous EDs. While processing the periodic list in a Frame, HC checks the
                                                         status of this bit when it finds an Isochronous ED (F=1). If set (enabled), HC
                                                         continues processing the EDs. If cleared (disabled), HC halts processing of the
                                                         periodic list (which now contains only isochronous EDs) and begins processing the
                                                         Bulk/Control lists. Setting this bit is guaranteed to take effect in the next
                                                         Frame (not the current Frame). */
	uint32_t ple                          : 1;  /**< PeriodicListEnable. This bit is set to enable the processing of the periodic list
                                                         in the next Frame. If cleared by HCD, processing of the periodic list does not
                                                         occur after the next SOF. HC must check this bit before it starts processing
                                                         the list. */
	uint32_t cbsr                         : 2;  /**< ControlBulkServiceRatio. This specifies the service ratio between Control and
                                                         Bulk EDs. Before processing any of the nonperiodic lists, HC must compare the
                                                         ratio specified with its internal count on how many nonempty Control EDs have
                                                         been processed, in determining whether to continue serving another Control ED
                                                         or switching to Bulk EDs. The internal count will be retained when crossing
                                                         the frame boundary. In case of reset, HCD is responsible for restoring this
                                                         value.

                                                           CBSR   No. of Control EDs Over Bulk EDs Served
                                                            0             1:1
                                                            1             2:1
                                                            2             3:1
                                                            3             4:1 */
#else
	uint32_t cbsr                         : 2;
	uint32_t ple                          : 1;
	uint32_t ie                           : 1;
	uint32_t cle                          : 1;
	uint32_t ble                          : 1;
	uint32_t hcfs                         : 2;
	uint32_t ir                           : 1;
	uint32_t rwc                          : 1;
	uint32_t rwe                          : 1;
	uint32_t reserved_11_31               : 21;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hccontrol_s   cn61xx;
	struct cvmx_uahcx_ohci0_hccontrol_s   cn63xx;
	struct cvmx_uahcx_ohci0_hccontrol_s   cn63xxp1;
	struct cvmx_uahcx_ohci0_hccontrol_s   cn66xx;
	struct cvmx_uahcx_ohci0_hccontrol_s   cn68xx;
	struct cvmx_uahcx_ohci0_hccontrol_s   cn68xxp1;
	struct cvmx_uahcx_ohci0_hccontrol_s   cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hccontrol cvmx_uahcx_ohci0_hccontrol_t;

/**
 * cvmx_uahc#_ohci0_hccontrolcurrented
 *
 * HCCONTROLCURRENTED = Host Controller Control Current ED Register
 *
 * The HcControlCurrentED register contains the physical address of the current Endpoint Descriptor of the Control list.
 */
union cvmx_uahcx_ohci0_hccontrolcurrented {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hccontrolcurrented_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t cced                         : 28; /**< ControlCurrentED. This pointer is advanced to the next ED after serving the
                                                         present one. HC will continue processing the list from where it left off in
                                                         the last Frame. When it reaches the end of the Control list, HC checks the
                                                         ControlListFilled of in HcCommandStatus. If set, it copies the content of
                                                         HcControlHeadED to HcControlCurrentED and clears the bit. If not set, it
                                                         does nothing. HCD is allowed to modify this register only when the
                                                         ControlListEnable of HcControl is cleared. When set, HCD only reads the
                                                         instantaneous value of this register. Initially, this is set to zero to
                                                         indicate the end of the Control list. */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t cced                         : 28;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hccontrolcurrented_s cn61xx;
	struct cvmx_uahcx_ohci0_hccontrolcurrented_s cn63xx;
	struct cvmx_uahcx_ohci0_hccontrolcurrented_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hccontrolcurrented_s cn66xx;
	struct cvmx_uahcx_ohci0_hccontrolcurrented_s cn68xx;
	struct cvmx_uahcx_ohci0_hccontrolcurrented_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hccontrolcurrented_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hccontrolcurrented cvmx_uahcx_ohci0_hccontrolcurrented_t;

/**
 * cvmx_uahc#_ohci0_hccontrolheaded
 *
 * HCCONTROLHEADED = Host Controller Control Head ED Register
 *
 * The HcControlHeadED register contains the physical address of the first Endpoint Descriptor of the Control list.
 */
union cvmx_uahcx_ohci0_hccontrolheaded {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hccontrolheaded_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ched                         : 28; /**< ControlHeadED. HC traverses the Control list starting with the HcControlHeadED
                                                         pointer. The content is loaded from HCCA during the initialization of HC. */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t ched                         : 28;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hccontrolheaded_s cn61xx;
	struct cvmx_uahcx_ohci0_hccontrolheaded_s cn63xx;
	struct cvmx_uahcx_ohci0_hccontrolheaded_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hccontrolheaded_s cn66xx;
	struct cvmx_uahcx_ohci0_hccontrolheaded_s cn68xx;
	struct cvmx_uahcx_ohci0_hccontrolheaded_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hccontrolheaded_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hccontrolheaded cvmx_uahcx_ohci0_hccontrolheaded_t;

/**
 * cvmx_uahc#_ohci0_hcdonehead
 *
 * HCDONEHEAD = Host Controller Done Head Register
 *
 * The HcDoneHead register contains the physical address of the last completed Transfer Descriptor that was added to the Done queue. In normal operation,
 * the Host Controller Driver should not need to read this register as its content is periodically written to the HCCA.
 */
union cvmx_uahcx_ohci0_hcdonehead {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcdonehead_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dh                           : 28; /**< DoneHead. When a TD is completed, HC writes the content of HcDoneHead to the
                                                         NextTD field of the TD. HC then overwrites the content of HcDoneHead with the
                                                         address of this TD. This is set to zero whenever HC writes the content of
                                                         this register to HCCA. It also sets the WritebackDoneHead of HcInterruptStatus. */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dh                           : 28;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcdonehead_s  cn61xx;
	struct cvmx_uahcx_ohci0_hcdonehead_s  cn63xx;
	struct cvmx_uahcx_ohci0_hcdonehead_s  cn63xxp1;
	struct cvmx_uahcx_ohci0_hcdonehead_s  cn66xx;
	struct cvmx_uahcx_ohci0_hcdonehead_s  cn68xx;
	struct cvmx_uahcx_ohci0_hcdonehead_s  cn68xxp1;
	struct cvmx_uahcx_ohci0_hcdonehead_s  cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcdonehead cvmx_uahcx_ohci0_hcdonehead_t;

/**
 * cvmx_uahc#_ohci0_hcfminterval
 *
 * HCFMINTERVAL = Host Controller Frame Interval Register
 *
 * The HcFmInterval register contains a 14-bit value which indicates the bit time interval in a Frame, (i.e., between two consecutive SOFs), and a 15-bit value
 * indicating the Full Speed maximum packet size that the Host Controller may transmit or receive without causing scheduling overrun. The Host Controller Driver
 * may carry out minor adjustment on the FrameInterval by writing a new value over the present one at each SOF. This provides the programmability necessary for
 * the Host Controller to synchronize with an external clocking resource and to adjust any unknown local clock offset.
 */
union cvmx_uahcx_ohci0_hcfminterval {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcfminterval_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t fit                          : 1;  /**< FrameIntervalToggle. HCD toggles this bit whenever it loads a new value to
                                                         FrameInterval. */
	uint32_t fsmps                        : 15; /**< FSLargestDataPacket. This field specifies a value which is loaded into the
                                                         Largest Data Packet Counter at the beginning of each frame. The counter value
                                                         represents the largest amount of data in bits which can be sent or received by
                                                         the HC in a single transaction at any given time without causing scheduling
                                                         overrun. The field value is calculated by the HCD. */
	uint32_t reserved_14_15               : 2;
	uint32_t fi                           : 14; /**< FrameInterval. This specifies the interval between two consecutive SOFs in bit
                                                         times. The nominal value is set to be 11,999. HCD should store the current
                                                         value of this field before resetting HC. By setting the HostControllerReset
                                                         field of HcCommandStatus as this will cause the HC to reset this field to its
                                                         nominal value. HCD may choose to restore the stored value upon the completion
                                                         of the Reset sequence. */
#else
	uint32_t fi                           : 14;
	uint32_t reserved_14_15               : 2;
	uint32_t fsmps                        : 15;
	uint32_t fit                          : 1;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcfminterval_s cn61xx;
	struct cvmx_uahcx_ohci0_hcfminterval_s cn63xx;
	struct cvmx_uahcx_ohci0_hcfminterval_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hcfminterval_s cn66xx;
	struct cvmx_uahcx_ohci0_hcfminterval_s cn68xx;
	struct cvmx_uahcx_ohci0_hcfminterval_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hcfminterval_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcfminterval cvmx_uahcx_ohci0_hcfminterval_t;

/**
 * cvmx_uahc#_ohci0_hcfmnumber
 *
 * HCFMNUMBER = Host Cotroller Frame Number Register
 *
 * The HcFmNumber register is a 16-bit counter. It provides a timing reference among events happening in the Host Controller and the Host Controller Driver.
 * The Host Controller Driver may use the 16-bit value specified in this register and generate a 32-bit frame number without requiring frequent access to
 * the register.
 */
union cvmx_uahcx_ohci0_hcfmnumber {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcfmnumber_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t fn                           : 16; /**< FrameNumber. This is incremented when HcFmRemaining is re-loaded. It will be
                                                         rolled over to 0h after ffffh. When entering the USBOPERATIONAL state,
                                                         this will be incremented automatically. The content will be written to HCCA
                                                         after HC has incremented the FrameNumber at each frame boundary and sent a
                                                         SOF but before HC reads the first ED in that Frame. After writing to HCCA,
                                                         HC will set the StartofFrame in HcInterruptStatus. */
#else
	uint32_t fn                           : 16;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcfmnumber_s  cn61xx;
	struct cvmx_uahcx_ohci0_hcfmnumber_s  cn63xx;
	struct cvmx_uahcx_ohci0_hcfmnumber_s  cn63xxp1;
	struct cvmx_uahcx_ohci0_hcfmnumber_s  cn66xx;
	struct cvmx_uahcx_ohci0_hcfmnumber_s  cn68xx;
	struct cvmx_uahcx_ohci0_hcfmnumber_s  cn68xxp1;
	struct cvmx_uahcx_ohci0_hcfmnumber_s  cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcfmnumber cvmx_uahcx_ohci0_hcfmnumber_t;

/**
 * cvmx_uahc#_ohci0_hcfmremaining
 *
 * HCFMREMAINING = Host Controller Frame Remaining Register
 * The HcFmRemaining register is a 14-bit down counter showing the bit time remaining in the current Frame.
 */
union cvmx_uahcx_ohci0_hcfmremaining {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcfmremaining_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t frt                          : 1;  /**< FrameRemainingToggle. This bit is loaded from the FrameIntervalToggle field
                                                         of HcFmInterval whenever FrameRemaining reaches 0. This bit is used by HCD
                                                         for the synchronization between FrameInterval and FrameRemaining. */
	uint32_t reserved_14_30               : 17;
	uint32_t fr                           : 14; /**< FrameRemaining. This counter is decremented at each bit time. When it
                                                         reaches zero, it is reset by loading the FrameInterval value specified in
                                                         HcFmInterval at the next bit time boundary. When entering the USBOPERATIONAL
                                                         state, HC re-loads the content with the FrameInterval of HcFmInterval and uses
                                                         the updated value from the next SOF. */
#else
	uint32_t fr                           : 14;
	uint32_t reserved_14_30               : 17;
	uint32_t frt                          : 1;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcfmremaining_s cn61xx;
	struct cvmx_uahcx_ohci0_hcfmremaining_s cn63xx;
	struct cvmx_uahcx_ohci0_hcfmremaining_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hcfmremaining_s cn66xx;
	struct cvmx_uahcx_ohci0_hcfmremaining_s cn68xx;
	struct cvmx_uahcx_ohci0_hcfmremaining_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hcfmremaining_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcfmremaining cvmx_uahcx_ohci0_hcfmremaining_t;

/**
 * cvmx_uahc#_ohci0_hchcca
 *
 * HCHCCA =  Host Controller Host Controller Communication Area Register
 *
 * The HcHCCA register contains the physical address of the Host Controller Communication Area. The Host Controller Driver determines the alignment restrictions
 * by writing all 1s to HcHCCA and reading the content of HcHCCA. The alignment is evaluated by examining the number of zeroes in the lower order bits. The
 * minimum alignment is 256 bytes; therefore, bits 0 through 7 must always return '0' when read. Detailed description can be found in Chapter 4. This area
 * is used to hold the control structures and the Interrupt table that are accessed by both the Host Controller and the Host Controller Driver.
 */
union cvmx_uahcx_ohci0_hchcca {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hchcca_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t hcca                         : 24; /**< This is the base address (bits [31:8]) of the Host Controller Communication Area. */
	uint32_t reserved_0_7                 : 8;
#else
	uint32_t reserved_0_7                 : 8;
	uint32_t hcca                         : 24;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hchcca_s      cn61xx;
	struct cvmx_uahcx_ohci0_hchcca_s      cn63xx;
	struct cvmx_uahcx_ohci0_hchcca_s      cn63xxp1;
	struct cvmx_uahcx_ohci0_hchcca_s      cn66xx;
	struct cvmx_uahcx_ohci0_hchcca_s      cn68xx;
	struct cvmx_uahcx_ohci0_hchcca_s      cn68xxp1;
	struct cvmx_uahcx_ohci0_hchcca_s      cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hchcca cvmx_uahcx_ohci0_hchcca_t;

/**
 * cvmx_uahc#_ohci0_hcinterruptdisable
 *
 * HCINTERRUPTDISABLE = Host Controller InterruptDisable Register
 *
 * Each disable bit in the HcInterruptDisable register corresponds to an associated interrupt bit in the HcInterruptStatus register. The HcInterruptDisable
 * register is coupled with the HcInterruptEnable register. Thus, writing a '1' to a bit in this register clears the corresponding bit in the HcInterruptEnable
 * register, whereas writing a '0' to a bit in this register leaves the corresponding bit in the HcInterruptEnable register unchanged. On read, the current
 * value of the HcInterruptEnable register is returned.
 */
union cvmx_uahcx_ohci0_hcinterruptdisable {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcinterruptdisable_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t mie                          : 1;  /**< A '0' written to this field is ignored by HC.
                                                         A '1' written to this field disables interrupt generation due to events
                                                         specified in the other bits of this register. This field is set after a
                                                         hardware or software reset. */
	uint32_t oc                           : 1;  /**< 0 - Ignore; 1 - Disable interrupt generation due to Ownership Change. */
	uint32_t reserved_7_29                : 23;
	uint32_t rhsc                         : 1;  /**< 0 - Ignore; 1 - Disable interrupt generation due to Root Hub Status Change. */
	uint32_t fno                          : 1;  /**< 0 - Ignore; 1 - Disable interrupt generation due to Frame Number Overflow. */
	uint32_t ue                           : 1;  /**< 0 - Ignore; 1 - Disable interrupt generation due to Unrecoverable Error. */
	uint32_t rd                           : 1;  /**< 0 - Ignore; 1 - Disable interrupt generation due to Resume Detect. */
	uint32_t sf                           : 1;  /**< 0 - Ignore; 1 - Disable interrupt generation due to Start of Frame. */
	uint32_t wdh                          : 1;  /**< 0 - Ignore; 1 - Disable interrupt generation due to HcDoneHead Writeback. */
	uint32_t so                           : 1;  /**< 0 - Ignore; 1 - Disable interrupt generation due to Scheduling Overrun. */
#else
	uint32_t so                           : 1;
	uint32_t wdh                          : 1;
	uint32_t sf                           : 1;
	uint32_t rd                           : 1;
	uint32_t ue                           : 1;
	uint32_t fno                          : 1;
	uint32_t rhsc                         : 1;
	uint32_t reserved_7_29                : 23;
	uint32_t oc                           : 1;
	uint32_t mie                          : 1;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcinterruptdisable_s cn61xx;
	struct cvmx_uahcx_ohci0_hcinterruptdisable_s cn63xx;
	struct cvmx_uahcx_ohci0_hcinterruptdisable_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hcinterruptdisable_s cn66xx;
	struct cvmx_uahcx_ohci0_hcinterruptdisable_s cn68xx;
	struct cvmx_uahcx_ohci0_hcinterruptdisable_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hcinterruptdisable_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcinterruptdisable cvmx_uahcx_ohci0_hcinterruptdisable_t;

/**
 * cvmx_uahc#_ohci0_hcinterruptenable
 *
 * HCINTERRUPTENABLE = Host Controller InterruptEnable Register
 *
 * Each enable bit in the HcInterruptEnable register corresponds to an associated interrupt bit in the HcInterruptStatus register. The HcInterruptEnable
 * register is used to control which events generate a hardware interrupt. When a bit is set in the HcInterruptStatus register AND the corresponding bit
 * in the HcInterruptEnable register is set AND the MasterInterruptEnable bit is set, then a hardware interrupt is requested on the host bus.
 * Writing a '1' to a bit in this register sets the corresponding bit, whereas writing a '0' to a bit in this register leaves the corresponding bit
 * unchanged. On read, the current value of this register is returned.
 */
union cvmx_uahcx_ohci0_hcinterruptenable {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcinterruptenable_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t mie                          : 1;  /**< A '0' written to this field is ignored by HC.
                                                         A '1' written to this field enables interrupt generation due to events
                                                         specified in the other bits of this register. This is used by HCD as a Master
                                                         Interrupt Enable. */
	uint32_t oc                           : 1;  /**< 0 - Ignore; 1 - Enable interrupt generation due to Ownership Change. */
	uint32_t reserved_7_29                : 23;
	uint32_t rhsc                         : 1;  /**< 0 - Ignore; 1 - Enable interrupt generation due to Root Hub Status Change. */
	uint32_t fno                          : 1;  /**< 0 - Ignore; 1 - Enable interrupt generation due to Frame Number Overflow. */
	uint32_t ue                           : 1;  /**< 0 - Ignore; 1 - Enable interrupt generation due to Unrecoverable Error. */
	uint32_t rd                           : 1;  /**< 0 - Ignore; 1 - Enable interrupt generation due to Resume Detect. */
	uint32_t sf                           : 1;  /**< 0 - Ignore; 1 - Enable interrupt generation due to Start of Frame. */
	uint32_t wdh                          : 1;  /**< 0 - Ignore; 1 - Enable interrupt generation due to HcDoneHead Writeback. */
	uint32_t so                           : 1;  /**< 0 - Ignore; 1 - Enable interrupt generation due to Scheduling Overrun. */
#else
	uint32_t so                           : 1;
	uint32_t wdh                          : 1;
	uint32_t sf                           : 1;
	uint32_t rd                           : 1;
	uint32_t ue                           : 1;
	uint32_t fno                          : 1;
	uint32_t rhsc                         : 1;
	uint32_t reserved_7_29                : 23;
	uint32_t oc                           : 1;
	uint32_t mie                          : 1;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcinterruptenable_s cn61xx;
	struct cvmx_uahcx_ohci0_hcinterruptenable_s cn63xx;
	struct cvmx_uahcx_ohci0_hcinterruptenable_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hcinterruptenable_s cn66xx;
	struct cvmx_uahcx_ohci0_hcinterruptenable_s cn68xx;
	struct cvmx_uahcx_ohci0_hcinterruptenable_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hcinterruptenable_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcinterruptenable cvmx_uahcx_ohci0_hcinterruptenable_t;

/**
 * cvmx_uahc#_ohci0_hcinterruptstatus
 *
 * HCINTERRUPTSTATUS = Host Controller InterruptStatus Register
 *
 * This register provides status on various events that cause hardware interrupts. When an event occurs, Host Controller sets the corresponding bit
 * in this register. When a bit becomes set, a hardware interrupt is generated if the interrupt is enabled in the HcInterruptEnable register
 * and the MasterInterruptEnable bit is set. The Host Controller Driver may clear specific bits in this register by writing '1' to bit positions
 * to be cleared. The Host Controller Driver may not set any of these bits. The Host Controller will never clear the bit.
 */
union cvmx_uahcx_ohci0_hcinterruptstatus {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcinterruptstatus_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t oc                           : 1;  /**< OwnershipChange. This bit is set by HC when HCD sets the OwnershipChangeRequest
                                                         field in HcCommandStatus. This event, when unmasked, will always generate an
                                                         System Management Interrupt (SMI) immediately. This bit is tied to 0b when the
                                                         SMI pin is not implemented. */
	uint32_t reserved_7_29                : 23;
	uint32_t rhsc                         : 1;  /**< RootHubStatusChange. This bit is set when the content of HcRhStatus or the
                                                         content of any of HcRhPortStatus[NumberofDownstreamPort] has changed. */
	uint32_t fno                          : 1;  /**< FrameNumberOverflow. This bit is set when the MSb of HcFmNumber (bit 15)
                                                         changes value, from 0 to 1 or from 1 to 0, and after HccaFrameNumber has been
                                                         updated. */
	uint32_t ue                           : 1;  /**< UnrecoverableError. This bit is set when HC detects a system error not related
                                                         to USB. HC should not proceed with any processing nor signaling before the
                                                         system error has been corrected. HCD clears this bit after HC has been reset. */
	uint32_t rd                           : 1;  /**< ResumeDetected. This bit is set when HC detects that a device on the USB is
                                                          asserting resume signaling. It is the transition from no resume signaling to
                                                         resume signaling causing this bit to be set. This bit is not set when HCD
                                                         sets the USBRESUME state. */
	uint32_t sf                           : 1;  /**< StartofFrame. This bit is set by HC at each start of a frame and after the
                                                         update of HccaFrameNumber. HC also generates a SOF token at the same time. */
	uint32_t wdh                          : 1;  /**< WritebackDoneHead. This bit is set immediately after HC has written
                                                         HcDoneHead to HccaDoneHead. Further updates of the HccaDoneHead will not
                                                         occur until this bit has been cleared. HCD should only clear this bit after
                                                         it has saved the content of HccaDoneHead. */
	uint32_t so                           : 1;  /**< SchedulingOverrun. This bit is set when the USB schedule for the current
                                                         Frame overruns and after the update of HccaFrameNumber. A scheduling overrun
                                                         will also cause the SchedulingOverrunCount of HcCommandStatus to be
                                                         incremented. */
#else
	uint32_t so                           : 1;
	uint32_t wdh                          : 1;
	uint32_t sf                           : 1;
	uint32_t rd                           : 1;
	uint32_t ue                           : 1;
	uint32_t fno                          : 1;
	uint32_t rhsc                         : 1;
	uint32_t reserved_7_29                : 23;
	uint32_t oc                           : 1;
	uint32_t reserved_31_31               : 1;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcinterruptstatus_s cn61xx;
	struct cvmx_uahcx_ohci0_hcinterruptstatus_s cn63xx;
	struct cvmx_uahcx_ohci0_hcinterruptstatus_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hcinterruptstatus_s cn66xx;
	struct cvmx_uahcx_ohci0_hcinterruptstatus_s cn68xx;
	struct cvmx_uahcx_ohci0_hcinterruptstatus_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hcinterruptstatus_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcinterruptstatus cvmx_uahcx_ohci0_hcinterruptstatus_t;

/**
 * cvmx_uahc#_ohci0_hclsthreshold
 *
 * HCLSTHRESHOLD = Host Controller LS Threshold Register
 *
 * The HcLSThreshold register contains an 11-bit value used by the Host Controller to determine whether to commit to the transfer of a maximum of 8-byte
 * LS packet before EOF. Neither the Host Controller nor the Host Controller Driver are allowed to change this value.
 */
union cvmx_uahcx_ohci0_hclsthreshold {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hclsthreshold_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_12_31               : 20;
	uint32_t lst                          : 12; /**< LSThreshold
                                                         This field contains a value which is compared to the FrameRemaining field
                                                         prior to initiating a Low Speed transaction. The transaction is started only
                                                         if FrameRemaining >= this field. The value is calculated by HCD
                                                         with the consideration of transmission and setup overhead. */
#else
	uint32_t lst                          : 12;
	uint32_t reserved_12_31               : 20;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hclsthreshold_s cn61xx;
	struct cvmx_uahcx_ohci0_hclsthreshold_s cn63xx;
	struct cvmx_uahcx_ohci0_hclsthreshold_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hclsthreshold_s cn66xx;
	struct cvmx_uahcx_ohci0_hclsthreshold_s cn68xx;
	struct cvmx_uahcx_ohci0_hclsthreshold_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hclsthreshold_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hclsthreshold cvmx_uahcx_ohci0_hclsthreshold_t;

/**
 * cvmx_uahc#_ohci0_hcperiodcurrented
 *
 * HCPERIODCURRENTED = Host Controller Period Current ED Register
 *
 * The HcPeriodCurrentED register contains the physical address of the current Isochronous or Interrupt Endpoint Descriptor.
 */
union cvmx_uahcx_ohci0_hcperiodcurrented {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcperiodcurrented_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pced                         : 28; /**< PeriodCurrentED. This is used by HC to point to the head of one of the
                                                         Periodic lists which will be processed in the current Frame. The content of
                                                         this register is updated by HC after a periodic ED has been processed. HCD
                                                         may read the content in determining which ED is currently being processed
                                                         at the time of reading. */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t pced                         : 28;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcperiodcurrented_s cn61xx;
	struct cvmx_uahcx_ohci0_hcperiodcurrented_s cn63xx;
	struct cvmx_uahcx_ohci0_hcperiodcurrented_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hcperiodcurrented_s cn66xx;
	struct cvmx_uahcx_ohci0_hcperiodcurrented_s cn68xx;
	struct cvmx_uahcx_ohci0_hcperiodcurrented_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hcperiodcurrented_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcperiodcurrented cvmx_uahcx_ohci0_hcperiodcurrented_t;

/**
 * cvmx_uahc#_ohci0_hcperiodicstart
 *
 * HCPERIODICSTART = Host Controller Periodic Start Register
 *
 * The HcPeriodicStart register has a 14-bit programmable value which determines when is the earliest time HC should start processing the periodic list.
 */
union cvmx_uahcx_ohci0_hcperiodicstart {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcperiodicstart_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_14_31               : 18;
	uint32_t ps                           : 14; /**< PeriodicStart After a hardware reset, this field is cleared. This is then set
                                                         by HCD during the HC initialization. The value is calculated roughly as 10%
                                                         off from HcFmInterval.. A typical value will be 3E67h. When HcFmRemaining
                                                         reaches the value specified, processing of the periodic lists will have
                                                         priority over Control/Bulk processing. HC will therefore start processing
                                                         the Interrupt list after completing the current Control or Bulk transaction
                                                         that is in progress. */
#else
	uint32_t ps                           : 14;
	uint32_t reserved_14_31               : 18;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcperiodicstart_s cn61xx;
	struct cvmx_uahcx_ohci0_hcperiodicstart_s cn63xx;
	struct cvmx_uahcx_ohci0_hcperiodicstart_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hcperiodicstart_s cn66xx;
	struct cvmx_uahcx_ohci0_hcperiodicstart_s cn68xx;
	struct cvmx_uahcx_ohci0_hcperiodicstart_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hcperiodicstart_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcperiodicstart cvmx_uahcx_ohci0_hcperiodicstart_t;

/**
 * cvmx_uahc#_ohci0_hcrevision
 *
 * HCREVISION = Host Controller Revision Register
 *
 */
union cvmx_uahcx_ohci0_hcrevision {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcrevision_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_8_31                : 24;
	uint32_t rev                          : 8;  /**< Revision This read-only field contains the BCD representation of the version
                                                         of the HCI specification that is implemented by this HC. For example, a value
                                                         of 11h corresponds to version 1.1. All of the HC implementations that are
                                                         compliant with this specification will have a value of 10h. */
#else
	uint32_t rev                          : 8;
	uint32_t reserved_8_31                : 24;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcrevision_s  cn61xx;
	struct cvmx_uahcx_ohci0_hcrevision_s  cn63xx;
	struct cvmx_uahcx_ohci0_hcrevision_s  cn63xxp1;
	struct cvmx_uahcx_ohci0_hcrevision_s  cn66xx;
	struct cvmx_uahcx_ohci0_hcrevision_s  cn68xx;
	struct cvmx_uahcx_ohci0_hcrevision_s  cn68xxp1;
	struct cvmx_uahcx_ohci0_hcrevision_s  cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcrevision cvmx_uahcx_ohci0_hcrevision_t;

/**
 * cvmx_uahc#_ohci0_hcrhdescriptora
 *
 * HCRHDESCRIPTORA = Host Controller Root Hub DescriptorA Register
 *
 * The HcRhDescriptorA register is the first register of two describing the characteristics of the Root Hub. Reset values are implementation-specific.
 * The descriptor length (11), descriptor type (0x29), and hub controller current (0) fields of the hub Class Descriptor are emulated by the HCD. All
 * other fields are located in the HcRhDescriptorA and HcRhDescriptorB registers.
 */
union cvmx_uahcx_ohci0_hcrhdescriptora {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcrhdescriptora_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t potpgt                       : 8;  /**< PowerOnToPowerGoodTime. This byte specifies the duration HCD has to wait before
                                                         accessing a powered-on port of the Root Hub. It is implementation-specific. The
                                                         unit of time is 2 ms. The duration is calculated as POTPGT * 2 ms. */
	uint32_t reserved_13_23               : 11;
	uint32_t nocp                         : 1;  /**< NoOverCurrentProtection. This bit describes how the overcurrent status for the
                                                         Root Hub ports are reported. When this bit is cleared, the
                                                         OverCurrentProtectionMode field specifies global or per-port reporting.
                                                         - 0: Over-current status is reported  collectively for all downstream ports
                                                         - 1: No overcurrent protection supported */
	uint32_t ocpm                         : 1;  /**< OverCurrentProtectionMode. This bit describes how the overcurrent status for
                                                         the Root Hub ports are reported. At reset, this fields should reflect the same
                                                         mode as PowerSwitchingMode. This field is valid only if the
                                                         NoOverCurrentProtection field is cleared. 0: over-current status is reported
                                                         collectively for all downstream ports 1: over-current status is reported on a
                                                         per-port basis */
	uint32_t dt                           : 1;  /**< DeviceType. This bit specifies that the Root Hub is not a compound device. The
                                                         Root Hub is not permitted to be a compound device. This field should always
                                                         read/write 0. */
	uint32_t psm                          : 1;  /**< PowerSwitchingMode. This bit is used to specify how the power switching of
                                                         the Root Hub ports is controlled. It is implementation-specific. This field
                                                         is only valid if the NoPowerSwitching field is cleared. 0: all ports are
                                                         powered at the same time. 1: each port is powered individually.  This mode
                                                         allows port power to be controlled by either the global switch or per-port
                                                         switching. If the PortPowerControlMask bit is set, the port responds only
                                                         to port power commands (Set/ClearPortPower). If the port mask is cleared,
                                                         then the port is controlled only by the global power switch
                                                         (Set/ClearGlobalPower). */
	uint32_t nps                          : 1;  /**< NoPowerSwitching These bits are used to specify whether power switching is
                                                         supported or port are always powered. It is implementation-specific. When
                                                         this bit is cleared, the PowerSwitchingMode specifies global or per-port
                                                         switching.
                                                          - 0: Ports are power switched
                                                          - 1: Ports are always powered on when the HC is powered on */
	uint32_t ndp                          : 8;  /**< NumberDownstreamPorts. These bits specify the number of downstream ports
                                                         supported by the Root Hub. It is implementation-specific. The minimum number
                                                         of ports is 1. The maximum number of ports supported by OpenHCI is 15. */
#else
	uint32_t ndp                          : 8;
	uint32_t nps                          : 1;
	uint32_t psm                          : 1;
	uint32_t dt                           : 1;
	uint32_t ocpm                         : 1;
	uint32_t nocp                         : 1;
	uint32_t reserved_13_23               : 11;
	uint32_t potpgt                       : 8;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcrhdescriptora_s cn61xx;
	struct cvmx_uahcx_ohci0_hcrhdescriptora_s cn63xx;
	struct cvmx_uahcx_ohci0_hcrhdescriptora_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hcrhdescriptora_s cn66xx;
	struct cvmx_uahcx_ohci0_hcrhdescriptora_s cn68xx;
	struct cvmx_uahcx_ohci0_hcrhdescriptora_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hcrhdescriptora_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcrhdescriptora cvmx_uahcx_ohci0_hcrhdescriptora_t;

/**
 * cvmx_uahc#_ohci0_hcrhdescriptorb
 *
 * HCRHDESCRIPTORB = Host Controller Root Hub DescriptorB Register
 *
 * The HcRhDescriptorB register is the second register of two describing the characteristics of the Root Hub. These fields are written during
 * initialization to correspond with the system implementation. Reset values are implementation-specific.
 */
union cvmx_uahcx_ohci0_hcrhdescriptorb {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcrhdescriptorb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ppcm                         : 16; /**< PortPowerControlMask.
                                                         Each bit indicates if a port is affected by a global power control command
                                                         when PowerSwitchingMode is set. When set, the port's power state is only
                                                         affected by per-port power control (Set/ClearPortPower). When cleared, the
                                                         port is controlled by the global power switch (Set/ClearGlobalPower). If
                                                         the device is configured to global switching mode (PowerSwitchingMode=0),
                                                         this field is not valid.
                                                            bit 0: Reserved
                                                            bit 1: Ganged-power mask on Port \#1
                                                            bit 2: Ganged-power mask on Port \#2
                                                            - ...
                                                            bit15: Ganged-power mask on Port \#15 */
	uint32_t dr                           : 16; /**< DeviceRemovable.
                                                         Each bit is dedicated to a port of the Root Hub. When cleared,the attached
                                                         device is removable. When set, the attached device is not removable.
                                                             bit 0: Reserved
                                                             bit 1: Device attached to Port \#1
                                                             bit 2: Device attached to Port \#2
                                                             - ...
                                                             bit15: Device attached to Port \#15 */
#else
	uint32_t dr                           : 16;
	uint32_t ppcm                         : 16;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcrhdescriptorb_s cn61xx;
	struct cvmx_uahcx_ohci0_hcrhdescriptorb_s cn63xx;
	struct cvmx_uahcx_ohci0_hcrhdescriptorb_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hcrhdescriptorb_s cn66xx;
	struct cvmx_uahcx_ohci0_hcrhdescriptorb_s cn68xx;
	struct cvmx_uahcx_ohci0_hcrhdescriptorb_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hcrhdescriptorb_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcrhdescriptorb cvmx_uahcx_ohci0_hcrhdescriptorb_t;

/**
 * cvmx_uahc#_ohci0_hcrhportstatus#
 *
 * HCRHPORTSTATUSX = Host Controller Root Hub Port X Status Registers
 *
 * The HcRhPortStatus[1:NDP] register is used to control and report port events on a per-port basis. NumberDownstreamPorts represents the number
 * of HcRhPortStatus registers that are implemented in hardware. The lower word is used to reflect the port status, whereas the upper word reflects
 * the status change bits. Some status bits are implemented with special write behavior (see below). If a transaction (token through handshake) is
 * in progress when a write to change port status occurs, the resulting port status change must be postponed until the transaction completes.
 * Reserved bits should always be written '0'.
 */
union cvmx_uahcx_ohci0_hcrhportstatusx {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcrhportstatusx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_21_31               : 11;
	uint32_t prsc                         : 1;  /**< PortResetStatusChange. This bit is set at the end of the 10-ms port reset
                                                         signal. The HCD writes a '1' to clear this bit. Writing a '0' has no effect.
                                                            0 = port reset is not complete
                                                            1 = port reset is complete */
	uint32_t ocic                         : 1;  /**< PortOverCurrentIndicatorChange. This bit is valid only if overcurrent
                                                         conditions are reported on a per-port basis. This bit is set when Root Hub
                                                         changes the PortOverCurrentIndicator bit. The HCD writes a '1' to clear this
                                                         bit. Writing a  '0'  has no effect.
                                                            0 = no change in PortOverCurrentIndicator
                                                            1 = PortOverCurrentIndicator has changed */
	uint32_t pssc                         : 1;  /**< PortSuspendStatusChange. This bit is set when the full resume sequence has
                                                         been completed. This sequence includes the 20-s resume pulse, LS EOP, and
                                                         3-ms resychronization delay.
                                                         The HCD writes a '1' to clear this bit. Writing a '0' has no effect. This
                                                         bit is also cleared when ResetStatusChange is set.
                                                            0 = resume is not completed
                                                            1 = resume completed */
	uint32_t pesc                         : 1;  /**< PortEnableStatusChange. This bit is set when hardware events cause the
                                                         PortEnableStatus bit to be cleared. Changes from HCD writes do not set this
                                                         bit. The HCD writes a '1' to clear this bit. Writing a '0' has no effect.
                                                           0 = no change in PortEnableStatus
                                                           1 = change in PortEnableStatus */
	uint32_t csc                          : 1;  /**< ConnectStatusChange. This bit is set whenever a connect or disconnect event
                                                         occurs. The HCD writes a '1' to clear this bit. Writing a '0' has no
                                                         effect. If CurrentConnectStatus is cleared when a SetPortReset,SetPortEnable,
                                                         or SetPortSuspend write occurs, this bit is set to force the driver to
                                                         re-evaluate the connection status since these writes should not occur if the
                                                         port is disconnected.
                                                           0 = no change in CurrentConnectStatus
                                                           1 = change in CurrentConnectStatus
                                                         Note: If the DeviceRemovable[NDP] bit is set, this bit is set only after a
                                                          Root Hub reset to inform the system that the device is attached. Description */
	uint32_t reserved_10_15               : 6;
	uint32_t lsda                         : 1;  /**< (read) LowSpeedDeviceAttached. This bit indicates the speed of the device
                                                              attached to this port. When set, a Low Speed device is attached to this
                                                              port. When clear, a Full Speed device is attached to this port. This
                                                              field is valid only when the CurrentConnectStatus is set.
                                                                 0 = full speed device attached
                                                                 1 = low speed device attached
                                                         (write) ClearPortPower. The HCD clears the PortPowerStatus bit by writing a
                                                              '1' to this bit. Writing a '0' has no effect. */
	uint32_t pps                          : 1;  /**< (read) PortPowerStatus. This bit reflects the port's power status, regardless
                                                             of the type of power switching implemented. This bit is cleared if an
                                                             overcurrent condition is detected. HCD sets this bit by writing
                                                             SetPortPower or SetGlobalPower. HCD clears this bit by writing
                                                             ClearPortPower or ClearGlobalPower. Which power control switches are
                                                             enabled is determined by PowerSwitchingMode and PortPortControlMask[NDP].
                                                             In global switching mode (PowerSwitchingMode=0), only Set/ClearGlobalPower
                                                               controls this bit. In per-port power switching (PowerSwitchingMode=1),
                                                               if the PortPowerControlMask[NDP] bit for the port is set, only
                                                               Set/ClearPortPower commands are enabled. If the mask is not set, only
                                                               Set/ClearGlobalPower commands are enabled. When port power is disabled,
                                                               CurrentConnectStatus, PortEnableStatus, PortSuspendStatus, and
                                                               PortResetStatus should be reset.
                                                                  0 = port power is off
                                                                  1 = port power is on
                                                         (write) SetPortPower. The HCD writes a '1' to set the PortPowerStatus bit.
                                                               Writing a '0' has no effect. Note: This bit is always reads '1'
                                                               if power switching is not supported. */
	uint32_t reserved_5_7                 : 3;
	uint32_t prs                          : 1;  /**< (read) PortResetStatus. When this bit is set by a write to SetPortReset, port
                                                               reset signaling is asserted. When reset is completed, this bit is
                                                               cleared when PortResetStatusChange is set. This bit cannot be set if
                                                               CurrentConnectStatus is cleared.
                                                                  0 = port reset signal is not active
                                                               1 = port reset signal is active
                                                         (write) SetPortReset. The HCD sets the port reset signaling by writing a '1'
                                                               to this bit. Writing a '0'has no effect. If CurrentConnectStatus is
                                                               cleared, this write does not set PortResetStatus, but instead sets
                                                               ConnectStatusChange. This informs the driver that it attempted to reset
                                                               a disconnected port. Description */
	uint32_t poci                         : 1;  /**< (read) PortOverCurrentIndicator. This bit is only valid when the Root Hub is
                                                                configured in such a way that overcurrent conditions are reported on a
                                                                per-port basis. If per-port overcurrent reporting is not supported, this
                                                                bit is set to 0. If cleared, all power operations are normal for this
                                                                port. If set, an overcurrent condition exists on this port. This bit
                                                                always reflects the overcurrent input signal
                                                                  0 = no overcurrent condition.
                                                                  1 = overcurrent condition detected.
                                                         (write) ClearSuspendStatus. The HCD writes a '1' to initiate a resume.
                                                                 Writing  a '0' has no effect. A resume is initiated only if
                                                                 PortSuspendStatus is set. */
	uint32_t pss                          : 1;  /**< (read) PortSuspendStatus. This bit indicates the port is suspended or in the
                                                              resume sequence. It is set by a SetSuspendState write and cleared when
                                                              PortSuspendStatusChange is set at the end of the resume interval. This
                                                              bit cannot be set if CurrentConnectStatus is cleared. This bit is also
                                                              cleared when PortResetStatusChange is set at the end of the port reset
                                                              or when the HC is placed in the USBRESUME state. If an upstream resume is
                                                              in progress, it should propagate to the HC.
                                                                 0 = port is not suspended
                                                                 1 = port is suspended
                                                         (write) SetPortSuspend. The HCD sets the PortSuspendStatus bit by writing a
                                                              '1' to this bit. Writing a '0' has no effect. If CurrentConnectStatus
                                                                is cleared, this write does not set PortSuspendStatus; instead it sets
                                                                ConnectStatusChange.This informs the driver that it attempted to suspend
                                                                a disconnected port. */
	uint32_t pes                          : 1;  /**< (read) PortEnableStatus. This bit indicates whether the port is enabled or
                                                              disabled. The Root Hub may clear this bit when an overcurrent condition,
                                                              disconnect event, switched-off power, or operational bus error such
                                                              as babble is detected. This change also causes PortEnabledStatusChange
                                                              to be set. HCD sets this bit by writing SetPortEnable and clears it by
                                                              writing ClearPortEnable. This bit cannot be set when CurrentConnectStatus
                                                              is cleared. This bit is also set, if not already, at the completion of a
                                                              port reset when ResetStatusChange is set or port suspend when
                                                              SuspendStatusChange is set.
                                                                0 = port is disabled
                                                                1 = port is enabled
                                                         (write) SetPortEnable. The HCD sets PortEnableStatus by writing a '1'.
                                                              Writing a '0' has no effect. If CurrentConnectStatus is cleared, this
                                                              write does not set PortEnableStatus, but instead sets ConnectStatusChange.
                                                              This informs the driver that it attempted to enable a disconnected port. */
	uint32_t ccs                          : 1;  /**< (read) CurrentConnectStatus. This bit reflects the current state of the
                                                               downstream port.
                                                                 0 = no device connected
                                                                 1 = device connected
                                                         (write) ClearPortEnable.
                                                                The HCD writes a '1' to this bit to clear the PortEnableStatus bit.
                                                                Writing a '0' has no effect. The CurrentConnectStatus is not
                                                                affected by any write.
                                                          Note: This bit is always read '1b' when the attached device is
                                                                nonremovable (DeviceRemoveable[NDP]). */
#else
	uint32_t ccs                          : 1;
	uint32_t pes                          : 1;
	uint32_t pss                          : 1;
	uint32_t poci                         : 1;
	uint32_t prs                          : 1;
	uint32_t reserved_5_7                 : 3;
	uint32_t pps                          : 1;
	uint32_t lsda                         : 1;
	uint32_t reserved_10_15               : 6;
	uint32_t csc                          : 1;
	uint32_t pesc                         : 1;
	uint32_t pssc                         : 1;
	uint32_t ocic                         : 1;
	uint32_t prsc                         : 1;
	uint32_t reserved_21_31               : 11;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcrhportstatusx_s cn61xx;
	struct cvmx_uahcx_ohci0_hcrhportstatusx_s cn63xx;
	struct cvmx_uahcx_ohci0_hcrhportstatusx_s cn63xxp1;
	struct cvmx_uahcx_ohci0_hcrhportstatusx_s cn66xx;
	struct cvmx_uahcx_ohci0_hcrhportstatusx_s cn68xx;
	struct cvmx_uahcx_ohci0_hcrhportstatusx_s cn68xxp1;
	struct cvmx_uahcx_ohci0_hcrhportstatusx_s cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcrhportstatusx cvmx_uahcx_ohci0_hcrhportstatusx_t;

/**
 * cvmx_uahc#_ohci0_hcrhstatus
 *
 * HCRHSTATUS = Host Controller Root Hub Status Register
 *
 * The HcRhStatus register is divided into two parts. The lower word of a Dword represents the Hub Status field and the upper word represents the Hub
 * Status Change field. Reserved bits should always be written '0'.
 */
union cvmx_uahcx_ohci0_hcrhstatus {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_hcrhstatus_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t crwe                         : 1;  /**< (write) ClearRemoteWakeupEnable Writing a '1' clears DeviceRemoveWakeupEnable.
                                                         Writing a '0' has no effect. */
	uint32_t reserved_18_30               : 13;
	uint32_t ccic                         : 1;  /**< OverCurrentIndicatorChange. This bit is set by hardware when a change has
                                                         occurred to the OCI field of this register. The HCD clears this bit by
                                                         writing a '1'. Writing a '0' has no effect. */
	uint32_t lpsc                         : 1;  /**< (read) LocalPowerStatusChange. The Root Hub does not support the local power
                                                                status feature; thus, this bit is always read as '0'.
                                                         (write) SetGlobalPower In global power mode (PowerSwitchingMode=0), This bit
                                                                 is written to '1' to turn on power to all ports (clear PortPowerStatus).
                                                                 In per-port power mode, it sets PortPowerStatus only on ports whose
                                                                 PortPowerControlMask bit is not set. Writing a '0' has no effect. */
	uint32_t drwe                         : 1;  /**< (read) DeviceRemoteWakeupEnable. This bit enables a ConnectStatusChange bit as
                                                                a resume event, causing a USBSUSPEND to USBRESUME state transition and
                                                                setting the ResumeDetected interrupt. 0 = ConnectStatusChange is not a
                                                                remote wakeup event. 1 = ConnectStatusChange is a remote wakeup event.
                                                         (write) SetRemoteWakeupEnable Writing a '1' sets DeviceRemoveWakeupEnable.
                                                                 Writing a '0' has no effect. */
	uint32_t reserved_2_14                : 13;
	uint32_t oci                          : 1;  /**< OverCurrentIndicator. This bit reports overcurrent conditions when the global
                                                         reporting is implemented. When set, an overcurrent condition exists. When
                                                         cleared, all power operations are normal. If per-port overcurrent protection
                                                         is implemented this bit is always '0' */
	uint32_t lps                          : 1;  /**< (read)  LocalPowerStatus. The Root Hub does not support the local power status
                                                                 feature; thus, this bit is always read as '0.
                                                         (write) ClearGlobalPower. In global power mode (PowerSwitchingMode=0), This
                                                                 bit is written to '1' to turn off power to all ports
                                                                 (clear PortPowerStatus). In per-port power mode, it clears
                                                                 PortPowerStatus only on ports whose PortPowerControlMask bit is not
                                                                 set. Writing a '0' has no effect. Description */
#else
	uint32_t lps                          : 1;
	uint32_t oci                          : 1;
	uint32_t reserved_2_14                : 13;
	uint32_t drwe                         : 1;
	uint32_t lpsc                         : 1;
	uint32_t ccic                         : 1;
	uint32_t reserved_18_30               : 13;
	uint32_t crwe                         : 1;
#endif
	} s;
	struct cvmx_uahcx_ohci0_hcrhstatus_s  cn61xx;
	struct cvmx_uahcx_ohci0_hcrhstatus_s  cn63xx;
	struct cvmx_uahcx_ohci0_hcrhstatus_s  cn63xxp1;
	struct cvmx_uahcx_ohci0_hcrhstatus_s  cn66xx;
	struct cvmx_uahcx_ohci0_hcrhstatus_s  cn68xx;
	struct cvmx_uahcx_ohci0_hcrhstatus_s  cn68xxp1;
	struct cvmx_uahcx_ohci0_hcrhstatus_s  cnf71xx;
};
typedef union cvmx_uahcx_ohci0_hcrhstatus cvmx_uahcx_ohci0_hcrhstatus_t;

/**
 * cvmx_uahc#_ohci0_insnreg06
 *
 * OHCI0_INSNREG06 = OHCI  AHB Error Status Register (Synopsys Speicific)
 *
 * This register contains AHB Error Status.
 */
union cvmx_uahcx_ohci0_insnreg06 {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_insnreg06_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t vld                          : 1;  /**< AHB Error Captured. Indicator that an AHB error was encountered and values were captured.
                                                         To clear this field the application must write a 0 to it. */
	uint32_t reserved_0_30                : 31;
#else
	uint32_t reserved_0_30                : 31;
	uint32_t vld                          : 1;
#endif
	} s;
	struct cvmx_uahcx_ohci0_insnreg06_s   cn61xx;
	struct cvmx_uahcx_ohci0_insnreg06_s   cn63xx;
	struct cvmx_uahcx_ohci0_insnreg06_s   cn63xxp1;
	struct cvmx_uahcx_ohci0_insnreg06_s   cn66xx;
	struct cvmx_uahcx_ohci0_insnreg06_s   cn68xx;
	struct cvmx_uahcx_ohci0_insnreg06_s   cn68xxp1;
	struct cvmx_uahcx_ohci0_insnreg06_s   cnf71xx;
};
typedef union cvmx_uahcx_ohci0_insnreg06 cvmx_uahcx_ohci0_insnreg06_t;

/**
 * cvmx_uahc#_ohci0_insnreg07
 *
 * OHCI0_INSNREG07 = OHCI  AHB Error Address Register (Synopsys Speicific)
 *
 * This register contains AHB Error Status.
 */
union cvmx_uahcx_ohci0_insnreg07 {
	uint32_t u32;
	struct cvmx_uahcx_ohci0_insnreg07_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t err_addr                     : 32; /**< AHB Master Error Address. AHB address of the control phase at which the AHB error occurred */
#else
	uint32_t err_addr                     : 32;
#endif
	} s;
	struct cvmx_uahcx_ohci0_insnreg07_s   cn61xx;
	struct cvmx_uahcx_ohci0_insnreg07_s   cn63xx;
	struct cvmx_uahcx_ohci0_insnreg07_s   cn63xxp1;
	struct cvmx_uahcx_ohci0_insnreg07_s   cn66xx;
	struct cvmx_uahcx_ohci0_insnreg07_s   cn68xx;
	struct cvmx_uahcx_ohci0_insnreg07_s   cn68xxp1;
	struct cvmx_uahcx_ohci0_insnreg07_s   cnf71xx;
};
typedef union cvmx_uahcx_ohci0_insnreg07 cvmx_uahcx_ohci0_insnreg07_t;

/**
 * cvmx_uahc#_pagesize
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.3.
 *
 */
union cvmx_uahcx_pagesize {
	uint32_t u32;
	struct cvmx_uahcx_pagesize_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t pagesize                     : 16; /**< Page size. */
#else
	uint32_t pagesize                     : 16;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_uahcx_pagesize_s          cn78xx;
	struct cvmx_uahcx_pagesize_s          cn78xxp1;
};
typedef union cvmx_uahcx_pagesize cvmx_uahcx_pagesize_t;

/**
 * cvmx_uahc#_porthlpmc_20#
 *
 * For information on this register, refer to the xHCI Specification, v1.1, section 5.4.11.2.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_porthlpmc_20x {
	uint32_t u32;
	struct cvmx_uahcx_porthlpmc_20x_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_14_31               : 18;
	uint32_t hirdd                        : 4;  /**< See section 5.4.11.2 of the XHCI Spec 1.1.
                                                         If UAHC()_SUPTPRT2_DW2[BLC] = 0, then HIRD timing is applied to this field.
                                                         If UAHC()_SUPTPRT2_DW2[BLC] = 1, then BESL timing is applied to this field. */
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
	struct cvmx_uahcx_porthlpmc_20x_s     cn78xx;
	struct cvmx_uahcx_porthlpmc_20x_s     cn78xxp1;
};
typedef union cvmx_uahcx_porthlpmc_20x cvmx_uahcx_porthlpmc_20x_t;

/**
 * cvmx_uahc#_porthlpmc_ss#
 *
 * The USB3 port hardware LPM control register is reserved and shall be treated as RsvdP by
 * software. See xHCI specification v1.1 section 5.4.11.1.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST].
 */
union cvmx_uahcx_porthlpmc_ssx {
	uint32_t u32;
	struct cvmx_uahcx_porthlpmc_ssx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_uahcx_porthlpmc_ssx_s     cn78xx;
	struct cvmx_uahcx_porthlpmc_ssx_s     cn78xxp1;
};
typedef union cvmx_uahcx_porthlpmc_ssx cvmx_uahcx_porthlpmc_ssx_t;

/**
 * cvmx_uahc#_portli_20#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.10.
 *
 */
union cvmx_uahcx_portli_20x {
	uint32_t u32;
	struct cvmx_uahcx_portli_20x_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_uahcx_portli_20x_s        cn78xx;
	struct cvmx_uahcx_portli_20x_s        cn78xxp1;
};
typedef union cvmx_uahcx_portli_20x cvmx_uahcx_portli_20x_t;

/**
 * cvmx_uahc#_portli_ss#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.10.
 *
 */
union cvmx_uahcx_portli_ssx {
	uint32_t u32;
	struct cvmx_uahcx_portli_ssx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t linkerrorcount               : 16; /**< Link error count. */
#else
	uint32_t linkerrorcount               : 16;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_uahcx_portli_ssx_s        cn78xx;
	struct cvmx_uahcx_portli_ssx_s        cn78xxp1;
};
typedef union cvmx_uahcx_portli_ssx cvmx_uahcx_portli_ssx_t;

/**
 * cvmx_uahc#_portpmsc_20#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.9.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST].
 */
union cvmx_uahcx_portpmsc_20x {
	uint32_t u32;
	struct cvmx_uahcx_portpmsc_20x_s {
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
	struct cvmx_uahcx_portpmsc_20x_s      cn78xx;
	struct cvmx_uahcx_portpmsc_20x_s      cn78xxp1;
};
typedef union cvmx_uahcx_portpmsc_20x cvmx_uahcx_portpmsc_20x_t;

/**
 * cvmx_uahc#_portpmsc_ss#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.9.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST].
 */
union cvmx_uahcx_portpmsc_ssx {
	uint32_t u32;
	struct cvmx_uahcx_portpmsc_ssx_s {
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
	struct cvmx_uahcx_portpmsc_ssx_s      cn78xx;
	struct cvmx_uahcx_portpmsc_ssx_s      cn78xxp1;
};
typedef union cvmx_uahcx_portpmsc_ssx cvmx_uahcx_portpmsc_ssx_t;

/**
 * cvmx_uahc#_portsc#
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.8. Port 1
 * is USB3.0 SuperSpeed link, Port 0 is USB2.0 high-speed/full-speed/low-speed link.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST].
 */
union cvmx_uahcx_portscx {
	uint32_t u32;
	struct cvmx_uahcx_portscx_s {
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
	struct cvmx_uahcx_portscx_s           cn78xx;
	struct cvmx_uahcx_portscx_s           cn78xxp1;
};
typedef union cvmx_uahcx_portscx cvmx_uahcx_portscx_t;

/**
 * cvmx_uahc#_rtsoff
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.3.8.
 *
 */
union cvmx_uahcx_rtsoff {
	uint32_t u32;
	struct cvmx_uahcx_rtsoff_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t rtsoff                       : 27; /**< Runtime register-space offset. */
	uint32_t reserved_0_4                 : 5;
#else
	uint32_t reserved_0_4                 : 5;
	uint32_t rtsoff                       : 27;
#endif
	} s;
	struct cvmx_uahcx_rtsoff_s            cn78xx;
	struct cvmx_uahcx_rtsoff_s            cn78xxp1;
};
typedef union cvmx_uahcx_rtsoff cvmx_uahcx_rtsoff_t;

/**
 * cvmx_uahc#_suptprt2_dw0
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.2.
 *
 */
union cvmx_uahcx_suptprt2_dw0 {
	uint32_t u32;
	struct cvmx_uahcx_suptprt2_dw0_s {
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
	struct cvmx_uahcx_suptprt2_dw0_s      cn78xx;
	struct cvmx_uahcx_suptprt2_dw0_s      cn78xxp1;
};
typedef union cvmx_uahcx_suptprt2_dw0 cvmx_uahcx_suptprt2_dw0_t;

/**
 * cvmx_uahc#_suptprt2_dw1
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.2.
 *
 */
union cvmx_uahcx_suptprt2_dw1 {
	uint32_t u32;
	struct cvmx_uahcx_suptprt2_dw1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t name                         : 32; /**< Name string: 'USB'. */
#else
	uint32_t name                         : 32;
#endif
	} s;
	struct cvmx_uahcx_suptprt2_dw1_s      cn78xx;
	struct cvmx_uahcx_suptprt2_dw1_s      cn78xxp1;
};
typedef union cvmx_uahcx_suptprt2_dw1 cvmx_uahcx_suptprt2_dw1_t;

/**
 * cvmx_uahc#_suptprt2_dw2
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.2.
 *
 */
union cvmx_uahcx_suptprt2_dw2 {
	uint32_t u32;
	struct cvmx_uahcx_suptprt2_dw2_s {
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
	struct cvmx_uahcx_suptprt2_dw2_s      cn78xx;
	struct cvmx_uahcx_suptprt2_dw2_s      cn78xxp1;
};
typedef union cvmx_uahcx_suptprt2_dw2 cvmx_uahcx_suptprt2_dw2_t;

/**
 * cvmx_uahc#_suptprt2_dw3
 *
 * For information on this register, refer to the xHCI Specification, v1.1, section 7.2.
 *
 */
union cvmx_uahcx_suptprt2_dw3 {
	uint32_t u32;
	struct cvmx_uahcx_suptprt2_dw3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_5_31                : 27;
	uint32_t protslottype                 : 5;  /**< Protocol slot type. */
#else
	uint32_t protslottype                 : 5;
	uint32_t reserved_5_31                : 27;
#endif
	} s;
	struct cvmx_uahcx_suptprt2_dw3_s      cn78xx;
	struct cvmx_uahcx_suptprt2_dw3_s      cn78xxp1;
};
typedef union cvmx_uahcx_suptprt2_dw3 cvmx_uahcx_suptprt2_dw3_t;

/**
 * cvmx_uahc#_suptprt3_dw0
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.2.
 *
 */
union cvmx_uahcx_suptprt3_dw0 {
	uint32_t u32;
	struct cvmx_uahcx_suptprt3_dw0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t majorrev                     : 8;  /**< Major revision. */
	uint32_t minorrev                     : 8;  /**< Minor revision. */
	uint32_t nextcapptr                   : 8;  /**< Next capability pointer. Value depends on UAHC()_GUCTL[EXTCAPSUPTEN]. If EXTCAPSUPTEN =
                                                         0, value is 0x0. If EXTCAPSUPTEN = 1, value is 0x4. */
	uint32_t capid                        : 8;  /**< Capability ID = supported protocol. */
#else
	uint32_t capid                        : 8;
	uint32_t nextcapptr                   : 8;
	uint32_t minorrev                     : 8;
	uint32_t majorrev                     : 8;
#endif
	} s;
	struct cvmx_uahcx_suptprt3_dw0_s      cn78xx;
	struct cvmx_uahcx_suptprt3_dw0_s      cn78xxp1;
};
typedef union cvmx_uahcx_suptprt3_dw0 cvmx_uahcx_suptprt3_dw0_t;

/**
 * cvmx_uahc#_suptprt3_dw1
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.2.
 *
 */
union cvmx_uahcx_suptprt3_dw1 {
	uint32_t u32;
	struct cvmx_uahcx_suptprt3_dw1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t name                         : 32; /**< Name string: 'USB'. */
#else
	uint32_t name                         : 32;
#endif
	} s;
	struct cvmx_uahcx_suptprt3_dw1_s      cn78xx;
	struct cvmx_uahcx_suptprt3_dw1_s      cn78xxp1;
};
typedef union cvmx_uahcx_suptprt3_dw1 cvmx_uahcx_suptprt3_dw1_t;

/**
 * cvmx_uahc#_suptprt3_dw2
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.2.
 *
 */
union cvmx_uahcx_suptprt3_dw2 {
	uint32_t u32;
	struct cvmx_uahcx_suptprt3_dw2_s {
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
	struct cvmx_uahcx_suptprt3_dw2_s      cn78xx;
	struct cvmx_uahcx_suptprt3_dw2_s      cn78xxp1;
};
typedef union cvmx_uahcx_suptprt3_dw2 cvmx_uahcx_suptprt3_dw2_t;

/**
 * cvmx_uahc#_suptprt3_dw3
 *
 * For information on this register, refer to the xHCI Specification, v1.1, section 7.2.
 *
 */
union cvmx_uahcx_suptprt3_dw3 {
	uint32_t u32;
	struct cvmx_uahcx_suptprt3_dw3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_5_31                : 27;
	uint32_t protslottype                 : 5;  /**< Protocol slot type. */
#else
	uint32_t protslottype                 : 5;
	uint32_t reserved_5_31                : 27;
#endif
	} s;
	struct cvmx_uahcx_suptprt3_dw3_s      cn78xx;
	struct cvmx_uahcx_suptprt3_dw3_s      cn78xxp1;
};
typedef union cvmx_uahcx_suptprt3_dw3 cvmx_uahcx_suptprt3_dw3_t;

/**
 * cvmx_uahc#_usbcmd
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.1.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_usbcmd {
	uint32_t u32;
	struct cvmx_uahcx_usbcmd_s {
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
	struct cvmx_uahcx_usbcmd_s            cn78xx;
	struct cvmx_uahcx_usbcmd_s            cn78xxp1;
};
typedef union cvmx_uahcx_usbcmd cvmx_uahcx_usbcmd_t;

/**
 * cvmx_uahc#_usblegctlsts
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.1.2. Note
 * that the SMI interrupts are not connected to anything in a CNXXXX configuration.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_usblegctlsts {
	uint32_t u32;
	struct cvmx_uahcx_usblegctlsts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t smi_on_bar                   : 1;  /**< System management interrupt on BAR. Never generated. */
	uint32_t smi_on_pci_command           : 1;  /**< System management interrupt on PCI command. Never generated. */
	uint32_t smi_on_os_ownership          : 1;  /**< System management interrupt on OS ownership change. This bit is set to 1 whenever
                                                         UAHC()_USBLEGSUP[HC_OS_OWNED_SEMAPHORES] transitions. */
	uint32_t reserved_21_28               : 8;
	uint32_t smi_on_hostsystemerr         : 1;  /**< System-management interrupt on host-system error. Shadow bit of UAHC()_USBSTS[HSE]. Refer
                                                         to
                                                         xHCI Section 5.4.2 for definition and effects of the events associated with this bit being
                                                         set to 1.
                                                         To clear this bit to a 0, system software must write a 1 to UAHC()_USBSTS[HSE]. */
	uint32_t reserved_17_19               : 3;
	uint32_t smi_on_event_interrupt       : 1;  /**< System-management interrupt on event interrupt. Shadow bit of UAHC()_USBSTS[EINT]. Refer
                                                         to
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
	struct cvmx_uahcx_usblegctlsts_s      cn78xx;
	struct cvmx_uahcx_usblegctlsts_s      cn78xxp1;
};
typedef union cvmx_uahcx_usblegctlsts cvmx_uahcx_usblegctlsts_t;

/**
 * cvmx_uahc#_usblegsup
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 7.1.1.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_usblegsup {
	uint32_t u32;
	struct cvmx_uahcx_usblegsup_s {
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
	struct cvmx_uahcx_usblegsup_s         cn78xx;
	struct cvmx_uahcx_usblegsup_s         cn78xxp1;
};
typedef union cvmx_uahcx_usblegsup cvmx_uahcx_usblegsup_t;

/**
 * cvmx_uahc#_usbsts
 *
 * For information on this register, refer to the xHCI Specification, v1.0, section 5.4.2.
 *
 * This register can be reset by IOI reset,
 * or UCTL()_CTL[UAHC_RST],
 * or UAHC()_GCTL[CORESOFTRESET],
 * or UAHC()_USBCMD[HCRST], or UAHC()_USBCMD[LHCRST].
 */
union cvmx_uahcx_usbsts {
	uint32_t u32;
	struct cvmx_uahcx_usbsts_s {
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
	struct cvmx_uahcx_usbsts_s            cn78xx;
	struct cvmx_uahcx_usbsts_s            cn78xxp1;
};
typedef union cvmx_uahcx_usbsts cvmx_uahcx_usbsts_t;

#endif
