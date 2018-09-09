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
 * cvmx-dtx-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon dtx.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_DTX_DEFS_H__
#define __CVMX_DTX_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_AGL_BCST_RSP CVMX_DTX_AGL_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_AGL_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_AGL_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE700080ull);
}
#else
#define CVMX_DTX_AGL_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE700080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_AGL_CTL CVMX_DTX_AGL_CTL_FUNC()
static inline uint64_t CVMX_DTX_AGL_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_AGL_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE700060ull);
}
#else
#define CVMX_DTX_AGL_CTL (CVMX_ADD_IO_SEG(0x00011800FE700060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_AGL_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_AGL_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE700040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_AGL_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE700040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_AGL_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_AGL_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE700020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_AGL_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE700020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_AGL_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_AGL_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE700000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_AGL_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE700000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_ASE_BCST_RSP CVMX_DTX_ASE_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_ASE_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_ASE_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE6E8080ull);
}
#else
#define CVMX_DTX_ASE_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE6E8080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_ASE_CTL CVMX_DTX_ASE_CTL_FUNC()
static inline uint64_t CVMX_DTX_ASE_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_ASE_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE6E8060ull);
}
#else
#define CVMX_DTX_ASE_CTL (CVMX_ADD_IO_SEG(0x00011800FE6E8060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_ASE_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_ASE_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE6E8040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_ASE_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE6E8040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_ASE_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_ASE_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE6E8020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_ASE_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE6E8020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_ASE_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_ASE_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE6E8000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_ASE_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE6E8000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_BGXX_BCST_RSP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 5)))))
		cvmx_warn("CVMX_DTX_BGXX_BCST_RSP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE700080ull) + ((offset) & 7) * 32768;
}
#else
#define CVMX_DTX_BGXX_BCST_RSP(offset) (CVMX_ADD_IO_SEG(0x00011800FE700080ull) + ((offset) & 7) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_BGXX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 5)))))
		cvmx_warn("CVMX_DTX_BGXX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE700060ull) + ((offset) & 7) * 32768;
}
#else
#define CVMX_DTX_BGXX_CTL(offset) (CVMX_ADD_IO_SEG(0x00011800FE700060ull) + ((offset) & 7) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_BGXX_DATX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_DTX_BGXX_DATX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE700040ull) + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_BGXX_DATX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE700040ull) + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_BGXX_ENAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_DTX_BGXX_ENAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE700020ull) + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_BGXX_ENAX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE700020ull) + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_BGXX_SELX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_DTX_BGXX_SELX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE700000ull) + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_BGXX_SELX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE700000ull) + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_BROADCAST_CTL CVMX_DTX_BROADCAST_CTL_FUNC()
static inline uint64_t CVMX_DTX_BROADCAST_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_BROADCAST_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE7F0060ull);
}
#else
#define CVMX_DTX_BROADCAST_CTL (CVMX_ADD_IO_SEG(0x00011800FE7F0060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_BROADCAST_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_BROADCAST_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE7F0020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_BROADCAST_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE7F0020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_BROADCAST_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_BROADCAST_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE7F0000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_BROADCAST_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE7F0000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_DFA_BCST_RSP CVMX_DTX_DFA_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_DFA_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_DFA_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE1B8080ull);
}
#else
#define CVMX_DTX_DFA_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE1B8080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_DFA_CTL CVMX_DTX_DFA_CTL_FUNC()
static inline uint64_t CVMX_DTX_DFA_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_DFA_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE1B8060ull);
}
#else
#define CVMX_DTX_DFA_CTL (CVMX_ADD_IO_SEG(0x00011800FE1B8060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_DFA_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_DFA_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE1B8040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_DFA_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE1B8040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_DFA_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_DFA_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE1B8020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_DFA_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE1B8020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_DFA_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_DFA_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE1B8000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_DFA_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE1B8000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_DPI_BCST_RSP CVMX_DTX_DPI_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_DPI_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_DPI_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FEEF8080ull);
}
#else
#define CVMX_DTX_DPI_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FEEF8080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_DPI_CTL CVMX_DTX_DPI_CTL_FUNC()
static inline uint64_t CVMX_DTX_DPI_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_DPI_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FEEF8060ull);
}
#else
#define CVMX_DTX_DPI_CTL (CVMX_ADD_IO_SEG(0x00011800FEEF8060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_DPI_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_DPI_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FEEF8040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_DPI_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FEEF8040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_DPI_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_DPI_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FEEF8020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_DPI_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FEEF8020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_DPI_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_DPI_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FEEF8000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_DPI_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FEEF8000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_FPA_BCST_RSP CVMX_DTX_FPA_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_FPA_BCST_RSP_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE140080ull);
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE940080ull);
			break;
	}
	cvmx_warn("CVMX_DTX_FPA_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE940080ull);
}
#else
#define CVMX_DTX_FPA_BCST_RSP CVMX_DTX_FPA_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_FPA_BCST_RSP_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE140080ull);
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE940080ull);
	}
	return CVMX_ADD_IO_SEG(0x00011800FE940080ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_FPA_CTL CVMX_DTX_FPA_CTL_FUNC()
static inline uint64_t CVMX_DTX_FPA_CTL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE140060ull);
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE940060ull);
			break;
	}
	cvmx_warn("CVMX_DTX_FPA_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE940060ull);
}
#else
#define CVMX_DTX_FPA_CTL CVMX_DTX_FPA_CTL_FUNC()
static inline uint64_t CVMX_DTX_FPA_CTL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE140060ull);
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE940060ull);
	}
	return CVMX_ADD_IO_SEG(0x00011800FE940060ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_FPA_DATX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800FE140040ull) + ((offset) & 1) * 8;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800FE940040ull) + ((offset) & 1) * 8;
			break;
	}
	cvmx_warn("CVMX_DTX_FPA_DATX (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE940040ull) + ((offset) & 1) * 8;
}
#else
static inline uint64_t CVMX_DTX_FPA_DATX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE140040ull) + (offset) * 8;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE940040ull) + (offset) * 8;
	}
	return CVMX_ADD_IO_SEG(0x00011800FE940040ull) + (offset) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_FPA_ENAX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800FE140020ull) + ((offset) & 1) * 8;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800FE940020ull) + ((offset) & 1) * 8;
			break;
	}
	cvmx_warn("CVMX_DTX_FPA_ENAX (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE940020ull) + ((offset) & 1) * 8;
}
#else
static inline uint64_t CVMX_DTX_FPA_ENAX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE140020ull) + (offset) * 8;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE940020ull) + (offset) * 8;
	}
	return CVMX_ADD_IO_SEG(0x00011800FE940020ull) + (offset) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_FPA_SELX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800FE140000ull) + ((offset) & 1) * 8;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800FE940000ull) + ((offset) & 1) * 8;
			break;
	}
	cvmx_warn("CVMX_DTX_FPA_SELX (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE940000ull) + ((offset) & 1) * 8;
}
#else
static inline uint64_t CVMX_DTX_FPA_SELX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE140000ull) + (offset) * 8;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE940000ull) + (offset) * 8;
	}
	return CVMX_ADD_IO_SEG(0x00011800FE940000ull) + (offset) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_GMXX_BCST_RSP(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_DTX_GMXX_BCST_RSP(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE040080ull) + ((block_id) & 1) * 0x40000ull;
}
#else
#define CVMX_DTX_GMXX_BCST_RSP(block_id) (CVMX_ADD_IO_SEG(0x00011800FE040080ull) + ((block_id) & 1) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_GMXX_CTL(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_DTX_GMXX_CTL(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE040060ull) + ((block_id) & 1) * 0x40000ull;
}
#else
#define CVMX_DTX_GMXX_CTL(block_id) (CVMX_ADD_IO_SEG(0x00011800FE040060ull) + ((block_id) & 1) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_GMXX_DATX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_DTX_GMXX_DATX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE040040ull) + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8;
}
#else
#define CVMX_DTX_GMXX_DATX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE040040ull) + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_GMXX_ENAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_DTX_GMXX_ENAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE040020ull) + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8;
}
#else
#define CVMX_DTX_GMXX_ENAX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE040020ull) + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_GMXX_SELX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_DTX_GMXX_SELX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE040000ull) + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8;
}
#else
#define CVMX_DTX_GMXX_SELX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE040000ull) + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_HNA_BCST_RSP CVMX_DTX_HNA_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_HNA_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_HNA_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE238080ull);
}
#else
#define CVMX_DTX_HNA_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE238080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_HNA_CTL CVMX_DTX_HNA_CTL_FUNC()
static inline uint64_t CVMX_DTX_HNA_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_HNA_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE238060ull);
}
#else
#define CVMX_DTX_HNA_CTL (CVMX_ADD_IO_SEG(0x00011800FE238060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_HNA_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_HNA_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE238040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_HNA_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE238040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_HNA_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_HNA_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE238020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_HNA_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE238020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_HNA_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_HNA_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE238000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_HNA_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE238000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_ILA_BCST_RSP CVMX_DTX_ILA_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_ILA_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_ILA_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE0B8080ull);
}
#else
#define CVMX_DTX_ILA_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE0B8080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_ILA_CTL CVMX_DTX_ILA_CTL_FUNC()
static inline uint64_t CVMX_DTX_ILA_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_ILA_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE0B8060ull);
}
#else
#define CVMX_DTX_ILA_CTL (CVMX_ADD_IO_SEG(0x00011800FE0B8060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_ILA_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_ILA_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE0B8040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_ILA_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE0B8040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_ILA_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_ILA_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE0B8020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_ILA_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE0B8020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_ILA_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_ILA_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE0B8000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_ILA_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE0B8000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_ILK_BCST_RSP CVMX_DTX_ILK_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_ILK_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_ILK_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE0A0080ull);
}
#else
#define CVMX_DTX_ILK_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE0A0080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_ILK_CTL CVMX_DTX_ILK_CTL_FUNC()
static inline uint64_t CVMX_DTX_ILK_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_ILK_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE0A0060ull);
}
#else
#define CVMX_DTX_ILK_CTL (CVMX_ADD_IO_SEG(0x00011800FE0A0060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_ILK_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_ILK_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE0A0040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_ILK_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE0A0040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_ILK_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_ILK_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE0A0020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_ILK_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE0A0020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_ILK_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_ILK_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE0A0000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_ILK_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE0A0000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_IOBN_BCST_RSP CVMX_DTX_IOBN_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_IOBN_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_IOBN_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE780080ull);
}
#else
#define CVMX_DTX_IOBN_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE780080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_IOBN_CTL CVMX_DTX_IOBN_CTL_FUNC()
static inline uint64_t CVMX_DTX_IOBN_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_IOBN_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE780060ull);
}
#else
#define CVMX_DTX_IOBN_CTL (CVMX_ADD_IO_SEG(0x00011800FE780060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_IOBN_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_IOBN_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE780040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_IOBN_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE780040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_IOBN_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_IOBN_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE780020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_IOBN_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE780020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_IOBN_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_IOBN_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE780000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_IOBN_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE780000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_IOBP_BCST_RSP CVMX_DTX_IOBP_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_IOBP_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_IOBP_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE7A0080ull);
}
#else
#define CVMX_DTX_IOBP_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE7A0080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_IOBP_CTL CVMX_DTX_IOBP_CTL_FUNC()
static inline uint64_t CVMX_DTX_IOBP_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_IOBP_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE7A0060ull);
}
#else
#define CVMX_DTX_IOBP_CTL (CVMX_ADD_IO_SEG(0x00011800FE7A0060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_IOBP_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_IOBP_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE7A0040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_IOBP_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE7A0040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_IOBP_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_IOBP_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE7A0020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_IOBP_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE7A0020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_IOBP_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_IOBP_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE7A0000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_IOBP_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE7A0000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_IOB_BCST_RSP CVMX_DTX_IOB_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_IOB_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_IOB_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE780080ull);
}
#else
#define CVMX_DTX_IOB_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE780080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_IOB_CTL CVMX_DTX_IOB_CTL_FUNC()
static inline uint64_t CVMX_DTX_IOB_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_IOB_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE780060ull);
}
#else
#define CVMX_DTX_IOB_CTL (CVMX_ADD_IO_SEG(0x00011800FE780060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_IOB_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_IOB_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE780040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_IOB_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE780040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_IOB_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_IOB_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE780020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_IOB_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE780020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_IOB_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_IOB_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE780000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_IOB_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE780000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_IPD_BCST_RSP CVMX_DTX_IPD_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_IPD_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_IPD_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE278080ull);
}
#else
#define CVMX_DTX_IPD_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE278080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_IPD_CTL CVMX_DTX_IPD_CTL_FUNC()
static inline uint64_t CVMX_DTX_IPD_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_IPD_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE278060ull);
}
#else
#define CVMX_DTX_IPD_CTL (CVMX_ADD_IO_SEG(0x00011800FE278060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_IPD_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_IPD_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE278040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_IPD_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE278040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_IPD_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_IPD_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE278020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_IPD_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE278020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_IPD_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_IPD_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE278000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_IPD_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE278000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_KEY_BCST_RSP CVMX_DTX_KEY_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_KEY_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_KEY_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE100080ull);
}
#else
#define CVMX_DTX_KEY_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE100080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_KEY_CTL CVMX_DTX_KEY_CTL_FUNC()
static inline uint64_t CVMX_DTX_KEY_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_KEY_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE100060ull);
}
#else
#define CVMX_DTX_KEY_CTL (CVMX_ADD_IO_SEG(0x00011800FE100060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_KEY_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_KEY_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE100040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_KEY_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE100040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_KEY_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_KEY_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE100020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_KEY_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE100020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_KEY_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_KEY_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE100000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_KEY_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE100000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_L2C_CBCX_BCST_RSP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_DTX_L2C_CBCX_BCST_RSP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE420080ull) + ((offset) & 3) * 32768;
}
#else
#define CVMX_DTX_L2C_CBCX_BCST_RSP(offset) (CVMX_ADD_IO_SEG(0x00011800FE420080ull) + ((offset) & 3) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_L2C_CBCX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_DTX_L2C_CBCX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE420060ull) + ((offset) & 3) * 32768;
}
#else
#define CVMX_DTX_L2C_CBCX_CTL(offset) (CVMX_ADD_IO_SEG(0x00011800FE420060ull) + ((offset) & 3) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_L2C_CBCX_DATX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_DTX_L2C_CBCX_DATX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE420040ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_L2C_CBCX_DATX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE420040ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_L2C_CBCX_ENAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_DTX_L2C_CBCX_ENAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE420020ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_L2C_CBCX_ENAX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE420020ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_L2C_CBCX_SELX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_DTX_L2C_CBCX_SELX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE420000ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_L2C_CBCX_SELX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE420000ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_L2C_MCIX_BCST_RSP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_DTX_L2C_MCIX_BCST_RSP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE2E0080ull) + ((offset) & 3) * 32768;
}
#else
#define CVMX_DTX_L2C_MCIX_BCST_RSP(offset) (CVMX_ADD_IO_SEG(0x00011800FE2E0080ull) + ((offset) & 3) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_L2C_MCIX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_DTX_L2C_MCIX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE2E0060ull) + ((offset) & 3) * 32768;
}
#else
#define CVMX_DTX_L2C_MCIX_CTL(offset) (CVMX_ADD_IO_SEG(0x00011800FE2E0060ull) + ((offset) & 3) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_L2C_MCIX_DATX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_DTX_L2C_MCIX_DATX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE2E0040ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_L2C_MCIX_DATX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE2E0040ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_L2C_MCIX_ENAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_DTX_L2C_MCIX_ENAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE2E0020ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_L2C_MCIX_ENAX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE2E0020ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_L2C_MCIX_SELX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_DTX_L2C_MCIX_SELX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE2E0000ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_L2C_MCIX_SELX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE2E0000ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_L2C_TADX_BCST_RSP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_DTX_L2C_TADX_BCST_RSP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE240080ull) + ((offset) & 7) * 32768;
}
#else
#define CVMX_DTX_L2C_TADX_BCST_RSP(offset) (CVMX_ADD_IO_SEG(0x00011800FE240080ull) + ((offset) & 7) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_L2C_TADX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_DTX_L2C_TADX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE240060ull) + ((offset) & 7) * 32768;
}
#else
#define CVMX_DTX_L2C_TADX_CTL(offset) (CVMX_ADD_IO_SEG(0x00011800FE240060ull) + ((offset) & 7) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_L2C_TADX_DATX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 7))))))
		cvmx_warn("CVMX_DTX_L2C_TADX_DATX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE240040ull) + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_L2C_TADX_DATX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE240040ull) + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_L2C_TADX_ENAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 7))))))
		cvmx_warn("CVMX_DTX_L2C_TADX_ENAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE240020ull) + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_L2C_TADX_ENAX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE240020ull) + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_L2C_TADX_SELX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 7))))))
		cvmx_warn("CVMX_DTX_L2C_TADX_SELX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE240000ull) + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_L2C_TADX_SELX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE240000ull) + (((offset) & 1) + ((block_id) & 7) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_LAPX_BCST_RSP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_LAPX_BCST_RSP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE060080ull) + ((offset) & 1) * 32768;
}
#else
#define CVMX_DTX_LAPX_BCST_RSP(offset) (CVMX_ADD_IO_SEG(0x00011800FE060080ull) + ((offset) & 1) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_LAPX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_LAPX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE060060ull) + ((offset) & 1) * 32768;
}
#else
#define CVMX_DTX_LAPX_CTL(offset) (CVMX_ADD_IO_SEG(0x00011800FE060060ull) + ((offset) & 1) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_LAPX_DATX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_DTX_LAPX_DATX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE060040ull) + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_LAPX_DATX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE060040ull) + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_LAPX_ENAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_DTX_LAPX_ENAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE060020ull) + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_LAPX_ENAX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE060020ull) + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_LAPX_SELX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_DTX_LAPX_SELX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE060000ull) + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_LAPX_SELX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE060000ull) + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_LBK_BCST_RSP CVMX_DTX_LBK_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_LBK_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_LBK_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE090080ull);
}
#else
#define CVMX_DTX_LBK_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE090080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_LBK_CTL CVMX_DTX_LBK_CTL_FUNC()
static inline uint64_t CVMX_DTX_LBK_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_LBK_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE090060ull);
}
#else
#define CVMX_DTX_LBK_CTL (CVMX_ADD_IO_SEG(0x00011800FE090060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_LBK_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_LBK_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE090040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_LBK_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE090040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_LBK_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_LBK_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE090020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_LBK_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE090020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_LBK_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_LBK_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE090000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_LBK_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE090000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_LMCX_BCST_RSP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_DTX_LMCX_BCST_RSP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE440080ull) + ((offset) & 3) * 32768;
}
#else
#define CVMX_DTX_LMCX_BCST_RSP(offset) (CVMX_ADD_IO_SEG(0x00011800FE440080ull) + ((offset) & 3) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_LMCX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_DTX_LMCX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE440060ull) + ((offset) & 3) * 32768;
}
#else
#define CVMX_DTX_LMCX_CTL(offset) (CVMX_ADD_IO_SEG(0x00011800FE440060ull) + ((offset) & 3) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_LMCX_DATX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_DTX_LMCX_DATX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE440040ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_LMCX_DATX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE440040ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_LMCX_ENAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_DTX_LMCX_ENAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE440020ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_LMCX_ENAX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE440020ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_LMCX_SELX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_DTX_LMCX_SELX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE440000ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_LMCX_SELX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE440000ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_MIO_BCST_RSP CVMX_DTX_MIO_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_MIO_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_MIO_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE000080ull);
}
#else
#define CVMX_DTX_MIO_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE000080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_MIO_CTL CVMX_DTX_MIO_CTL_FUNC()
static inline uint64_t CVMX_DTX_MIO_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_MIO_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE000060ull);
}
#else
#define CVMX_DTX_MIO_CTL (CVMX_ADD_IO_SEG(0x00011800FE000060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_MIO_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_MIO_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE000040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_MIO_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE000040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_MIO_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_MIO_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE000020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_MIO_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE000020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_MIO_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_MIO_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE000000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_MIO_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE000000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OCX_LNK_DTXDIDX_BCST_RSP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2)))))
		cvmx_warn("CVMX_DTX_OCX_LNK_DTXDIDX_BCST_RSP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE180080ull) + ((offset) & 3) * 32768;
}
#else
#define CVMX_DTX_OCX_LNK_DTXDIDX_BCST_RSP(offset) (CVMX_ADD_IO_SEG(0x00011800FE180080ull) + ((offset) & 3) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OCX_LNK_DTXDIDX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2)))))
		cvmx_warn("CVMX_DTX_OCX_LNK_DTXDIDX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE180060ull) + ((offset) & 3) * 32768;
}
#else
#define CVMX_DTX_OCX_LNK_DTXDIDX_CTL(offset) (CVMX_ADD_IO_SEG(0x00011800FE180060ull) + ((offset) & 3) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OCX_LNK_DTXDIDX_DATX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 2))))))
		cvmx_warn("CVMX_DTX_OCX_LNK_DTXDIDX_DATX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE180040ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_OCX_LNK_DTXDIDX_DATX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE180040ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OCX_LNK_DTXDIDX_ENAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 2))))))
		cvmx_warn("CVMX_DTX_OCX_LNK_DTXDIDX_ENAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE180020ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_OCX_LNK_DTXDIDX_ENAX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE180020ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OCX_LNK_DTXDIDX_SELX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 2))))))
		cvmx_warn("CVMX_DTX_OCX_LNK_DTXDIDX_SELX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE180000ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_OCX_LNK_DTXDIDX_SELX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE180000ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OCX_OLEX_BCST_RSP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2)))))
		cvmx_warn("CVMX_DTX_OCX_OLEX_BCST_RSP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE1A0080ull) + ((offset) & 3) * 32768;
}
#else
#define CVMX_DTX_OCX_OLEX_BCST_RSP(offset) (CVMX_ADD_IO_SEG(0x00011800FE1A0080ull) + ((offset) & 3) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OCX_OLEX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2)))))
		cvmx_warn("CVMX_DTX_OCX_OLEX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE1A0060ull) + ((offset) & 3) * 32768;
}
#else
#define CVMX_DTX_OCX_OLEX_CTL(offset) (CVMX_ADD_IO_SEG(0x00011800FE1A0060ull) + ((offset) & 3) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OCX_OLEX_DATX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 2))))))
		cvmx_warn("CVMX_DTX_OCX_OLEX_DATX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE1A0040ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_OCX_OLEX_DATX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE1A0040ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OCX_OLEX_ENAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 2))))))
		cvmx_warn("CVMX_DTX_OCX_OLEX_ENAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE1A0020ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_OCX_OLEX_ENAX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE1A0020ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OCX_OLEX_SELX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 2))))))
		cvmx_warn("CVMX_DTX_OCX_OLEX_SELX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE1A0000ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_OCX_OLEX_SELX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE1A0000ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_OCX_TOP_DTXDID_BCST_RSP CVMX_DTX_OCX_TOP_DTXDID_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_OCX_TOP_DTXDID_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_OCX_TOP_DTXDID_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE088080ull);
}
#else
#define CVMX_DTX_OCX_TOP_DTXDID_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE088080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_OCX_TOP_DTXDID_CTL CVMX_DTX_OCX_TOP_DTXDID_CTL_FUNC()
static inline uint64_t CVMX_DTX_OCX_TOP_DTXDID_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_OCX_TOP_DTXDID_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE088060ull);
}
#else
#define CVMX_DTX_OCX_TOP_DTXDID_CTL (CVMX_ADD_IO_SEG(0x00011800FE088060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OCX_TOP_DTXDID_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_OCX_TOP_DTXDID_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE088040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_OCX_TOP_DTXDID_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE088040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OCX_TOP_DTXDID_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_OCX_TOP_DTXDID_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE088020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_OCX_TOP_DTXDID_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE088020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OCX_TOP_DTXDID_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_OCX_TOP_DTXDID_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE088000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_OCX_TOP_DTXDID_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE088000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_OSM_BCST_RSP CVMX_DTX_OSM_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_OSM_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_OSM_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE6E0080ull);
}
#else
#define CVMX_DTX_OSM_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE6E0080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_OSM_CTL CVMX_DTX_OSM_CTL_FUNC()
static inline uint64_t CVMX_DTX_OSM_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_OSM_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE6E0060ull);
}
#else
#define CVMX_DTX_OSM_CTL (CVMX_ADD_IO_SEG(0x00011800FE6E0060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OSM_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_OSM_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE6E0040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_OSM_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE6E0040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OSM_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_OSM_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE6E0020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_OSM_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE6E0020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_OSM_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_OSM_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE6E0000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_OSM_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE6E0000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PCSX_BCST_RSP(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_DTX_PCSX_BCST_RSP(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE580080ull) + ((block_id) & 1) * 0x40000ull;
}
#else
#define CVMX_DTX_PCSX_BCST_RSP(block_id) (CVMX_ADD_IO_SEG(0x00011800FE580080ull) + ((block_id) & 1) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PCSX_CTL(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_DTX_PCSX_CTL(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE580060ull) + ((block_id) & 1) * 0x40000ull;
}
#else
#define CVMX_DTX_PCSX_CTL(block_id) (CVMX_ADD_IO_SEG(0x00011800FE580060ull) + ((block_id) & 1) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PCSX_DATX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_DTX_PCSX_DATX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE580040ull) + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8;
}
#else
#define CVMX_DTX_PCSX_DATX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE580040ull) + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PCSX_ENAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_DTX_PCSX_ENAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE580020ull) + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8;
}
#else
#define CVMX_DTX_PCSX_ENAX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE580020ull) + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PCSX_SELX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_DTX_PCSX_SELX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE580000ull) + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8;
}
#else
#define CVMX_DTX_PCSX_SELX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE580000ull) + (((offset) & 1) + ((block_id) & 1) * 0x8000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PEMX_BCST_RSP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_DTX_PEMX_BCST_RSP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE600080ull) + ((offset) & 3) * 32768;
}
#else
#define CVMX_DTX_PEMX_BCST_RSP(offset) (CVMX_ADD_IO_SEG(0x00011800FE600080ull) + ((offset) & 3) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PEMX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_DTX_PEMX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE600060ull) + ((offset) & 3) * 32768;
}
#else
#define CVMX_DTX_PEMX_CTL(offset) (CVMX_ADD_IO_SEG(0x00011800FE600060ull) + ((offset) & 3) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PEMX_DATX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id <= 2)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_DTX_PEMX_DATX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE600040ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_PEMX_DATX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE600040ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PEMX_ENAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id <= 2)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_DTX_PEMX_ENAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE600020ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_PEMX_ENAX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE600020ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PEMX_SELX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id <= 2)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_DTX_PEMX_SELX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE600000ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_PEMX_SELX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE600000ull) + (((offset) & 1) + ((block_id) & 3) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_PIP_BCST_RSP CVMX_DTX_PIP_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_PIP_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_PIP_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE500080ull);
}
#else
#define CVMX_DTX_PIP_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE500080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_PIP_CTL CVMX_DTX_PIP_CTL_FUNC()
static inline uint64_t CVMX_DTX_PIP_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_PIP_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE500060ull);
}
#else
#define CVMX_DTX_PIP_CTL (CVMX_ADD_IO_SEG(0x00011800FE500060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PIP_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_PIP_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE500040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_PIP_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE500040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PIP_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_PIP_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE500020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_PIP_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE500020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PIP_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_PIP_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE500000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_PIP_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE500000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_PKI_PBE_BCST_RSP CVMX_DTX_PKI_PBE_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_PKI_PBE_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_PKI_PBE_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE228080ull);
}
#else
#define CVMX_DTX_PKI_PBE_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE228080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_PKI_PBE_CTL CVMX_DTX_PKI_PBE_CTL_FUNC()
static inline uint64_t CVMX_DTX_PKI_PBE_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_PKI_PBE_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE228060ull);
}
#else
#define CVMX_DTX_PKI_PBE_CTL (CVMX_ADD_IO_SEG(0x00011800FE228060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PKI_PBE_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_PKI_PBE_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE228040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_PKI_PBE_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE228040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PKI_PBE_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_PKI_PBE_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE228020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_PKI_PBE_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE228020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PKI_PBE_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_PKI_PBE_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE228000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_PKI_PBE_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE228000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_PKI_PFE_BCST_RSP CVMX_DTX_PKI_PFE_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_PKI_PFE_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_PKI_PFE_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE220080ull);
}
#else
#define CVMX_DTX_PKI_PFE_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE220080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_PKI_PFE_CTL CVMX_DTX_PKI_PFE_CTL_FUNC()
static inline uint64_t CVMX_DTX_PKI_PFE_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_PKI_PFE_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE220060ull);
}
#else
#define CVMX_DTX_PKI_PFE_CTL (CVMX_ADD_IO_SEG(0x00011800FE220060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PKI_PFE_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_PKI_PFE_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE220040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_PKI_PFE_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE220040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PKI_PFE_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_PKI_PFE_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE220020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_PKI_PFE_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE220020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PKI_PFE_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_PKI_PFE_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE220000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_PKI_PFE_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE220000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_PKI_PIX_BCST_RSP CVMX_DTX_PKI_PIX_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_PKI_PIX_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_PKI_PIX_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE230080ull);
}
#else
#define CVMX_DTX_PKI_PIX_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE230080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_PKI_PIX_CTL CVMX_DTX_PKI_PIX_CTL_FUNC()
static inline uint64_t CVMX_DTX_PKI_PIX_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_PKI_PIX_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE230060ull);
}
#else
#define CVMX_DTX_PKI_PIX_CTL (CVMX_ADD_IO_SEG(0x00011800FE230060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PKI_PIX_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_PKI_PIX_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE230040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_PKI_PIX_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE230040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PKI_PIX_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_PKI_PIX_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE230020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_PKI_PIX_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE230020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PKI_PIX_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_PKI_PIX_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE230000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_PKI_PIX_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE230000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_PKO_BCST_RSP CVMX_DTX_PKO_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_PKO_BCST_RSP_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE280080ull);
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FEAA0080ull);
			break;
	}
	cvmx_warn("CVMX_DTX_PKO_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FEAA0080ull);
}
#else
#define CVMX_DTX_PKO_BCST_RSP CVMX_DTX_PKO_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_PKO_BCST_RSP_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE280080ull);
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FEAA0080ull);
	}
	return CVMX_ADD_IO_SEG(0x00011800FEAA0080ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_PKO_CTL CVMX_DTX_PKO_CTL_FUNC()
static inline uint64_t CVMX_DTX_PKO_CTL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE280060ull);
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FEAA0060ull);
			break;
	}
	cvmx_warn("CVMX_DTX_PKO_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FEAA0060ull);
}
#else
#define CVMX_DTX_PKO_CTL CVMX_DTX_PKO_CTL_FUNC()
static inline uint64_t CVMX_DTX_PKO_CTL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE280060ull);
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FEAA0060ull);
	}
	return CVMX_ADD_IO_SEG(0x00011800FEAA0060ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PKO_DATX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800FE280040ull) + ((offset) & 1) * 8;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800FEAA0040ull) + ((offset) & 1) * 8;
			break;
	}
	cvmx_warn("CVMX_DTX_PKO_DATX (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FEAA0040ull) + ((offset) & 1) * 8;
}
#else
static inline uint64_t CVMX_DTX_PKO_DATX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE280040ull) + (offset) * 8;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FEAA0040ull) + (offset) * 8;
	}
	return CVMX_ADD_IO_SEG(0x00011800FEAA0040ull) + (offset) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PKO_ENAX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800FE280020ull) + ((offset) & 1) * 8;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800FEAA0020ull) + ((offset) & 1) * 8;
			break;
	}
	cvmx_warn("CVMX_DTX_PKO_ENAX (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FEAA0020ull) + ((offset) & 1) * 8;
}
#else
static inline uint64_t CVMX_DTX_PKO_ENAX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE280020ull) + (offset) * 8;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FEAA0020ull) + (offset) * 8;
	}
	return CVMX_ADD_IO_SEG(0x00011800FEAA0020ull) + (offset) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_PKO_SELX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800FE280000ull) + ((offset) & 1) * 8;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800FEAA0000ull) + ((offset) & 1) * 8;
			break;
	}
	cvmx_warn("CVMX_DTX_PKO_SELX (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FEAA0000ull) + ((offset) & 1) * 8;
}
#else
static inline uint64_t CVMX_DTX_PKO_SELX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FE280000ull) + (offset) * 8;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800FEAA0000ull) + (offset) * 8;
	}
	return CVMX_ADD_IO_SEG(0x00011800FEAA0000ull) + (offset) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_POW_BCST_RSP CVMX_DTX_POW_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_POW_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_POW_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE338080ull);
}
#else
#define CVMX_DTX_POW_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE338080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_POW_CTL CVMX_DTX_POW_CTL_FUNC()
static inline uint64_t CVMX_DTX_POW_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_POW_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE338060ull);
}
#else
#define CVMX_DTX_POW_CTL (CVMX_ADD_IO_SEG(0x00011800FE338060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_POW_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_POW_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE338040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_POW_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE338040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_POW_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_POW_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE338020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_POW_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE338020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_POW_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_POW_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE338000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_POW_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE338000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_RAD_BCST_RSP CVMX_DTX_RAD_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_RAD_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_RAD_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE380080ull);
}
#else
#define CVMX_DTX_RAD_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE380080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_RAD_CTL CVMX_DTX_RAD_CTL_FUNC()
static inline uint64_t CVMX_DTX_RAD_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_RAD_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE380060ull);
}
#else
#define CVMX_DTX_RAD_CTL (CVMX_ADD_IO_SEG(0x00011800FE380060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_RAD_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_RAD_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE380040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_RAD_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE380040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_RAD_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_RAD_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE380020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_RAD_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE380020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_RAD_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_RAD_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE380000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_RAD_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE380000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_RST_BCST_RSP CVMX_DTX_RST_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_RST_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_RST_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE030080ull);
}
#else
#define CVMX_DTX_RST_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE030080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_RST_CTL CVMX_DTX_RST_CTL_FUNC()
static inline uint64_t CVMX_DTX_RST_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_RST_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE030060ull);
}
#else
#define CVMX_DTX_RST_CTL (CVMX_ADD_IO_SEG(0x00011800FE030060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_RST_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_RST_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE030040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_RST_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE030040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_RST_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_RST_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE030020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_RST_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE030020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_RST_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_RST_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE030000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_RST_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE030000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_SATA_BCST_RSP CVMX_DTX_SATA_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_SATA_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_SATA_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE360080ull);
}
#else
#define CVMX_DTX_SATA_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE360080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_SATA_CTL CVMX_DTX_SATA_CTL_FUNC()
static inline uint64_t CVMX_DTX_SATA_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_DTX_SATA_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE360060ull);
}
#else
#define CVMX_DTX_SATA_CTL (CVMX_ADD_IO_SEG(0x00011800FE360060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_SATA_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_SATA_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE360040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_SATA_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE360040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_SATA_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_SATA_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE360020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_SATA_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE360020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_SATA_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_SATA_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE360000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_SATA_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE360000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_SLI_BCST_RSP CVMX_DTX_SLI_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_SLI_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_SLI_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE8F8080ull);
}
#else
#define CVMX_DTX_SLI_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE8F8080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_SLI_CTL CVMX_DTX_SLI_CTL_FUNC()
static inline uint64_t CVMX_DTX_SLI_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_SLI_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE8F8060ull);
}
#else
#define CVMX_DTX_SLI_CTL (CVMX_ADD_IO_SEG(0x00011800FE8F8060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_SLI_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_SLI_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE8F8040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_SLI_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE8F8040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_SLI_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_SLI_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE8F8020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_SLI_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE8F8020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_SLI_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_SLI_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE8F8000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_SLI_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE8F8000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_SSO_BCST_RSP CVMX_DTX_SSO_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_SSO_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_SSO_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE338080ull);
}
#else
#define CVMX_DTX_SSO_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE338080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_SSO_CTL CVMX_DTX_SSO_CTL_FUNC()
static inline uint64_t CVMX_DTX_SSO_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_SSO_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE338060ull);
}
#else
#define CVMX_DTX_SSO_CTL (CVMX_ADD_IO_SEG(0x00011800FE338060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_SSO_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_SSO_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE338040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_SSO_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE338040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_SSO_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_SSO_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE338020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_SSO_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE338020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_SSO_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_SSO_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE338000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_SSO_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE338000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_TIM_BCST_RSP CVMX_DTX_TIM_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_TIM_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_TIM_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE2C0080ull);
}
#else
#define CVMX_DTX_TIM_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE2C0080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_TIM_CTL CVMX_DTX_TIM_CTL_FUNC()
static inline uint64_t CVMX_DTX_TIM_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_TIM_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE2C0060ull);
}
#else
#define CVMX_DTX_TIM_CTL (CVMX_ADD_IO_SEG(0x00011800FE2C0060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_TIM_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_TIM_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE2C0040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_TIM_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE2C0040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_TIM_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_TIM_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE2C0020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_TIM_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE2C0020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_TIM_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_TIM_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE2C0000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_TIM_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE2C0000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_USBDRDX_BCST_RSP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_USBDRDX_BCST_RSP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE340080ull) + ((offset) & 1) * 32768;
}
#else
#define CVMX_DTX_USBDRDX_BCST_RSP(offset) (CVMX_ADD_IO_SEG(0x00011800FE340080ull) + ((offset) & 1) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_USBDRDX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_USBDRDX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE340060ull) + ((offset) & 1) * 32768;
}
#else
#define CVMX_DTX_USBDRDX_CTL(offset) (CVMX_ADD_IO_SEG(0x00011800FE340060ull) + ((offset) & 1) * 32768)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_USBDRDX_DATX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_DTX_USBDRDX_DATX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE340040ull) + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_USBDRDX_DATX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE340040ull) + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_USBDRDX_ENAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_DTX_USBDRDX_ENAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE340020ull) + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_USBDRDX_ENAX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE340020ull) + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_USBDRDX_SELX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_DTX_USBDRDX_SELX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE340000ull) + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8;
}
#else
#define CVMX_DTX_USBDRDX_SELX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE340000ull) + (((offset) & 1) + ((block_id) & 1) * 0x1000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_USBHX_BCST_RSP(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id == 0)))))
		cvmx_warn("CVMX_DTX_USBHX_BCST_RSP(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE340080ull);
}
#else
#define CVMX_DTX_USBHX_BCST_RSP(block_id) (CVMX_ADD_IO_SEG(0x00011800FE340080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_USBHX_CTL(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id == 0)))))
		cvmx_warn("CVMX_DTX_USBHX_CTL(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE340060ull);
}
#else
#define CVMX_DTX_USBHX_CTL(block_id) (CVMX_ADD_IO_SEG(0x00011800FE340060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_USBHX_DATX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_DTX_USBHX_DATX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE340040ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8;
}
#else
#define CVMX_DTX_USBHX_DATX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE340040ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_USBHX_ENAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_DTX_USBHX_ENAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE340020ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8;
}
#else
#define CVMX_DTX_USBHX_ENAX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE340020ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_USBHX_SELX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_DTX_USBHX_SELX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800FE340000ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8;
}
#else
#define CVMX_DTX_USBHX_SELX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800FE340000ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_ZIP_BCST_RSP CVMX_DTX_ZIP_BCST_RSP_FUNC()
static inline uint64_t CVMX_DTX_ZIP_BCST_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_ZIP_BCST_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE1C0080ull);
}
#else
#define CVMX_DTX_ZIP_BCST_RSP (CVMX_ADD_IO_SEG(0x00011800FE1C0080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DTX_ZIP_CTL CVMX_DTX_ZIP_CTL_FUNC()
static inline uint64_t CVMX_DTX_ZIP_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_DTX_ZIP_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800FE1C0060ull);
}
#else
#define CVMX_DTX_ZIP_CTL (CVMX_ADD_IO_SEG(0x00011800FE1C0060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_ZIP_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_ZIP_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE1C0040ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_ZIP_DATX(offset) (CVMX_ADD_IO_SEG(0x00011800FE1C0040ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_ZIP_ENAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_ZIP_ENAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE1C0020ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_ZIP_ENAX(offset) (CVMX_ADD_IO_SEG(0x00011800FE1C0020ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DTX_ZIP_SELX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_DTX_ZIP_SELX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800FE1C0000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_DTX_ZIP_SELX(offset) (CVMX_ADD_IO_SEG(0x00011800FE1C0000ull) + ((offset) & 1) * 8)
#endif

