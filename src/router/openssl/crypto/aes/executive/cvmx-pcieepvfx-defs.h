/***********************license start***************
 * Copyright (c) 2003-2013  Cavium Inc. (support@cavium.com). All rights
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
static inline uint64_t CVMX_PCIEEPVFX_CFG000(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG000(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000000ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG000(block_id) (0x0000050000000000ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG001(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG001(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000004ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG001(block_id) (0x0000050000000004ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG002(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG002(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000008ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG002(block_id) (0x0000050000000008ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG003(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG003(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000000Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG003(block_id) (0x000005000000000Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG004(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG004(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000010ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG004(block_id) (0x0000050000000010ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG005(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG005(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000014ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG005(block_id) (0x0000050000000014ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG006(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG006(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000018ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG006(block_id) (0x0000050000000018ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG007(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG007(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000001Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG007(block_id) (0x000005000000001Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG008(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG008(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000020ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG008(block_id) (0x0000050000000020ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG009(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG009(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000024ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG009(block_id) (0x0000050000000024ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG010(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG010(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000028ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG010(block_id) (0x0000050000000028ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG011(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG011(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000002Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG011(block_id) (0x000005000000002Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG012(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG012(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000030ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG012(block_id) (0x0000050000000030ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG013(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG013(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000034ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG013(block_id) (0x0000050000000034ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG015(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG015(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000003Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG015(block_id) (0x000005000000003Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG028(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG028(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000070ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG028(block_id) (0x0000050000000070ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG029(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG029(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000074ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG029(block_id) (0x0000050000000074ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG030(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG030(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000078ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG030(block_id) (0x0000050000000078ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG031(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG031(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000007Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG031(block_id) (0x000005000000007Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG032(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG032(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000080ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG032(block_id) (0x0000050000000080ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG037(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG037(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000094ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG037(block_id) (0x0000050000000094ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG038(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG038(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000098ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG038(block_id) (0x0000050000000098ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG039(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG039(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000009Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG039(block_id) (0x000005000000009Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG040(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG040(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000000A0ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG040(block_id) (0x00000500000000A0ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG044(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG044(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000000B0ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG044(block_id) (0x00000500000000B0ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG045(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG045(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000000B4ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG045(block_id) (0x00000500000000B4ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG046(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG046(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000000B8ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG046(block_id) (0x00000500000000B8ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG048(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG048(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000000C0ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG048(block_id) (0x00000500000000C0ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG049(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG049(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000000C4ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG049(block_id) (0x00000500000000C4ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG064(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG064(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000100ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG064(block_id) (0x0000050000000100ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG082(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG082(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000148ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG082(block_id) (0x0000050000000148ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG083(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG083(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000014Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG083(block_id) (0x000005000000014Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG448(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG448(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000700ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG448(block_id) (0x0000050000000700ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG449(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG449(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000704ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG449(block_id) (0x0000050000000704ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG450(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG450(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000708ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG450(block_id) (0x0000050000000708ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG451(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG451(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000070Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG451(block_id) (0x000005000000070Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG452(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG452(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000710ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG452(block_id) (0x0000050000000710ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG453(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG453(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000714ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG453(block_id) (0x0000050000000714ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG454(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG454(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000718ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG454(block_id) (0x0000050000000718ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG455(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG455(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000071Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG455(block_id) (0x000005000000071Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG456(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG456(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000720ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG456(block_id) (0x0000050000000720ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG458(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG458(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000728ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG458(block_id) (0x0000050000000728ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG459(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG459(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000072Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG459(block_id) (0x000005000000072Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG460(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG460(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000730ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG460(block_id) (0x0000050000000730ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG461(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG461(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000734ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG461(block_id) (0x0000050000000734ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG462(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG462(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000738ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG462(block_id) (0x0000050000000738ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG463(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG463(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000073Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG463(block_id) (0x000005000000073Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG464(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG464(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000740ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG464(block_id) (0x0000050000000740ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG465(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG465(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000744ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG465(block_id) (0x0000050000000744ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG466(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG466(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000748ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG466(block_id) (0x0000050000000748ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG467(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG467(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000074Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG467(block_id) (0x000005000000074Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG468(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG468(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000750ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG468(block_id) (0x0000050000000750ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG490(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG490(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000007A8ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG490(block_id) (0x00000500000007A8ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG491(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG491(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000007ACull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG491(block_id) (0x00000500000007ACull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG492(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG492(%lu) is invalid on this chip\n", block_id);
	return 0x00000500000007B0ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG492(block_id) (0x00000500000007B0ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG515(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG515(%lu) is invalid on this chip\n", block_id);
	return 0x000005000000080Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG515(block_id) (0x000005000000080Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG516(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG516(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000810ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG516(block_id) (0x0000050000000810ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPVFX_CFG517(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPVFX_CFG517(%lu) is invalid on this chip\n", block_id);
	return 0x0000050000000814ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPVFX_CFG517(block_id) (0x0000050000000814ull + ((block_id) & 3) * 0x100000000ull)
#endif

/**
 * cvmx_pcieepvf#_cfg000
 *
 * PCIE_CFG000 = First 32-bits of PCIE type 0 config space (Device ID and Vendor ID Register)
 *
 */
union cvmx_pcieepvfx_cfg000 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg000_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t devid                        : 16; /**< Device ID */
	uint32_t vendid                       : 16; /**< Vendor ID */
#else
	uint32_t vendid                       : 16;
	uint32_t devid                        : 16;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg000_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg000 cvmx_pcieepvfx_cfg000_t;

/**
 * cvmx_pcieepvf#_cfg001
 *
 * PCIE_CFG001 = Second 32-bits of PCIE type 0 config space (Command/Status Register)
 *
 */
union cvmx_pcieepvfx_cfg001 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg001_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dpe                          : 1;  /**< Detected Parity Error */
	uint32_t sse                          : 1;  /**< Signaled System Error */
	uint32_t rma                          : 1;  /**< Received Master Abort */
	uint32_t rta                          : 1;  /**< Received Target Abort */
	uint32_t sta                          : 1;  /**< Signaled Target Abort */
	uint32_t devt                         : 2;  /**< DEVSEL Timing
                                                         Not applicable for PCI Express. Hardwired to 0. */
	uint32_t mdpe                         : 1;  /**< Master Data Parity Error */
	uint32_t fbb                          : 1;  /**< Fast Back-to-Back Capable
                                                         Not applicable for PCI Express. Hardwired to 0. */
	uint32_t reserved_22_22               : 1;
	uint32_t m66                          : 1;  /**< 66 MHz Capable
                                                         Not applicable for PCI Express. Hardwired to 0. */
	uint32_t cl                           : 1;  /**< Capabilities List
                                                         Indicates presence of an extended capability item.
                                                         Hardwired to 1. */
	uint32_t i_stat                       : 1;  /**< INTx Status */
	uint32_t reserved_11_18               : 8;
	uint32_t i_dis                        : 1;  /**< INTx Assertion Disable */
	uint32_t fbbe                         : 1;  /**< Fast Back-to-Back Enable
                                                         Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t see                          : 1;  /**< SERR# Enable */
	uint32_t ids_wcc                      : 1;  /**< IDSEL Stepping/Wait Cycle Control
                                                         Not applicable for PCI Express. Must be hardwired to 0 */
	uint32_t per                          : 1;  /**< Parity Error Response */
	uint32_t vps                          : 1;  /**< VGA Palette Snoop
                                                         Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t mwice                        : 1;  /**< Memory Write and Invalidate
                                                         Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t scse                         : 1;  /**< Special Cycle Enable
                                                         Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t me                           : 1;  /**< Bus Master Enable */
	uint32_t msae                         : 1;  /**< Memory Space Enable */
	uint32_t isae                         : 1;  /**< I/O Space Enable */
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
	struct cvmx_pcieepvfx_cfg001_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg001 cvmx_pcieepvfx_cfg001_t;

/**
 * cvmx_pcieepvf#_cfg002
 *
 * PCIE_CFG002 = Third 32-bits of PCIE type 0 config space (Revision ID/Class Code Register)
 *
 */
union cvmx_pcieepvfx_cfg002 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg002_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t bcc                          : 8;  /**< Base Class Code */
	uint32_t sc                           : 8;  /**< Subclass Code */
	uint32_t pi                           : 8;  /**< Programming Interface */
	uint32_t rid                          : 8;  /**< Revision ID */
#else
	uint32_t rid                          : 8;
	uint32_t pi                           : 8;
	uint32_t sc                           : 8;
	uint32_t bcc                          : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg002_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg002 cvmx_pcieepvfx_cfg002_t;

/**
 * cvmx_pcieepvf#_cfg003
 *
 * PCIE_CFG003 = Fourth 32-bits of PCIE type 0 config space (Cache Line Size/Master Latency
 * Timer/Header Type Register/BIST Register)
 */
union cvmx_pcieepvfx_cfg003 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg003_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t bist                         : 8;  /**< The BIST register functions are not supported.
                                                         All 8 bits of the BIST register are hardwired to 0. */
	uint32_t mfd                          : 1;  /**< Multi Function Device */
	uint32_t chf                          : 7;  /**< Configuration Header Format
                                                         Hardwired to 0 for type 0. */
	uint32_t lt                           : 8;  /**< Master Latency Timer
                                                         Not applicable for PCI Express, hardwired to 0. */
	uint32_t cls                          : 8;  /**< Cache Line Size
                                                         The Cache Line Size register is RW for legacy compatibility
                                                         purposes and is not applicable to PCI Express device
                                                         functionality. */
#else
	uint32_t cls                          : 8;
	uint32_t lt                           : 8;
	uint32_t chf                          : 7;
	uint32_t mfd                          : 1;
	uint32_t bist                         : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg003_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg003 cvmx_pcieepvfx_cfg003_t;

