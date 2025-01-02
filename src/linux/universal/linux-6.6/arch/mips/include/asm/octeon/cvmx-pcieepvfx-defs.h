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
 * cvmx-pcieepvfx-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon pcieepvfx.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_PCIEEPVFX_DEFS_H__
#define __CVMX_PCIEEPVFX_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG000(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG000(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000000ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG000(offset) (0x0000050000000000ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG001(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG001(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000004ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG001(offset) (0x0000050000000004ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG002(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG002(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000008ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG002(offset) (0x0000050000000008ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG003(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG003(%lu) is invalid on this chip\n", offset);
	return 0x000005000000000Cull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG003(offset) (0x000005000000000Cull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG004(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG004(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000010ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG004(offset) (0x0000050000000010ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG005(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG005(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000014ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG005(offset) (0x0000050000000014ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG006(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG006(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000018ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG006(offset) (0x0000050000000018ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG007(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG007(%lu) is invalid on this chip\n", offset);
	return 0x000005000000001Cull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG007(offset) (0x000005000000001Cull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG008(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG008(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000020ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG008(offset) (0x0000050000000020ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG009(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG009(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000024ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG009(offset) (0x0000050000000024ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG010(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG010(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000028ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG010(offset) (0x0000050000000028ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG011(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG011(%lu) is invalid on this chip\n", offset);
	return 0x000005000000002Cull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG011(offset) (0x000005000000002Cull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG012(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG012(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000030ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG012(offset) (0x0000050000000030ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG013(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG013(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000034ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG013(offset) (0x0000050000000034ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG015(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG015(%lu) is invalid on this chip\n", offset);
	return 0x000005000000003Cull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG015(offset) (0x000005000000003Cull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG028(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG028(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000070ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG028(offset) (0x0000050000000070ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG029(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG029(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000074ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG029(offset) (0x0000050000000074ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG030(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG030(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000078ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG030(offset) (0x0000050000000078ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG031(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG031(%lu) is invalid on this chip\n", offset);
	return 0x000005000000007Cull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG031(offset) (0x000005000000007Cull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG032(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG032(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000080ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG032(offset) (0x0000050000000080ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG037(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG037(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000094ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG037(offset) (0x0000050000000094ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG038(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG038(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000098ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG038(offset) (0x0000050000000098ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG039(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG039(%lu) is invalid on this chip\n", offset);
	return 0x000005000000009Cull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG039(offset) (0x000005000000009Cull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG040(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG040(%lu) is invalid on this chip\n", offset);
	return 0x00000500000000A0ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG040(offset) (0x00000500000000A0ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG044(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG044(%lu) is invalid on this chip\n", offset);
	return 0x00000500000000B0ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG044(offset) (0x00000500000000B0ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG045(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG045(%lu) is invalid on this chip\n", offset);
	return 0x00000500000000B4ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG045(offset) (0x00000500000000B4ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG046(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG046(%lu) is invalid on this chip\n", offset);
	return 0x00000500000000B8ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG046(offset) (0x00000500000000B8ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG064(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG064(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000100ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG064(offset) (0x0000050000000100ull + ((offset) & 7) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG065(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2) || (offset == 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG065(%lu) is invalid on this chip\n", offset);
	return 0x0000050000000104ull + ((offset) & 7) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG065(offset) (0x0000050000000104ull + ((offset) & 7) * 0x100000000ull)
#endif

/**
 * cvmx_pcieepvf#_cfg000
 *
 * This register contains the first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg000 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg000_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t devid                        : 16; /**< VF Device ID. */
	uint32_t vendid                       : 16; /**< VF Vendor ID. */
#else
	uint32_t vendid                       : 16;
	uint32_t devid                        : 16;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg000_s        cn73xx;
	struct cvmx_pcieepvfx_cfg000_s        cn78xx;
	struct cvmx_pcieepvfx_cfg000_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg000_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg000 cvmx_pcieepvfx_cfg000_t;

/**
 * cvmx_pcieepvf#_cfg001
 *
 * This register contains the second 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg001 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg001_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dpe                          : 1;  /**< Detected parity error. */
	uint32_t sse                          : 1;  /**< Signaled system error. */
	uint32_t rma                          : 1;  /**< Received master abort. */
	uint32_t rta                          : 1;  /**< Received target abort. */
	uint32_t sta                          : 1;  /**< Signaled target abort. */
	uint32_t devt                         : 2;  /**< DEVSEL timing. Not applicable for PCI Express. Hardwired to 0x0. */
	uint32_t mdpe                         : 1;  /**< Master data parity error. */
	uint32_t fbb                          : 1;  /**< Fast back-to-back capable. Not applicable for PCI Express. Hardwired to 0. */
	uint32_t reserved_22_22               : 1;
	uint32_t m66                          : 1;  /**< 66 MHz capable. Not applicable for PCI Express. Hardwired to 0. */
	uint32_t cl                           : 1;  /**< Capabilities list. Indicates presence of an extended capability item. Hardwired to 1. */
	uint32_t i_stat                       : 1;  /**< INTx status. Not applicable for SR-IOV.  Hardwired to 0. */
	uint32_t reserved_11_18               : 8;
	uint32_t i_dis                        : 1;  /**< VF read-only zero. */
	uint32_t fbbe                         : 1;  /**< Fast back-to-back transaction enable. Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t see                          : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG001[SEE]. */
	uint32_t ids_wcc                      : 1;  /**< IDSEL stepping/wait cycle control. Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t per                          : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG001[PER]. */
	uint32_t vps                          : 1;  /**< VGA palette snoop. Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t mwice                        : 1;  /**< Memory write and invalidate. Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t scse                         : 1;  /**< Special cycle enable. Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t me                           : 1;  /**< Bus master enable. If the VF tries to master the bus when this bit is not set,
                                                         the request is discarded. A interrupt will be generated setting
                                                         PEM()_DBG_INFO[BMD_E].
                                                         Transactions are dropped in the Client.  Non-posted transactions returns a SWI_RSP_ERROR
                                                         to SLI/DPI/NQM soon thereafter.
                                                         Bus master enable mimics the behavior of PEM()_FLR_PF0_VF_STOPREQ. */
	uint32_t msae                         : 1;  /**< VF read-only zero. */
	uint32_t isae                         : 1;  /**< VF read-only zero. */
#else
	uint32_t isae                         : 1;
	uint32_t msae                         : 1;
	uint32_t me                           : 1;
	uint32_t scse                         : 1;
	uint32_t mwice                        : 1;
	uint32_t vps                          : 1;
	uint32_t per                          : 1;
	uint32_t ids_wcc                      : 1;
	uint32_t see                          : 1;
	uint32_t fbbe                         : 1;
	uint32_t i_dis                        : 1;
	uint32_t reserved_11_18               : 8;
	uint32_t i_stat                       : 1;
	uint32_t cl                           : 1;
	uint32_t m66                          : 1;
	uint32_t reserved_22_22               : 1;
	uint32_t fbb                          : 1;
	uint32_t mdpe                         : 1;
	uint32_t devt                         : 2;
	uint32_t sta                          : 1;
	uint32_t rta                          : 1;
	uint32_t rma                          : 1;
	uint32_t sse                          : 1;
	uint32_t dpe                          : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg001_s        cn73xx;
	struct cvmx_pcieepvfx_cfg001_s        cn78xx;
	struct cvmx_pcieepvfx_cfg001_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg001_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg001 cvmx_pcieepvfx_cfg001_t;

/**
 * cvmx_pcieepvf#_cfg002
 *
 * This register contains the third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg002 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg002_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t bcc                          : 8;  /**< Read-only copy of the associated PF's PCIEEP()_CFG002[BCC]. */
	uint32_t sc                           : 8;  /**< Read-only copy of the associated PF's PCIEEP()_CFG002[SC]. */
	uint32_t pi                           : 8;  /**< Read-only copy of the associated PF's PCIEEP()_CFG002[PI]. */
	uint32_t rid                          : 8;  /**< Read-only copy of the associated PF's PCIEP()_CFG002[RID]. */
#else
	uint32_t rid                          : 8;
	uint32_t pi                           : 8;
	uint32_t sc                           : 8;
	uint32_t bcc                          : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg002_s        cn73xx;
	struct cvmx_pcieepvfx_cfg002_s        cn78xx;
	struct cvmx_pcieepvfx_cfg002_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg002_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg002 cvmx_pcieepvfx_cfg002_t;

/**
 * cvmx_pcieepvf#_cfg003
 *
 * This register contains the fourth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg003 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg003_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t bist                         : 8;  /**< The BIST register functions are not supported. All 8 bits of the BIST register are
                                                         hardwired to 0x0. */
	uint32_t mfd                          : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG003[MFD]. */
	uint32_t chf                          : 7;  /**< Configuration header format. Hardwired to 0x0 for type 0. */
	uint32_t lt                           : 8;  /**< Master latency timer. Not applicable for PCI Express, hardwired to 0x0. */
	uint32_t cls                          : 8;  /**< Read-only copy of the associated PF's PCIEEP()_CFG003[CLS].
                                                         The cache line size register is R/W for legacy compatibility purposes and
                                                         is not applicable to PCI Express device functionality. Writing to the cache line size
                                                         register does not impact functionality of the PCI Express bus. */
#else
	uint32_t cls                          : 8;
	uint32_t lt                           : 8;
	uint32_t chf                          : 7;
	uint32_t mfd                          : 1;
	uint32_t bist                         : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg003_s        cn73xx;
	struct cvmx_pcieepvfx_cfg003_s        cn78xx;
	struct cvmx_pcieepvfx_cfg003_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg003_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg003 cvmx_pcieepvfx_cfg003_t;

/**
 * cvmx_pcieepvf#_cfg004
 *
 * This register contains the fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg004 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg004_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg004_s        cn73xx;
	struct cvmx_pcieepvfx_cfg004_s        cn78xx;
	struct cvmx_pcieepvfx_cfg004_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg004_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg004 cvmx_pcieepvfx_cfg004_t;

/**
 * cvmx_pcieepvf#_cfg005
 *
 * This register contains the sixth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg005 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg005_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg005_s        cn73xx;
	struct cvmx_pcieepvfx_cfg005_s        cn78xx;
	struct cvmx_pcieepvfx_cfg005_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg005_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg005 cvmx_pcieepvfx_cfg005_t;

/**
 * cvmx_pcieepvf#_cfg006
 *
 * This register contains the seventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg006 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg006_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg006_s        cn73xx;
	struct cvmx_pcieepvfx_cfg006_s        cn78xx;
	struct cvmx_pcieepvfx_cfg006_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg006_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg006 cvmx_pcieepvfx_cfg006_t;

/**
 * cvmx_pcieepvf#_cfg007
 *
 * This register contains the eighth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg007 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg007_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg007_s        cn73xx;
	struct cvmx_pcieepvfx_cfg007_s        cn78xx;
	struct cvmx_pcieepvfx_cfg007_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg007_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg007 cvmx_pcieepvfx_cfg007_t;

/**
 * cvmx_pcieepvf#_cfg008
 *
 * This register contains the ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg008 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg008_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg008_s        cn73xx;
	struct cvmx_pcieepvfx_cfg008_s        cn78xx;
	struct cvmx_pcieepvfx_cfg008_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg008_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg008 cvmx_pcieepvfx_cfg008_t;

/**
 * cvmx_pcieepvf#_cfg009
 *
 * This register contains the tenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg009 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg009_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg009_s        cn73xx;
	struct cvmx_pcieepvfx_cfg009_s        cn78xx;
	struct cvmx_pcieepvfx_cfg009_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg009_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg009 cvmx_pcieepvfx_cfg009_t;

/**
 * cvmx_pcieepvf#_cfg010
 *
 * This register contains the eleventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg010 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg010_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t cisp                         : 32; /**< Read-only copy of the associated PF's PCIEEP()_CFG010[CISP]. */
#else
	uint32_t cisp                         : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg010_s        cn73xx;
	struct cvmx_pcieepvfx_cfg010_s        cn78xx;
	struct cvmx_pcieepvfx_cfg010_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg010_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg010 cvmx_pcieepvfx_cfg010_t;

/**
 * cvmx_pcieepvf#_cfg011
 *
 * This register contains the twelfth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg011 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg011_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ssid                         : 16; /**< Read-only copy of the associated PF's PCIEEP()_CFG011[SSID]. */
	uint32_t ssvid                        : 16; /**< Read-only copy of the associated PF's PCIEEP()_CFG011[SSVID]. */
#else
	uint32_t ssvid                        : 16;
	uint32_t ssid                         : 16;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg011_s        cn73xx;
	struct cvmx_pcieepvfx_cfg011_s        cn78xx;
	struct cvmx_pcieepvfx_cfg011_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg011_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg011 cvmx_pcieepvfx_cfg011_t;

/**
 * cvmx_pcieepvf#_cfg012
 *
 * This register contains the thirteenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg012 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg012_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t eraddr                       : 16; /**< Read-only copy of the associated PF's PCIEEP()_CFG012[ERADDR]. */
	uint32_t reserved_1_15                : 15;
	uint32_t er_en                        : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG012[ER_EN]. */
#else
	uint32_t er_en                        : 1;
	uint32_t reserved_1_15                : 15;
	uint32_t eraddr                       : 16;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg012_s        cn73xx;
	struct cvmx_pcieepvfx_cfg012_s        cn78xx;
	struct cvmx_pcieepvfx_cfg012_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg012_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg012 cvmx_pcieepvfx_cfg012_t;

/**
 * cvmx_pcieepvf#_cfg013
 *
 * This register contains the fourteenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg013 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg013_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_8_31                : 24;
	uint32_t cp                           : 8;  /**< Next capability pointer. Points to PCI Express capabilities. */
#else
	uint32_t cp                           : 8;
	uint32_t reserved_8_31                : 24;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg013_s        cn73xx;
	struct cvmx_pcieepvfx_cfg013_s        cn78xx;
	struct cvmx_pcieepvfx_cfg013_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg013_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg013 cvmx_pcieepvfx_cfg013_t;

/**
 * cvmx_pcieepvf#_cfg015
 *
 * This register contains the sixteenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg015 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg015_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ml                           : 8;  /**< VF's read-only zeros. */
	uint32_t mg                           : 8;  /**< VF's read-only zeros. */
	uint32_t inta                         : 8;  /**< VF's read-only zeros. */
	uint32_t il                           : 8;  /**< VF's read-only zeros. */
#else
	uint32_t il                           : 8;
	uint32_t inta                         : 8;
	uint32_t mg                           : 8;
	uint32_t ml                           : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg015_s        cn73xx;
	struct cvmx_pcieepvfx_cfg015_s        cn78xx;
	struct cvmx_pcieepvfx_cfg015_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg015_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg015 cvmx_pcieepvfx_cfg015_t;

/**
 * cvmx_pcieepvf#_cfg028
 *
 * This register contains the twenty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg028 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg028_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t imn                          : 5;  /**< Read-only copy of the associated PF's PCIEEP()_CFG028[IMN]. */
	uint32_t si                           : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG028[SI]. */
	uint32_t dpt                          : 4;  /**< Read-only copy of the associated PF's PCIEEP()_CFG028[DPT]. */
	uint32_t pciecv                       : 4;  /**< Read-only copy of the associated PF's PCIEEP()_CFG028[PCIECV]. */
	uint32_t ncp                          : 8;  /**< Next capability pointer. Points to the MSI-X capabilities by default. */
	uint32_t pcieid                       : 8;  /**< PCI Express capability ID. */
#else
	uint32_t pcieid                       : 8;
	uint32_t ncp                          : 8;
	uint32_t pciecv                       : 4;
	uint32_t dpt                          : 4;
	uint32_t si                           : 1;
	uint32_t imn                          : 5;
	uint32_t reserved_30_31               : 2;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg028_s        cn73xx;
	struct cvmx_pcieepvfx_cfg028_s        cn78xx;
	struct cvmx_pcieepvfx_cfg028_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg028_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg028 cvmx_pcieepvfx_cfg028_t;

/**
 * cvmx_pcieepvf#_cfg029
 *
 * This register contains the thirtieth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg029 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg029_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_29_31               : 3;
	uint32_t flr_cap                      : 1;  /**< Function level reset capability. Set to 1 for SR-IOV core. */
	uint32_t cspls                        : 2;  /**< Read-only copy of the associated PF's PCIEEP()_CFG029[CSPLS]. */
	uint32_t csplv                        : 8;  /**< Read-only copy of the associated PF's PCIEEP()_CFG029[CSPLV]. */
	uint32_t reserved_16_17               : 2;
	uint32_t rber                         : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG029[RBER]. */
	uint32_t reserved_12_14               : 3;
	uint32_t el1al                        : 3;  /**< Read-only copy of the associated PF's PCIEEP()_CFG029[EL1AL]. */
	uint32_t el0al                        : 3;  /**< Read-only copy of the associated PF's PCIEEP()_CFG029[EL0AL]. */
	uint32_t etfs                         : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG029[ETFS]. */
	uint32_t pfs                          : 2;  /**< Read-only copy of the associated PF's PCIEEP()_CFG029[PFS]. */
	uint32_t mpss                         : 3;  /**< Read-only copy of the associated PF's PCIEEP()_CFG029[MPSS]. */
#else
	uint32_t mpss                         : 3;
	uint32_t pfs                          : 2;
	uint32_t etfs                         : 1;
	uint32_t el0al                        : 3;
	uint32_t el1al                        : 3;
	uint32_t reserved_12_14               : 3;
	uint32_t rber                         : 1;
	uint32_t reserved_16_17               : 2;
	uint32_t csplv                        : 8;
	uint32_t cspls                        : 2;
	uint32_t flr_cap                      : 1;
	uint32_t reserved_29_31               : 3;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg029_s        cn73xx;
	struct cvmx_pcieepvfx_cfg029_s        cn78xx;
	struct cvmx_pcieepvfx_cfg029_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg029_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg029 cvmx_pcieepvfx_cfg029_t;

/**
 * cvmx_pcieepvf#_cfg030
 *
 * This register contains the thirty-first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg030 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg030_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_22_31               : 10;
	uint32_t tp                           : 1;  /**< Transaction pending. Set to 1 when nonposted requests are not yet completed and set to 0
                                                         when they are completed. */
	uint32_t ap_d                         : 1;  /**< VF's read-only zeros. */
	uint32_t ur_d                         : 1;  /**< Unsupported request detected. Errors are logged in this register regardless of whether or
                                                         not error reporting is enabled in the device control register. [UR_D] occurs when we
                                                         receive
                                                         something unsupported. Unsupported requests are nonfatal errors, so [UR_D] should cause
                                                         [NFE_D]. Receiving a vendor-defined message should cause an unsupported request. */
	uint32_t fe_d                         : 1;  /**< Fatal error detected. Errors are logged in this register regardless of whether or not
                                                         error reporting is enabled in the device control register. This field is set if we receive
                                                         any of the errors in PCIEEPVF()_CFG066 that has a severity set to fatal. Malformed TLPs
                                                         generally fit into this category. */
	uint32_t nfe_d                        : 1;  /**< Nonfatal error detected. Errors are logged in this register regardless of whether or not
                                                         error reporting is enabled in the device control register. This field is set if we receive
                                                         any of the errors in PCIEEPVF()_CFG066 that has a severity set to nonfatal and does not
                                                         meet advisory nonfatal criteria, which most poisoned TLPs should. */
	uint32_t ce_d                         : 1;  /**< Correctable error detected. Errors are logged in this register regardless of whether or
                                                         not error reporting is enabled in the device control register. This field is set if we
                                                         receive any of the errors in PCIEEPVF()_CFG068, for example a replay-timer timeout.
                                                         Also, it can be set if we get any of the errors in PCIEEPVF()_CFG066 that has a severity
                                                         set to Nonfatal and meets the Advisory Nonfatal criteria, which most ECRC errors should. */
	uint32_t i_flr                        : 1;  /**< Initiate function level reset when written to one.
                                                         [I_FLR] must not be written to one via the indirect PEM()_CFG_WR. It should only ever
                                                         be written to one via a direct PCIe access. */
	uint32_t mrrs                         : 3;  /**< Read-only copy of the associated PF's PCIEEP()_CFG030[MRRS]. */
	uint32_t ns_en                        : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG030[NS_EN]. */
	uint32_t ap_en                        : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG030[AP_EN]. */
	uint32_t pf_en                        : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG030[PF_EN]. */
	uint32_t etf_en                       : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG030[ETF_EN]. */
	uint32_t mps                          : 3;  /**< Read-only copy of the associated PF's PCIEEP()_CFG030[MPS]. */
	uint32_t ro_en                        : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG030[RO_EN]. */
	uint32_t ur_en                        : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG030[UR_EN]. */
	uint32_t fe_en                        : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG030[FE_EN]. */
	uint32_t nfe_en                       : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG030[NFE_EN]. */
	uint32_t ce_en                        : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG030[CE_EN]. */
#else
	uint32_t ce_en                        : 1;
	uint32_t nfe_en                       : 1;
	uint32_t fe_en                        : 1;
	uint32_t ur_en                        : 1;
	uint32_t ro_en                        : 1;
	uint32_t mps                          : 3;
	uint32_t etf_en                       : 1;
	uint32_t pf_en                        : 1;
	uint32_t ap_en                        : 1;
	uint32_t ns_en                        : 1;
	uint32_t mrrs                         : 3;
	uint32_t i_flr                        : 1;
	uint32_t ce_d                         : 1;
	uint32_t nfe_d                        : 1;
	uint32_t fe_d                         : 1;
	uint32_t ur_d                         : 1;
	uint32_t ap_d                         : 1;
	uint32_t tp                           : 1;
	uint32_t reserved_22_31               : 10;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg030_s        cn73xx;
	struct cvmx_pcieepvfx_cfg030_s        cn78xx;
	struct cvmx_pcieepvfx_cfg030_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg030_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg030 cvmx_pcieepvfx_cfg030_t;

/**
 * cvmx_pcieepvf#_cfg031
 *
 * This register contains the thirty-second 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg031 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg031_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pnum                         : 8;  /**< Read-only copy of the associated PF's PCIEEP()_CFG031[PNUM]. */
	uint32_t reserved_22_23               : 2;
	uint32_t lbnc                         : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG031[LBNC]. */
	uint32_t dllarc                       : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG031[DLLARC]. */
	uint32_t sderc                        : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG031[SDERC]. */
	uint32_t cpm                          : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG031[CPM]. */
	uint32_t l1el                         : 3;  /**< Read-only copy of the associated PF's PCIEEP()_CFG031[L1EL]. */
	uint32_t l0el                         : 3;  /**< Read-only copy of the associated PF's PCIEEP()_CFG031[L0EL]. */
	uint32_t aslpms                       : 2;  /**< Read-only copy of the associated PF's PCIEEP()_CFG031[ASLPMS]. */
	uint32_t mlw                          : 6;  /**< Read-only copy of the associated PF's PCIEEP()_CFG031[MLW]. */
	uint32_t mls                          : 4;  /**< Read-only copy of the associated PF's PCIEEP()_CFG031[MLS]. */
#else
	uint32_t mls                          : 4;
	uint32_t mlw                          : 6;
	uint32_t aslpms                       : 2;
	uint32_t l0el                         : 3;
	uint32_t l1el                         : 3;
	uint32_t cpm                          : 1;
	uint32_t sderc                        : 1;
	uint32_t dllarc                       : 1;
	uint32_t lbnc                         : 1;
	uint32_t reserved_22_23               : 2;
	uint32_t pnum                         : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg031_s        cn73xx;
	struct cvmx_pcieepvfx_cfg031_s        cn78xx;
	struct cvmx_pcieepvfx_cfg031_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg031_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg031 cvmx_pcieepvfx_cfg031_t;

/**
 * cvmx_pcieepvf#_cfg032
 *
 * This register contains the thirty-third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg032 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg032_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_12_31               : 20;
	uint32_t lab_int_enb                  : 1;  /**< Link autonomous bandwidth interrupt enable. This bit is not applicable and is reserved for
                                                         endpoints. */
	uint32_t lbm_int_enb                  : 1;  /**< Link bandwidth management interrupt enable. This bit is not applicable and is reserved for
                                                         endpoints. */
	uint32_t hawd                         : 1;  /**< Hardware autonomous width disable (not supported). */
	uint32_t ecpm                         : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG032[ECPM]. */
	uint32_t es                           : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG032[ES]. */
	uint32_t ccc                          : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG032[CCC]. */
	uint32_t rl                           : 1;  /**< Retrain link. Not applicable for an upstream port or endpoint device. Hardwired to 0. */
	uint32_t ld                           : 1;  /**< Link disable. Not applicable for an upstream port or endpoint device. Hardwired to 0. */
	uint32_t rcb                          : 1;  /**< VF's read-only zeros. */
	uint32_t reserved_2_2                 : 1;
	uint32_t aslpc                        : 2;  /**< Read-only copy of the associated PF's PCIEEP()_CFG032[ASLPC]. */
#else
	uint32_t aslpc                        : 2;
	uint32_t reserved_2_2                 : 1;
	uint32_t rcb                          : 1;
	uint32_t ld                           : 1;
	uint32_t rl                           : 1;
	uint32_t ccc                          : 1;
	uint32_t es                           : 1;
	uint32_t ecpm                         : 1;
	uint32_t hawd                         : 1;
	uint32_t lbm_int_enb                  : 1;
	uint32_t lab_int_enb                  : 1;
	uint32_t reserved_12_31               : 20;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg032_s        cn73xx;
	struct cvmx_pcieepvfx_cfg032_s        cn78xx;
	struct cvmx_pcieepvfx_cfg032_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg032_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg032 cvmx_pcieepvfx_cfg032_t;

/**
 * cvmx_pcieepvf#_cfg037
 *
 * This register contains the thirty-eighth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg037 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg037_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t meetp                        : 2;  /**< Read-only copy of the associated PF's PCIEEP()_CFG037[MEETP]. */
	uint32_t eetps                        : 1;  /**< End-end TLP prefix supported (not supported). */
	uint32_t effs                         : 1;  /**< Extended fmt field supported (not supported). */
	uint32_t obffs                        : 2;  /**< Optimized buffer flush fill (OBFF) supported (not supported). */
	uint32_t reserved_14_17               : 4;
	uint32_t tphs                         : 2;  /**< TPH completer supported (not supported). */
	uint32_t ltrs                         : 1;  /**< Latency tolerance reporting (LTR) mechanism supported (not supported). */
	uint32_t noroprpr                     : 1;  /**< No RO-enabled PR-PR passing. (This bit applies to RCs.) */
	uint32_t atom128s                     : 1;  /**< 128-bit AtomicOp supported (not supported). */
	uint32_t atom64s                      : 1;  /**< 64-bit AtomicOp supported.
                                                         Note that inbound AtomicOps targeting BAR0 are not supported and are dropped as an
                                                         unsupported request.
                                                         Since VF's are tied to BAR0, all AtomicOp's will be dropped as unsupported requests.
                                                         ATOM64S is set as an inherited attribute from the PF. */
	uint32_t atom32s                      : 1;  /**< 32-bit AtomicOp supported.
                                                         Note that inbound AtomicOps targeting BAR0 are not supported and are dropped as an
                                                         unsupported request.
                                                         Since VF's are tied to BAR0, all AtomicOp's will be dropped as unsupported requests.
                                                         ATOM64S is set as an inherited attribute from the PF. */
	uint32_t atom_ops                     : 1;  /**< AtomicOp routing supported (not applicable for EP). */
	uint32_t ari                          : 1;  /**< Alternate routing ID forwarding supported (not applicable for EP). */
	uint32_t ctds                         : 1;  /**< Completion timeout disable supported. */
	uint32_t ctrs                         : 4;  /**< Completion timeout ranges supported. */
#else
	uint32_t ctrs                         : 4;
	uint32_t ctds                         : 1;
	uint32_t ari                          : 1;
	uint32_t atom_ops                     : 1;
	uint32_t atom32s                      : 1;
	uint32_t atom64s                      : 1;
	uint32_t atom128s                     : 1;
	uint32_t noroprpr                     : 1;
	uint32_t ltrs                         : 1;
	uint32_t tphs                         : 2;
	uint32_t reserved_14_17               : 4;
	uint32_t obffs                        : 2;
	uint32_t effs                         : 1;
	uint32_t eetps                        : 1;
	uint32_t meetp                        : 2;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg037_s        cn73xx;
	struct cvmx_pcieepvfx_cfg037_s        cn78xx;
	struct cvmx_pcieepvfx_cfg037_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg037_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg037 cvmx_pcieepvfx_cfg037_t;

/**
 * cvmx_pcieepvf#_cfg038
 *
 * This register contains the thirty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg038 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg038_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t eetpb                        : 1;  /**< End-end TLP prefix blocking (not supported). */
	uint32_t obffe                        : 2;  /**< Optimized buffer flush fill (OBFF) enable (not supported). */
	uint32_t reserved_10_12               : 3;
	uint32_t id0_cp                       : 1;  /**< ID based ordering completion enable (not supported). */
	uint32_t id0_rq                       : 1;  /**< ID based ordering request enable (not supported). */
	uint32_t reserved_7_7                 : 1;
	uint32_t atom_op                      : 1;  /**< Read-only copy of the associated PF's PCIEP()_CFG038[ATOM_OP]. */
	uint32_t ari                          : 1;  /**< Alternate routing ID forwarding supported (not supported). */
	uint32_t ctd                          : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG038[CTD]. */
	uint32_t ctv                          : 4;  /**< Read-only copy of the associated PF's PCIEEP()_CFG038[CTV]. */
#else
	uint32_t ctv                          : 4;
	uint32_t ctd                          : 1;
	uint32_t ari                          : 1;
	uint32_t atom_op                      : 1;
	uint32_t reserved_7_7                 : 1;
	uint32_t id0_rq                       : 1;
	uint32_t id0_cp                       : 1;
	uint32_t reserved_10_12               : 3;
	uint32_t obffe                        : 2;
	uint32_t eetpb                        : 1;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg038_s        cn73xx;
	struct cvmx_pcieepvfx_cfg038_s        cn78xx;
	struct cvmx_pcieepvfx_cfg038_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg038_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg038 cvmx_pcieepvfx_cfg038_t;

/**
 * cvmx_pcieepvf#_cfg039
 *
 * This register contains the fortieth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg039 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg039_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_9_31                : 23;
	uint32_t cls                          : 1;  /**< Crosslink supported. */
	uint32_t slsv                         : 7;  /**< Read-only copy of the associated PF's PCIEEP()_CFG039[SLSV]. */
	uint32_t reserved_0_0                 : 1;
#else
	uint32_t reserved_0_0                 : 1;
	uint32_t slsv                         : 7;
	uint32_t cls                          : 1;
	uint32_t reserved_9_31                : 23;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg039_s        cn73xx;
	struct cvmx_pcieepvfx_cfg039_s        cn78xx;
	struct cvmx_pcieepvfx_cfg039_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg039_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg039 cvmx_pcieepvfx_cfg039_t;

/**
 * cvmx_pcieepvf#_cfg040
 *
 * This register contains the forty-first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg040 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg040_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_17_31               : 15;
	uint32_t cdl                          : 1;  /**< Read-only copy of the associated PF's PCIEEP()_CFG040[CDL]. */
	uint32_t reserved_13_15               : 3;
	uint32_t cde                          : 1;  /**< VF's read-only zeros. */
	uint32_t csos                         : 1;  /**< VF's read-only zeros. */
	uint32_t emc                          : 1;  /**< VF's read-only zeros. */
	uint32_t tm                           : 3;  /**< VF's read-only zeros. */
	uint32_t sde                          : 1;  /**< VF's read-only zeros. */
	uint32_t hasd                         : 1;  /**< VF's read-only zeros. */
	uint32_t ec                           : 1;  /**< VF's read-only zeros. */
	uint32_t tls                          : 4;  /**< VF's read-only zeros. */
#else
	uint32_t tls                          : 4;
	uint32_t ec                           : 1;
	uint32_t hasd                         : 1;
	uint32_t sde                          : 1;
	uint32_t tm                           : 3;
	uint32_t emc                          : 1;
	uint32_t csos                         : 1;
	uint32_t cde                          : 1;
	uint32_t reserved_13_15               : 3;
	uint32_t cdl                          : 1;
	uint32_t reserved_17_31               : 15;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg040_s        cn73xx;
	struct cvmx_pcieepvfx_cfg040_s        cn78xx;
	struct cvmx_pcieepvfx_cfg040_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg040_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg040 cvmx_pcieepvfx_cfg040_t;

/**
 * cvmx_pcieepvf#_cfg044
 *
 * This register contains the forty-fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg044 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg044_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msixen                       : 1;  /**< MSI-X enable. */
	uint32_t funm                         : 1;  /**< Function mask.
                                                         0 = Each vectors mask bit determines whether the vector is masked or not.
                                                         1 = All vectors associated with the function are masked, regardless of their respective
                                                         per-vector mask bits. */
	uint32_t reserved_27_29               : 3;
	uint32_t msixts                       : 11; /**< MSI-X table size encoded as (table size - 1).
                                                         This field is writable through PEM()_CFG_WR when PEM()_CFG_WR[ADDR[31]] is set. */
	uint32_t ncp                          : 8;  /**< Next capability pointer. */
	uint32_t msixcid                      : 8;  /**< MSI-X capability ID. */
#else
	uint32_t msixcid                      : 8;
	uint32_t ncp                          : 8;
	uint32_t msixts                       : 11;
	uint32_t reserved_27_29               : 3;
	uint32_t funm                         : 1;
	uint32_t msixen                       : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg044_s        cn73xx;
	struct cvmx_pcieepvfx_cfg044_s        cn78xx;
	struct cvmx_pcieepvfx_cfg044_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg044_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg044 cvmx_pcieepvfx_cfg044_t;

/**
 * cvmx_pcieepvf#_cfg045
 *
 * This register contains the forty-sixth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg045 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg045_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msixtoffs                    : 29; /**< Read-only copy of the associated PF's PCIEEP()_CFG045[MSIXTS]. */
	uint32_t msixtbir                     : 3;  /**< Read-only copy of the associated PF's PCIEEP()_CFG045[MSIXTBIR]. */
#else
	uint32_t msixtbir                     : 3;
	uint32_t msixtoffs                    : 29;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg045_s        cn73xx;
	struct cvmx_pcieepvfx_cfg045_s        cn78xx;
	struct cvmx_pcieepvfx_cfg045_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg045_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg045 cvmx_pcieepvfx_cfg045_t;

/**
 * cvmx_pcieepvf#_cfg046
 *
 * This register contains the forty-seventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg046 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg046_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msixpoffs                    : 29; /**< Read-only copy of the associated PF's PCIEP()_CFG046[MSIXPOFFS]. */
	uint32_t msixpbir                     : 3;  /**< Read-only copy of the associated PF's PCIEEP()_CFG046[MSIXPBIR]. */
#else
	uint32_t msixpbir                     : 3;
	uint32_t msixpoffs                    : 29;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg046_s        cn73xx;
	struct cvmx_pcieepvfx_cfg046_s        cn78xx;
	struct cvmx_pcieepvfx_cfg046_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg046_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg046 cvmx_pcieepvfx_cfg046_t;

/**
 * cvmx_pcieepvf#_cfg064
 *
 * This register contains the sixty-fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg064 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg064_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next capability offset. */
	uint32_t cv                           : 4;  /**< Capability version. */
	uint32_t ariid                        : 16; /**< PCIE Express extended capability */
#else
	uint32_t ariid                        : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg064_s        cn73xx;
	struct cvmx_pcieepvfx_cfg064_s        cn78xx;
	struct cvmx_pcieepvfx_cfg064_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg064_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg064 cvmx_pcieepvfx_cfg064_t;

/**
 * cvmx_pcieepvf#_cfg065
 *
 * This register contains the sixty-sixth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepvfx_cfg065 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg065_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t fg                           : 4;  /**< Function group. */
	uint32_t reserved_18_19               : 2;
	uint32_t acsfge                       : 1;  /**< ACS function groups enable (A). */
	uint32_t mfvcfge                      : 1;  /**< MFVC function groups enable (M). */
	uint32_t reserved_2_15                : 14;
	uint32_t acsfgc                       : 1;  /**< ACS function groups capability. */
	uint32_t mfvcfgc                      : 1;  /**< MFVC function groups capability. */
#else
	uint32_t mfvcfgc                      : 1;
	uint32_t acsfgc                       : 1;
	uint32_t reserved_2_15                : 14;
	uint32_t mfvcfge                      : 1;
	uint32_t acsfge                       : 1;
	uint32_t reserved_18_19               : 2;
	uint32_t fg                           : 4;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg065_s        cn73xx;
	struct cvmx_pcieepvfx_cfg065_s        cn78xx;
	struct cvmx_pcieepvfx_cfg065_s        cn78xxp1;
	struct cvmx_pcieepvfx_cfg065_s        cnf75xx;
};
typedef union cvmx_pcieepvfx_cfg065 cvmx_pcieepvfx_cfg065_t;

#endif