/**
 * cvmx_dtx_agl_bcst_rsp
 */
union cvmx_dtx_agl_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_agl_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_agl_bcst_rsp_s        cn70xx;
};
typedef union cvmx_dtx_agl_bcst_rsp cvmx_dtx_agl_bcst_rsp_t;

/**
 * cvmx_dtx_agl_ctl
 */
union cvmx_dtx_agl_ctl {
	uint64_t u64;
	struct cvmx_dtx_agl_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in DTX_MIO_ENA(0..1) instead of normal block debug data.
                                                         Not applicable when SW directly reads the DAT(0..1) registers.  For diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_agl_ctl_s             cn70xx;
};
typedef union cvmx_dtx_agl_ctl cvmx_dtx_agl_ctl_t;

/**
 * cvmx_dtx_agl_dat#
 */
union cvmx_dtx_agl_datx {
	uint64_t u64;
	struct cvmx_dtx_agl_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_agl_datx_s            cn70xx;
};
typedef union cvmx_dtx_agl_datx cvmx_dtx_agl_datx_t;

/**
 * cvmx_dtx_agl_ena#
 */
union cvmx_dtx_agl_enax {
	uint64_t u64;
	struct cvmx_dtx_agl_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_agl_enax_s            cn70xx;
};
typedef union cvmx_dtx_agl_enax cvmx_dtx_agl_enax_t;