/**
 * cvmx_pcieepvf#_cfg004
 *
 * PCIE_CFG004 = Fifth 32-bits of PCIE type 0 config space (Base Address Register 0 - Low)
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
	struct cvmx_pcieepvfx_cfg004_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg004 cvmx_pcieepvfx_cfg004_t;

/**
 * cvmx_pcieepvf#_cfg005
 *
 * PCIE_CFG005 = Sixth 32-bits of PCIE type 0 config space (Base Address Register 0 - High)
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
	struct cvmx_pcieepvfx_cfg005_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg005 cvmx_pcieepvfx_cfg005_t;

/**
 * cvmx_pcieepvf#_cfg006
 *
 * PCIE_CFG006 = Seventh 32-bits of PCIE type 0 config space (Base Address Register 1 - Low)
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
	struct cvmx_pcieepvfx_cfg006_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg006 cvmx_pcieepvfx_cfg006_t;

/**
 * cvmx_pcieepvf#_cfg007
 *
 * PCIE_CFG007 = Eighth 32-bits of PCIE type 0 config space (Base Address Register 1 - High)
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
	struct cvmx_pcieepvfx_cfg007_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg007 cvmx_pcieepvfx_cfg007_t;

/**
 * cvmx_pcieepvf#_cfg008
 *
 * PCIE_CFG008 = Ninth 32-bits of PCIE type 0 config space (Base Address Register 2 - Low)
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
	struct cvmx_pcieepvfx_cfg008_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg008 cvmx_pcieepvfx_cfg008_t;

/**
 * cvmx_pcieepvf#_cfg009
 *
 * PCIE_CFG009 = Tenth 32-bits of PCIE type 0 config space (Base Address Register 2 - High)
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
	struct cvmx_pcieepvfx_cfg009_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg009 cvmx_pcieepvfx_cfg009_t;

/**
 * cvmx_pcieepvf#_cfg010
 *
 * PCIE_CFG010 = Eleventh 32-bits of PCIE type 0 config space (CardBus CIS Pointer Register)
 *
 */
union cvmx_pcieepvfx_cfg010 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg010_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t cisp                         : 32; /**< CardBus CIS Pointer */
#else
	uint32_t cisp                         : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg010_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg010 cvmx_pcieepvfx_cfg010_t;

/**
 * cvmx_pcieepvf#_cfg011
 *
 * PCIE_CFG011 = Twelfth 32-bits of PCIE type 0 config space (Subsystem ID and Subsystem Vendor
 * ID Register)
 */
union cvmx_pcieepvfx_cfg011 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg011_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ssid                         : 16; /**< Subsystem ID
                                                         Assigned by PCI-SIG */
	uint32_t ssvid                        : 16; /**< Subsystem Vendor ID
                                                         Assigned by PCI-SIG */
#else
	uint32_t ssvid                        : 16;
	uint32_t ssid                         : 16;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg011_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg011 cvmx_pcieepvfx_cfg011_t;

/**
 * cvmx_pcieepvf#_cfg012
 *
 * PCIE_CFG012 = Thirteenth 32-bits of PCIE type 0 config space (Expansion ROM Base Address Register)
 *
 */
union cvmx_pcieepvfx_cfg012 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg012_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t eraddr                       : 16; /**< Expansion ROM Address */
	uint32_t reserved_1_15                : 15;
	uint32_t er_en                        : 1;  /**< Expansion ROM Enable */
#else
	uint32_t er_en                        : 1;
	uint32_t reserved_1_15                : 15;
	uint32_t eraddr                       : 16;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg012_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg012 cvmx_pcieepvfx_cfg012_t;

/**
 * cvmx_pcieepvf#_cfg013
 *
 * PCIE_CFG013 = Fourteenth 32-bits of PCIE type 0 config space (Capability Pointer Register)
 *
 */
union cvmx_pcieepvfx_cfg013 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg013_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_8_31                : 24;
	uint32_t cp                           : 8;  /**< First Capability Pointer.
                                                         Points to the PCI Express Capability Pointer structure (VF's) */
#else
	uint32_t cp                           : 8;
	uint32_t reserved_8_31                : 24;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg013_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg013 cvmx_pcieepvfx_cfg013_t;

/**
 * cvmx_pcieepvf#_cfg015
 *
 * PCIE_CFG015 = Sixteenth 32-bits of PCIE type 0 config space (Interrupt Line Register/Interrupt
 * Pin/Bridge Control Register)
 */
union cvmx_pcieepvfx_cfg015 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg015_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ml                           : 8;  /**< Maximum Latency     (Hardwired to 0) */
	uint32_t mg                           : 8;  /**< Minimum Grant       (Hardwired to 0) */
	uint32_t inta                         : 8;  /**< Interrupt Pin */
	uint32_t il                           : 8;  /**< Interrupt Line */
#else
	uint32_t il                           : 8;
	uint32_t inta                         : 8;
	uint32_t mg                           : 8;
	uint32_t ml                           : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg015_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg015 cvmx_pcieepvfx_cfg015_t;

/**
 * cvmx_pcieepvf#_cfg028
 *
 * PCIE_CFG028 = Twenty-ninth 32-bits of PCIE type 0 config space
 * (PCI Express Capabilities List Register/
 * PCI Express Capabilities Register)
 */
union cvmx_pcieepvfx_cfg028 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg028_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t imn                          : 5;  /**< Interrupt Message Number */
	uint32_t si                           : 1;  /**< Slot Implemented */
	uint32_t dpt                          : 4;  /**< Device Port Type */
	uint32_t pciecv                       : 4;  /**< PCI Express Capability Version */
	uint32_t ncp                          : 8;  /**< Next Capability Pointer
                                                         Points to the MSI-X Capabilities by default, */
	uint32_t pcieid                       : 8;  /**< PCIE Capability ID */
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
	struct cvmx_pcieepvfx_cfg028_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg028 cvmx_pcieepvfx_cfg028_t;

/**
 * cvmx_pcieepvf#_cfg029
 *
 * PCIE_CFG029 = Thirtieth 32-bits of PCIE type 0 config space (Device Capabilities Register)
 *
 */
union cvmx_pcieepvfx_cfg029 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg029_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_29_31               : 3;
	uint32_t flr_cap                      : 1;  /**< Function Level Reset Capable
                                                         o 1 for SRIOV core */
	uint32_t cspls                        : 2;  /**< Captured Slot Power Limit Scale
                                                         From Message from RC, upstream port only. */
	uint32_t csplv                        : 8;  /**< Captured Slot Power Limit Value
                                                         From Message from RC, upstream port only. */
	uint32_t reserved_16_17               : 2;
	uint32_t rber                         : 1;  /**< Role-Based Error Reporting */
	uint32_t reserved_12_14               : 3;
	uint32_t el1al                        : 3;  /**< Endpoint L1 Acceptable Latency */
	uint32_t el0al                        : 3;  /**< Endpoint L0s Acceptable Latency */
	uint32_t etfs                         : 1;  /**< Extended Tag Field Supported */
	uint32_t pfs                          : 2;  /**< Phantom Function Supported */
	uint32_t mpss                         : 3;  /**< Max_Payload_Size Supported */
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
	struct cvmx_pcieepvfx_cfg029_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg029 cvmx_pcieepvfx_cfg029_t;

/**
 * cvmx_pcieepvf#_cfg030
 *
 * PCIE_CFG030 = Thirty-first 32-bits of PCIE type 0 config space
 * (Device Control Register/Device Status Register)
 */
union cvmx_pcieepvfx_cfg030 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg030_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_22_31               : 10;
	uint32_t tp                           : 1;  /**< Transaction Pending
                                                         Set to 1 when Non-Posted Requests are not yet completed
                                                         and clear when they are completed. */
	uint32_t ap_d                         : 1;  /**< Aux Power Detected
                                                         Set to 1 if Aux power detected. */
	uint32_t ur_d                         : 1;  /**< Unsupported Request Detected
                                                         Errors are logged in this register regardless of whether
                                                         error reporting is enabled in the Device Control register.
                                                         UR_D occurs when we receive something we don't support.
                                                         Unsupported requests are Nonfatal errors, so UR_D should
                                                         cause NFE_D.  Receiving a  vendor defined message should
                                                         cause an unsupported request. */
	uint32_t fe_d                         : 1;  /**< Fatal Error Detected
                                                         All fatal errors re non-function specific and get
                                                         reported only in the PF. */
	uint32_t nfe_d                        : 1;  /**< Non-Fatal Error detected
                                                         Errors are logged in this register regardless of whether
                                                         error reporting is enabled in the Device Control register.
                                                         NFE_D is set if we receive any of the errors in PCIE_CFG066
                                                         that has a severity set to Nonfatal and does NOT meet Advisory
                                                         Nonfatal criteria , which
                                                         most poisoned TLP's should be. */
	uint32_t ce_d                         : 1;  /**< Correctable Error Detected
                                                         All correctable errors re non-function specific and get
                                                         reported only in the PF. */
	uint32_t i_flr                        : 1;  /**< Initiate Function Level Reset
                                                         (Not Supported) */
	uint32_t mrrs                         : 3;  /**< Max Read Request Size
                                                         0 = 128B
                                                         1 = 256B
                                                         2 = 512B
                                                         3 = 1024B
                                                         4 = 2048B
                                                         5 = 4096B */
	uint32_t ns_en                        : 1;  /**< Enable No Snoop */
	uint32_t ap_en                        : 1;  /**< AUX Power PM Enable */
	uint32_t pf_en                        : 1;  /**< Phantom Function Enable */
	uint32_t etf_en                       : 1;  /**< Extended Tag Field Enable */
	uint32_t mps                          : 3;  /**< "Max Payload Size.
                                                         Legal values:
                                                         0  = 128B
                                                         1  = 256B
                                                         Larger sizes not supported by OCTEON.
                                                         Note: DPI_SLI_PRT#_CFG[MPS] must be set to the same
                                                         value for proper functionality." */
	uint32_t ro_en                        : 1;  /**< Enable Relaxed Ordering */
	uint32_t ur_en                        : 1;  /**< Unsupported Request Reporting Enable */
	uint32_t fe_en                        : 1;  /**< Fatal Error Reporting Enable */
	uint32_t nfe_en                       : 1;  /**< Non-Fatal Error Reporting Enable */
	uint32_t ce_en                        : 1;  /**< Correctable Error Reporting Enable */
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
	struct cvmx_pcieepvfx_cfg030_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg030 cvmx_pcieepvfx_cfg030_t;

/**
 * cvmx_pcieepvf#_cfg031
 *
 * PCIE_CFG031 = Thirty-second 32-bits of PCIE type 0 config space
 * (Link Capabilities Register)
 */