/**
 * cvmx_dtx_agl_sel#
 */
union cvmx_dtx_agl_selx {
	uint64_t u64;
	struct cvmx_dtx_agl_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_agl_selx_s            cn70xx;
};
typedef union cvmx_dtx_agl_selx cvmx_dtx_agl_selx_t;

/**
 * cvmx_dtx_ase_bcst_rsp
 */
union cvmx_dtx_ase_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_ase_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_ase_bcst_rsp_s        cn78xx;
};
typedef union cvmx_dtx_ase_bcst_rsp cvmx_dtx_ase_bcst_rsp_t;

/**
 * cvmx_dtx_ase_ctl
 */
union cvmx_dtx_ase_ctl {
	uint64_t u64;
	struct cvmx_dtx_ase_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_ase_ctl_s             cn78xx;
};
typedef union cvmx_dtx_ase_ctl cvmx_dtx_ase_ctl_t;

/**
 * cvmx_dtx_ase_dat#
 */
union cvmx_dtx_ase_datx {
	uint64_t u64;
	struct cvmx_dtx_ase_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_ase_datx_s            cn78xx;
};
typedef union cvmx_dtx_ase_datx cvmx_dtx_ase_datx_t;

/**
 * cvmx_dtx_ase_ena#
 */
union cvmx_dtx_ase_enax {
	uint64_t u64;
	struct cvmx_dtx_ase_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_ase_enax_s            cn78xx;
};
typedef union cvmx_dtx_ase_enax cvmx_dtx_ase_enax_t;

/**
 * cvmx_dtx_ase_sel#
 */
union cvmx_dtx_ase_selx {
	uint64_t u64;
	struct cvmx_dtx_ase_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_ase_selx_s            cn78xx;
};
typedef union cvmx_dtx_ase_selx cvmx_dtx_ase_selx_t;

/**
 * cvmx_dtx_bgx#_bcst_rsp
 */
union cvmx_dtx_bgxx_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_bgxx_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_bgxx_bcst_rsp_s       cn78xx;
};
typedef union cvmx_dtx_bgxx_bcst_rsp cvmx_dtx_bgxx_bcst_rsp_t;

/**
 * cvmx_dtx_bgx#_ctl
 */
union cvmx_dtx_bgxx_ctl {
	uint64_t u64;
	struct cvmx_dtx_bgxx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_bgxx_ctl_s            cn78xx;
};
typedef union cvmx_dtx_bgxx_ctl cvmx_dtx_bgxx_ctl_t;

/**
 * cvmx_dtx_bgx#_dat#
 */
union cvmx_dtx_bgxx_datx {
	uint64_t u64;
	struct cvmx_dtx_bgxx_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_bgxx_datx_s           cn78xx;
};
typedef union cvmx_dtx_bgxx_datx cvmx_dtx_bgxx_datx_t;

/**
 * cvmx_dtx_bgx#_ena#
 */
union cvmx_dtx_bgxx_enax {
	uint64_t u64;
	struct cvmx_dtx_bgxx_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_bgxx_enax_s           cn78xx;
};
typedef union cvmx_dtx_bgxx_enax cvmx_dtx_bgxx_enax_t;

/**
 * cvmx_dtx_bgx#_sel#
 */
union cvmx_dtx_bgxx_selx {
	uint64_t u64;
	struct cvmx_dtx_bgxx_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_bgxx_selx_s           cn78xx;
};
typedef union cvmx_dtx_bgxx_selx cvmx_dtx_bgxx_selx_t;

/**
 * cvmx_dtx_broadcast_ctl
 */
union cvmx_dtx_broadcast_ctl {
	uint64_t u64;
	struct cvmx_dtx_broadcast_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_broadcast_ctl_s       cn70xx;
	struct cvmx_dtx_broadcast_ctl_s       cn78xx;
};
typedef union cvmx_dtx_broadcast_ctl cvmx_dtx_broadcast_ctl_t;

/**
 * cvmx_dtx_broadcast_ena#
 */
union cvmx_dtx_broadcast_enax {
	uint64_t u64;
	struct cvmx_dtx_broadcast_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_broadcast_enax_s      cn70xx;
	struct cvmx_dtx_broadcast_enax_s      cn78xx;
};
typedef union cvmx_dtx_broadcast_enax cvmx_dtx_broadcast_enax_t;

/**
 * cvmx_dtx_broadcast_sel#
 */
union cvmx_dtx_broadcast_selx {
	uint64_t u64;
	struct cvmx_dtx_broadcast_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_broadcast_selx_s      cn70xx;
	struct cvmx_dtx_broadcast_selx_s      cn78xx;
};
typedef union cvmx_dtx_broadcast_selx cvmx_dtx_broadcast_selx_t;

/**
 * cvmx_dtx_dfa_bcst_rsp
 */
union cvmx_dtx_dfa_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_dfa_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_dfa_bcst_rsp_s        cn70xx;
	struct cvmx_dtx_dfa_bcst_rsp_s        cn78xx;
};
typedef union cvmx_dtx_dfa_bcst_rsp cvmx_dtx_dfa_bcst_rsp_t;

/**
 * cvmx_dtx_dfa_ctl
 */
union cvmx_dtx_dfa_ctl {
	uint64_t u64;
	struct cvmx_dtx_dfa_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_dfa_ctl_s             cn70xx;
	struct cvmx_dtx_dfa_ctl_s             cn78xx;
};
typedef union cvmx_dtx_dfa_ctl cvmx_dtx_dfa_ctl_t;

/**
 * cvmx_dtx_dfa_dat#
 */
union cvmx_dtx_dfa_datx {
	uint64_t u64;
	struct cvmx_dtx_dfa_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_dfa_datx_s            cn70xx;
	struct cvmx_dtx_dfa_datx_s            cn78xx;
};
typedef union cvmx_dtx_dfa_datx cvmx_dtx_dfa_datx_t;

/**
 * cvmx_dtx_dfa_ena#
 */
union cvmx_dtx_dfa_enax {
	uint64_t u64;
	struct cvmx_dtx_dfa_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_dfa_enax_s            cn70xx;
	struct cvmx_dtx_dfa_enax_s            cn78xx;
};
typedef union cvmx_dtx_dfa_enax cvmx_dtx_dfa_enax_t;

/**
 * cvmx_dtx_dfa_sel#
 */
union cvmx_dtx_dfa_selx {
	uint64_t u64;
	struct cvmx_dtx_dfa_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_dfa_selx_s            cn70xx;
	struct cvmx_dtx_dfa_selx_s            cn78xx;
};
typedef union cvmx_dtx_dfa_selx cvmx_dtx_dfa_selx_t;

/**
 * cvmx_dtx_dpi_bcst_rsp
 */
union cvmx_dtx_dpi_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_dpi_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_dpi_bcst_rsp_s        cn70xx;
	struct cvmx_dtx_dpi_bcst_rsp_s        cn78xx;
};
typedef union cvmx_dtx_dpi_bcst_rsp cvmx_dtx_dpi_bcst_rsp_t;

/**
 * cvmx_dtx_dpi_ctl
 */
union cvmx_dtx_dpi_ctl {
	uint64_t u64;
	struct cvmx_dtx_dpi_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_dpi_ctl_s             cn70xx;
	struct cvmx_dtx_dpi_ctl_s             cn78xx;
};
typedef union cvmx_dtx_dpi_ctl cvmx_dtx_dpi_ctl_t;

/**
 * cvmx_dtx_dpi_dat#
 */
union cvmx_dtx_dpi_datx {
	uint64_t u64;
	struct cvmx_dtx_dpi_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_dpi_datx_s            cn70xx;
	struct cvmx_dtx_dpi_datx_s            cn78xx;
};
typedef union cvmx_dtx_dpi_datx cvmx_dtx_dpi_datx_t;

/**
 * cvmx_dtx_dpi_ena#
 */
union cvmx_dtx_dpi_enax {
	uint64_t u64;
	struct cvmx_dtx_dpi_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_dpi_enax_s            cn70xx;
	struct cvmx_dtx_dpi_enax_s            cn78xx;
};
typedef union cvmx_dtx_dpi_enax cvmx_dtx_dpi_enax_t;

/**
 * cvmx_dtx_dpi_sel#
 */
union cvmx_dtx_dpi_selx {
	uint64_t u64;
	struct cvmx_dtx_dpi_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_dpi_selx_s            cn70xx;
	struct cvmx_dtx_dpi_selx_s            cn78xx;
};
typedef union cvmx_dtx_dpi_selx cvmx_dtx_dpi_selx_t;

/**
 * cvmx_dtx_fpa_bcst_rsp
 */
union cvmx_dtx_fpa_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_fpa_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_fpa_bcst_rsp_s        cn70xx;
	struct cvmx_dtx_fpa_bcst_rsp_s        cn78xx;
};
typedef union cvmx_dtx_fpa_bcst_rsp cvmx_dtx_fpa_bcst_rsp_t;

/**
 * cvmx_dtx_fpa_ctl
 */
union cvmx_dtx_fpa_ctl {
	uint64_t u64;
	struct cvmx_dtx_fpa_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_fpa_ctl_s             cn70xx;
	struct cvmx_dtx_fpa_ctl_s             cn78xx;
};
typedef union cvmx_dtx_fpa_ctl cvmx_dtx_fpa_ctl_t;

/**
 * cvmx_dtx_fpa_dat#
 */
union cvmx_dtx_fpa_datx {
	uint64_t u64;
	struct cvmx_dtx_fpa_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_fpa_datx_s            cn70xx;
	struct cvmx_dtx_fpa_datx_s            cn78xx;
};
typedef union cvmx_dtx_fpa_datx cvmx_dtx_fpa_datx_t;

/**
 * cvmx_dtx_fpa_ena#
 */
union cvmx_dtx_fpa_enax {
	uint64_t u64;
	struct cvmx_dtx_fpa_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_fpa_enax_s            cn70xx;
	struct cvmx_dtx_fpa_enax_s            cn78xx;
};
typedef union cvmx_dtx_fpa_enax cvmx_dtx_fpa_enax_t;

/**
 * cvmx_dtx_fpa_sel#
 */
union cvmx_dtx_fpa_selx {
	uint64_t u64;
	struct cvmx_dtx_fpa_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_fpa_selx_s            cn70xx;
	struct cvmx_dtx_fpa_selx_s            cn78xx;
};
typedef union cvmx_dtx_fpa_selx cvmx_dtx_fpa_selx_t;

/**
 * cvmx_dtx_gmx#_bcst_rsp
 */
union cvmx_dtx_gmxx_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_gmxx_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_gmxx_bcst_rsp_s       cn70xx;
};
typedef union cvmx_dtx_gmxx_bcst_rsp cvmx_dtx_gmxx_bcst_rsp_t;

/**
 * cvmx_dtx_gmx#_ctl
 */
union cvmx_dtx_gmxx_ctl {
	uint64_t u64;
	struct cvmx_dtx_gmxx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in DTX_MIO_ENA(0..1) instead of normal block debug data.
                                                         Not applicable when SW directly reads the DAT(0..1) registers.  For diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_gmxx_ctl_s            cn70xx;
};
typedef union cvmx_dtx_gmxx_ctl cvmx_dtx_gmxx_ctl_t;

/**
 * cvmx_dtx_gmx#_dat#
 */
union cvmx_dtx_gmxx_datx {
	uint64_t u64;
	struct cvmx_dtx_gmxx_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_gmxx_datx_s           cn70xx;
};
typedef union cvmx_dtx_gmxx_datx cvmx_dtx_gmxx_datx_t;

/**
 * cvmx_dtx_gmx#_ena#
 */
union cvmx_dtx_gmxx_enax {
	uint64_t u64;
	struct cvmx_dtx_gmxx_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_gmxx_enax_s           cn70xx;
};
typedef union cvmx_dtx_gmxx_enax cvmx_dtx_gmxx_enax_t;

/**
 * cvmx_dtx_gmx#_sel#
 */
union cvmx_dtx_gmxx_selx {
	uint64_t u64;
	struct cvmx_dtx_gmxx_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_gmxx_selx_s           cn70xx;
};
typedef union cvmx_dtx_gmxx_selx cvmx_dtx_gmxx_selx_t;

/**
 * cvmx_dtx_hna_bcst_rsp
 */
union cvmx_dtx_hna_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_hna_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_hna_bcst_rsp_s        cn78xx;
};
typedef union cvmx_dtx_hna_bcst_rsp cvmx_dtx_hna_bcst_rsp_t;

/**
 * cvmx_dtx_hna_ctl
 */
union cvmx_dtx_hna_ctl {
	uint64_t u64;
	struct cvmx_dtx_hna_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_hna_ctl_s             cn78xx;
};
typedef union cvmx_dtx_hna_ctl cvmx_dtx_hna_ctl_t;

/**
 * cvmx_dtx_hna_dat#
 */
union cvmx_dtx_hna_datx {
	uint64_t u64;
	struct cvmx_dtx_hna_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_hna_datx_s            cn78xx;
};
typedef union cvmx_dtx_hna_datx cvmx_dtx_hna_datx_t;

/**
 * cvmx_dtx_hna_ena#
 */
union cvmx_dtx_hna_enax {
	uint64_t u64;
	struct cvmx_dtx_hna_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_hna_enax_s            cn78xx;
};
typedef union cvmx_dtx_hna_enax cvmx_dtx_hna_enax_t;

/**
 * cvmx_dtx_hna_sel#
 */
union cvmx_dtx_hna_selx {
	uint64_t u64;
	struct cvmx_dtx_hna_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_hna_selx_s            cn78xx;
};
typedef union cvmx_dtx_hna_selx cvmx_dtx_hna_selx_t;

/**
 * cvmx_dtx_ila_bcst_rsp
 */
union cvmx_dtx_ila_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_ila_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_ila_bcst_rsp_s        cn78xx;
};
typedef union cvmx_dtx_ila_bcst_rsp cvmx_dtx_ila_bcst_rsp_t;

/**
 * cvmx_dtx_ila_ctl
 */
union cvmx_dtx_ila_ctl {
	uint64_t u64;
	struct cvmx_dtx_ila_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_ila_ctl_s             cn78xx;
};
typedef union cvmx_dtx_ila_ctl cvmx_dtx_ila_ctl_t;

/**
 * cvmx_dtx_ila_dat#
 */
union cvmx_dtx_ila_datx {
	uint64_t u64;
	struct cvmx_dtx_ila_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_ila_datx_s            cn78xx;
};
typedef union cvmx_dtx_ila_datx cvmx_dtx_ila_datx_t;

/**
 * cvmx_dtx_ila_ena#
 */
union cvmx_dtx_ila_enax {
	uint64_t u64;
	struct cvmx_dtx_ila_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_ila_enax_s            cn78xx;
};
typedef union cvmx_dtx_ila_enax cvmx_dtx_ila_enax_t;

/**
 * cvmx_dtx_ila_sel#
 */
union cvmx_dtx_ila_selx {
	uint64_t u64;
	struct cvmx_dtx_ila_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_ila_selx_s            cn78xx;
};
typedef union cvmx_dtx_ila_selx cvmx_dtx_ila_selx_t;

/**
 * cvmx_dtx_ilk_bcst_rsp
 */
union cvmx_dtx_ilk_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_ilk_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_ilk_bcst_rsp_s        cn78xx;
};
typedef union cvmx_dtx_ilk_bcst_rsp cvmx_dtx_ilk_bcst_rsp_t;

/**
 * cvmx_dtx_ilk_ctl
 */
union cvmx_dtx_ilk_ctl {
	uint64_t u64;
	struct cvmx_dtx_ilk_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_ilk_ctl_s             cn78xx;
};
typedef union cvmx_dtx_ilk_ctl cvmx_dtx_ilk_ctl_t;

/**
 * cvmx_dtx_ilk_dat#
 */
union cvmx_dtx_ilk_datx {
	uint64_t u64;
	struct cvmx_dtx_ilk_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_ilk_datx_s            cn78xx;
};
typedef union cvmx_dtx_ilk_datx cvmx_dtx_ilk_datx_t;

/**
 * cvmx_dtx_ilk_ena#
 */
union cvmx_dtx_ilk_enax {
	uint64_t u64;
	struct cvmx_dtx_ilk_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_ilk_enax_s            cn78xx;
};
typedef union cvmx_dtx_ilk_enax cvmx_dtx_ilk_enax_t;

/**
 * cvmx_dtx_ilk_sel#
 */
union cvmx_dtx_ilk_selx {
	uint64_t u64;
	struct cvmx_dtx_ilk_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_ilk_selx_s            cn78xx;
};
typedef union cvmx_dtx_ilk_selx cvmx_dtx_ilk_selx_t;

/**
 * cvmx_dtx_iob_bcst_rsp
 */
union cvmx_dtx_iob_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_iob_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_iob_bcst_rsp_s        cn70xx;
};
typedef union cvmx_dtx_iob_bcst_rsp cvmx_dtx_iob_bcst_rsp_t;

/**
 * cvmx_dtx_iob_ctl
 */