union cvmx_pcieepvfx_cfg031 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg031_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pnum                         : 8;  /**< Port Number */
	uint32_t reserved_23_23               : 1;
	uint32_t aspm                         : 1;  /**< ASPM Optionality Compliance */
	uint32_t lbnc                         : 1;  /**< Link Bandwidth Notification Capability
                                                         Set 0 for Endpoint devices. */
	uint32_t dllarc                       : 1;  /**< Data Link Layer Active Reporting Capable */
	uint32_t sderc                        : 1;  /**< Surprise Down Error Reporting Capable
                                                         Not supported, hardwired to 0x0. */
	uint32_t cpm                          : 1;  /**< Clock Power Management
                                                         The default value is the value you specify during hardware
                                                         configuration */
	uint32_t l1el                         : 3;  /**< L1 Exit Latency
                                                         The default value is the value you specify during hardware
                                                         configuration */
	uint32_t l0el                         : 3;  /**< L0s Exit Latency
                                                         The default value is the value you specify during hardware
                                                         configuration */
	uint32_t aslpms                       : 2;  /**< Active State Link PM Support
                                                         The default value is the value you specify during hardware
                                                         configuration */
	uint32_t mlw                          : 6;  /**< Maximum Link Width
                                                         The default value is the value you specify during hardware
                                                         configuration (x1, x4, x8, or x16) */
	uint32_t mls                          : 4;  /**< "Maximum Link Speed
                                                         The reset value of this field is controlled by the value read from
                                                         the PEM csr PEM(0..3)_CFG.MD.
                                                         PEM(0..2)_CFG.MD  RST_VALUE   NOTE
                                                         00                0001b       2.5 GHz supported
                                                         01                0010b       5.0 GHz and 2.5 GHz supported
                                                         10                0011b       8.0 Ghz, 5.0 GHz and 2.5 GHz supported
                                                         11                0011b       8.0 Ghz, 5.0 GHz and 2.5 GHz supported (RC Mode)
                                                         This field is writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
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
	uint32_t aspm                         : 1;
	uint32_t reserved_23_23               : 1;
	uint32_t pnum                         : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg031_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg031 cvmx_pcieepvfx_cfg031_t;

/**
 * cvmx_pcieepvf#_cfg032
 *
 * PCIE_CFG032 = Thirty-third 32-bits of PCIE type 0 config space
 * (Link Control Register/Link Status Register)
 */
union cvmx_pcieepvfx_cfg032 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg032_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lab                          : 1;  /**< Link Autonomous Bandwidth Status */
	uint32_t lbm                          : 1;  /**< Link Bandwidth Management Status */
	uint32_t dlla                         : 1;  /**< Data Link Layer Active
                                                         Not applicable for an upstream Port or Endpoint device,
                                                         hardwired to 0. */
	uint32_t scc                          : 1;  /**< Slot Clock Configuration
                                                         Indicates that the component uses the same physical reference
                                                         clock that the platform provides on the connector. */
	uint32_t lt                           : 1;  /**< Link Training
                                                         Not applicable for an upstream Port or Endpoint device,
                                                         hardwired to 0. */
	uint32_t reserved_26_26               : 1;
	uint32_t nlw                          : 6;  /**< Negotiated Link Width
                                                         Set automatically by hardware after Link initialization. */
	uint32_t ls                           : 4;  /**< Link Speed
                                                         1 == The negotiated Link speed: 2.5 Gbps
                                                         2 == The negotiated Link speed: 5.0 Gbps
                                                         4 == The negotiated Link speed: 8.0 Gbps */
	uint32_t reserved_12_15               : 4;
	uint32_t lab_int_enb                  : 1;  /**< Link Autonomous Bandwidth Interrupt Enable
                                                         This bit is not applicable and is reserved for endpoints */
	uint32_t lbm_int_enb                  : 1;  /**< Link Bandwidth Management Interrupt Enable
                                                         This bit is not applicable and is reserved for endpoints */
	uint32_t hawd                         : 1;  /**< Hardware Autonomous Width Disable
                                                         (Not Supported) */
	uint32_t ecpm                         : 1;  /**< Enable Clock Power Management
                                                         Hardwired to 0 if Clock Power Management is disabled in
                                                         the Link Capabilities register. */
	uint32_t es                           : 1;  /**< Extended Synch */
	uint32_t ccc                          : 1;  /**< Common Clock Configuration */
	uint32_t rl                           : 1;  /**< Retrain Link
                                                         Not applicable for an upstream Port or Endpoint device,
                                                         hardwired to 0. */
	uint32_t ld                           : 1;  /**< Link Disable
                                                         Not applicable for an upstream Port or Endpoint device,
                                                         hardwired to 0. */
	uint32_t rcb                          : 1;  /**< Read Completion Boundary (RCB) */
	uint32_t reserved_2_2                 : 1;
	uint32_t aslpc                        : 2;  /**< Active State Link PM Control */
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
	uint32_t reserved_12_15               : 4;
	uint32_t ls                           : 4;
	uint32_t nlw                          : 6;
	uint32_t reserved_26_26               : 1;
	uint32_t lt                           : 1;
	uint32_t scc                          : 1;
	uint32_t dlla                         : 1;
	uint32_t lbm                          : 1;
	uint32_t lab                          : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg032_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg032 cvmx_pcieepvfx_cfg032_t;

/**
 * cvmx_pcieepvf#_cfg037
 *
 * PCIE_CFG037 = Thirty-eighth 32-bits of PCIE type 0 config space
 * (Device Capabilities 2 Register)
 */
union cvmx_pcieepvfx_cfg037 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg037_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t meetp                        : 2;  /**< "Max End-End TLP Prefixes
                                                         o 01b: 1
                                                         o 10b: 2
                                                         o 11b: 3
                                                         o 00b: 4" */
	uint32_t eetps                        : 1;  /**< End-End TLP Prefix Supported */
	uint32_t effs                         : 1;  /**< Extended Fmt Field Supported */
	uint32_t obffs                        : 2;  /**< Optimized Buffer Flush Fill (OBFF) Supported
                                                         (Not Supported) */
	uint32_t reserved_14_17               : 4;
	uint32_t tphs                         : 2;  /**< TPH Completer Supported
                                                         (Not Supported) */
	uint32_t ltrs                         : 1;  /**< Latency Tolerance Reporting (LTR) Mechanism Supported
                                                         (Not Supported) */
	uint32_t noroprpr                     : 1;  /**< No RO-enabled PR-PR Passing
                                                         (This bit applies to RCs) */
	uint32_t atom128s                     : 1;  /**< 128-bit AtomicOp Supported
                                                         (Not Supported) */
	uint32_t atom64s                      : 1;  /**< 64-bit AtomicOp Supported
                                                         (Not Supported) */
	uint32_t atom32s                      : 1;  /**< 32-bit AtomicOp Supported
                                                         (Not Supported) */
	uint32_t atom_ops                     : 1;  /**< AtomicOp Routing Supported
                                                         (Not Applicable for EP) */
	uint32_t ari                          : 1;  /**< Alternate Routing ID Forwarding Supported */
	uint32_t ctds                         : 1;  /**< Completion Timeout Disable Supported */
	uint32_t ctrs                         : 4;  /**< Completion Timeout Ranges Supported */
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
	struct cvmx_pcieepvfx_cfg037_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg037 cvmx_pcieepvfx_cfg037_t;

/**
 * cvmx_pcieepvf#_cfg038
 *
 * PCIE_CFG038 = Thirty-ninth 32-bits of PCIE type 0 config space
 * (Device Control 2 Register/Device Status 2 Register)
 */
union cvmx_pcieepvfx_cfg038 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg038_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t eetpb                        : 1;  /**< Unsupported End-End TLP Prefix Blocking */
	uint32_t obffe                        : 2;  /**< Optimized Buffer Flush Fill (OBFF) Enable
                                                         (Not Supported) */
	uint32_t reserved_10_12               : 3;
	uint32_t id0_cp                       : 1;  /**< ID Based Ordering Completion Enable
                                                         (Not Supported) */
	uint32_t id0_rq                       : 1;  /**< ID Based Ordering Request Enable
                                                         (Not Supported) */
	uint32_t atom_op_eb                   : 1;  /**< AtomicOp Egress Blocking
                                                         (Not Supported)m */
	uint32_t atom_op                      : 1;  /**< AtomicOp Requester Enable
                                                         (Not Supported) */
	uint32_t ari                          : 1;  /**< Alternate Routing ID Forwarding Supported */
	uint32_t ctd                          : 1;  /**< Completion Timeout Disable */
	uint32_t ctv                          : 4;  /**< Completion Timeout Value
                                                         Completion Timeout Programming is not supported
                                                         Completion timeout is the range of 16 ms to 55 ms. */
#else
	uint32_t ctv                          : 4;
	uint32_t ctd                          : 1;
	uint32_t ari                          : 1;
	uint32_t atom_op                      : 1;
	uint32_t atom_op_eb                   : 1;
	uint32_t id0_rq                       : 1;
	uint32_t id0_cp                       : 1;
	uint32_t reserved_10_12               : 3;
	uint32_t obffe                        : 2;
	uint32_t eetpb                        : 1;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg038_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg038 cvmx_pcieepvfx_cfg038_t;

/**
 * cvmx_pcieepvf#_cfg039
 *
 * PCIE_CFG039 = Fortieth 32-bits of PCIE type 0 config space
 * (Link Capabilities 2 Register)
 */
union cvmx_pcieepvfx_cfg039 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg039_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_9_31                : 23;
	uint32_t cls                          : 1;  /**< Crosslink Supported */
	uint32_t slsv                         : 7;  /**< "Supported Link Speeds Vector
                                                         Indicates the supported Link speeds of the associated Port.
                                                         For each bit, a value of 1b indicates that the cooresponding
                                                         Link speed is supported; otherwise, the Link speed is not
                                                         supported.
                                                         Bit definitions are:
                                                         Bit 1 2.5 GT/s
                                                         Bit 2 5.0 GT/s
                                                         Bit 3 8.0 GT/s
                                                         Bits 7:4 reserved
                                                         The reset value of this field is controlled by a value read from
                                                         the PEM csr PEM(0..3)_CFG.MD.
                                                         PEM(0..3)_CFG.MD   RST_VALUE   NOTE
                                                         00                 0001b       2.5 GHz supported
                                                         01                 0011b       5.0 GHz and 2.5 GHz supported
                                                         10                 0111b       8.0 GHz, 5.0 GHz and 2.5 GHz supported
                                                         11                 0111b       8.0 Ghz, 5.0 GHz and 2.5 GHz supported (RC Mode)" */
	uint32_t reserved_0_0                 : 1;
#else
	uint32_t reserved_0_0                 : 1;
	uint32_t slsv                         : 7;
	uint32_t cls                          : 1;
	uint32_t reserved_9_31                : 23;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg039_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg039 cvmx_pcieepvfx_cfg039_t;