union cvmx_dtx_iob_ctl {
	uint64_t u64;
	struct cvmx_dtx_iob_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in DTX_MIO_ENA(0..1) instead of normal block debug data.
                                                         Not applicable when SW directly reads the DAT(0..1) registers.  For diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_iob_ctl_s             cn70xx;
};
typedef union cvmx_dtx_iob_ctl cvmx_dtx_iob_ctl_t;

/**
 * cvmx_dtx_iob_dat#
 */
union cvmx_dtx_iob_datx {
	uint64_t u64;
	struct cvmx_dtx_iob_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_iob_datx_s            cn70xx;
};
typedef union cvmx_dtx_iob_datx cvmx_dtx_iob_datx_t;

/**
 * cvmx_dtx_iob_ena#
 */
union cvmx_dtx_iob_enax {
	uint64_t u64;
	struct cvmx_dtx_iob_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_iob_enax_s            cn70xx;
};
typedef union cvmx_dtx_iob_enax cvmx_dtx_iob_enax_t;

/**
 * cvmx_dtx_iob_sel#
 */
union cvmx_dtx_iob_selx {
	uint64_t u64;
	struct cvmx_dtx_iob_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_iob_selx_s            cn70xx;
};
typedef union cvmx_dtx_iob_selx cvmx_dtx_iob_selx_t;

/**
 * cvmx_dtx_iobn_bcst_rsp
 */
union cvmx_dtx_iobn_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_iobn_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_iobn_bcst_rsp_s       cn78xx;
};
typedef union cvmx_dtx_iobn_bcst_rsp cvmx_dtx_iobn_bcst_rsp_t;

/**
 * cvmx_dtx_iobn_ctl
 */
union cvmx_dtx_iobn_ctl {
	uint64_t u64;
	struct cvmx_dtx_iobn_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_iobn_ctl_s            cn78xx;
};
typedef union cvmx_dtx_iobn_ctl cvmx_dtx_iobn_ctl_t;

/**
 * cvmx_dtx_iobn_dat#
 */
union cvmx_dtx_iobn_datx {
	uint64_t u64;
	struct cvmx_dtx_iobn_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_iobn_datx_s           cn78xx;
};
typedef union cvmx_dtx_iobn_datx cvmx_dtx_iobn_datx_t;

/**
 * cvmx_dtx_iobn_ena#
 */
union cvmx_dtx_iobn_enax {
	uint64_t u64;
	struct cvmx_dtx_iobn_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_iobn_enax_s           cn78xx;
};
typedef union cvmx_dtx_iobn_enax cvmx_dtx_iobn_enax_t;

/**
 * cvmx_dtx_iobn_sel#
 */
union cvmx_dtx_iobn_selx {
	uint64_t u64;
	struct cvmx_dtx_iobn_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_iobn_selx_s           cn78xx;
};
typedef union cvmx_dtx_iobn_selx cvmx_dtx_iobn_selx_t;

/**
 * cvmx_dtx_iobp_bcst_rsp
 */
union cvmx_dtx_iobp_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_iobp_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_iobp_bcst_rsp_s       cn78xx;
};
typedef union cvmx_dtx_iobp_bcst_rsp cvmx_dtx_iobp_bcst_rsp_t;

/**
 * cvmx_dtx_iobp_ctl
 */
union cvmx_dtx_iobp_ctl {
	uint64_t u64;
	struct cvmx_dtx_iobp_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_iobp_ctl_s            cn78xx;
};
typedef union cvmx_dtx_iobp_ctl cvmx_dtx_iobp_ctl_t;

/**
 * cvmx_dtx_iobp_dat#
 */
union cvmx_dtx_iobp_datx {
	uint64_t u64;
	struct cvmx_dtx_iobp_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_iobp_datx_s           cn78xx;
};
typedef union cvmx_dtx_iobp_datx cvmx_dtx_iobp_datx_t;

/**
 * cvmx_dtx_iobp_ena#
 */
union cvmx_dtx_iobp_enax {
	uint64_t u64;
	struct cvmx_dtx_iobp_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_iobp_enax_s           cn78xx;
};
typedef union cvmx_dtx_iobp_enax cvmx_dtx_iobp_enax_t;

/**
 * cvmx_dtx_iobp_sel#
 */
union cvmx_dtx_iobp_selx {
	uint64_t u64;
	struct cvmx_dtx_iobp_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_iobp_selx_s           cn78xx;
};
typedef union cvmx_dtx_iobp_selx cvmx_dtx_iobp_selx_t;

/**
 * cvmx_dtx_ipd_bcst_rsp
 */
union cvmx_dtx_ipd_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_ipd_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_ipd_bcst_rsp_s        cn70xx;
};
typedef union cvmx_dtx_ipd_bcst_rsp cvmx_dtx_ipd_bcst_rsp_t;

/**
 * cvmx_dtx_ipd_ctl
 */
union cvmx_dtx_ipd_ctl {
	uint64_t u64;
	struct cvmx_dtx_ipd_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in DTX_MIO_ENA(0..1) instead of normal block debug data.
                                                         Not applicable when SW directly reads the DAT(0..1) registers.  For diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_ipd_ctl_s             cn70xx;
};
typedef union cvmx_dtx_ipd_ctl cvmx_dtx_ipd_ctl_t;

/**
 * cvmx_dtx_ipd_dat#
 */
union cvmx_dtx_ipd_datx {
	uint64_t u64;
	struct cvmx_dtx_ipd_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_ipd_datx_s            cn70xx;
};
typedef union cvmx_dtx_ipd_datx cvmx_dtx_ipd_datx_t;

/**
 * cvmx_dtx_ipd_ena#
 */
union cvmx_dtx_ipd_enax {
	uint64_t u64;
	struct cvmx_dtx_ipd_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_ipd_enax_s            cn70xx;
};
typedef union cvmx_dtx_ipd_enax cvmx_dtx_ipd_enax_t;

/**
 * cvmx_dtx_ipd_sel#
 */
union cvmx_dtx_ipd_selx {
	uint64_t u64;
	struct cvmx_dtx_ipd_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_ipd_selx_s            cn70xx;
};
typedef union cvmx_dtx_ipd_selx cvmx_dtx_ipd_selx_t;

/**
 * cvmx_dtx_key_bcst_rsp
 */
union cvmx_dtx_key_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_key_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_key_bcst_rsp_s        cn70xx;
};
typedef union cvmx_dtx_key_bcst_rsp cvmx_dtx_key_bcst_rsp_t;

/**
 * cvmx_dtx_key_ctl
 */
union cvmx_dtx_key_ctl {
	uint64_t u64;
	struct cvmx_dtx_key_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in DTX_MIO_ENA(0..1) instead of normal block debug data.
                                                         Not applicable when SW directly reads the DAT(0..1) registers.  For diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_key_ctl_s             cn70xx;
};
typedef union cvmx_dtx_key_ctl cvmx_dtx_key_ctl_t;

/**
 * cvmx_dtx_key_dat#
 */
union cvmx_dtx_key_datx {
	uint64_t u64;
	struct cvmx_dtx_key_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_key_datx_s            cn70xx;
};
typedef union cvmx_dtx_key_datx cvmx_dtx_key_datx_t;

/**
 * cvmx_dtx_key_ena#
 */
union cvmx_dtx_key_enax {
	uint64_t u64;
	struct cvmx_dtx_key_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_key_enax_s            cn70xx;
};
typedef union cvmx_dtx_key_enax cvmx_dtx_key_enax_t;

/**
 * cvmx_dtx_key_sel#
 */
union cvmx_dtx_key_selx {
	uint64_t u64;
	struct cvmx_dtx_key_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select. Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_key_selx_s            cn70xx;
};
typedef union cvmx_dtx_key_selx cvmx_dtx_key_selx_t;

/**
 * cvmx_dtx_l2c_cbc#_bcst_rsp
 */
union cvmx_dtx_l2c_cbcx_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_l2c_cbcx_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_l2c_cbcx_bcst_rsp_s   cn70xx;
	struct cvmx_dtx_l2c_cbcx_bcst_rsp_s   cn78xx;
};
typedef union cvmx_dtx_l2c_cbcx_bcst_rsp cvmx_dtx_l2c_cbcx_bcst_rsp_t;

/**
 * cvmx_dtx_l2c_cbc#_ctl
 */
union cvmx_dtx_l2c_cbcx_ctl {
	uint64_t u64;
	struct cvmx_dtx_l2c_cbcx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_l2c_cbcx_ctl_s        cn70xx;
	struct cvmx_dtx_l2c_cbcx_ctl_s        cn78xx;
};
typedef union cvmx_dtx_l2c_cbcx_ctl cvmx_dtx_l2c_cbcx_ctl_t;

/**
 * cvmx_dtx_l2c_cbc#_dat#
 */
union cvmx_dtx_l2c_cbcx_datx {
	uint64_t u64;
	struct cvmx_dtx_l2c_cbcx_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_l2c_cbcx_datx_s       cn70xx;
	struct cvmx_dtx_l2c_cbcx_datx_s       cn78xx;
};
typedef union cvmx_dtx_l2c_cbcx_datx cvmx_dtx_l2c_cbcx_datx_t;

/**
 * cvmx_dtx_l2c_cbc#_ena#
 */
union cvmx_dtx_l2c_cbcx_enax {
	uint64_t u64;
	struct cvmx_dtx_l2c_cbcx_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_l2c_cbcx_enax_s       cn70xx;
	struct cvmx_dtx_l2c_cbcx_enax_s       cn78xx;
};
typedef union cvmx_dtx_l2c_cbcx_enax cvmx_dtx_l2c_cbcx_enax_t;

/**
 * cvmx_dtx_l2c_cbc#_sel#
 */
union cvmx_dtx_l2c_cbcx_selx {
	uint64_t u64;
	struct cvmx_dtx_l2c_cbcx_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_l2c_cbcx_selx_s       cn70xx;
	struct cvmx_dtx_l2c_cbcx_selx_s       cn78xx;
};
typedef union cvmx_dtx_l2c_cbcx_selx cvmx_dtx_l2c_cbcx_selx_t;

/**
 * cvmx_dtx_l2c_mci#_bcst_rsp
 */
union cvmx_dtx_l2c_mcix_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_l2c_mcix_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_l2c_mcix_bcst_rsp_s   cn70xx;
	struct cvmx_dtx_l2c_mcix_bcst_rsp_s   cn78xx;
};
typedef union cvmx_dtx_l2c_mcix_bcst_rsp cvmx_dtx_l2c_mcix_bcst_rsp_t;

/**
 * cvmx_dtx_l2c_mci#_ctl
 */
union cvmx_dtx_l2c_mcix_ctl {
	uint64_t u64;
	struct cvmx_dtx_l2c_mcix_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_l2c_mcix_ctl_s        cn70xx;
	struct cvmx_dtx_l2c_mcix_ctl_s        cn78xx;
};
typedef union cvmx_dtx_l2c_mcix_ctl cvmx_dtx_l2c_mcix_ctl_t;

/**
 * cvmx_dtx_l2c_mci#_dat#
 */
union cvmx_dtx_l2c_mcix_datx {
	uint64_t u64;
	struct cvmx_dtx_l2c_mcix_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_l2c_mcix_datx_s       cn70xx;
	struct cvmx_dtx_l2c_mcix_datx_s       cn78xx;
};
typedef union cvmx_dtx_l2c_mcix_datx cvmx_dtx_l2c_mcix_datx_t;

/**
 * cvmx_dtx_l2c_mci#_ena#
 */
union cvmx_dtx_l2c_mcix_enax {
	uint64_t u64;
	struct cvmx_dtx_l2c_mcix_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_l2c_mcix_enax_s       cn70xx;
	struct cvmx_dtx_l2c_mcix_enax_s       cn78xx;
};
typedef union cvmx_dtx_l2c_mcix_enax cvmx_dtx_l2c_mcix_enax_t;

/**
 * cvmx_dtx_l2c_mci#_sel#
 */
union cvmx_dtx_l2c_mcix_selx {
	uint64_t u64;
	struct cvmx_dtx_l2c_mcix_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_l2c_mcix_selx_s       cn70xx;
	struct cvmx_dtx_l2c_mcix_selx_s       cn78xx;
};
typedef union cvmx_dtx_l2c_mcix_selx cvmx_dtx_l2c_mcix_selx_t;

/**
 * cvmx_dtx_l2c_tad#_bcst_rsp
 */
union cvmx_dtx_l2c_tadx_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_l2c_tadx_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_l2c_tadx_bcst_rsp_s   cn70xx;
	struct cvmx_dtx_l2c_tadx_bcst_rsp_s   cn78xx;
};
typedef union cvmx_dtx_l2c_tadx_bcst_rsp cvmx_dtx_l2c_tadx_bcst_rsp_t;

/**
 * cvmx_dtx_l2c_tad#_ctl
 */
union cvmx_dtx_l2c_tadx_ctl {
	uint64_t u64;
	struct cvmx_dtx_l2c_tadx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_l2c_tadx_ctl_s        cn70xx;
	struct cvmx_dtx_l2c_tadx_ctl_s        cn78xx;
};
typedef union cvmx_dtx_l2c_tadx_ctl cvmx_dtx_l2c_tadx_ctl_t;

/**
 * cvmx_dtx_l2c_tad#_dat#
 */
union cvmx_dtx_l2c_tadx_datx {
	uint64_t u64;
	struct cvmx_dtx_l2c_tadx_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_l2c_tadx_datx_s       cn70xx;
	struct cvmx_dtx_l2c_tadx_datx_s       cn78xx;
};
typedef union cvmx_dtx_l2c_tadx_datx cvmx_dtx_l2c_tadx_datx_t;

/**
 * cvmx_dtx_l2c_tad#_ena#
 */
union cvmx_dtx_l2c_tadx_enax {
	uint64_t u64;
	struct cvmx_dtx_l2c_tadx_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_l2c_tadx_enax_s       cn70xx;
	struct cvmx_dtx_l2c_tadx_enax_s       cn78xx;
};
typedef union cvmx_dtx_l2c_tadx_enax cvmx_dtx_l2c_tadx_enax_t;

/**
 * cvmx_dtx_l2c_tad#_sel#
 */
union cvmx_dtx_l2c_tadx_selx {
	uint64_t u64;
	struct cvmx_dtx_l2c_tadx_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_l2c_tadx_selx_s       cn70xx;
	struct cvmx_dtx_l2c_tadx_selx_s       cn78xx;
};
typedef union cvmx_dtx_l2c_tadx_selx cvmx_dtx_l2c_tadx_selx_t;

/**
 * cvmx_dtx_lap#_bcst_rsp
 */
union cvmx_dtx_lapx_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_lapx_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_lapx_bcst_rsp_s       cn78xx;
};
typedef union cvmx_dtx_lapx_bcst_rsp cvmx_dtx_lapx_bcst_rsp_t;

/**
 * cvmx_dtx_lap#_ctl
 */