/**
 * cvmx_pcieepvf#_cfg040
 *
 * PCIE_CFG040 = Forty-first 32-bits of PCIE type 0 config space
 * (Link Control 2 Register/Link Status 2 Register)
 */
union cvmx_pcieepvfx_cfg040 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg040_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_17_31               : 15;
	uint32_t cdl                          : 1;  /**< Current De-emphasis Level
                                                         When the Link is operating at 5 GT/s speed, this bit
                                                         reflects the level of de-emphasis. Encodings:
                                                         1b: -3.5 dB
                                                         0b: -6 dB
                                                         Note: The value in this bit is undefined when the Link is
                                                         operating at 2.5 GT/s speed */
	uint32_t reserved_13_15               : 3;
	uint32_t cde                          : 1;  /**< Compliance De-emphasis
                                                         This bit sets the de-emphasis level in Polling. Compliance
                                                         state if the entry occurred due to the Tx Compliance
                                                         Receive bit being 1b. Encodings:
                                                         1b: -3.5 dB
                                                         0b: -6 dB
                                                         Note: When the Link is operating at 2.5 GT/s, the setting
                                                         of this bit has no effect. */
	uint32_t csos                         : 1;  /**< Compliance SOS
                                                         When set to 1b, the LTSSM is required to send SKP
                                                         Ordered Sets periodically in between the (modified)
                                                         compliance patterns.
                                                         Note: When the Link is operating at 2.5 GT/s, the setting
                                                         of this bit has no effect. */
	uint32_t emc                          : 1;  /**< Enter Modified Compliance
                                                         When this bit is set to 1b, the device transmits a modified
                                                         compliance pattern if the LTSSM enters Polling.
                                                         Compliance state. */
	uint32_t tm                           : 3;  /**< Transmit Margin
                                                         This field controls the value of the non-de-emphasized
                                                         voltage level at the Transmitter pins:
                                                         - 000: 800-1200 mV for full swing 400-600 mV for half-swing
                                                         - 001-010: values must be monotonic with a non-zero slope
                                                         - 011: 200-400 mV for full-swing and 100-200 mV for halfswing
                                                         - 100-111: reserved
                                                         This field is reset to 000b on entry to the LTSSM Polling.
                                                         Compliance substate.
                                                         When operating in 5.0 GT/s mode with full swing, the
                                                         de-emphasis ratio must be maintained within +/- 1 dB
                                                         from the specification-defined operational value
                                                         either -3.5 or -6 dB). */
	uint32_t sde                          : 1;  /**< Selectable De-emphasis
                                                         Not applicable for an upstream Port or Endpoint device.
                                                         Hardwired to 0. */
	uint32_t hasd                         : 1;  /**< Hardware Autonomous Speed Disable
                                                         When asserted, the
                                                         application must disable hardware from changing the Link
                                                         speed for device-specific reasons other than attempting to
                                                         correct unreliable Link operation by reducing Link speed.
                                                         Initial transition to the highest supported common link
                                                         speed is not blocked by this signal. */
	uint32_t ec                           : 1;  /**< Enter Compliance
                                                         Software is permitted to force a link to enter Compliance
                                                         mode at the speed indicated in the Target Link Speed
                                                         field by setting this bit to 1b in both components on a link
                                                         and then initiating a hot reset on the link. */
	uint32_t tls                          : 4;  /**< "Target Link Speed
                                                         For Downstream ports, this field sets an upper limit on link
                                                         operational speed by restricting the values advertised by
                                                         the upstream component in its training sequences:
                                                         - 0001: 2.5Gb/s Target Link Speed
                                                         - 0010: 5Gb/s Target Link Speed
                                                         - 0100: 8Gb/s Target Link Speed (Not Supported)
                                                         All other encodings are reserved.
                                                         If a value is written to this field that does not correspond to
                                                         a speed included in the Supported Link Speeds field, the
                                                         result is undefined.
                                                         For both Upstream and Downstream ports, this field is
                                                         used to set the target compliance mode speed when
                                                         software is using the Enter Compliance bit to force a link
                                                         into compliance mode.
                                                         The reset value of this field is controlled by the value read from
                                                         the PEM csr PEM(0..3)_CFG.MD.
                                                         PEM(0..2)_CFG.MD  RST_VALUE   NOTE
                                                         00                0001b       2.5 GHz supported
                                                         01                0010b       5.0 GHz and 2.5 GHz supported
                                                         10                0011b       8.0 GHz, 5.0 GHz and 2.5 GHz supported
                                                         11                0011b       8.0 Ghz, 5.0 GHz and 2.5 GHz supported (RC Mode)" */
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
	struct cvmx_pcieepvfx_cfg040_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg040 cvmx_pcieepvfx_cfg040_t;

/**
 * cvmx_pcieepvf#_cfg044
 *
 * PCIE_CFG044 = Forty-fifth 32-bits of PCIE type 0 config space
 * (MSI-X Capability ID/
 * MSI-X Next Item Pointer/
 * MSI-X Control Register)
 */
union cvmx_pcieepvfx_cfg044 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg044_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msixen                       : 1;  /**< MSI-X Enable
                                                         If MSI-X is enabled, MIS and INTx must be disabled. */
	uint32_t funm                         : 1;  /**< Function Mask
                                                         1b: All vectors associated with the function are masked,
                                                         regardless of their respective per-vector mask bits.
                                                         0b: Each vectors Mask bit determines whether the vector
                                                         is masked or not. */
	uint32_t reserved_27_29               : 3;
	uint32_t msixts                       : 11; /**< MSI-X Table Size
                                                         Encoded as (Table Size - 1) */
	uint32_t ncp                          : 8;  /**< Next Capability Pointer
                                                         Points to the PCI Power Management Capability Registers */
	uint32_t msixcid                      : 8;  /**< MSI-X Capability ID */
#else
	uint32_t msixcid                      : 8;
	uint32_t ncp                          : 8;
	uint32_t msixts                       : 11;
	uint32_t reserved_27_29               : 3;
	uint32_t funm                         : 1;
	uint32_t msixen                       : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg044_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg044 cvmx_pcieepvfx_cfg044_t;

/**
 * cvmx_pcieepvf#_cfg045
 *
 * PCIE_CFG045 = Forty-sixth 32-bits of PCIE type 0 config space
 * (MSI-X Table Offset and BIR Register)
 */
union cvmx_pcieepvfx_cfg045 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg045_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msixtoffs                    : 29; /**< MSI-X Table Offset Register
                                                         Base address of the MSI-X Table, as an offset from the base
                                                         address of te BAR indicated by the Table BIR bits. */
	uint32_t msixtbir                     : 3;  /**< "MSI-X Table BAR Indicator Register (BIR)
                                                         Indicates which BAR is used to map the MSI-X Table
                                                         into memory space
                                                         000 - 100: BAR#
                                                         110 - 111: Reserved" */
#else
	uint32_t msixtbir                     : 3;
	uint32_t msixtoffs                    : 29;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg045_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg045 cvmx_pcieepvfx_cfg045_t;

/**
 * cvmx_pcieepvf#_cfg046
 *
 * PCIE_CFG046 = Forty-seventh 32-bits of PCIE type 0 config space
 * (MSI-X PBA Offset and BIR Register)
 */
union cvmx_pcieepvfx_cfg046 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg046_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msixpoffs                    : 29; /**< "MSI-X Table Offset Register
                                                         Base address of the MSI-X PBA, as an offset from the base
                                                         address of te BAR indicated by the Table PBA bits.
                                                         writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t msixpbir                     : 3;  /**< "MSI-X PBA BAR Indicator Register (BIR)
                                                         Indicates which BAR is used to map the MSI-X Pending Bit Array
                                                         into memory space
                                                         000 - 100: BAR#
                                                         110 - 111: Reserved
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
#else
	uint32_t msixpbir                     : 3;
	uint32_t msixpoffs                    : 29;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg046_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg046 cvmx_pcieepvfx_cfg046_t;

/**
 * cvmx_pcieepvf#_cfg048
 *
 * PCIE_CFG048 = Forty-ninth 32-bits of PCIE type 0 config space
 * (Power Management Capability ID/
 * Power Management Next Item Pointer/
 * Power Management Capabilities Register)
 */
union cvmx_pcieepvfx_cfg048 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg048_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pmes                         : 5;  /**< PME_Support
                                                         o Bit 11: If set, PME Messages can be generated from D0
                                                         o Bit 12: If set, PME Messages can be generated from D1
                                                         o Bit 13: If set, PME Messages can be generated from D2
                                                         o Bit 14: If set, PME Messages can be generated from D3hot
                                                         o Bit 15: always zero.   VF's do not support D3cold */
	uint32_t d2s                          : 1;  /**< D2 Support */
	uint32_t d1s                          : 1;  /**< D1 Support */
	uint32_t auxc                         : 3;  /**< AUX Current */
	uint32_t dsi                          : 1;  /**< Device Specific Initialization (DSI) */
	uint32_t reserved_20_20               : 1;
	uint32_t pme_clock                    : 1;  /**< PME Clock, hardwired to 0 */
	uint32_t pmsv                         : 3;  /**< Power Management Specification Version */
	uint32_t ncp                          : 8;  /**< Next Capability Pointer */
	uint32_t pmcid                        : 8;  /**< Power Management Capability ID */
#else
	uint32_t pmcid                        : 8;
	uint32_t ncp                          : 8;
	uint32_t pmsv                         : 3;
	uint32_t pme_clock                    : 1;
	uint32_t reserved_20_20               : 1;
	uint32_t dsi                          : 1;
	uint32_t auxc                         : 3;
	uint32_t d1s                          : 1;
	uint32_t d2s                          : 1;
	uint32_t pmes                         : 5;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg048_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg048 cvmx_pcieepvfx_cfg048_t;

/**
 * cvmx_pcieepvf#_cfg049
 *
 * PCIE_CFG049 = Fiftieth 32-bits of PCIE type 0 config space (Power Management Control and
 * Status Register)
 */
union cvmx_pcieepvfx_cfg049 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg049_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pmdia                        : 8;  /**< Data register for additional information (not supported) */
	uint32_t bpccee                       : 1;  /**< Bus Power/Clock Control Enable, hardwired to 0 */
	uint32_t bd3h                         : 1;  /**< B2/B3 Support, hardwired to 0 */
	uint32_t reserved_16_21               : 6;
	uint32_t pmess                        : 1;  /**< PME Status
                                                         Indicates if a previously enabled PME event occurred or not. */
	uint32_t pmedsia                      : 2;  /**< Data Scale (not supported) */
	uint32_t pmds                         : 4;  /**< Data Select (not supported) */
	uint32_t pmeens                       : 1;  /**< PME Enable
                                                         A value of 1 indicates that the device is enabled to
                                                         generate PME. */
	uint32_t reserved_4_7                 : 4;
	uint32_t nsr                          : 1;  /**< No Soft Reset */
	uint32_t reserved_2_2                 : 1;
	uint32_t ps                           : 2;  /**< Power State
                                                         Controls the device power state:
                                                         o 00b: D0
                                                         o 01b: D1
                                                         o 10b: D2
                                                         o 11b: D3
                                                         The written value is ignored if the specific state is
                                                         not supported. */
#else
	uint32_t ps                           : 2;
	uint32_t reserved_2_2                 : 1;
	uint32_t nsr                          : 1;
	uint32_t reserved_4_7                 : 4;
	uint32_t pmeens                       : 1;
	uint32_t pmds                         : 4;
	uint32_t pmedsia                      : 2;
	uint32_t pmess                        : 1;
	uint32_t reserved_16_21               : 6;
	uint32_t bd3h                         : 1;
	uint32_t bpccee                       : 1;
	uint32_t pmdia                        : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg049_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg049 cvmx_pcieepvfx_cfg049_t;

/**
 * cvmx_pcieepvf#_cfg064
 *
 * PCIE_CFG064 = Sixty-fifth 32-bits of PCIE type 0 config space
 * (PCI Express Extended Capability Header)
 */
union cvmx_pcieepvfx_cfg064 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg064_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next Capability Offset
                                                         Points to the ARI Capabilities by default, */
	uint32_t cv                           : 4;  /**< Capability Version */
	uint32_t pcieec                       : 16; /**< PCIE Express Extended Capability ID */
#else
	uint32_t pcieec                       : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg064_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg064 cvmx_pcieepvfx_cfg064_t;

/**
 * cvmx_pcieepvf#_cfg082
 *
 * PCIE_CFG082 = Eighty-third 32-bits of PCIE type 0 config space
 * (PCI Express ARI Capability Header)
 */
union cvmx_pcieepvfx_cfg082 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg082_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next Capability Offset */
	uint32_t cv                           : 4;  /**< Capability Version */
	uint32_t ariid                        : 16; /**< PCIE Express Extended Capability */
#else
	uint32_t ariid                        : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg082_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg082 cvmx_pcieepvfx_cfg082_t;

/**
 * cvmx_pcieepvf#_cfg083
 *
 * PCIE_CFG083 = Eighty-fourth 32-bits of PCIE type 0 config space
 * (PCI Express ARI Capability Register/
 * PCI Express ARI Control Register)
 */
union cvmx_pcieepvfx_cfg083 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg083_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t fg                           : 4;  /**< Function Group */
	uint32_t reserved_18_19               : 2;
	uint32_t acsfge                       : 1;  /**< ACS Function Groups Enable (A) */
	uint32_t mfvcfge                      : 1;  /**< MFVC Function Groups Enable (M) */
	uint32_t nfn                          : 8;  /**< Next Function Number */
	uint32_t reserved_2_7                 : 6;
	uint32_t acsfgc                       : 1;  /**< ACS Function Groups Capability */
	uint32_t mfvcfgc                      : 1;  /**< MFVC Function Groups Capability */
#else
	uint32_t mfvcfgc                      : 1;
	uint32_t acsfgc                       : 1;
	uint32_t reserved_2_7                 : 6;
	uint32_t nfn                          : 8;
	uint32_t mfvcfge                      : 1;
	uint32_t acsfge                       : 1;
	uint32_t reserved_18_19               : 2;
	uint32_t fg                           : 4;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg083_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg083 cvmx_pcieepvfx_cfg083_t;

/**
 * cvmx_pcieepvf#_cfg448
 *
 * PCIE_CFG448 = Four hundred forty-ninth 32-bits of PCIE type 0 config space
 * (Ack Latency Timer and Replay Timer Register)
 */
union cvmx_pcieepvfx_cfg448 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg448_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t rtl                          : 16; /**< Replay Time Limit
                                                         The replay timer expires when it reaches this limit. The PCI
                                                         Express bus initiates a replay upon reception of a Nak or when
                                                         the replay timer expires.
                                                         This value will be set correctly by the hardware out of reset
                                                         or when the negotiated Link-Width or Payload-Size changes. If
                                                         the user changes this value through a CSR write or by an
                                                         EEPROM load then they should refer to the PCIe Specification
                                                         for the correct value. */
	uint32_t rtltl                        : 16; /**< Round Trip Latency Time Limit
                                                         The Ack/Nak latency timer expires when it reaches this limit.
                                                         This value will be set correctly by the hardware out of reset
                                                         or when the negotiated Link-Width or Payload-Size changes. If
                                                         the user changes this value through a CSR write or by an
                                                         EEPROM load then they should refer to the PCIe Specification
                                                         for the correct value. */
#else
	uint32_t rtltl                        : 16;
	uint32_t rtl                          : 16;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg448_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg448 cvmx_pcieepvfx_cfg448_t;

/**
 * cvmx_pcieepvf#_cfg449
 *
 * PCIE_CFG449 = Four hundred fiftieth 32-bits of PCIE type 0 config space
 * (Other Message Register)
 */
union cvmx_pcieepvfx_cfg449 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg449_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t omr                          : 32; /**< Other Message Register
                                                         This register can be used for either of the following purposes:
                                                         o To send a specific PCI Express Message, the application
                                                         writes the payload of the Message into this register, then
                                                         sets bit 0 of the Port Link Control Register to send the
                                                         Message.
                                                         o To store a corruption pattern for corrupting the LCRC on all
                                                         TLPs, the application places a 32-bit corruption pattern into
                                                         this register and enables this function by setting bit 25 of
                                                         the Port Link Control Register. When enabled, the transmit
                                                         LCRC result is XOR'd with this pattern before inserting
                                                         it into the packet. */
#else
	uint32_t omr                          : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg449_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg449 cvmx_pcieepvfx_cfg449_t;

/**
 * cvmx_pcieepvf#_cfg450
 *
 * PCIE_CFG450 = Four hundred fifty-first 32-bits of PCIE type 0 config space
 * (Port Force Link Register)
 */
union cvmx_pcieepvfx_cfg450 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg450_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lpec                         : 8;  /**< Low Power Entrance Count
                                                         The Power Management state will wait for this many clock cycles
                                                         for the associated completion of a CfgWr to PCIE_CFG017 register
                                                         Power State (PS) field register to go low-power. This register
                                                         is intended for applications that do not let the PCI Express
                                                         bus handle a completion for configuration request to the
                                                         Power Management Control and Status (PCIE_CFG017) register. */
	uint32_t reserved_22_23               : 2;
	uint32_t link_state                   : 6;  /**< Link State
                                                         The Link state that the PCI Express Bus will be forced to
                                                         when bit 15 (Force Link) is set.
                                                         State encoding:
                                                         o DETECT_QUIET              00h
                                                         o DETECT_ACT                01h
                                                         o POLL_ACTIVE               02h
                                                         o POLL_COMPLIANCE           03h
                                                         o POLL_CONFIG               04h
                                                         o PRE_DETECT_QUIET          05h
                                                         o DETECT_WAIT               06h
                                                         o CFG_LINKWD_START          07h
                                                         o CFG_LINKWD_ACEPT          08h
                                                         o CFG_LANENUM_WAIT          09h
                                                         o CFG_LANENUM_ACEPT         0Ah
                                                         o CFG_COMPLETE              0Bh
                                                         o CFG_IDLE                  0Ch
                                                         o RCVRY_LOCK                0Dh
                                                         o RCVRY_SPEED               0Eh
                                                         o RCVRY_RCVRCFG             0Fh
                                                         o RCVRY_IDLE                10h
                                                         o L0                        11h
                                                         o L0S                       12h
                                                         o L123_SEND_EIDLE           13h
                                                         o L1_IDLE                   14h
                                                         o L2_IDLE                   15h
                                                         o L2_WAKE                   16h
                                                         o DISABLED_ENTRY            17h
                                                         o DISABLED_IDLE             18h
                                                         o DISABLED                  19h
                                                         o LPBK_ENTRY                1Ah
                                                         o LPBK_ACTIVE               1Bh
                                                         o LPBK_EXIT                 1Ch
                                                         o LPBK_EXIT_TIMEOUT         1Dh
                                                         o HOT_RESET_ENTRY           1Eh
                                                         o HOT_RESET                 1Fh */
	uint32_t force_link                   : 1;  /**< Force Link
                                                         Forces the Link to the state specified by the Link State field.
                                                         The Force Link pulse will trigger Link re-negotiation.
                                                         * As the The Force Link is a pulse, writing a 1 to it does
                                                         trigger the forced link state event, even thought reading it
                                                         always returns a 0. */
	uint32_t reserved_8_14                : 7;
	uint32_t link_num                     : 8;  /**< Link Number
                                                         Not used for Endpoint */
#else
	uint32_t link_num                     : 8;
	uint32_t reserved_8_14                : 7;
	uint32_t force_link                   : 1;
	uint32_t link_state                   : 6;
	uint32_t reserved_22_23               : 2;
	uint32_t lpec                         : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg450_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg450 cvmx_pcieepvfx_cfg450_t;

/**
 * cvmx_pcieepvf#_cfg451
 *
 * PCIE_CFG451 = Four hundred fifty-second 32-bits of PCIE type 0 config space
 * (Ack Frequency Register)
 */