union cvmx_dtx_lapx_ctl {
	uint64_t u64;
	struct cvmx_dtx_lapx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_lapx_ctl_s            cn78xx;
};
typedef union cvmx_dtx_lapx_ctl cvmx_dtx_lapx_ctl_t;

/**
 * cvmx_dtx_lap#_dat#
 */
union cvmx_dtx_lapx_datx {
	uint64_t u64;
	struct cvmx_dtx_lapx_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_lapx_datx_s           cn78xx;
};
typedef union cvmx_dtx_lapx_datx cvmx_dtx_lapx_datx_t;

/**
 * cvmx_dtx_lap#_ena#
 */
union cvmx_dtx_lapx_enax {
	uint64_t u64;
	struct cvmx_dtx_lapx_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_lapx_enax_s           cn78xx;
};
typedef union cvmx_dtx_lapx_enax cvmx_dtx_lapx_enax_t;

/**
 * cvmx_dtx_lap#_sel#
 */
union cvmx_dtx_lapx_selx {
	uint64_t u64;
	struct cvmx_dtx_lapx_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_lapx_selx_s           cn78xx;
};
typedef union cvmx_dtx_lapx_selx cvmx_dtx_lapx_selx_t;

/**
 * cvmx_dtx_lbk_bcst_rsp
 */
union cvmx_dtx_lbk_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_lbk_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_lbk_bcst_rsp_s        cn78xx;
};
typedef union cvmx_dtx_lbk_bcst_rsp cvmx_dtx_lbk_bcst_rsp_t;

/**
 * cvmx_dtx_lbk_ctl
 */
union cvmx_dtx_lbk_ctl {
	uint64_t u64;
	struct cvmx_dtx_lbk_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_lbk_ctl_s             cn78xx;
};
typedef union cvmx_dtx_lbk_ctl cvmx_dtx_lbk_ctl_t;

/**
 * cvmx_dtx_lbk_dat#
 */
union cvmx_dtx_lbk_datx {
	uint64_t u64;
	struct cvmx_dtx_lbk_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_lbk_datx_s            cn78xx;
};
typedef union cvmx_dtx_lbk_datx cvmx_dtx_lbk_datx_t;

/**
 * cvmx_dtx_lbk_ena#
 */
union cvmx_dtx_lbk_enax {
	uint64_t u64;
	struct cvmx_dtx_lbk_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_lbk_enax_s            cn78xx;
};
typedef union cvmx_dtx_lbk_enax cvmx_dtx_lbk_enax_t;

/**
 * cvmx_dtx_lbk_sel#
 */
union cvmx_dtx_lbk_selx {
	uint64_t u64;
	struct cvmx_dtx_lbk_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_lbk_selx_s            cn78xx;
};
typedef union cvmx_dtx_lbk_selx cvmx_dtx_lbk_selx_t;

/**
 * cvmx_dtx_lmc#_bcst_rsp
 */
union cvmx_dtx_lmcx_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_lmcx_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_lmcx_bcst_rsp_s       cn70xx;
	struct cvmx_dtx_lmcx_bcst_rsp_s       cn78xx;
};
typedef union cvmx_dtx_lmcx_bcst_rsp cvmx_dtx_lmcx_bcst_rsp_t;

/**
 * cvmx_dtx_lmc#_ctl
 */
union cvmx_dtx_lmcx_ctl {
	uint64_t u64;
	struct cvmx_dtx_lmcx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_lmcx_ctl_s            cn70xx;
	struct cvmx_dtx_lmcx_ctl_s            cn78xx;
};
typedef union cvmx_dtx_lmcx_ctl cvmx_dtx_lmcx_ctl_t;

/**
 * cvmx_dtx_lmc#_dat#
 */
union cvmx_dtx_lmcx_datx {
	uint64_t u64;
	struct cvmx_dtx_lmcx_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_lmcx_datx_s           cn70xx;
	struct cvmx_dtx_lmcx_datx_s           cn78xx;
};
typedef union cvmx_dtx_lmcx_datx cvmx_dtx_lmcx_datx_t;

/**
 * cvmx_dtx_lmc#_ena#
 */
union cvmx_dtx_lmcx_enax {
	uint64_t u64;
	struct cvmx_dtx_lmcx_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_lmcx_enax_s           cn70xx;
	struct cvmx_dtx_lmcx_enax_s           cn78xx;
};
typedef union cvmx_dtx_lmcx_enax cvmx_dtx_lmcx_enax_t;

/**
 * cvmx_dtx_lmc#_sel#
 */
union cvmx_dtx_lmcx_selx {
	uint64_t u64;
	struct cvmx_dtx_lmcx_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_lmcx_selx_s           cn70xx;
	struct cvmx_dtx_lmcx_selx_s           cn78xx;
};
typedef union cvmx_dtx_lmcx_selx cvmx_dtx_lmcx_selx_t;

/**
 * cvmx_dtx_mio_bcst_rsp
 */
union cvmx_dtx_mio_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_mio_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_mio_bcst_rsp_s        cn70xx;
};
typedef union cvmx_dtx_mio_bcst_rsp cvmx_dtx_mio_bcst_rsp_t;

/**
 * cvmx_dtx_mio_ctl
 */
union cvmx_dtx_mio_ctl {
	uint64_t u64;
	struct cvmx_dtx_mio_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in DTX_MIO_ENA(0..1) instead of normal block debug data.
                                                         Not applicable when SW directly reads the DAT(0..1) registers.  For diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_mio_ctl_s             cn70xx;
};
typedef union cvmx_dtx_mio_ctl cvmx_dtx_mio_ctl_t;

/**
 * cvmx_dtx_mio_dat#
 */
union cvmx_dtx_mio_datx {
	uint64_t u64;
	struct cvmx_dtx_mio_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_mio_datx_s            cn70xx;
};
typedef union cvmx_dtx_mio_datx cvmx_dtx_mio_datx_t;

/**
 * cvmx_dtx_mio_ena#
 */
union cvmx_dtx_mio_enax {
	uint64_t u64;
	struct cvmx_dtx_mio_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_mio_enax_s            cn70xx;
};
typedef union cvmx_dtx_mio_enax cvmx_dtx_mio_enax_t;

/**
 * cvmx_dtx_mio_sel#
 */
union cvmx_dtx_mio_selx {
	uint64_t u64;
	struct cvmx_dtx_mio_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_mio_selx_s            cn70xx;
};
typedef union cvmx_dtx_mio_selx cvmx_dtx_mio_selx_t;

/**
 * cvmx_dtx_ocx_lnk_dtxdid#_bcst_rsp
 */
union cvmx_dtx_ocx_lnk_dtxdidx_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_ocx_lnk_dtxdidx_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_ocx_lnk_dtxdidx_bcst_rsp_s cn78xx;
};
typedef union cvmx_dtx_ocx_lnk_dtxdidx_bcst_rsp cvmx_dtx_ocx_lnk_dtxdidx_bcst_rsp_t;

/**
 * cvmx_dtx_ocx_lnk_dtxdid#_ctl
 */
union cvmx_dtx_ocx_lnk_dtxdidx_ctl {
	uint64_t u64;
	struct cvmx_dtx_ocx_lnk_dtxdidx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_ocx_lnk_dtxdidx_ctl_s cn78xx;
};
typedef union cvmx_dtx_ocx_lnk_dtxdidx_ctl cvmx_dtx_ocx_lnk_dtxdidx_ctl_t;

/**
 * cvmx_dtx_ocx_lnk_dtxdid#_dat#
 */
union cvmx_dtx_ocx_lnk_dtxdidx_datx {
	uint64_t u64;
	struct cvmx_dtx_ocx_lnk_dtxdidx_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_ocx_lnk_dtxdidx_datx_s cn78xx;
};
typedef union cvmx_dtx_ocx_lnk_dtxdidx_datx cvmx_dtx_ocx_lnk_dtxdidx_datx_t;

/**
 * cvmx_dtx_ocx_lnk_dtxdid#_ena#
 */
union cvmx_dtx_ocx_lnk_dtxdidx_enax {
	uint64_t u64;
	struct cvmx_dtx_ocx_lnk_dtxdidx_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_ocx_lnk_dtxdidx_enax_s cn78xx;
};
typedef union cvmx_dtx_ocx_lnk_dtxdidx_enax cvmx_dtx_ocx_lnk_dtxdidx_enax_t;

/**
 * cvmx_dtx_ocx_lnk_dtxdid#_sel#
 */
union cvmx_dtx_ocx_lnk_dtxdidx_selx {
	uint64_t u64;
	struct cvmx_dtx_ocx_lnk_dtxdidx_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_ocx_lnk_dtxdidx_selx_s cn78xx;
};
typedef union cvmx_dtx_ocx_lnk_dtxdidx_selx cvmx_dtx_ocx_lnk_dtxdidx_selx_t;

/**
 * cvmx_dtx_ocx_ole#_bcst_rsp
 */
union cvmx_dtx_ocx_olex_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_ocx_olex_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_ocx_olex_bcst_rsp_s   cn78xx;
};
typedef union cvmx_dtx_ocx_olex_bcst_rsp cvmx_dtx_ocx_olex_bcst_rsp_t;

/**
 * cvmx_dtx_ocx_ole#_ctl
 */
union cvmx_dtx_ocx_olex_ctl {
	uint64_t u64;
	struct cvmx_dtx_ocx_olex_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_ocx_olex_ctl_s        cn78xx;
};
typedef union cvmx_dtx_ocx_olex_ctl cvmx_dtx_ocx_olex_ctl_t;

/**
 * cvmx_dtx_ocx_ole#_dat#
 */
union cvmx_dtx_ocx_olex_datx {
	uint64_t u64;
	struct cvmx_dtx_ocx_olex_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_ocx_olex_datx_s       cn78xx;
};
typedef union cvmx_dtx_ocx_olex_datx cvmx_dtx_ocx_olex_datx_t;

/**
 * cvmx_dtx_ocx_ole#_ena#
 */
union cvmx_dtx_ocx_olex_enax {
	uint64_t u64;
	struct cvmx_dtx_ocx_olex_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_ocx_olex_enax_s       cn78xx;
};
typedef union cvmx_dtx_ocx_olex_enax cvmx_dtx_ocx_olex_enax_t;

/**
 * cvmx_dtx_ocx_ole#_sel#
 */
union cvmx_dtx_ocx_olex_selx {
	uint64_t u64;
	struct cvmx_dtx_ocx_olex_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_ocx_olex_selx_s       cn78xx;
};
typedef union cvmx_dtx_ocx_olex_selx cvmx_dtx_ocx_olex_selx_t;

/**
 * cvmx_dtx_ocx_top_dtxdid_bcst_rsp
 */
union cvmx_dtx_ocx_top_dtxdid_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_ocx_top_dtxdid_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_ocx_top_dtxdid_bcst_rsp_s cn78xx;
};
typedef union cvmx_dtx_ocx_top_dtxdid_bcst_rsp cvmx_dtx_ocx_top_dtxdid_bcst_rsp_t;

/**
 * cvmx_dtx_ocx_top_dtxdid_ctl
 */
union cvmx_dtx_ocx_top_dtxdid_ctl {
	uint64_t u64;
	struct cvmx_dtx_ocx_top_dtxdid_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_ocx_top_dtxdid_ctl_s  cn78xx;
};
typedef union cvmx_dtx_ocx_top_dtxdid_ctl cvmx_dtx_ocx_top_dtxdid_ctl_t;

/**
 * cvmx_dtx_ocx_top_dtxdid_dat#
 */
union cvmx_dtx_ocx_top_dtxdid_datx {
	uint64_t u64;
	struct cvmx_dtx_ocx_top_dtxdid_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_ocx_top_dtxdid_datx_s cn78xx;
};
typedef union cvmx_dtx_ocx_top_dtxdid_datx cvmx_dtx_ocx_top_dtxdid_datx_t;

/**
 * cvmx_dtx_ocx_top_dtxdid_ena#
 */
union cvmx_dtx_ocx_top_dtxdid_enax {
	uint64_t u64;
	struct cvmx_dtx_ocx_top_dtxdid_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_ocx_top_dtxdid_enax_s cn78xx;
};
typedef union cvmx_dtx_ocx_top_dtxdid_enax cvmx_dtx_ocx_top_dtxdid_enax_t;

/**
 * cvmx_dtx_ocx_top_dtxdid_sel#
 */
union cvmx_dtx_ocx_top_dtxdid_selx {
	uint64_t u64;
	struct cvmx_dtx_ocx_top_dtxdid_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_ocx_top_dtxdid_selx_s cn78xx;
};
typedef union cvmx_dtx_ocx_top_dtxdid_selx cvmx_dtx_ocx_top_dtxdid_selx_t;

/**
 * cvmx_dtx_osm_bcst_rsp
 */
union cvmx_dtx_osm_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_osm_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_osm_bcst_rsp_s        cn78xx;
};
typedef union cvmx_dtx_osm_bcst_rsp cvmx_dtx_osm_bcst_rsp_t;

/**
 * cvmx_dtx_osm_ctl
 */
union cvmx_dtx_osm_ctl {
	uint64_t u64;
	struct cvmx_dtx_osm_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_osm_ctl_s             cn78xx;
};
typedef union cvmx_dtx_osm_ctl cvmx_dtx_osm_ctl_t;

/**
 * cvmx_dtx_osm_dat#
 */
union cvmx_dtx_osm_datx {
	uint64_t u64;
	struct cvmx_dtx_osm_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_osm_datx_s            cn78xx;
};
typedef union cvmx_dtx_osm_datx cvmx_dtx_osm_datx_t;

/**
 * cvmx_dtx_osm_ena#
 */
union cvmx_dtx_osm_enax {
	uint64_t u64;
	struct cvmx_dtx_osm_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_osm_enax_s            cn78xx;
};
typedef union cvmx_dtx_osm_enax cvmx_dtx_osm_enax_t;

/**
 * cvmx_dtx_osm_sel#
 */
union cvmx_dtx_osm_selx {
	uint64_t u64;
	struct cvmx_dtx_osm_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_osm_selx_s            cn78xx;
};
typedef union cvmx_dtx_osm_selx cvmx_dtx_osm_selx_t;

/**
 * cvmx_dtx_pcs#_bcst_rsp
 */
union cvmx_dtx_pcsx_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_pcsx_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_pcsx_bcst_rsp_s       cn70xx;
};
typedef union cvmx_dtx_pcsx_bcst_rsp cvmx_dtx_pcsx_bcst_rsp_t;

/**
 * cvmx_dtx_pcs#_ctl
 */
union cvmx_dtx_pcsx_ctl {
	uint64_t u64;
	struct cvmx_dtx_pcsx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in DTX_MIO_ENA(0..1) instead of normal block debug data.
                                                         Not applicable when SW directly reads the DAT(0..1) registers.  For diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_pcsx_ctl_s            cn70xx;
};
typedef union cvmx_dtx_pcsx_ctl cvmx_dtx_pcsx_ctl_t;