union cvmx_pcieepvfx_cfg451 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg451_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t easpml1                      : 1;  /**< Enter ASPM L1 without receive in L0s
                                                         Allow core to enter ASPM L1 even when link partner did
                                                         not go to L0s (receive is not in L0s).
                                                         When not set, core goes to ASPM L1 only after idle period
                                                         during which both receive and transmit are in L0s. */
	uint32_t l1el                         : 3;  /**< L1 Entrance Latency
                                                         Values correspond to:
                                                         o 000: 1 ms
                                                         o 001: 2 ms
                                                         o 010: 4 ms
                                                         o 011: 8 ms
                                                         o 100: 16 ms
                                                         o 101: 32 ms
                                                         o 110 or 111: 64 ms */
	uint32_t l0el                         : 3;  /**< L0s Entrance Latency
                                                         Values correspond to:
                                                         o 000: 1 ms
                                                         o 001: 2 ms
                                                         o 010: 3 ms
                                                         o 011: 4 ms
                                                         o 100: 5 ms
                                                         o 101: 6 ms
                                                         o 110 or 111: 7 ms */
	uint32_t n_fts_cc                     : 8;  /**< N_FTS when common clock is used.
                                                         The number of Fast Training Sequence ordered sets to be
                                                         transmitted when transitioning from L0s to L0. The maximum
                                                         number of FTS ordered-sets that a component can request is 255.
                                                         Note: A value of zero is not supported; a value of
                                                         zero can cause the LTSSM to go into the recovery state
                                                         when exiting from L0s. */
	uint32_t n_fts                        : 8;  /**< N_FTS
                                                         The number of Fast Training Sequence ordered sets to be
                                                         transmitted when transitioning from L0s to L0. The maximum
                                                         number of FTS ordered-sets that a component can request is 255.
                                                         Note: A value of zero is not supported; a value of
                                                         zero can cause the LTSSM to go into the recovery state
                                                         when exiting from L0s. */
	uint32_t ack_freq                     : 8;  /**< Ack Frequency
                                                         The number of pending Ack's specified here (up to 255) before
                                                         sending an Ack. */
#else
	uint32_t ack_freq                     : 8;
	uint32_t n_fts                        : 8;
	uint32_t n_fts_cc                     : 8;
	uint32_t l0el                         : 3;
	uint32_t l1el                         : 3;
	uint32_t easpml1                      : 1;
	uint32_t reserved_31_31               : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg451_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg451 cvmx_pcieepvfx_cfg451_t;

/**
 * cvmx_pcieepvf#_cfg452
 *
 * PCIE_CFG452 = Four hundred fifty-third 32-bits of PCIE type 0 config space
 * (Port Link Control Register)
 */
union cvmx_pcieepvfx_cfg452 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg452_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_22_31               : 10;
	uint32_t lme                          : 6;  /**< Link Mode Enable
                                                         o 000001: x1
                                                         o 000011: x2
                                                         o 000111: x4
                                                         o 001111: x8  (not supported)
                                                         o 011111: x16 (not supported)
                                                         o 111111: x32 (not supported)
                                                         This field indicates the MAXIMUM number of lanes supported
                                                         by the PCIe port. The value can be set less than 0x7
                                                         to limit the number of lanes the PCIe will attempt to use.
                                                         If the value of 0xF set by the HW is not desired,
                                                         this field can be programmed to a smaller value (i.e. EEPROM)
                                                         See also MLW.
                                                         (Note: The value of this field does NOT indicate the number
                                                         of lanes in use by the PCIe. LME sets the max number of lanes
                                                         in the PCIe core that COULD be used. As per the PCIe specs,
                                                         the PCIe core can negotiate a smaller link width, so all
                                                         of x8, x4, x2, and x1 are supported when LME=0xF,
                                                         for example.) */
	uint32_t reserved_12_15               : 4;
	uint32_t link_rate                    : 4;  /**< Reserved. */
	uint32_t flm                          : 1;  /**< Fast Link Mode
                                                         Sets all internal timers to fast mode for simulation purposes.
                                                         If during an eeprom load, the first word loaded is 0xffffffff,
                                                         then the EEPROM load will be terminated and this bit will be set. */
	uint32_t reserved_6_6                 : 1;
	uint32_t dllle                        : 1;  /**< DLL Link Enable
                                                         Enables Link initialization. If DLL Link Enable = 0, the PCI
                                                         Express bus does not transmit InitFC DLLPs and does not
                                                         establish a Link. */
	uint32_t reserved_4_4                 : 1;
	uint32_t ra                           : 1;  /**< Reset Assert
                                                         Triggers a recovery and forces the LTSSM to the Hot Reset
                                                         state (downstream port only). */
	uint32_t le                           : 1;  /**< Loopback Enable
                                                         Initiate loopback mode as a master. On a 0->1 transition,
                                                         the PCIe core sends TS ordered sets with the loopback bit set
                                                         to cause the link partner to enter into loopback mode as a
                                                         slave. Normal transmission is not possible when LE=1. To exit
                                                         loopback mode, take the link through a reset sequence. */
	uint32_t sd                           : 1;  /**< Scramble Disable
                                                         Turns off data scrambling. */
	uint32_t omr                          : 1;  /**< Other Message Request
                                                         When software writes a `1' to this bit, the PCI Express bus
                                                         transmits the Message contained in the Other Message register. */
#else
	uint32_t omr                          : 1;
	uint32_t sd                           : 1;
	uint32_t le                           : 1;
	uint32_t ra                           : 1;
	uint32_t reserved_4_4                 : 1;
	uint32_t dllle                        : 1;
	uint32_t reserved_6_6                 : 1;
	uint32_t flm                          : 1;
	uint32_t link_rate                    : 4;
	uint32_t reserved_12_15               : 4;
	uint32_t lme                          : 6;
	uint32_t reserved_22_31               : 10;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg452_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg452 cvmx_pcieepvfx_cfg452_t;

/**
 * cvmx_pcieepvf#_cfg453
 *
 * PCIE_CFG453 = Four hundred fifty-fourth 32-bits of PCIE type 0 config space
 * (Lane Skew Register)
 */
union cvmx_pcieepvfx_cfg453 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg453_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dlld                         : 1;  /**< Disable Lane-to-Lane Deskew
                                                         Disables the internal Lane-to-Lane deskew logic. */
	uint32_t reserved_26_30               : 5;
	uint32_t ack_nak                      : 1;  /**< Ack/Nak Disable
                                                         Prevents the PCI Express bus from sending Ack and Nak DLLPs. */
	uint32_t fcd                          : 1;  /**< Flow Control Disable
                                                         Prevents the PCI Express bus from sending FC DLLPs. */
	uint32_t ilst                         : 24; /**< Insert Lane Skew for Transmit
                                                         Causes skew between lanes for test purposes. There are three
                                                         bits per Lane. The value is in units of one symbol time. For
                                                         example, the value 010b for a Lane forces a skew of two symbol
                                                         times for that Lane. The maximum skew value for any Lane is 5
                                                         symbol times. */
#else
	uint32_t ilst                         : 24;
	uint32_t fcd                          : 1;
	uint32_t ack_nak                      : 1;
	uint32_t reserved_26_30               : 5;
	uint32_t dlld                         : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg453_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg453 cvmx_pcieepvfx_cfg453_t;

/**
 * cvmx_pcieepvf#_cfg454
 *
 * PCIE_CFG454 = Four hundred fifty-fifth 32-bits of PCIE type 0 config space
 * (Symbol Number Register)
 */
union cvmx_pcieepvfx_cfg454 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg454_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_29_31               : 3;
	uint32_t tmfcwt                       : 5;  /**< Used to be "Timer Modifier for Flow Control Watchdog Timer"
                                                         No longer used. Repl and enhanced func moved to "Queue Status"
                                                         register - CFG463. Kept for now to prevent s/w from breaking. */
	uint32_t tmanlt                       : 5;  /**< Timer Modifier for Ack/Nak Latency Timer
                                                         Increases the timer value for the Ack/Nak latency timer, in
                                                         increments of 64 clock cycles. */
	uint32_t tmrt                         : 5;  /**< Timer Modifier for Replay Timer
                                                         Increases the timer value for the replay timer, in increments
                                                         of 64 clock cycles. */
	uint32_t reserved_8_13                : 6;
	uint32_t mfuncn                       : 8;  /**< Max Number of Functions Supported */
#else
	uint32_t mfuncn                       : 8;
	uint32_t reserved_8_13                : 6;
	uint32_t tmrt                         : 5;
	uint32_t tmanlt                       : 5;
	uint32_t tmfcwt                       : 5;
	uint32_t reserved_29_31               : 3;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg454_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg454 cvmx_pcieepvfx_cfg454_t;

/**
 * cvmx_pcieepvf#_cfg455
 *
 * PCIE_CFG455 = Four hundred fifty-sixth 32-bits of PCIE type 0 config space
 * (Symbol Timer Register/Filter Mask Register 1)
 */
union cvmx_pcieepvfx_cfg455 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg455_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t m_cfg0_filt                  : 1;  /**< Mask filtering of received Configuration Requests (RC mode only) */
	uint32_t m_io_filt                    : 1;  /**< Mask filtering of received I/O Requests (RC mode only) */
	uint32_t msg_ctrl                     : 1;  /**< Message Control
                                                         The application must not change this field. */
	uint32_t m_cpl_ecrc_filt              : 1;  /**< Mask ECRC error filtering for Completions */
	uint32_t m_ecrc_filt                  : 1;  /**< Mask ECRC error filtering */
	uint32_t m_cpl_len_err                : 1;  /**< Mask Length mismatch error for received Completions */
	uint32_t m_cpl_attr_err               : 1;  /**< Mask Attributes mismatch error for received Completions */
	uint32_t m_cpl_tc_err                 : 1;  /**< Mask Traffic Class mismatch error for received Completions */
	uint32_t m_cpl_fun_err                : 1;  /**< Mask function mismatch error for received Completions */
	uint32_t m_cpl_rid_err                : 1;  /**< Mask Requester ID mismatch error for received Completions */
	uint32_t m_cpl_tag_err                : 1;  /**< Mask Tag error rules for received Completions */
	uint32_t m_lk_filt                    : 1;  /**< Mask Locked Request filtering */
	uint32_t m_cfg1_filt                  : 1;  /**< Mask Type 1 Configuration Request filtering */
	uint32_t m_bar_match                  : 1;  /**< Mask BAR match filtering */
	uint32_t m_pois_filt                  : 1;  /**< Mask poisoned TLP filtering */
	uint32_t m_fun                        : 1;  /**< Mask function */
	uint32_t dfcwt                        : 1;  /**< Disable FC Watchdog Timer */
	uint32_t reserved_11_14               : 4;
	uint32_t skpiv                        : 11; /**< SKP Interval Value */