/**
 * cvmx_dtx_pcs#_dat#
 */
union cvmx_dtx_pcsx_datx {
	uint64_t u64;
	struct cvmx_dtx_pcsx_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pcsx_datx_s           cn70xx;
};
typedef union cvmx_dtx_pcsx_datx cvmx_dtx_pcsx_datx_t;

/**
 * cvmx_dtx_pcs#_ena#
 */
union cvmx_dtx_pcsx_enax {
	uint64_t u64;
	struct cvmx_dtx_pcsx_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pcsx_enax_s           cn70xx;
};
typedef union cvmx_dtx_pcsx_enax cvmx_dtx_pcsx_enax_t;

/**
 * cvmx_dtx_pcs#_sel#
 */
union cvmx_dtx_pcsx_selx {
	uint64_t u64;
	struct cvmx_dtx_pcsx_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_pcsx_selx_s           cn70xx;
};
typedef union cvmx_dtx_pcsx_selx cvmx_dtx_pcsx_selx_t;

/**
 * cvmx_dtx_pem#_bcst_rsp
 */
union cvmx_dtx_pemx_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_pemx_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_pemx_bcst_rsp_s       cn70xx;
	struct cvmx_dtx_pemx_bcst_rsp_s       cn78xx;
};
typedef union cvmx_dtx_pemx_bcst_rsp cvmx_dtx_pemx_bcst_rsp_t;

/**
 * cvmx_dtx_pem#_ctl
 */
union cvmx_dtx_pemx_ctl {
	uint64_t u64;
	struct cvmx_dtx_pemx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_pemx_ctl_s            cn70xx;
	struct cvmx_dtx_pemx_ctl_s            cn78xx;
};
typedef union cvmx_dtx_pemx_ctl cvmx_dtx_pemx_ctl_t;

/**
 * cvmx_dtx_pem#_dat#
 */
union cvmx_dtx_pemx_datx {
	uint64_t u64;
	struct cvmx_dtx_pemx_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pemx_datx_s           cn70xx;
	struct cvmx_dtx_pemx_datx_s           cn78xx;
};
typedef union cvmx_dtx_pemx_datx cvmx_dtx_pemx_datx_t;

/**
 * cvmx_dtx_pem#_ena#
 */
union cvmx_dtx_pemx_enax {
	uint64_t u64;
	struct cvmx_dtx_pemx_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pemx_enax_s           cn70xx;
	struct cvmx_dtx_pemx_enax_s           cn78xx;
};
typedef union cvmx_dtx_pemx_enax cvmx_dtx_pemx_enax_t;

/**
 * cvmx_dtx_pem#_sel#
 */
union cvmx_dtx_pemx_selx {
	uint64_t u64;
	struct cvmx_dtx_pemx_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_pemx_selx_s           cn70xx;
	struct cvmx_dtx_pemx_selx_s           cn78xx;
};
typedef union cvmx_dtx_pemx_selx cvmx_dtx_pemx_selx_t;

/**
 * cvmx_dtx_pip_bcst_rsp
 */
union cvmx_dtx_pip_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_pip_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_pip_bcst_rsp_s        cn70xx;
};
typedef union cvmx_dtx_pip_bcst_rsp cvmx_dtx_pip_bcst_rsp_t;

/**
 * cvmx_dtx_pip_ctl
 */
union cvmx_dtx_pip_ctl {
	uint64_t u64;
	struct cvmx_dtx_pip_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in DTX_MIO_ENA(0..1) instead of normal block debug data.
                                                         Not applicable when SW directly reads the DAT(0..1) registers.  For diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_pip_ctl_s             cn70xx;
};
typedef union cvmx_dtx_pip_ctl cvmx_dtx_pip_ctl_t;

/**
 * cvmx_dtx_pip_dat#
 */
union cvmx_dtx_pip_datx {
	uint64_t u64;
	struct cvmx_dtx_pip_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pip_datx_s            cn70xx;
};
typedef union cvmx_dtx_pip_datx cvmx_dtx_pip_datx_t;

/**
 * cvmx_dtx_pip_ena#
 */
union cvmx_dtx_pip_enax {
	uint64_t u64;
	struct cvmx_dtx_pip_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pip_enax_s            cn70xx;
};
typedef union cvmx_dtx_pip_enax cvmx_dtx_pip_enax_t;

/**
 * cvmx_dtx_pip_sel#
 */
union cvmx_dtx_pip_selx {
	uint64_t u64;
	struct cvmx_dtx_pip_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_pip_selx_s            cn70xx;
};
typedef union cvmx_dtx_pip_selx cvmx_dtx_pip_selx_t;

/**
 * cvmx_dtx_pki_pbe_bcst_rsp
 */
union cvmx_dtx_pki_pbe_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_pki_pbe_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_pki_pbe_bcst_rsp_s    cn78xx;
};
typedef union cvmx_dtx_pki_pbe_bcst_rsp cvmx_dtx_pki_pbe_bcst_rsp_t;

/**
 * cvmx_dtx_pki_pbe_ctl
 */
union cvmx_dtx_pki_pbe_ctl {
	uint64_t u64;
	struct cvmx_dtx_pki_pbe_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_pki_pbe_ctl_s         cn78xx;
};
typedef union cvmx_dtx_pki_pbe_ctl cvmx_dtx_pki_pbe_ctl_t;

/**
 * cvmx_dtx_pki_pbe_dat#
 */
union cvmx_dtx_pki_pbe_datx {
	uint64_t u64;
	struct cvmx_dtx_pki_pbe_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pki_pbe_datx_s        cn78xx;
};
typedef union cvmx_dtx_pki_pbe_datx cvmx_dtx_pki_pbe_datx_t;

/**
 * cvmx_dtx_pki_pbe_ena#
 */
union cvmx_dtx_pki_pbe_enax {
	uint64_t u64;
	struct cvmx_dtx_pki_pbe_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pki_pbe_enax_s        cn78xx;
};
typedef union cvmx_dtx_pki_pbe_enax cvmx_dtx_pki_pbe_enax_t;

/**
 * cvmx_dtx_pki_pbe_sel#
 */
union cvmx_dtx_pki_pbe_selx {
	uint64_t u64;
	struct cvmx_dtx_pki_pbe_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_pki_pbe_selx_s        cn78xx;
};
typedef union cvmx_dtx_pki_pbe_selx cvmx_dtx_pki_pbe_selx_t;

/**
 * cvmx_dtx_pki_pfe_bcst_rsp
 */
union cvmx_dtx_pki_pfe_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_pki_pfe_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_pki_pfe_bcst_rsp_s    cn78xx;
};
typedef union cvmx_dtx_pki_pfe_bcst_rsp cvmx_dtx_pki_pfe_bcst_rsp_t;

/**
 * cvmx_dtx_pki_pfe_ctl
 */
union cvmx_dtx_pki_pfe_ctl {
	uint64_t u64;
	struct cvmx_dtx_pki_pfe_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_pki_pfe_ctl_s         cn78xx;
};
typedef union cvmx_dtx_pki_pfe_ctl cvmx_dtx_pki_pfe_ctl_t;

/**
 * cvmx_dtx_pki_pfe_dat#
 */
union cvmx_dtx_pki_pfe_datx {
	uint64_t u64;
	struct cvmx_dtx_pki_pfe_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pki_pfe_datx_s        cn78xx;
};
typedef union cvmx_dtx_pki_pfe_datx cvmx_dtx_pki_pfe_datx_t;

/**
 * cvmx_dtx_pki_pfe_ena#
 */
union cvmx_dtx_pki_pfe_enax {
	uint64_t u64;
	struct cvmx_dtx_pki_pfe_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pki_pfe_enax_s        cn78xx;
};
typedef union cvmx_dtx_pki_pfe_enax cvmx_dtx_pki_pfe_enax_t;

/**
 * cvmx_dtx_pki_pfe_sel#
 */
union cvmx_dtx_pki_pfe_selx {
	uint64_t u64;
	struct cvmx_dtx_pki_pfe_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_pki_pfe_selx_s        cn78xx;
};
typedef union cvmx_dtx_pki_pfe_selx cvmx_dtx_pki_pfe_selx_t;

/**
 * cvmx_dtx_pki_pix_bcst_rsp
 */
union cvmx_dtx_pki_pix_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_pki_pix_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_pki_pix_bcst_rsp_s    cn78xx;
};
typedef union cvmx_dtx_pki_pix_bcst_rsp cvmx_dtx_pki_pix_bcst_rsp_t;

/**
 * cvmx_dtx_pki_pix_ctl
 */
union cvmx_dtx_pki_pix_ctl {
	uint64_t u64;
	struct cvmx_dtx_pki_pix_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_pki_pix_ctl_s         cn78xx;
};
typedef union cvmx_dtx_pki_pix_ctl cvmx_dtx_pki_pix_ctl_t;

/**
 * cvmx_dtx_pki_pix_dat#
 */
union cvmx_dtx_pki_pix_datx {
	uint64_t u64;
	struct cvmx_dtx_pki_pix_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pki_pix_datx_s        cn78xx;
};
typedef union cvmx_dtx_pki_pix_datx cvmx_dtx_pki_pix_datx_t;

/**
 * cvmx_dtx_pki_pix_ena#
 */
union cvmx_dtx_pki_pix_enax {
	uint64_t u64;
	struct cvmx_dtx_pki_pix_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pki_pix_enax_s        cn78xx;
};
typedef union cvmx_dtx_pki_pix_enax cvmx_dtx_pki_pix_enax_t;

/**
 * cvmx_dtx_pki_pix_sel#
 */
union cvmx_dtx_pki_pix_selx {
	uint64_t u64;
	struct cvmx_dtx_pki_pix_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_pki_pix_selx_s        cn78xx;
};
typedef union cvmx_dtx_pki_pix_selx cvmx_dtx_pki_pix_selx_t;

/**
 * cvmx_dtx_pko_bcst_rsp
 */
union cvmx_dtx_pko_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_pko_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_pko_bcst_rsp_s        cn70xx;
	struct cvmx_dtx_pko_bcst_rsp_s        cn78xx;
};
typedef union cvmx_dtx_pko_bcst_rsp cvmx_dtx_pko_bcst_rsp_t;

/**
 * cvmx_dtx_pko_ctl
 */
union cvmx_dtx_pko_ctl {
	uint64_t u64;
	struct cvmx_dtx_pko_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_pko_ctl_s             cn70xx;
	struct cvmx_dtx_pko_ctl_s             cn78xx;
};
typedef union cvmx_dtx_pko_ctl cvmx_dtx_pko_ctl_t;

/**
 * cvmx_dtx_pko_dat#
 */
union cvmx_dtx_pko_datx {
	uint64_t u64;
	struct cvmx_dtx_pko_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pko_datx_s            cn70xx;
	struct cvmx_dtx_pko_datx_s            cn78xx;
};
typedef union cvmx_dtx_pko_datx cvmx_dtx_pko_datx_t;

/**
 * cvmx_dtx_pko_ena#
 */
union cvmx_dtx_pko_enax {
	uint64_t u64;
	struct cvmx_dtx_pko_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pko_enax_s            cn70xx;
	struct cvmx_dtx_pko_enax_s            cn78xx;
};
typedef union cvmx_dtx_pko_enax cvmx_dtx_pko_enax_t;

/**
 * cvmx_dtx_pko_sel#
 */
union cvmx_dtx_pko_selx {
	uint64_t u64;
	struct cvmx_dtx_pko_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_pko_selx_s            cn70xx;
	struct cvmx_dtx_pko_selx_s            cn78xx;
};
typedef union cvmx_dtx_pko_selx cvmx_dtx_pko_selx_t;

/**
 * cvmx_dtx_pow_bcst_rsp
 */
union cvmx_dtx_pow_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_pow_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_pow_bcst_rsp_s        cn70xx;
};
typedef union cvmx_dtx_pow_bcst_rsp cvmx_dtx_pow_bcst_rsp_t;

/**
 * cvmx_dtx_pow_ctl
 */
union cvmx_dtx_pow_ctl {
	uint64_t u64;
	struct cvmx_dtx_pow_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in DTX_MIO_ENA(0..1) instead of normal block debug data.
                                                         Not applicable when SW directly reads the DAT(0..1) registers.  For diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_pow_ctl_s             cn70xx;
};
typedef union cvmx_dtx_pow_ctl cvmx_dtx_pow_ctl_t;

/**
 * cvmx_dtx_pow_dat#
 */
union cvmx_dtx_pow_datx {
	uint64_t u64;
	struct cvmx_dtx_pow_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pow_datx_s            cn70xx;
};
typedef union cvmx_dtx_pow_datx cvmx_dtx_pow_datx_t;

/**
 * cvmx_dtx_pow_ena#
 */
union cvmx_dtx_pow_enax {
	uint64_t u64;
	struct cvmx_dtx_pow_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_pow_enax_s            cn70xx;
};
typedef union cvmx_dtx_pow_enax cvmx_dtx_pow_enax_t;

/**
 * cvmx_dtx_pow_sel#
 */
union cvmx_dtx_pow_selx {
	uint64_t u64;
	struct cvmx_dtx_pow_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_pow_selx_s            cn70xx;
};
typedef union cvmx_dtx_pow_selx cvmx_dtx_pow_selx_t;

/**
 * cvmx_dtx_rad_bcst_rsp
 */
union cvmx_dtx_rad_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_rad_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_rad_bcst_rsp_s        cn78xx;
};
typedef union cvmx_dtx_rad_bcst_rsp cvmx_dtx_rad_bcst_rsp_t;

/**
 * cvmx_dtx_rad_ctl
 */
union cvmx_dtx_rad_ctl {
	uint64_t u64;
	struct cvmx_dtx_rad_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_rad_ctl_s             cn78xx;
};
typedef union cvmx_dtx_rad_ctl cvmx_dtx_rad_ctl_t;

/**
 * cvmx_dtx_rad_dat#
 */
union cvmx_dtx_rad_datx {
	uint64_t u64;
	struct cvmx_dtx_rad_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_rad_datx_s            cn78xx;
};
typedef union cvmx_dtx_rad_datx cvmx_dtx_rad_datx_t;

/**
 * cvmx_dtx_rad_ena#
 */
union cvmx_dtx_rad_enax {
	uint64_t u64;
	struct cvmx_dtx_rad_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_rad_enax_s            cn78xx;
};
typedef union cvmx_dtx_rad_enax cvmx_dtx_rad_enax_t;

/**
 * cvmx_dtx_rad_sel#
 */
union cvmx_dtx_rad_selx {
	uint64_t u64;
	struct cvmx_dtx_rad_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_rad_selx_s            cn78xx;
};
typedef union cvmx_dtx_rad_selx cvmx_dtx_rad_selx_t;