#else
	uint32_t skpiv                        : 11;
	uint32_t reserved_11_14               : 4;
	uint32_t dfcwt                        : 1;
	uint32_t m_fun                        : 1;
	uint32_t m_pois_filt                  : 1;
	uint32_t m_bar_match                  : 1;
	uint32_t m_cfg1_filt                  : 1;
	uint32_t m_lk_filt                    : 1;
	uint32_t m_cpl_tag_err                : 1;
	uint32_t m_cpl_rid_err                : 1;
	uint32_t m_cpl_fun_err                : 1;
	uint32_t m_cpl_tc_err                 : 1;
	uint32_t m_cpl_attr_err               : 1;
	uint32_t m_cpl_len_err                : 1;
	uint32_t m_ecrc_filt                  : 1;
	uint32_t m_cpl_ecrc_filt              : 1;
	uint32_t msg_ctrl                     : 1;
	uint32_t m_io_filt                    : 1;
	uint32_t m_cfg0_filt                  : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg455_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg455 cvmx_pcieepvfx_cfg455_t;

/**
 * cvmx_pcieepvf#_cfg456
 *
 * PCIE_CFG456 = Four hundred fifty-seventh 32-bits of PCIE type 0 config space
 * (Filter Mask Register 2)
 */
union cvmx_pcieepvfx_cfg456 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg456_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_4_31                : 28;
	uint32_t m_handle_flush               : 1;  /**< Mask Core Filter to handle flush request */
	uint32_t m_dabort_4ucpl               : 1;  /**< Mask DLLP abort for unexpected CPL */
	uint32_t m_vend1_drp                  : 1;  /**< Mask Vendor MSG Type 1 dropped silently */
	uint32_t m_vend0_drp                  : 1;  /**< Mask Vendor MSG Type 0 dropped with UR error reporting. */
#else
	uint32_t m_vend0_drp                  : 1;
	uint32_t m_vend1_drp                  : 1;
	uint32_t m_dabort_4ucpl               : 1;
	uint32_t m_handle_flush               : 1;
	uint32_t reserved_4_31                : 28;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg456_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg456 cvmx_pcieepvfx_cfg456_t;

/**
 * cvmx_pcieepvf#_cfg458
 *
 * PCIE_CFG458 = Four hundred fifty-ninth 32-bits of PCIE type 0 config space
 * (Debug Register 0)
 */
union cvmx_pcieepvfx_cfg458 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg458_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dbg_info_l32                 : 32; /**< Debug Info Lower 32 Bits */
#else
	uint32_t dbg_info_l32                 : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg458_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg458 cvmx_pcieepvfx_cfg458_t;

/**
 * cvmx_pcieepvf#_cfg459
 *
 * PCIE_CFG459 = Four hundred sixtieth 32-bits of PCIE type 0 config space
 * (Debug Register 1)
 */
union cvmx_pcieepvfx_cfg459 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg459_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dbg_info_u32                 : 32; /**< Debug Info Upper 32 Bits */
#else
	uint32_t dbg_info_u32                 : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg459_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg459 cvmx_pcieepvfx_cfg459_t;

/**
 * cvmx_pcieepvf#_cfg460
 *
 * PCIE_CFG460 = Four hundred sixty-first 32-bits of PCIE type 0 config space
 * (Transmit Posted FC Credit Status)
 */
union cvmx_pcieepvfx_cfg460 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg460_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t tphfcc                       : 8;  /**< Transmit Posted Header FC Credits
                                                         The Posted Header credits advertised by the receiver at the
                                                         other end of the Link, updated with each UpdateFC DLLP. */
	uint32_t tpdfcc                       : 12; /**< Transmit Posted Data FC Credits
                                                         The Posted Data credits advertised by the receiver at the other
                                                         end of the Link, updated with each UpdateFC DLLP. */
#else
	uint32_t tpdfcc                       : 12;
	uint32_t tphfcc                       : 8;
	uint32_t reserved_20_31               : 12;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg460_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg460 cvmx_pcieepvfx_cfg460_t;

/**
 * cvmx_pcieepvf#_cfg461
 *
 * PCIE_CFG461 = Four hundred sixty-second 32-bits of PCIE type 0 config space
 * (Transmit Non-Posted FC Credit Status)
 */
union cvmx_pcieepvfx_cfg461 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg461_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t tchfcc                       : 8;  /**< Transmit Non-Posted Header FC Credits
                                                         The Non-Posted Header credits advertised by the receiver at the
                                                         other end of the Link, updated with each UpdateFC DLLP. */
	uint32_t tcdfcc                       : 12; /**< Transmit Non-Posted Data FC Credits
                                                         The Non-Posted Data credits advertised by the receiver at the
                                                         other end of the Link, updated with each UpdateFC DLLP. */
#else
	uint32_t tcdfcc                       : 12;
	uint32_t tchfcc                       : 8;
	uint32_t reserved_20_31               : 12;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg461_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg461 cvmx_pcieepvfx_cfg461_t;

/**
 * cvmx_pcieepvf#_cfg462
 *
 * PCIE_CFG462 = Four hundred sixty-third 32-bits of PCIE type 0 config space
 * (Transmit Completion FC Credit Status )
 */
union cvmx_pcieepvfx_cfg462 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg462_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t tchfcc                       : 8;  /**< Transmit Completion Header FC Credits
                                                         The Completion Header credits advertised by the receiver at the
                                                         other end of the Link, updated with each UpdateFC DLLP. */
	uint32_t tcdfcc                       : 12; /**< Transmit Completion Data FC Credits
                                                         The Completion Data credits advertised by the receiver at the
                                                         other end of the Link, updated with each UpdateFC DLLP. */
#else
	uint32_t tcdfcc                       : 12;
	uint32_t tchfcc                       : 8;
	uint32_t reserved_20_31               : 12;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg462_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg462 cvmx_pcieepvfx_cfg462_t;

/**
 * cvmx_pcieepvf#_cfg463
 *
 * PCIE_CFG463 = Four hundred sixty-fourth 32-bits of PCIE type 0 config space
 * (Queue Status)
 */
union cvmx_pcieepvfx_cfg463 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg463_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t fcltoe                       : 1;  /**< FC Latency Timer Override Enable
                                                         When this bit is set, the value from the "FC Latency Timer Override
                                                         Value" field in this register will override the FC latency timer
                                                         value that the core calculates according to the PCIe specification. */
	uint32_t reserved_29_30               : 2;
	uint32_t fcltov                       : 13; /**< FC Latency Timer Override Value
                                                         When you set the "FC Latency Timer Override Enable" in this register,
                                                         the value in this field will override the FC latency timer value
                                                         that the core calculates according to the PCIe specification. */
	uint32_t reserved_3_15                : 13;
	uint32_t rqne                         : 1;  /**< Received Queue Not Empty
                                                         Indicates there is data in one or more of the receive buffers. */
	uint32_t trbne                        : 1;  /**< Transmit Retry Buffer Not Empty
                                                         Indicates that there is data in the transmit retry buffer. */
	uint32_t rtlpfccnr                    : 1;  /**< Received TLP FC Credits Not Returned
                                                         Indicates that the PCI Express bus has sent a TLP but has not
                                                         yet received an UpdateFC DLLP indicating that the credits for
                                                         that TLP have been restored by the receiver at the other end of
                                                         the Link. */
#else
	uint32_t rtlpfccnr                    : 1;
	uint32_t trbne                        : 1;
	uint32_t rqne                         : 1;
	uint32_t reserved_3_15                : 13;
	uint32_t fcltov                       : 13;
	uint32_t reserved_29_30               : 2;
	uint32_t fcltoe                       : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg463_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg463 cvmx_pcieepvfx_cfg463_t;

/**
 * cvmx_pcieepvf#_cfg464
 *
 * PCIE_CFG464 = Four hundred sixty-fifth 32-bits of PCIE type 0 config space
 * (VC Transmit Arbitration Register 1)
 */
union cvmx_pcieepvfx_cfg464 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg464_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t wrr_vc3                      : 8;  /**< WRR Weight for VC3 */
	uint32_t wrr_vc2                      : 8;  /**< WRR Weight for VC2 */
	uint32_t wrr_vc1                      : 8;  /**< WRR Weight for VC1 */
	uint32_t wrr_vc0                      : 8;  /**< WRR Weight for VC0 */
#else
	uint32_t wrr_vc0                      : 8;
	uint32_t wrr_vc1                      : 8;
	uint32_t wrr_vc2                      : 8;
	uint32_t wrr_vc3                      : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg464_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg464 cvmx_pcieepvfx_cfg464_t;

/**
 * cvmx_pcieepvf#_cfg465
 *
 * PCIE_CFG465 = Four hundred sixty-sixth 32-bits of PCIE type 0 config space
 * (VC Transmit Arbitration Register 2)
 */
union cvmx_pcieepvfx_cfg465 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg465_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t wrr_vc7                      : 8;  /**< WRR Weight for VC7 */
	uint32_t wrr_vc6                      : 8;  /**< WRR Weight for VC6 */
	uint32_t wrr_vc5                      : 8;  /**< WRR Weight for VC5 */
	uint32_t wrr_vc4                      : 8;  /**< WRR Weight for VC4 */
#else
	uint32_t wrr_vc4                      : 8;
	uint32_t wrr_vc5                      : 8;
	uint32_t wrr_vc6                      : 8;
	uint32_t wrr_vc7                      : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg465_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg465 cvmx_pcieepvfx_cfg465_t;

/**
 * cvmx_pcieepvf#_cfg466
 *
 * PCIE_CFG466 = Four hundred sixty-seventh 32-bits of PCIE type 0 config space
 * (VC0 Posted Receive Queue Control)
 */
union cvmx_pcieepvfx_cfg466 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg466_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t rx_queue_order               : 1;  /**< "VC Ordering for Receive Queues
                                                         Determines the VC ordering rule for the receive queues, used
                                                         only in the segmented-buffer configuration,
                                                         writable through PEM#_CFG_WR:
                                                         o 1: Strict ordering, higher numbered VCs have higher priority
                                                         o 0: Round robin
                                                         However, the application must not change this field." */
	uint32_t type_ordering                : 1;  /**< "TLP Type Ordering for VC0
                                                         Determines the TLP type ordering rule for VC0 receive queues,
                                                         used only in the segmented-buffer configuration,
                                                         through PEM#_CFG_WR:
                                                         o 1: Ordering of received TLPs follows the rules in
                                                         PCI Express Base Specification
                                                         o 0: Strict ordering for received TLPs: Posted, then
                                                         Completion, then Non-Posted
                                                         However, the application must not change this field." */
	uint32_t reserved_24_29               : 6;
	uint32_t queue_mode                   : 3;  /**< "VC0 Posted TLP Queue Mode
                                                         The operating mode of the Posted receive queue for VC0, used
                                                         only in the segmented-buffer configuration, writable through
                                                         PEM#_CFG_WR.
                                                         However, the application must not change this field.
                                                         Only one bit can be set at a time:
                                                         o Bit 23: Bypass
                                                         o Bit 22: Cut-through
                                                         o Bit 21: Store-and-forward" */
	uint32_t reserved_20_20               : 1;
	uint32_t header_credits               : 8;  /**< "VC0 Posted Header Credits
                                                         The number of initial Posted header credits for VC0, used for
                                                         all receive queue buffer configurations.
                                                         This field is writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t data_credits                 : 12; /**< "VC0 Posted Data Credits
                                                         The number of initial Posted data credits for VC0, used for all
                                                         receive queue buffer configurations.
                                                         This field is writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
#else
	uint32_t data_credits                 : 12;
	uint32_t header_credits               : 8;
	uint32_t reserved_20_20               : 1;
	uint32_t queue_mode                   : 3;
	uint32_t reserved_24_29               : 6;
	uint32_t type_ordering                : 1;
	uint32_t rx_queue_order               : 1;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg466_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg466 cvmx_pcieepvfx_cfg466_t;

/**
 * cvmx_pcieepvf#_cfg467
 *
 * PCIE_CFG467 = Four hundred sixty-eighth 32-bits of PCIE type 0 config space
 * (VC0 Non-Posted Receive Queue Control)
 */
union cvmx_pcieepvfx_cfg467 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg467_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t queue_mode                   : 3;  /**< "VC0 Non-Posted TLP Queue Mode
                                                         The operating mode of the Non-Posted receive queue for VC0,
                                                         used only in the segmented-buffer configuration, writable
                                                         through PEM#_CFG_WR.
                                                         Only one bit can be set at a time:
                                                         o Bit 23: Bypass
                                                         o Bit 22: Cut-through
                                                         o Bit 21: Store-and-forward
                                                         However, the application must not change this field." */
	uint32_t reserved_20_20               : 1;
	uint32_t header_credits               : 8;  /**< "VC0 Non-Posted Header Credits
                                                         The number of initial Non-Posted header credits for VC0, used
                                                         for all receive queue buffer configurations.
                                                         This field is writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t data_credits                 : 12; /**< "VC0 Non-Posted Data Credits
                                                         The number of initial Non-Posted data credits for VC0, used for
                                                         all receive queue buffer configurations.
                                                         This field is writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
#else
	uint32_t data_credits                 : 12;
	uint32_t header_credits               : 8;
	uint32_t reserved_20_20               : 1;
	uint32_t queue_mode                   : 3;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg467_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg467 cvmx_pcieepvfx_cfg467_t;

/**
 * cvmx_pcieepvf#_cfg468
 *
 * PCIE_CFG468 = Four hundred sixty-ninth 32-bits of PCIE type 0 config space
 * (VC0 Completion Receive Queue Control)
 */
union cvmx_pcieepvfx_cfg468 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg468_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t queue_mode                   : 3;  /**< "VC0 Completion TLP Queue Mode
                                                         The operating mode of the Completion receive queue for VC0,
                                                         used only in the segmented-buffer configuration, writable
                                                         through PEM#_CFG_WR.
                                                         Only one bit can be set at a time:
                                                         o Bit 23: Bypass
                                                         o Bit 22: Cut-through
                                                         o Bit 21: Store-and-forward
                                                         However, the application must not change this field." */
	uint32_t reserved_20_20               : 1;
	uint32_t header_credits               : 8;  /**< "VC0 Completion Header Credits
                                                         The number of initial Completion header credits for VC0, used
                                                         for all receive queue buffer configurations.
                                                         This field is writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t data_credits                 : 12; /**< "VC0 Completion Data Credits
                                                         The number of initial Completion data credits for VC0, used for
                                                         all receive queue buffer configurations.
                                                         This field is writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
#else
	uint32_t data_credits                 : 12;
	uint32_t header_credits               : 8;
	uint32_t reserved_20_20               : 1;
	uint32_t queue_mode                   : 3;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg468_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg468 cvmx_pcieepvfx_cfg468_t;

/**
 * cvmx_pcieepvf#_cfg490
 *
 * PCIE_CFG490 = Four hundred ninety-first 32-bits of PCIE type 0 config space
 * (VC0 Posted Buffer Depth)
 */
union cvmx_pcieepvfx_cfg490 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg490_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t header_depth                 : 10; /**< VC0 Posted Header Queue Depth
                                                         Sets the number of entries in the Posted header queue for VC0
                                                         when using the segmented-buffer configuration */
	uint32_t reserved_14_15               : 2;
	uint32_t data_depth                   : 14; /**< VC0 Posted Data Queue Depth
                                                         Sets the number of entries in the Posted data queue for VC0
                                                         when using the segmented-buffer configuration */
#else
	uint32_t data_depth                   : 14;
	uint32_t reserved_14_15               : 2;
	uint32_t header_depth                 : 10;
	uint32_t reserved_26_31               : 6;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg490_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg490 cvmx_pcieepvfx_cfg490_t;

/**
 * cvmx_pcieepvf#_cfg491
 *
 * PCIE_CFG491 = Four hundred ninety-second 32-bits of PCIE type 0 config space
 * (VC0 Non-Posted Buffer Depth)
 */
union cvmx_pcieepvfx_cfg491 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg491_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t header_depth                 : 10; /**< VC0 Non-Posted Header Queue Depth
                                                         Sets the number of entries in the Non-Posted header queue for
                                                         VC0 when using the segmented-buffer configuration */
	uint32_t reserved_14_15               : 2;
	uint32_t data_depth                   : 14; /**< VC0 Non-Posted Data Queue Depth
                                                         Sets the number of entries in the Non-Posted data queue for VC0
                                                         when using the segmented-buffer configuration */
#else
	uint32_t data_depth                   : 14;
	uint32_t reserved_14_15               : 2;
	uint32_t header_depth                 : 10;
	uint32_t reserved_26_31               : 6;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg491_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg491 cvmx_pcieepvfx_cfg491_t;

/**
 * cvmx_pcieepvf#_cfg492
 *
 * PCIE_CFG492 = Four hundred ninety-third 32-bits of PCIE type 0 config space
 * (VC0 Completion Buffer Depth)
 */
union cvmx_pcieepvfx_cfg492 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg492_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t header_depth                 : 10; /**< VC0 Completion Header Queue Depth
                                                         Sets the number of entries in the Completion header queue for
                                                         VC0 when using the segmented-buffer configuration */
	uint32_t reserved_14_15               : 2;
	uint32_t data_depth                   : 14; /**< VC0 Completion Data Queue Depth
                                                         Sets the number of entries in the Completion data queue for VC0
                                                         when using the segmented-buffer configuration */
#else
	uint32_t data_depth                   : 14;
	uint32_t reserved_14_15               : 2;
	uint32_t header_depth                 : 10;
	uint32_t reserved_26_31               : 6;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg492_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg492 cvmx_pcieepvfx_cfg492_t;

/**
 * cvmx_pcieepvf#_cfg515
 *
 * PCIE_CFG515 = Five hundred sixteenth 32-bits of PCIE type 0 config space
 * (Port Logic Register (Gen2))
 */
union cvmx_pcieepvfx_cfg515 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg515_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_21_31               : 11;
	uint32_t s_d_e                        : 1;  /**< SEL_DE_EMPHASIS
                                                         Used to set the de-emphasis level for upstream ports. */
	uint32_t ctcrb                        : 1;  /**< Config Tx Compliance Receive Bit
                                                         When set to 1, signals LTSSM to transmit TS ordered sets
                                                         with the compliance receive bit assert (equal to 1). */
	uint32_t cpyts                        : 1;  /**< Config PHY Tx Swing
                                                         Indicates the voltage level the PHY should drive. When set to
                                                         1, indicates Full Swing. When set to 0, indicates Low Swing */
	uint32_t dsc                          : 1;  /**< Directed Speed Change
                                                         o a write of '1' will initiate a speed change
                                                         o always reads a zero */
	uint32_t le                           : 9;  /**< Lane Enable
                                                         Indicates the number of lanes to check for exit from electrical
                                                         idle in Polling.Active and Polling.Compliance. 1 = x1, 2 = x2,
                                                         etc. Used to limit the maximum link width to ignore broken
                                                         lanes that detect a receiver, but will not exit electrical
                                                         idle and
                                                         would otherwise prevent a valid link from being configured. */
	uint32_t n_fts                        : 8;  /**< N_FTS
                                                         Sets the Number of Fast Training Sequences (N_FTS) that
                                                         the core advertises as its N_FTS during GEN2 Link training.
                                                         This value is used to inform the Link partner about the PHYs
                                                         ability to recover synchronization after a low power state.
                                                         Note: Do not set N_FTS to zero; doing so can cause the
                                                         LTSSM to go into the recovery state when exiting from
                                                         L0s. */
#else
	uint32_t n_fts                        : 8;
	uint32_t le                           : 9;
	uint32_t dsc                          : 1;
	uint32_t cpyts                        : 1;
	uint32_t ctcrb                        : 1;
	uint32_t s_d_e                        : 1;
	uint32_t reserved_21_31               : 11;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg515_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg515 cvmx_pcieepvfx_cfg515_t;

/**
 * cvmx_pcieepvf#_cfg516
 *
 * PCIE_CFG516 = Five hundred seventeenth 32-bits of PCIE type 0 config space
 * (PHY Status Register)
 */
union cvmx_pcieepvfx_cfg516 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg516_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t phy_stat                     : 32; /**< PHY Status */
#else
	uint32_t phy_stat                     : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg516_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg516 cvmx_pcieepvfx_cfg516_t;

/**
 * cvmx_pcieepvf#_cfg517
 *
 * PCIE_CFG517 = Five hundred eighteenth 32-bits of PCIE type 0 config space
 * (PHY Control Register)
 */
union cvmx_pcieepvfx_cfg517 {
	uint32_t u32;
	struct cvmx_pcieepvfx_cfg517_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t phy_ctrl                     : 32; /**< PHY Control */
#else
	uint32_t phy_ctrl                     : 32;
#endif
	} s;
	struct cvmx_pcieepvfx_cfg517_s        cn78xx;
};
typedef union cvmx_pcieepvfx_cfg517 cvmx_pcieepvfx_cfg517_t;

#endif