/**
 * cvmx_dtx_rst_bcst_rsp
 */
union cvmx_dtx_rst_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_rst_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_rst_bcst_rsp_s        cn70xx;
};
typedef union cvmx_dtx_rst_bcst_rsp cvmx_dtx_rst_bcst_rsp_t;

/**
 * cvmx_dtx_rst_ctl
 */
union cvmx_dtx_rst_ctl {
	uint64_t u64;
	struct cvmx_dtx_rst_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in DTX_MIO_ENA(0..1) instead of normal block debug data.
                                                         Not applicable when SW directly reads the DAT(0..1) registers.  For diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_rst_ctl_s             cn70xx;
};
typedef union cvmx_dtx_rst_ctl cvmx_dtx_rst_ctl_t;

/**
 * cvmx_dtx_rst_dat#
 */
union cvmx_dtx_rst_datx {
	uint64_t u64;
	struct cvmx_dtx_rst_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_rst_datx_s            cn70xx;
};
typedef union cvmx_dtx_rst_datx cvmx_dtx_rst_datx_t;

/**
 * cvmx_dtx_rst_ena#
 */
union cvmx_dtx_rst_enax {
	uint64_t u64;
	struct cvmx_dtx_rst_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_rst_enax_s            cn70xx;
};
typedef union cvmx_dtx_rst_enax cvmx_dtx_rst_enax_t;

/**
 * cvmx_dtx_rst_sel#
 */
union cvmx_dtx_rst_selx {
	uint64_t u64;
	struct cvmx_dtx_rst_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_rst_selx_s            cn70xx;
};
typedef union cvmx_dtx_rst_selx cvmx_dtx_rst_selx_t;

/**
 * cvmx_dtx_sata_bcst_rsp
 */
union cvmx_dtx_sata_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_sata_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_sata_bcst_rsp_s       cn70xx;
};
typedef union cvmx_dtx_sata_bcst_rsp cvmx_dtx_sata_bcst_rsp_t;

/**
 * cvmx_dtx_sata_ctl
 */
union cvmx_dtx_sata_ctl {
	uint64_t u64;
	struct cvmx_dtx_sata_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in DTX_MIO_ENA(0..1) instead of normal block debug data.
                                                         Not applicable when SW directly reads the DAT(0..1) registers.  For diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_sata_ctl_s            cn70xx;
};
typedef union cvmx_dtx_sata_ctl cvmx_dtx_sata_ctl_t;

/**
 * cvmx_dtx_sata_dat#
 */
union cvmx_dtx_sata_datx {
	uint64_t u64;
	struct cvmx_dtx_sata_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_sata_datx_s           cn70xx;
};
typedef union cvmx_dtx_sata_datx cvmx_dtx_sata_datx_t;

/**
 * cvmx_dtx_sata_ena#
 */
union cvmx_dtx_sata_enax {
	uint64_t u64;
	struct cvmx_dtx_sata_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_sata_enax_s           cn70xx;
};
typedef union cvmx_dtx_sata_enax cvmx_dtx_sata_enax_t;

/**
 * cvmx_dtx_sata_sel#
 */
union cvmx_dtx_sata_selx {
	uint64_t u64;
	struct cvmx_dtx_sata_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_sata_selx_s           cn70xx;
};
typedef union cvmx_dtx_sata_selx cvmx_dtx_sata_selx_t;

/**
 * cvmx_dtx_sli_bcst_rsp
 */
union cvmx_dtx_sli_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_sli_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_sli_bcst_rsp_s        cn70xx;
	struct cvmx_dtx_sli_bcst_rsp_s        cn78xx;
};
typedef union cvmx_dtx_sli_bcst_rsp cvmx_dtx_sli_bcst_rsp_t;

/**
 * cvmx_dtx_sli_ctl
 */
union cvmx_dtx_sli_ctl {
	uint64_t u64;
	struct cvmx_dtx_sli_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_sli_ctl_s             cn70xx;
	struct cvmx_dtx_sli_ctl_s             cn78xx;
};
typedef union cvmx_dtx_sli_ctl cvmx_dtx_sli_ctl_t;

/**
 * cvmx_dtx_sli_dat#
 */
union cvmx_dtx_sli_datx {
	uint64_t u64;
	struct cvmx_dtx_sli_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_sli_datx_s            cn70xx;
	struct cvmx_dtx_sli_datx_s            cn78xx;
};
typedef union cvmx_dtx_sli_datx cvmx_dtx_sli_datx_t;

/**
 * cvmx_dtx_sli_ena#
 */
union cvmx_dtx_sli_enax {
	uint64_t u64;
	struct cvmx_dtx_sli_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_sli_enax_s            cn70xx;
	struct cvmx_dtx_sli_enax_s            cn78xx;
};
typedef union cvmx_dtx_sli_enax cvmx_dtx_sli_enax_t;

/**
 * cvmx_dtx_sli_sel#
 */
union cvmx_dtx_sli_selx {
	uint64_t u64;
	struct cvmx_dtx_sli_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_sli_selx_s            cn70xx;
	struct cvmx_dtx_sli_selx_s            cn78xx;
};
typedef union cvmx_dtx_sli_selx cvmx_dtx_sli_selx_t;

/**
 * cvmx_dtx_sso_bcst_rsp
 */
union cvmx_dtx_sso_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_sso_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_sso_bcst_rsp_s        cn78xx;
};
typedef union cvmx_dtx_sso_bcst_rsp cvmx_dtx_sso_bcst_rsp_t;

/**
 * cvmx_dtx_sso_ctl
 */
union cvmx_dtx_sso_ctl {
	uint64_t u64;
	struct cvmx_dtx_sso_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_sso_ctl_s             cn78xx;
};
typedef union cvmx_dtx_sso_ctl cvmx_dtx_sso_ctl_t;

/**
 * cvmx_dtx_sso_dat#
 */
union cvmx_dtx_sso_datx {
	uint64_t u64;
	struct cvmx_dtx_sso_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_sso_datx_s            cn78xx;
};
typedef union cvmx_dtx_sso_datx cvmx_dtx_sso_datx_t;

/**
 * cvmx_dtx_sso_ena#
 */
union cvmx_dtx_sso_enax {
	uint64_t u64;
	struct cvmx_dtx_sso_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_sso_enax_s            cn78xx;
};
typedef union cvmx_dtx_sso_enax cvmx_dtx_sso_enax_t;

/**
 * cvmx_dtx_sso_sel#
 */
union cvmx_dtx_sso_selx {
	uint64_t u64;
	struct cvmx_dtx_sso_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_sso_selx_s            cn78xx;
};
typedef union cvmx_dtx_sso_selx cvmx_dtx_sso_selx_t;

/**
 * cvmx_dtx_tim_bcst_rsp
 */
union cvmx_dtx_tim_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_tim_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_tim_bcst_rsp_s        cn70xx;
	struct cvmx_dtx_tim_bcst_rsp_s        cn78xx;
};
typedef union cvmx_dtx_tim_bcst_rsp cvmx_dtx_tim_bcst_rsp_t;

/**
 * cvmx_dtx_tim_ctl
 */
union cvmx_dtx_tim_ctl {
	uint64_t u64;
	struct cvmx_dtx_tim_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_tim_ctl_s             cn70xx;
	struct cvmx_dtx_tim_ctl_s             cn78xx;
};
typedef union cvmx_dtx_tim_ctl cvmx_dtx_tim_ctl_t;

/**
 * cvmx_dtx_tim_dat#
 */
union cvmx_dtx_tim_datx {
	uint64_t u64;
	struct cvmx_dtx_tim_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_tim_datx_s            cn70xx;
	struct cvmx_dtx_tim_datx_s            cn78xx;
};
typedef union cvmx_dtx_tim_datx cvmx_dtx_tim_datx_t;

/**
 * cvmx_dtx_tim_ena#
 */
union cvmx_dtx_tim_enax {
	uint64_t u64;
	struct cvmx_dtx_tim_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_tim_enax_s            cn70xx;
	struct cvmx_dtx_tim_enax_s            cn78xx;
};
typedef union cvmx_dtx_tim_enax cvmx_dtx_tim_enax_t;

/**
 * cvmx_dtx_tim_sel#
 */
union cvmx_dtx_tim_selx {
	uint64_t u64;
	struct cvmx_dtx_tim_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_tim_selx_s            cn70xx;
	struct cvmx_dtx_tim_selx_s            cn78xx;
};
typedef union cvmx_dtx_tim_selx cvmx_dtx_tim_selx_t;

/**
 * cvmx_dtx_usbdrd#_bcst_rsp
 */
union cvmx_dtx_usbdrdx_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_usbdrdx_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_usbdrdx_bcst_rsp_s    cn70xx;
};
typedef union cvmx_dtx_usbdrdx_bcst_rsp cvmx_dtx_usbdrdx_bcst_rsp_t;

/**
 * cvmx_dtx_usbdrd#_ctl
 */
union cvmx_dtx_usbdrdx_ctl {
	uint64_t u64;
	struct cvmx_dtx_usbdrdx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in DTX_MIO_ENA(0..1) instead of normal block debug data.
                                                         Not applicable when SW directly reads the DAT(0..1) registers.  For diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_usbdrdx_ctl_s         cn70xx;
};
typedef union cvmx_dtx_usbdrdx_ctl cvmx_dtx_usbdrdx_ctl_t;

/**
 * cvmx_dtx_usbdrd#_dat#
 */
union cvmx_dtx_usbdrdx_datx {
	uint64_t u64;
	struct cvmx_dtx_usbdrdx_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_usbdrdx_datx_s        cn70xx;
};
typedef union cvmx_dtx_usbdrdx_datx cvmx_dtx_usbdrdx_datx_t;

/**
 * cvmx_dtx_usbdrd#_ena#
 */
union cvmx_dtx_usbdrdx_enax {
	uint64_t u64;
	struct cvmx_dtx_usbdrdx_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_usbdrdx_enax_s        cn70xx;
};
typedef union cvmx_dtx_usbdrdx_enax cvmx_dtx_usbdrdx_enax_t;

/**
 * cvmx_dtx_usbdrd#_sel#
 */
union cvmx_dtx_usbdrdx_selx {
	uint64_t u64;
	struct cvmx_dtx_usbdrdx_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_usbdrdx_selx_s        cn70xx;
};
typedef union cvmx_dtx_usbdrdx_selx cvmx_dtx_usbdrdx_selx_t;

/**
 * cvmx_dtx_usbh#_bcst_rsp
 */
union cvmx_dtx_usbhx_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_usbhx_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_usbhx_bcst_rsp_s      cn78xx;
};
typedef union cvmx_dtx_usbhx_bcst_rsp cvmx_dtx_usbhx_bcst_rsp_t;

/**
 * cvmx_dtx_usbh#_ctl
 */
union cvmx_dtx_usbhx_ctl {
	uint64_t u64;
	struct cvmx_dtx_usbhx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_usbhx_ctl_s           cn78xx;
};
typedef union cvmx_dtx_usbhx_ctl cvmx_dtx_usbhx_ctl_t;

/**
 * cvmx_dtx_usbh#_dat#
 */
union cvmx_dtx_usbhx_datx {
	uint64_t u64;
	struct cvmx_dtx_usbhx_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_usbhx_datx_s          cn78xx;
};
typedef union cvmx_dtx_usbhx_datx cvmx_dtx_usbhx_datx_t;

/**
 * cvmx_dtx_usbh#_ena#
 */
union cvmx_dtx_usbhx_enax {
	uint64_t u64;
	struct cvmx_dtx_usbhx_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_usbhx_enax_s          cn78xx;
};
typedef union cvmx_dtx_usbhx_enax cvmx_dtx_usbhx_enax_t;

/**
 * cvmx_dtx_usbh#_sel#
 */
union cvmx_dtx_usbhx_selx {
	uint64_t u64;
	struct cvmx_dtx_usbhx_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_usbhx_selx_s          cn78xx;
};
typedef union cvmx_dtx_usbhx_selx cvmx_dtx_usbhx_selx_t;

/**
 * cvmx_dtx_zip_bcst_rsp
 */
union cvmx_dtx_zip_bcst_rsp {
	uint64_t u64;
	struct cvmx_dtx_zip_bcst_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ena                          : 1;  /**< Enable this DTX instance as the responder to DTX Broadcast reads/writes. */
#else
	uint64_t ena                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dtx_zip_bcst_rsp_s        cn78xx;
};
typedef union cvmx_dtx_zip_bcst_rsp cvmx_dtx_zip_bcst_rsp_t;

/**
 * cvmx_dtx_zip_ctl
 */
union cvmx_dtx_zip_ctl {
	uint64_t u64;
	struct cvmx_dtx_zip_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t active                       : 1;  /**< Force block's gated clocks on, so that the state of idle signals may be captured. */
	uint64_t reserved_2_3                 : 2;
	uint64_t echoen                       : 1;  /**< Drive debug bus with the value in ENA(0..1) instead of normal block debug data. For
                                                         diagnostic use only. */
	uint64_t swap                         : 1;  /**< Swap the high and low 36-bit debug bus outputs. */
#else
	uint64_t swap                         : 1;
	uint64_t echoen                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t active                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_dtx_zip_ctl_s             cn78xx;
};
typedef union cvmx_dtx_zip_ctl cvmx_dtx_zip_ctl_t;

/**
 * cvmx_dtx_zip_dat#
 */
union cvmx_dtx_zip_datx {
	uint64_t u64;
	struct cvmx_dtx_zip_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t raw                          : 36; /**< Raw debug data captured by the DTX before the ENA is applied. This gives the ability to
                                                         peek into blocks during an OCLA capture without OCLA reconfiguration. */
#else
	uint64_t raw                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_zip_datx_s            cn78xx;
};
typedef union cvmx_dtx_zip_datx cvmx_dtx_zip_datx_t;

/**
 * cvmx_dtx_zip_ena#
 */
union cvmx_dtx_zip_enax {
	uint64_t u64;
	struct cvmx_dtx_zip_enax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t ena                          : 36; /**< Output enable vector of which bits to drive onto the low/high 36-bit debug buses. Normally
                                                         only one block will drive each bit. */
#else
	uint64_t ena                          : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_dtx_zip_enax_s            cn78xx;
};
typedef union cvmx_dtx_zip_enax cvmx_dtx_zip_enax_t;

/**
 * cvmx_dtx_zip_sel#
 */
union cvmx_dtx_zip_selx {
	uint64_t u64;
	struct cvmx_dtx_zip_selx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t value                        : 24; /**< Debug select.  Selects which signals to drive onto low/high 36-bit debug buses. */
#else
	uint64_t value                        : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dtx_zip_selx_s            cn78xx;
};
typedef union cvmx_dtx_zip_selx cvmx_dtx_zip_selx_t;

#endif
