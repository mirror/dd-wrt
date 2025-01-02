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
 * cvmx-sata-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon sata.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_SATA_DEFS_H__
#define __CVMX_SATA_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_BISTAFR CVMX_SATA_UAHC_GBL_BISTAFR_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_BISTAFR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_BISTAFR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C00000000A0ull);
}
#else
#define CVMX_SATA_UAHC_GBL_BISTAFR (CVMX_ADD_IO_SEG(0x00016C00000000A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_BISTCR CVMX_SATA_UAHC_GBL_BISTCR_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_BISTCR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_BISTCR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C00000000A4ull);
}
#else
#define CVMX_SATA_UAHC_GBL_BISTCR (CVMX_ADD_IO_SEG(0x00016C00000000A4ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_BISTDECR CVMX_SATA_UAHC_GBL_BISTDECR_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_BISTDECR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_BISTDECR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C00000000B0ull);
}
#else
#define CVMX_SATA_UAHC_GBL_BISTDECR (CVMX_ADD_IO_SEG(0x00016C00000000B0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_BISTFCTR CVMX_SATA_UAHC_GBL_BISTFCTR_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_BISTFCTR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_BISTFCTR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C00000000A8ull);
}
#else
#define CVMX_SATA_UAHC_GBL_BISTFCTR (CVMX_ADD_IO_SEG(0x00016C00000000A8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_BISTSR CVMX_SATA_UAHC_GBL_BISTSR_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_BISTSR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_BISTSR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C00000000ACull);
}
#else
#define CVMX_SATA_UAHC_GBL_BISTSR (CVMX_ADD_IO_SEG(0x00016C00000000ACull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_CAP CVMX_SATA_UAHC_GBL_CAP_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_CAP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_CAP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C0000000000ull);
}
#else
#define CVMX_SATA_UAHC_GBL_CAP (CVMX_ADD_IO_SEG(0x00016C0000000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_CAP2 CVMX_SATA_UAHC_GBL_CAP2_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_CAP2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_CAP2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C0000000024ull);
}
#else
#define CVMX_SATA_UAHC_GBL_CAP2 (CVMX_ADD_IO_SEG(0x00016C0000000024ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_CCC_CTL CVMX_SATA_UAHC_GBL_CCC_CTL_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_CCC_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_CCC_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C0000000014ull);
}
#else
#define CVMX_SATA_UAHC_GBL_CCC_CTL (CVMX_ADD_IO_SEG(0x00016C0000000014ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_CCC_PORTS CVMX_SATA_UAHC_GBL_CCC_PORTS_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_CCC_PORTS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_CCC_PORTS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C0000000018ull);
}
#else
#define CVMX_SATA_UAHC_GBL_CCC_PORTS (CVMX_ADD_IO_SEG(0x00016C0000000018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_GHC CVMX_SATA_UAHC_GBL_GHC_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_GHC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_GHC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C0000000004ull);
}
#else
#define CVMX_SATA_UAHC_GBL_GHC (CVMX_ADD_IO_SEG(0x00016C0000000004ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_GPARAM1R CVMX_SATA_UAHC_GBL_GPARAM1R_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_GPARAM1R_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_GPARAM1R not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C00000000E8ull);
}
#else
#define CVMX_SATA_UAHC_GBL_GPARAM1R (CVMX_ADD_IO_SEG(0x00016C00000000E8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_GPARAM2R CVMX_SATA_UAHC_GBL_GPARAM2R_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_GPARAM2R_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_GPARAM2R not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C00000000ECull);
}
#else
#define CVMX_SATA_UAHC_GBL_GPARAM2R (CVMX_ADD_IO_SEG(0x00016C00000000ECull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_IDR CVMX_SATA_UAHC_GBL_IDR_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_IDR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_IDR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C00000000FCull);
}
#else
#define CVMX_SATA_UAHC_GBL_IDR (CVMX_ADD_IO_SEG(0x00016C00000000FCull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_IS CVMX_SATA_UAHC_GBL_IS_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_IS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_IS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C0000000008ull);
}
#else
#define CVMX_SATA_UAHC_GBL_IS (CVMX_ADD_IO_SEG(0x00016C0000000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_OOBR CVMX_SATA_UAHC_GBL_OOBR_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_OOBR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_OOBR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C00000000BCull);
}
#else
#define CVMX_SATA_UAHC_GBL_OOBR (CVMX_ADD_IO_SEG(0x00016C00000000BCull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_PI CVMX_SATA_UAHC_GBL_PI_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_PI_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_PI not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C000000000Cull);
}
#else
#define CVMX_SATA_UAHC_GBL_PI (CVMX_ADD_IO_SEG(0x00016C000000000Cull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_PPARAMR CVMX_SATA_UAHC_GBL_PPARAMR_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_PPARAMR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_PPARAMR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C00000000F0ull);
}
#else
#define CVMX_SATA_UAHC_GBL_PPARAMR (CVMX_ADD_IO_SEG(0x00016C00000000F0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_TESTR CVMX_SATA_UAHC_GBL_TESTR_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_TESTR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_TESTR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C00000000F4ull);
}
#else
#define CVMX_SATA_UAHC_GBL_TESTR (CVMX_ADD_IO_SEG(0x00016C00000000F4ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_TIMER1MS CVMX_SATA_UAHC_GBL_TIMER1MS_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_TIMER1MS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_TIMER1MS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C00000000E0ull);
}
#else
#define CVMX_SATA_UAHC_GBL_TIMER1MS (CVMX_ADD_IO_SEG(0x00016C00000000E0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_VERSIONR CVMX_SATA_UAHC_GBL_VERSIONR_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_VERSIONR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_VERSIONR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C00000000F8ull);
}
#else
#define CVMX_SATA_UAHC_GBL_VERSIONR (CVMX_ADD_IO_SEG(0x00016C00000000F8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UAHC_GBL_VS CVMX_SATA_UAHC_GBL_VS_FUNC()
static inline uint64_t CVMX_SATA_UAHC_GBL_VS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UAHC_GBL_VS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016C0000000010ull);
}
#else
#define CVMX_SATA_UAHC_GBL_VS (CVMX_ADD_IO_SEG(0x00016C0000000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_CI(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_CI(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C0000000138ull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_CI(offset) (CVMX_ADD_IO_SEG(0x00016C0000000138ull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_CLB(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_CLB(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C0000000100ull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_CLB(offset) (CVMX_ADD_IO_SEG(0x00016C0000000100ull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_CMD(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_CMD(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C0000000118ull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_CMD(offset) (CVMX_ADD_IO_SEG(0x00016C0000000118ull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_DMACR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_DMACR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C0000000170ull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_DMACR(offset) (CVMX_ADD_IO_SEG(0x00016C0000000170ull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_FB(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_FB(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C0000000108ull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_FB(offset) (CVMX_ADD_IO_SEG(0x00016C0000000108ull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_FBS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_FBS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C0000000140ull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_FBS(offset) (CVMX_ADD_IO_SEG(0x00016C0000000140ull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_IE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_IE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C0000000114ull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_IE(offset) (CVMX_ADD_IO_SEG(0x00016C0000000114ull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_IS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_IS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C0000000110ull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_IS(offset) (CVMX_ADD_IO_SEG(0x00016C0000000110ull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_PHYCR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_PHYCR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C0000000178ull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_PHYCR(offset) (CVMX_ADD_IO_SEG(0x00016C0000000178ull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_PHYSR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_PHYSR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C000000017Cull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_PHYSR(offset) (CVMX_ADD_IO_SEG(0x00016C000000017Cull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_SACT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_SACT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C0000000134ull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_SACT(offset) (CVMX_ADD_IO_SEG(0x00016C0000000134ull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_SCTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_SCTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C000000012Cull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_SCTL(offset) (CVMX_ADD_IO_SEG(0x00016C000000012Cull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_SERR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_SERR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C0000000130ull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_SERR(offset) (CVMX_ADD_IO_SEG(0x00016C0000000130ull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_SIG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_SIG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C0000000124ull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_SIG(offset) (CVMX_ADD_IO_SEG(0x00016C0000000124ull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_SNTF(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_SNTF(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C000000013Cull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_SNTF(offset) (CVMX_ADD_IO_SEG(0x00016C000000013Cull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_SSTS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_SSTS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C0000000128ull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_SSTS(offset) (CVMX_ADD_IO_SEG(0x00016C0000000128ull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SATA_UAHC_PX_TFD(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SATA_UAHC_PX_TFD(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016C0000000120ull) + ((offset) & 1) * 128;
}
#else
#define CVMX_SATA_UAHC_PX_TFD(offset) (CVMX_ADD_IO_SEG(0x00016C0000000120ull) + ((offset) & 1) * 128)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UCTL_BIST_STATUS CVMX_SATA_UCTL_BIST_STATUS_FUNC()
static inline uint64_t CVMX_SATA_UCTL_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UCTL_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x000118006C000008ull);
}
#else
#define CVMX_SATA_UCTL_BIST_STATUS (CVMX_ADD_IO_SEG(0x000118006C000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UCTL_CTL CVMX_SATA_UCTL_CTL_FUNC()
static inline uint64_t CVMX_SATA_UCTL_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UCTL_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x000118006C000000ull);
}
#else
#define CVMX_SATA_UCTL_CTL (CVMX_ADD_IO_SEG(0x000118006C000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UCTL_ECC CVMX_SATA_UCTL_ECC_FUNC()
static inline uint64_t CVMX_SATA_UCTL_ECC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UCTL_ECC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x000118006C0000F0ull);
}
#else
#define CVMX_SATA_UCTL_ECC (CVMX_ADD_IO_SEG(0x000118006C0000F0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UCTL_INTSTAT CVMX_SATA_UCTL_INTSTAT_FUNC()
static inline uint64_t CVMX_SATA_UCTL_INTSTAT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UCTL_INTSTAT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x000118006C000030ull);
}
#else
#define CVMX_SATA_UCTL_INTSTAT (CVMX_ADD_IO_SEG(0x000118006C000030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UCTL_SHIM_CFG CVMX_SATA_UCTL_SHIM_CFG_FUNC()
static inline uint64_t CVMX_SATA_UCTL_SHIM_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UCTL_SHIM_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x000118006C0000E8ull);
}
#else
#define CVMX_SATA_UCTL_SHIM_CFG (CVMX_ADD_IO_SEG(0x000118006C0000E8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UCTL_SPARE0 CVMX_SATA_UCTL_SPARE0_FUNC()
static inline uint64_t CVMX_SATA_UCTL_SPARE0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_SATA_UCTL_SPARE0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x000118006C000010ull);
}
#else
#define CVMX_SATA_UCTL_SPARE0 (CVMX_ADD_IO_SEG(0x000118006C000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UCTL_SPARE0_ECO CVMX_SATA_UCTL_SPARE0_ECO_FUNC()
static inline uint64_t CVMX_SATA_UCTL_SPARE0_ECO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UCTL_SPARE0_ECO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x000118006C000010ull);
}
#else
#define CVMX_SATA_UCTL_SPARE0_ECO (CVMX_ADD_IO_SEG(0x000118006C000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UCTL_SPARE1 CVMX_SATA_UCTL_SPARE1_FUNC()
static inline uint64_t CVMX_SATA_UCTL_SPARE1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_SATA_UCTL_SPARE1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x000118006C0000F8ull);
}
#else
#define CVMX_SATA_UCTL_SPARE1 (CVMX_ADD_IO_SEG(0x000118006C0000F8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SATA_UCTL_SPARE1_ECO CVMX_SATA_UCTL_SPARE1_ECO_FUNC()
static inline uint64_t CVMX_SATA_UCTL_SPARE1_ECO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SATA_UCTL_SPARE1_ECO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x000118006C0000F8ull);
}
#else
#define CVMX_SATA_UCTL_SPARE1_ECO (CVMX_ADD_IO_SEG(0x000118006C0000F8ull))
#endif

/**
 * cvmx_sata_uahc_gbl_bistafr
 *
 * This register is shared between SATA ports. Before accessing this
 * register, first select the required port by writing the port number
 * to the SATA()_UAHC_GBL_TESTR[PSEL] field.
 *
 * This register contains the pattern definition (bits 23:16 of the
 * first DWORD) and the data pattern (bits 7:0 of the second DWORD)
 * fields of the received BIST activate FIS.
 */
union cvmx_sata_uahc_gbl_bistafr {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_bistafr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t ncp                          : 8;  /**< Bits 7:0 of the second DWORD of BIST activate FIS.
                                                         0xF1 = Low transition density pattern (LTDP).
                                                         0xB5 = High transition density pattern (HTDP).
                                                         0xAB = Low frequency spectral component pattern (LFSCP).
                                                         0x7F = Simultaneous switching outputs pattern (SSOP).
                                                         0x78 = Mid frequency test pattern (MFTP).
                                                         0x4A = High frequency test pattern (HFTP).
                                                         0x7E = Low frequency test pattern (LFTP).
                                                         else = Lone bit pattern (LBP). */
	uint32_t pd                           : 8;  /**< Bits 23:16 of the first DWORD of the BIST activate FIS. Only the following values are
                                                         supported:
                                                         0x10 = Far-end retimed.
                                                         0xC0 = Far-end transmit only.
                                                         0xE0 = Far-end transmit only with scrambler bypassed. */
#else
	uint32_t pd                           : 8;
	uint32_t ncp                          : 8;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_bistafr_s   cn70xx;
	struct cvmx_sata_uahc_gbl_bistafr_s   cn70xxp1;
	struct cvmx_sata_uahc_gbl_bistafr_s   cn73xx;
};
typedef union cvmx_sata_uahc_gbl_bistafr cvmx_sata_uahc_gbl_bistafr_t;

/**
 * cvmx_sata_uahc_gbl_bistcr
 *
 * This register is shared between SATA ports. Before accessing this
 * register, first select the required port by writing the port number
 * to the SATA_UAHC_GBL_TESTR[PSEL] field.
 */
union cvmx_sata_uahc_gbl_bistcr {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_bistcr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t old_phy_ready                : 1;  /**< Old phy_ready. */
	uint32_t late_phy_ready               : 1;  /**< Late phy_ready. */
	uint32_t reserved_21_23               : 3;
	uint32_t ferlib                       : 1;  /**< Far-end retimed loopback. */
	uint32_t reserved_19_19               : 1;
	uint32_t txo                          : 1;  /**< Transmit only. */
	uint32_t cntclr                       : 1;  /**< Counter clear. */
	uint32_t nealb                        : 1;  /**< Near-end analog loopback. */
	uint32_t llb                          : 1;  /**< Lab loopback mode. */
	uint32_t reserved_14_14               : 1;
	uint32_t errlossen                    : 1;  /**< Error loss detect enable. */
	uint32_t sdfe                         : 1;  /**< Signal detect feature enable. */
	uint32_t reserved_11_11               : 1;
	uint32_t llc                          : 3;  /**< Link layer control.
                                                         <10> = RPD - repeat primitive drop enable.
                                                         <9> = DESCRAM - descrambler enable.
                                                         <8> = SCRAM - scrambler enable. */
	uint32_t reserved_7_7                 : 1;
	uint32_t erren                        : 1;  /**< Error enable. */
	uint32_t flip                         : 1;  /**< Flip disparity. */
	uint32_t pv                           : 1;  /**< Pattern version. */
	uint32_t pattern                      : 4;  /**< SATA compliant pattern selection. */
#else
	uint32_t pattern                      : 4;
	uint32_t pv                           : 1;
	uint32_t flip                         : 1;
	uint32_t erren                        : 1;
	uint32_t reserved_7_7                 : 1;
	uint32_t llc                          : 3;
	uint32_t reserved_11_11               : 1;
	uint32_t sdfe                         : 1;
	uint32_t errlossen                    : 1;
	uint32_t reserved_14_14               : 1;
	uint32_t llb                          : 1;
	uint32_t nealb                        : 1;
	uint32_t cntclr                       : 1;
	uint32_t txo                          : 1;
	uint32_t reserved_19_19               : 1;
	uint32_t ferlib                       : 1;
	uint32_t reserved_21_23               : 3;
	uint32_t late_phy_ready               : 1;
	uint32_t old_phy_ready                : 1;
	uint32_t reserved_26_31               : 6;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_bistcr_s    cn70xx;
	struct cvmx_sata_uahc_gbl_bistcr_s    cn70xxp1;
	struct cvmx_sata_uahc_gbl_bistcr_s    cn73xx;
};
typedef union cvmx_sata_uahc_gbl_bistcr cvmx_sata_uahc_gbl_bistcr_t;

/**
 * cvmx_sata_uahc_gbl_bistdecr
 *
 * This register is shared between SATA ports. Before accessing this
 * register, first select the required port by writing the port number
 * to the SATA()_UAHC_GBL_TESTR[PSEL] field.
 */
union cvmx_sata_uahc_gbl_bistdecr {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_bistdecr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dwerr                        : 32; /**< DWORD error count. */
#else
	uint32_t dwerr                        : 32;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_bistdecr_s  cn70xx;
	struct cvmx_sata_uahc_gbl_bistdecr_s  cn70xxp1;
	struct cvmx_sata_uahc_gbl_bistdecr_s  cn73xx;
};
typedef union cvmx_sata_uahc_gbl_bistdecr cvmx_sata_uahc_gbl_bistdecr_t;

/**
 * cvmx_sata_uahc_gbl_bistfctr
 *
 * This register is shared between SATA ports. Before accessing this
 * register, first select the required port by writing the port number
 * to the SATA()_UAHC_GBL_TESTR[PSEL] field.
 */
union cvmx_sata_uahc_gbl_bistfctr {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_bistfctr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t count                        : 32; /**< Received BIST FIS count. */
#else
	uint32_t count                        : 32;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_bistfctr_s  cn70xx;
	struct cvmx_sata_uahc_gbl_bistfctr_s  cn70xxp1;
	struct cvmx_sata_uahc_gbl_bistfctr_s  cn73xx;
};
typedef union cvmx_sata_uahc_gbl_bistfctr cvmx_sata_uahc_gbl_bistfctr_t;

/**
 * cvmx_sata_uahc_gbl_bistsr
 */
union cvmx_sata_uahc_gbl_bistsr {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_bistsr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t brsterr                      : 8;  /**< Burst error. */
	uint32_t framerr                      : 16; /**< Frame error. */
#else
	uint32_t framerr                      : 16;
	uint32_t brsterr                      : 8;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_bistsr_s    cn70xx;
	struct cvmx_sata_uahc_gbl_bistsr_s    cn70xxp1;
	struct cvmx_sata_uahc_gbl_bistsr_s    cn73xx;
};
typedef union cvmx_sata_uahc_gbl_bistsr cvmx_sata_uahc_gbl_bistsr_t;

/**
 * cvmx_sata_uahc_gbl_cap
 *
 * See AHCI specification v1.3 section 3.1
 *
 */
union cvmx_sata_uahc_gbl_cap {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_cap_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t s64a                         : 1;  /**< Supports 64-bit addressing. */
	uint32_t sncq                         : 1;  /**< Supports native command queuing. */
	uint32_t ssntf                        : 1;  /**< Supports SNotification register. */
	uint32_t smps                         : 1;  /**< Supports mechanical presence switch. */
	uint32_t sss                          : 1;  /**< Supports staggered spin-up. */
	uint32_t salp                         : 1;  /**< Supports aggressive link power management. */
	uint32_t sal                          : 1;  /**< Supports activity LED. */
	uint32_t sclo                         : 1;  /**< Supports command list override. */
	uint32_t iss                          : 4;  /**< Interface speed support. */
	uint32_t snzo                         : 1;  /**< Supports nonzero DMA offsets. */
	uint32_t sam                          : 1;  /**< Supports AHCI mode only. */
	uint32_t spm                          : 1;  /**< Supports port multiplier. */
	uint32_t fbss                         : 1;  /**< Supports FIS-based switching. */
	uint32_t pmd                          : 1;  /**< PIO multiple DRQ block. */
	uint32_t ssc                          : 1;  /**< Slumber state capable. */
	uint32_t psc                          : 1;  /**< Partial state capable. */
	uint32_t ncs                          : 5;  /**< Number of command slots. */
	uint32_t cccs                         : 1;  /**< Command completion coalescing support. */
	uint32_t ems                          : 1;  /**< Enclosure management support. */
	uint32_t sxs                          : 1;  /**< Supports external SATA. */
	uint32_t np                           : 5;  /**< Number of ports. 0x1 = two ports, all other values reserved. */
#else
	uint32_t np                           : 5;
	uint32_t sxs                          : 1;
	uint32_t ems                          : 1;
	uint32_t cccs                         : 1;
	uint32_t ncs                          : 5;
	uint32_t psc                          : 1;
	uint32_t ssc                          : 1;
	uint32_t pmd                          : 1;
	uint32_t fbss                         : 1;
	uint32_t spm                          : 1;
	uint32_t sam                          : 1;
	uint32_t snzo                         : 1;
	uint32_t iss                          : 4;
	uint32_t sclo                         : 1;
	uint32_t sal                          : 1;
	uint32_t salp                         : 1;
	uint32_t sss                          : 1;
	uint32_t smps                         : 1;
	uint32_t ssntf                        : 1;
	uint32_t sncq                         : 1;
	uint32_t s64a                         : 1;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_cap_s       cn70xx;
	struct cvmx_sata_uahc_gbl_cap_s       cn70xxp1;
	struct cvmx_sata_uahc_gbl_cap_s       cn73xx;
};
typedef union cvmx_sata_uahc_gbl_cap cvmx_sata_uahc_gbl_cap_t;

/**
 * cvmx_sata_uahc_gbl_cap2
 *
 * See AHCI specification v1.3 section 3.1.
 *
 */
union cvmx_sata_uahc_gbl_cap2 {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_cap2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_6_31                : 26;
	uint32_t deso                         : 1;  /**< Device sleep entrance from slumber only. */
	uint32_t sadm                         : 1;  /**< Supports aggressive device sleep management. */
	uint32_t sds                          : 1;  /**< Supports device sleep. */
	uint32_t apst                         : 1;  /**< Automatic partial to slumber transitions. */
	uint32_t nvmp                         : 1;  /**< NVMHCI present. */
	uint32_t boh                          : 1;  /**< Supports BIOS/OS handoff. */
#else
	uint32_t boh                          : 1;
	uint32_t nvmp                         : 1;
	uint32_t apst                         : 1;
	uint32_t sds                          : 1;
	uint32_t sadm                         : 1;
	uint32_t deso                         : 1;
	uint32_t reserved_6_31                : 26;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_cap2_s      cn70xx;
	struct cvmx_sata_uahc_gbl_cap2_s      cn70xxp1;
	struct cvmx_sata_uahc_gbl_cap2_s      cn73xx;
};
typedef union cvmx_sata_uahc_gbl_cap2 cvmx_sata_uahc_gbl_cap2_t;

/**
 * cvmx_sata_uahc_gbl_ccc_ctl
 *
 * See AHCI specification v1.3 section 3.1.
 *
 */
union cvmx_sata_uahc_gbl_ccc_ctl {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_ccc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t tv                           : 16; /**< Time-out value. Writable only when EN = 0. */
	uint32_t cc                           : 8;  /**< Command completions. Writable only when EN = 0. */
	uint32_t intr                         : 5;  /**< Specifies the port interrupt used by the CCC feature. */
	uint32_t reserved_1_2                 : 2;
	uint32_t en                           : 1;  /**< CCC enable. */
#else
	uint32_t en                           : 1;
	uint32_t reserved_1_2                 : 2;
	uint32_t intr                         : 5;
	uint32_t cc                           : 8;
	uint32_t tv                           : 16;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_ccc_ctl_s   cn70xx;
	struct cvmx_sata_uahc_gbl_ccc_ctl_s   cn70xxp1;
	struct cvmx_sata_uahc_gbl_ccc_ctl_s   cn73xx;
};
typedef union cvmx_sata_uahc_gbl_ccc_ctl cvmx_sata_uahc_gbl_ccc_ctl_t;

/**
 * cvmx_sata_uahc_gbl_ccc_ports
 *
 * See AHCI specification v1.3 section 3.1.
 *
 */
union cvmx_sata_uahc_gbl_ccc_ports {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_ccc_ports_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_2_31                : 30;
	uint32_t prt                          : 2;  /**< Per port CCC enable. */
#else
	uint32_t prt                          : 2;
	uint32_t reserved_2_31                : 30;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_ccc_ports_s cn70xx;
	struct cvmx_sata_uahc_gbl_ccc_ports_s cn70xxp1;
	struct cvmx_sata_uahc_gbl_ccc_ports_s cn73xx;
};
typedef union cvmx_sata_uahc_gbl_ccc_ports cvmx_sata_uahc_gbl_ccc_ports_t;

/**
 * cvmx_sata_uahc_gbl_ghc
 *
 * See AHCI specification v1.3 section 3.1.
 *
 */
union cvmx_sata_uahc_gbl_ghc {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_ghc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ae                           : 1;  /**< AHCI enable. */
	uint32_t reserved_2_30                : 29;
	uint32_t ie                           : 1;  /**< Interrupt enable. */
	uint32_t hr                           : 1;  /**< HBA reset. Writing a 1 resets the UAHC. Hardware clears this bit once reset is complete. */
#else
	uint32_t hr                           : 1;
	uint32_t ie                           : 1;
	uint32_t reserved_2_30                : 29;
	uint32_t ae                           : 1;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_ghc_s       cn70xx;
	struct cvmx_sata_uahc_gbl_ghc_s       cn70xxp1;
	struct cvmx_sata_uahc_gbl_ghc_s       cn73xx;
};
typedef union cvmx_sata_uahc_gbl_ghc cvmx_sata_uahc_gbl_ghc_t;

/**
 * cvmx_sata_uahc_gbl_gparam1r
 */
union cvmx_sata_uahc_gbl_gparam1r {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_gparam1r_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t align_m                      : 1;  /**< RX data alignment mode (ALIGN_MODE). */
	uint32_t rx_buffer                    : 1;  /**< RX data buffer mode (RX_BUFFER_MODE). */
	uint32_t phy_data                     : 2;  /**< PHY data width (PHY_DATA_WIDTH). */
	uint32_t phy_rst                      : 1;  /**< PHY reset mode (PHY_RST_MODE). */
	uint32_t phy_ctrl                     : 6;  /**< PHY control width (PHY_CTRL_W). */
	uint32_t phy_stat                     : 6;  /**< PHY status width (PHY_STAT_W). */
	uint32_t latch_m                      : 1;  /**< Latch mode (LATCH_MODE). */
	uint32_t phy_type                     : 3;  /**< PHY interface type (PHY_INTERFACE_TYPE). */
	uint32_t return_err                   : 1;  /**< AMBA error response (RETURN_ERR_RESP). */
	uint32_t ahb_endian                   : 2;  /**< AHB bus endianness (AHB_ENDIANNESS). */
	uint32_t s_haddr                      : 1;  /**< AMBA slave address bus width (S_HADDR_WIDTH). */
	uint32_t m_haddr                      : 1;  /**< AMBA master address bus width (M_HADDR_WIDTH). */
	uint32_t s_hdata                      : 3;  /**< AMBA slave data width (S_HDATA_WIDTH). */
	uint32_t m_hdata                      : 3;  /**< AMBA master data width (M_HDATA_WIDTH). */
#else
	uint32_t m_hdata                      : 3;
	uint32_t s_hdata                      : 3;
	uint32_t m_haddr                      : 1;
	uint32_t s_haddr                      : 1;
	uint32_t ahb_endian                   : 2;
	uint32_t return_err                   : 1;
	uint32_t phy_type                     : 3;
	uint32_t latch_m                      : 1;
	uint32_t phy_stat                     : 6;
	uint32_t phy_ctrl                     : 6;
	uint32_t phy_rst                      : 1;
	uint32_t phy_data                     : 2;
	uint32_t rx_buffer                    : 1;
	uint32_t align_m                      : 1;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_gparam1r_s  cn70xx;
	struct cvmx_sata_uahc_gbl_gparam1r_s  cn70xxp1;
	struct cvmx_sata_uahc_gbl_gparam1r_s  cn73xx;
};
typedef union cvmx_sata_uahc_gbl_gparam1r cvmx_sata_uahc_gbl_gparam1r_t;

/**
 * cvmx_sata_uahc_gbl_gparam2r
 */
union cvmx_sata_uahc_gbl_gparam2r {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_gparam2r_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t rxoob_clk_units              : 1;  /**< RX OOB clock frequency units. */
	uint32_t rxoob_clk_u                  : 10; /**< Upper bits of the RX OOB clock frequency. */
	uint32_t bist_m                       : 1;  /**< BIST loopback checking depth (BIST_MODE). */
	uint32_t fbs_mem_s                    : 1;  /**< Context RAM memory location (FBS_MEM_S). */
	uint32_t fbs_pmpn                     : 2;  /**< Maximum number of port multiplier ports (FBS_PMPN_MAX). */
	uint32_t fbs_support                  : 1;  /**< FIS-based switching support (FBS_SUPPORT). */
	uint32_t dev_cp                       : 1;  /**< Cold presence detect (DEV_CP_DET). */
	uint32_t dev_mp                       : 1;  /**< Mechanical presence switch (DEV_MP_SWITCH). */
	uint32_t encode_m                     : 1;  /**< 8/10 bit encoding/decoding (ENCODE_MODE). */
	uint32_t rxoob_clk_m                  : 1;  /**< RX OOB clock mode (RXOOB_CLK_MODE). */
	uint32_t rx_oob_m                     : 1;  /**< RX OOB mode (RX_OOB_MODE). */
	uint32_t tx_oob_m                     : 1;  /**< TX OOB mode (TX_OOB_MODE). */
	uint32_t reserved_0_8                 : 9;
#else
	uint32_t reserved_0_8                 : 9;
	uint32_t tx_oob_m                     : 1;
	uint32_t rx_oob_m                     : 1;
	uint32_t rxoob_clk_m                  : 1;
	uint32_t encode_m                     : 1;
	uint32_t dev_mp                       : 1;
	uint32_t dev_cp                       : 1;
	uint32_t fbs_support                  : 1;
	uint32_t fbs_pmpn                     : 2;
	uint32_t fbs_mem_s                    : 1;
	uint32_t bist_m                       : 1;
	uint32_t rxoob_clk_u                  : 10;
	uint32_t rxoob_clk_units              : 1;
	uint32_t reserved_31_31               : 1;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_gparam2r_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t bist_m                       : 1;  /**< BIST loopback checking depth (BIST_MODE). */
	uint32_t fbs_mem_s                    : 1;  /**< Context RAM memory location (FBS_MEM_S). */
	uint32_t fbs_pmpn                     : 2;  /**< Maximum number of port multiplier ports (FBS_PMPN_MAX). */
	uint32_t fbs_support                  : 1;  /**< FIS-based switching support (FBS_SUPPORT). */
	uint32_t dev_cp                       : 1;  /**< Cold presence detect (DEV_CP_DET). */
	uint32_t dev_mp                       : 1;  /**< Mechanical presence switch (DEV_MP_SWITCH). */
	uint32_t encode_m                     : 1;  /**< 8/10 bit encoding/decoding (ENCODE_MODE). */
	uint32_t rxoob_clk_m                  : 1;  /**< RX OOB clock mode (RXOOB_CLK_MODE). */
	uint32_t rx_oob_m                     : 1;  /**< RX OOB mode (RX_OOB_MODE). */
	uint32_t tx_oob_m                     : 1;  /**< TX OOB mode (TX_OOB_MODE). */
	uint32_t rxoob_clk                    : 9;  /**< RX OOB clock frequency (RXOOB_CLK). */
#else
	uint32_t rxoob_clk                    : 9;
	uint32_t tx_oob_m                     : 1;
	uint32_t rx_oob_m                     : 1;
	uint32_t rxoob_clk_m                  : 1;
	uint32_t encode_m                     : 1;
	uint32_t dev_mp                       : 1;
	uint32_t dev_cp                       : 1;
	uint32_t fbs_support                  : 1;
	uint32_t fbs_pmpn                     : 2;
	uint32_t fbs_mem_s                    : 1;
	uint32_t bist_m                       : 1;
	uint32_t reserved_20_31               : 12;
#endif
	} cn70xx;
	struct cvmx_sata_uahc_gbl_gparam2r_cn70xx cn70xxp1;
	struct cvmx_sata_uahc_gbl_gparam2r_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t rxoob_clk_units              : 1;  /**< RX OOB clock frequency units. */
	uint32_t rxoob_clk_u                  : 10; /**< Upper bits of the RX OOB clock frequency. */
	uint32_t bist_m                       : 1;  /**< BIST loopback checking depth (BIST_MODE). */
	uint32_t fbs_mem_s                    : 1;  /**< Context RAM memory location (FBS_MEM_S). */
	uint32_t fbs_pmpn                     : 2;  /**< Maximum number of port multiplier ports (FBS_PMPN_MAX). */
	uint32_t fbs_support                  : 1;  /**< FIS-based switching support (FBS_SUPPORT). */
	uint32_t dev_cp                       : 1;  /**< Cold presence detect (DEV_CP_DET). */
	uint32_t dev_mp                       : 1;  /**< Mechanical presence switch (DEV_MP_SWITCH). */
	uint32_t encode_m                     : 1;  /**< 8/10 bit encoding/decoding (ENCODE_MODE). */
	uint32_t rxoob_clk_m                  : 1;  /**< RX OOB clock mode (RXOOB_CLK_MODE). */
	uint32_t rx_oob_m                     : 1;  /**< RX OOB mode (RX_OOB_MODE). */
	uint32_t tx_oob_m                     : 1;  /**< TX OOB mode (TX_OOB_MODE). */
	uint32_t rxoob_clk_l                  : 9;  /**< RX OOB clock frequency (RXOOB_CLK). */
#else
	uint32_t rxoob_clk_l                  : 9;
	uint32_t tx_oob_m                     : 1;
	uint32_t rx_oob_m                     : 1;
	uint32_t rxoob_clk_m                  : 1;
	uint32_t encode_m                     : 1;
	uint32_t dev_mp                       : 1;
	uint32_t dev_cp                       : 1;
	uint32_t fbs_support                  : 1;
	uint32_t fbs_pmpn                     : 2;
	uint32_t fbs_mem_s                    : 1;
	uint32_t bist_m                       : 1;
	uint32_t rxoob_clk_u                  : 10;
	uint32_t rxoob_clk_units              : 1;
	uint32_t reserved_31_31               : 1;
#endif
	} cn73xx;
};
typedef union cvmx_sata_uahc_gbl_gparam2r cvmx_sata_uahc_gbl_gparam2r_t;

/**
 * cvmx_sata_uahc_gbl_idr
 */
union cvmx_sata_uahc_gbl_idr {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_idr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t id                           : 32; /**< Core ID. */
#else
	uint32_t id                           : 32;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_idr_s       cn70xx;
	struct cvmx_sata_uahc_gbl_idr_s       cn70xxp1;
	struct cvmx_sata_uahc_gbl_idr_s       cn73xx;
};
typedef union cvmx_sata_uahc_gbl_idr cvmx_sata_uahc_gbl_idr_t;

/**
 * cvmx_sata_uahc_gbl_is
 *
 * See AHCI specification v1.3 section 3.1.
 *
 */
union cvmx_sata_uahc_gbl_is {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_is_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_3_31                : 29;
	uint32_t ips                          : 3;  /**< Interrupt pending status. */
#else
	uint32_t ips                          : 3;
	uint32_t reserved_3_31                : 29;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_is_s        cn70xx;
	struct cvmx_sata_uahc_gbl_is_s        cn70xxp1;
	struct cvmx_sata_uahc_gbl_is_s        cn73xx;
};
typedef union cvmx_sata_uahc_gbl_is cvmx_sata_uahc_gbl_is_t;

/**
 * cvmx_sata_uahc_gbl_oobr
 *
 * This register is shared between SATA ports. Before accessing this
 * register, first select the required port by writing the port number
 * to the SATA_UAHC_GBL_TESTR[PSEL] field.
 */
union cvmx_sata_uahc_gbl_oobr {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_oobr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t we                           : 1;  /**< Write enable. */
	uint32_t cwmin                        : 7;  /**< COMWAKE minimum value. Writable only if WE is set. */
	uint32_t cwmax                        : 8;  /**< COMWAKE maximum value. Writable only if WE is set. */
	uint32_t cimin                        : 8;  /**< COMINIT minimum value. Writable only if WE is set. */
	uint32_t cimax                        : 8;  /**< COMINIT maximum value. Writable only if WE is set. */
#else
	uint32_t cimax                        : 8;
	uint32_t cimin                        : 8;
	uint32_t cwmax                        : 8;
	uint32_t cwmin                        : 7;
	uint32_t we                           : 1;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_oobr_s      cn70xx;
	struct cvmx_sata_uahc_gbl_oobr_s      cn70xxp1;
	struct cvmx_sata_uahc_gbl_oobr_s      cn73xx;
};
typedef union cvmx_sata_uahc_gbl_oobr cvmx_sata_uahc_gbl_oobr_t;

/**
 * cvmx_sata_uahc_gbl_pi
 *
 * See AHCI specification v1.3 section 3.1.
 *
 */
union cvmx_sata_uahc_gbl_pi {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_pi_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_2_31                : 30;
	uint32_t pi                           : 2;  /**< Number of ports implemented. This field is one-time writable, then becomes read-only. */
#else
	uint32_t pi                           : 2;
	uint32_t reserved_2_31                : 30;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_pi_s        cn70xx;
	struct cvmx_sata_uahc_gbl_pi_s        cn70xxp1;
	struct cvmx_sata_uahc_gbl_pi_s        cn73xx;
};
typedef union cvmx_sata_uahc_gbl_pi cvmx_sata_uahc_gbl_pi_t;

/**
 * cvmx_sata_uahc_gbl_pparamr
 *
 * Port is selected by the SATA_UAHC_GBL_TESTR[PSEL] field.
 *
 */
union cvmx_sata_uahc_gbl_pparamr {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_pparamr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_12_31               : 20;
	uint32_t tx_mem_m                     : 1;  /**< TX FIFO memory read port type (Pn_TX_MEM_MODE). */
	uint32_t tx_mem_s                     : 1;  /**< TX FIFO memory type (Pn_TX_MEM_SELECT). */
	uint32_t rx_mem_m                     : 1;  /**< RX FIFO memory read port type (Pn_RX_MEM_MODE). */
	uint32_t rx_mem_s                     : 1;  /**< RX FIFO memory type (Pn_RX_MEM_SELECT). */
	uint32_t txfifo_depth                 : 4;  /**< TX FIFO depth in FIFO words. */
	uint32_t rxfifo_depth                 : 4;  /**< RX FIFO depth in FIFO words. */
#else
	uint32_t rxfifo_depth                 : 4;
	uint32_t txfifo_depth                 : 4;
	uint32_t rx_mem_s                     : 1;
	uint32_t rx_mem_m                     : 1;
	uint32_t tx_mem_s                     : 1;
	uint32_t tx_mem_m                     : 1;
	uint32_t reserved_12_31               : 20;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_pparamr_s   cn70xx;
	struct cvmx_sata_uahc_gbl_pparamr_s   cn70xxp1;
	struct cvmx_sata_uahc_gbl_pparamr_s   cn73xx;
};
typedef union cvmx_sata_uahc_gbl_pparamr cvmx_sata_uahc_gbl_pparamr_t;

/**
 * cvmx_sata_uahc_gbl_testr
 */
union cvmx_sata_uahc_gbl_testr {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_testr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_25_31               : 7;
	uint32_t bsel                         : 1;  /**< This field is used to select a bank for BIST or Data Protection
                                                         operation. The options for this field are:
                                                         0x0 = BIST registers selected.
                                                         0x1 = Data Protection registers selected. */
	uint32_t reserved_19_23               : 5;
	uint32_t psel                         : 3;  /**< Port select. */
	uint32_t reserved_1_15                : 15;
	uint32_t test_if                      : 1;  /**< Test interface. */
#else
	uint32_t test_if                      : 1;
	uint32_t reserved_1_15                : 15;
	uint32_t psel                         : 3;
	uint32_t reserved_19_23               : 5;
	uint32_t bsel                         : 1;
	uint32_t reserved_25_31               : 7;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_testr_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_19_31               : 13;
	uint32_t psel                         : 3;  /**< Port select. */
	uint32_t reserved_1_15                : 15;
	uint32_t test_if                      : 1;  /**< Test interface. */
#else
	uint32_t test_if                      : 1;
	uint32_t reserved_1_15                : 15;
	uint32_t psel                         : 3;
	uint32_t reserved_19_31               : 13;
#endif
	} cn70xx;
	struct cvmx_sata_uahc_gbl_testr_cn70xx cn70xxp1;
	struct cvmx_sata_uahc_gbl_testr_s     cn73xx;
};
typedef union cvmx_sata_uahc_gbl_testr cvmx_sata_uahc_gbl_testr_t;

/**
 * cvmx_sata_uahc_gbl_timer1ms
 */
union cvmx_sata_uahc_gbl_timer1ms {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_timer1ms_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t timv                         : 20; /**< 1ms timer value. Writable only when SATA_UAHC_GBL_CCC_CTL[EN] = 0. */
#else
	uint32_t timv                         : 20;
	uint32_t reserved_20_31               : 12;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_timer1ms_s  cn70xx;
	struct cvmx_sata_uahc_gbl_timer1ms_s  cn70xxp1;
	struct cvmx_sata_uahc_gbl_timer1ms_s  cn73xx;
};
typedef union cvmx_sata_uahc_gbl_timer1ms cvmx_sata_uahc_gbl_timer1ms_t;

/**
 * cvmx_sata_uahc_gbl_versionr
 */
union cvmx_sata_uahc_gbl_versionr {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_versionr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ver                          : 32; /**< SATA IP version number. */
#else
	uint32_t ver                          : 32;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_versionr_s  cn70xx;
	struct cvmx_sata_uahc_gbl_versionr_s  cn70xxp1;
	struct cvmx_sata_uahc_gbl_versionr_s  cn73xx;
};
typedef union cvmx_sata_uahc_gbl_versionr cvmx_sata_uahc_gbl_versionr_t;

/**
 * cvmx_sata_uahc_gbl_vs
 *
 * See AHCI specification v1.3 section 3.1.
 *
 */
union cvmx_sata_uahc_gbl_vs {
	uint32_t u32;
	struct cvmx_sata_uahc_gbl_vs_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t mjr                          : 16; /**< Major version number. */
	uint32_t mnr                          : 16; /**< Minor version number. No DEVSLP support. */
#else
	uint32_t mnr                          : 16;
	uint32_t mjr                          : 16;
#endif
	} s;
	struct cvmx_sata_uahc_gbl_vs_s        cn70xx;
	struct cvmx_sata_uahc_gbl_vs_s        cn70xxp1;
	struct cvmx_sata_uahc_gbl_vs_s        cn73xx;
};
typedef union cvmx_sata_uahc_gbl_vs cvmx_sata_uahc_gbl_vs_t;

/**
 * cvmx_sata_uahc_p#_ci
 */
union cvmx_sata_uahc_px_ci {
	uint32_t u32;
	struct cvmx_sata_uahc_px_ci_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ci                           : 32; /**< Command issued. */
#else
	uint32_t ci                           : 32;
#endif
	} s;
	struct cvmx_sata_uahc_px_ci_s         cn70xx;
	struct cvmx_sata_uahc_px_ci_s         cn70xxp1;
	struct cvmx_sata_uahc_px_ci_s         cn73xx;
};
typedef union cvmx_sata_uahc_px_ci cvmx_sata_uahc_px_ci_t;

/**
 * cvmx_sata_uahc_p#_clb
 */
union cvmx_sata_uahc_px_clb {
	uint64_t u64;
	struct cvmx_sata_uahc_px_clb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t clb                          : 54; /**< Command-list base address. */
	uint64_t reserved_0_9                 : 10;
#else
	uint64_t reserved_0_9                 : 10;
	uint64_t clb                          : 54;
#endif
	} s;
	struct cvmx_sata_uahc_px_clb_s        cn70xx;
	struct cvmx_sata_uahc_px_clb_s        cn70xxp1;
	struct cvmx_sata_uahc_px_clb_s        cn73xx;
};
typedef union cvmx_sata_uahc_px_clb cvmx_sata_uahc_px_clb_t;

/**
 * cvmx_sata_uahc_p#_cmd
 */
union cvmx_sata_uahc_px_cmd {
	uint32_t u32;
	struct cvmx_sata_uahc_px_cmd_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t icc                          : 4;  /**< Interface communication control. */
	uint32_t asp                          : 1;  /**< Aggressive slumber/partial. */
	uint32_t alpe                         : 1;  /**< Aggressive link-power-management enable. */
	uint32_t dlae                         : 1;  /**< Drive LED on ATAPI enable. */
	uint32_t atapi                        : 1;  /**< Device is ATAPI. */
	uint32_t apste                        : 1;  /**< Automatic partial to slumber transitions enable. */
	uint32_t fbscp                        : 1;  /**< FIS-based switching capable port. Write-once. */
	uint32_t esp                          : 1;  /**< External SATA port. Write-once. */
	uint32_t cpd                          : 1;  /**< Cold-presence detection. Write-once. */
	uint32_t mpsp                         : 1;  /**< Mechanical presence switch attached to port. Write-once. */
	uint32_t hpcp                         : 1;  /**< Hot-plug-capable support. Write-once. */
	uint32_t pma                          : 1;  /**< Port multiplier attached. */
	uint32_t cps                          : 1;  /**< Cold presence state. */
	uint32_t cr                           : 1;  /**< Command list running. */
	uint32_t fr                           : 1;  /**< FIS receive running. */
	uint32_t mpss                         : 1;  /**< Mechanical presence switch state. */
	uint32_t ccs                          : 5;  /**< Current-command slot. */
	uint32_t reserved_5_7                 : 3;
	uint32_t fre                          : 1;  /**< FIS-receive enable. */
	uint32_t clo                          : 1;  /**< Command-list override. */
	uint32_t pod                          : 1;  /**< Power-on device. R/W only if CPD = 1, else read only. */
	uint32_t sud                          : 1;  /**< Spin-up device. R/W only if SATA_UAHC_GBL_CAP[SSS]=1, else read only.
                                                         Setting this bit triggers a COMRESET initialization sequence. */
	uint32_t st                           : 1;  /**< Start. */
#else
	uint32_t st                           : 1;
	uint32_t sud                          : 1;
	uint32_t pod                          : 1;
	uint32_t clo                          : 1;
	uint32_t fre                          : 1;
	uint32_t reserved_5_7                 : 3;
	uint32_t ccs                          : 5;
	uint32_t mpss                         : 1;
	uint32_t fr                           : 1;
	uint32_t cr                           : 1;
	uint32_t cps                          : 1;
	uint32_t pma                          : 1;
	uint32_t hpcp                         : 1;
	uint32_t mpsp                         : 1;
	uint32_t cpd                          : 1;
	uint32_t esp                          : 1;
	uint32_t fbscp                        : 1;
	uint32_t apste                        : 1;
	uint32_t atapi                        : 1;
	uint32_t dlae                         : 1;
	uint32_t alpe                         : 1;
	uint32_t asp                          : 1;
	uint32_t icc                          : 4;
#endif
	} s;
	struct cvmx_sata_uahc_px_cmd_s        cn70xx;
	struct cvmx_sata_uahc_px_cmd_s        cn70xxp1;
	struct cvmx_sata_uahc_px_cmd_s        cn73xx;
};
typedef union cvmx_sata_uahc_px_cmd cvmx_sata_uahc_px_cmd_t;

/**
 * cvmx_sata_uahc_p#_dmacr
 */
union cvmx_sata_uahc_px_dmacr {
	uint32_t u32;
	struct cvmx_sata_uahc_px_dmacr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_8_31                : 24;
	uint32_t rxts                         : 4;  /**< Receive transaction size. This field is R/W when SATA_UAHC_P(0..1)_CMD[ST] = 0
                                                         and read only when SATA_UAHC_P(0..1)_CMD[ST] = 1. */
	uint32_t txts                         : 4;  /**< Transmit transaction size. This field is R/W when SATA_UAHC_P(0..1)_CMD[ST] = 0
                                                         and read only when SATA_UAHC_P(0..1)_CMD[ST] = 1. */
#else
	uint32_t txts                         : 4;
	uint32_t rxts                         : 4;
	uint32_t reserved_8_31                : 24;
#endif
	} s;
	struct cvmx_sata_uahc_px_dmacr_s      cn70xx;
	struct cvmx_sata_uahc_px_dmacr_s      cn70xxp1;
	struct cvmx_sata_uahc_px_dmacr_s      cn73xx;
};
typedef union cvmx_sata_uahc_px_dmacr cvmx_sata_uahc_px_dmacr_t;

/**
 * cvmx_sata_uahc_p#_fb
 */
union cvmx_sata_uahc_px_fb {
	uint64_t u64;
	struct cvmx_sata_uahc_px_fb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t fb                           : 56; /**< FIS base address. */
	uint64_t reserved_0_7                 : 8;
#else
	uint64_t reserved_0_7                 : 8;
	uint64_t fb                           : 56;
#endif
	} s;
	struct cvmx_sata_uahc_px_fb_s         cn70xx;
	struct cvmx_sata_uahc_px_fb_s         cn70xxp1;
	struct cvmx_sata_uahc_px_fb_s         cn73xx;
};
typedef union cvmx_sata_uahc_px_fb cvmx_sata_uahc_px_fb_t;

/**
 * cvmx_sata_uahc_p#_fbs
 */
union cvmx_sata_uahc_px_fbs {
	uint32_t u32;
	struct cvmx_sata_uahc_px_fbs_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t dwe                          : 4;  /**< Device with error. */
	uint32_t ado                          : 4;  /**< Active device optimization. */
	uint32_t dev                          : 4;  /**< Device to issue. */
	uint32_t reserved_3_7                 : 5;
	uint32_t sde                          : 1;  /**< Single device error. */
	uint32_t dec                          : 1;  /**< Device error clear. */
	uint32_t en                           : 1;  /**< Enable. */
#else
	uint32_t en                           : 1;
	uint32_t dec                          : 1;
	uint32_t sde                          : 1;
	uint32_t reserved_3_7                 : 5;
	uint32_t dev                          : 4;
	uint32_t ado                          : 4;
	uint32_t dwe                          : 4;
	uint32_t reserved_20_31               : 12;
#endif
	} s;
	struct cvmx_sata_uahc_px_fbs_s        cn70xx;
	struct cvmx_sata_uahc_px_fbs_s        cn70xxp1;
	struct cvmx_sata_uahc_px_fbs_s        cn73xx;
};
typedef union cvmx_sata_uahc_px_fbs cvmx_sata_uahc_px_fbs_t;

/**
 * cvmx_sata_uahc_p#_ie
 */
union cvmx_sata_uahc_px_ie {
	uint32_t u32;
	struct cvmx_sata_uahc_px_ie_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t cpde                         : 1;  /**< Cold-port-detect enable. */
	uint32_t tfee                         : 1;  /**< Task-file-error enable. */
	uint32_t hbfe                         : 1;  /**< Host-bus fatal-error enable. */
	uint32_t hbde                         : 1;  /**< Host-bus data-error enable. */
	uint32_t ife                          : 1;  /**< Interface fatal-error enable. */
	uint32_t infe                         : 1;  /**< Interface non-fatal-error enable. */
	uint32_t reserved_25_25               : 1;
	uint32_t ofe                          : 1;  /**< Overflow enable. */
	uint32_t impe                         : 1;  /**< Incorrect port-multiplier enable. */
	uint32_t prce                         : 1;  /**< PHY-ready-change enable. */
	uint32_t reserved_8_21                : 14;
	uint32_t dmpe                         : 1;  /**< Device mechanical-presence enable. */
	uint32_t pce                          : 1;  /**< Port-connect-change enable. */
	uint32_t dpe                          : 1;  /**< Descriptor-processed enable. */
	uint32_t ufe                          : 1;  /**< Unknown-FIS-interrupt enable. */
	uint32_t sdbe                         : 1;  /**< Set device-bits-interrupt enable. */
	uint32_t dse                          : 1;  /**< DMA-setup FIS interrupt enable. */
	uint32_t pse                          : 1;  /**< PIO-setup FIS interrupt enable. */
	uint32_t dhre                         : 1;  /**< Device-to-host register FIS interrupt enable. */
#else
	uint32_t dhre                         : 1;
	uint32_t pse                          : 1;
	uint32_t dse                          : 1;
	uint32_t sdbe                         : 1;
	uint32_t ufe                          : 1;
	uint32_t dpe                          : 1;
	uint32_t pce                          : 1;
	uint32_t dmpe                         : 1;
	uint32_t reserved_8_21                : 14;
	uint32_t prce                         : 1;
	uint32_t impe                         : 1;
	uint32_t ofe                          : 1;
	uint32_t reserved_25_25               : 1;
	uint32_t infe                         : 1;
	uint32_t ife                          : 1;
	uint32_t hbde                         : 1;
	uint32_t hbfe                         : 1;
	uint32_t tfee                         : 1;
	uint32_t cpde                         : 1;
#endif
	} s;
	struct cvmx_sata_uahc_px_ie_s         cn70xx;
	struct cvmx_sata_uahc_px_ie_s         cn70xxp1;
	struct cvmx_sata_uahc_px_ie_s         cn73xx;
};
typedef union cvmx_sata_uahc_px_ie cvmx_sata_uahc_px_ie_t;

/**
 * cvmx_sata_uahc_p#_is
 */
union cvmx_sata_uahc_px_is {
	uint32_t u32;
	struct cvmx_sata_uahc_px_is_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t cpds                         : 1;  /**< Cold-port detect status. */
	uint32_t tfes                         : 1;  /**< Task-file error status. */
	uint32_t hbfs                         : 1;  /**< Host-bus fatal-error status. */
	uint32_t hbds                         : 1;  /**< Host-bus data-error status. */
	uint32_t ifs                          : 1;  /**< Interface fatal-error status. */
	uint32_t infs                         : 1;  /**< Interface non-fatal-error status. */
	uint32_t reserved_25_25               : 1;
	uint32_t ofs                          : 1;  /**< Overflow status. */
	uint32_t imps                         : 1;  /**< Incorrect port-multiplier status. */
	uint32_t prcs                         : 1;  /**< PHY-ready change status. */
	uint32_t reserved_8_21                : 14;
	uint32_t dmps                         : 1;  /**< Device mechanical-presence status. */
	uint32_t pcs                          : 1;  /**< Port-connect-change status. */
	uint32_t dps                          : 1;  /**< Descriptor processed. */
	uint32_t ufs                          : 1;  /**< Unknown FIS interrupt. */
	uint32_t sdbs                         : 1;  /**< Set device bits interrupt. */
	uint32_t dss                          : 1;  /**< DMA setup FIS interrupt. */
	uint32_t pss                          : 1;  /**< PIO setup FIS interrupt. */
	uint32_t dhrs                         : 1;  /**< Device-to-host register FIS interrupt. */
#else
	uint32_t dhrs                         : 1;
	uint32_t pss                          : 1;
	uint32_t dss                          : 1;
	uint32_t sdbs                         : 1;
	uint32_t ufs                          : 1;
	uint32_t dps                          : 1;
	uint32_t pcs                          : 1;
	uint32_t dmps                         : 1;
	uint32_t reserved_8_21                : 14;
	uint32_t prcs                         : 1;
	uint32_t imps                         : 1;
	uint32_t ofs                          : 1;
	uint32_t reserved_25_25               : 1;
	uint32_t infs                         : 1;
	uint32_t ifs                          : 1;
	uint32_t hbds                         : 1;
	uint32_t hbfs                         : 1;
	uint32_t tfes                         : 1;
	uint32_t cpds                         : 1;
#endif
	} s;
	struct cvmx_sata_uahc_px_is_s         cn70xx;
	struct cvmx_sata_uahc_px_is_s         cn70xxp1;
	struct cvmx_sata_uahc_px_is_s         cn73xx;
};
typedef union cvmx_sata_uahc_px_is cvmx_sata_uahc_px_is_t;

/**
 * cvmx_sata_uahc_p#_phycr
 */
union cvmx_sata_uahc_px_phycr {
	uint32_t u32;
	struct cvmx_sata_uahc_px_phycr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ctrl                         : 32; /**< Port PHY control. */
#else
	uint32_t ctrl                         : 32;
#endif
	} s;
	struct cvmx_sata_uahc_px_phycr_s      cn70xx;
	struct cvmx_sata_uahc_px_phycr_s      cn70xxp1;
	struct cvmx_sata_uahc_px_phycr_s      cn73xx;
};
typedef union cvmx_sata_uahc_px_phycr cvmx_sata_uahc_px_phycr_t;

/**
 * cvmx_sata_uahc_p#_physr
 */
union cvmx_sata_uahc_px_physr {
	uint32_t u32;
	struct cvmx_sata_uahc_px_physr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t stat                         : 32; /**< Port PHY status. */
#else
	uint32_t stat                         : 32;
#endif
	} s;
	struct cvmx_sata_uahc_px_physr_s      cn70xx;
	struct cvmx_sata_uahc_px_physr_s      cn70xxp1;
	struct cvmx_sata_uahc_px_physr_s      cn73xx;
};
typedef union cvmx_sata_uahc_px_physr cvmx_sata_uahc_px_physr_t;

/**
 * cvmx_sata_uahc_p#_sact
 */
union cvmx_sata_uahc_px_sact {
	uint32_t u32;
	struct cvmx_sata_uahc_px_sact_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ds                           : 32; /**< Device status. */
#else
	uint32_t ds                           : 32;
#endif
	} s;
	struct cvmx_sata_uahc_px_sact_s       cn70xx;
	struct cvmx_sata_uahc_px_sact_s       cn70xxp1;
	struct cvmx_sata_uahc_px_sact_s       cn73xx;
};
typedef union cvmx_sata_uahc_px_sact cvmx_sata_uahc_px_sact_t;

/**
 * cvmx_sata_uahc_p#_sctl
 */
union cvmx_sata_uahc_px_sctl {
	uint32_t u32;
	struct cvmx_sata_uahc_px_sctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_10_31               : 22;
	uint32_t ipm                          : 2;  /**< Interface power-management transitions allowed. */
	uint32_t reserved_6_7                 : 2;
	uint32_t spd                          : 2;  /**< Speed allowed. */
	uint32_t reserved_3_3                 : 1;
	uint32_t det                          : 3;  /**< Device-detection initialization. */
#else
	uint32_t det                          : 3;
	uint32_t reserved_3_3                 : 1;
	uint32_t spd                          : 2;
	uint32_t reserved_6_7                 : 2;
	uint32_t ipm                          : 2;
	uint32_t reserved_10_31               : 22;
#endif
	} s;
	struct cvmx_sata_uahc_px_sctl_s       cn70xx;
	struct cvmx_sata_uahc_px_sctl_s       cn70xxp1;
	struct cvmx_sata_uahc_px_sctl_s       cn73xx;
};
typedef union cvmx_sata_uahc_px_sctl cvmx_sata_uahc_px_sctl_t;

/**
 * cvmx_sata_uahc_p#_serr
 */
union cvmx_sata_uahc_px_serr {
	uint32_t u32;
	struct cvmx_sata_uahc_px_serr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_27_31               : 5;
	uint32_t diag_x                       : 1;  /**< Exchanged. */
	uint32_t diag_f                       : 1;  /**< Unknown FIS type. */
	uint32_t diag_t                       : 1;  /**< Transport state transition error. */
	uint32_t diag_s                       : 1;  /**< Link sequence error. */
	uint32_t diag_h                       : 1;  /**< Handshake error. */
	uint32_t diag_c                       : 1;  /**< CRC error. */
	uint32_t diag_d                       : 1;  /**< Disparity error. */
	uint32_t diag_b                       : 1;  /**< 10/8 bit decode error. */
	uint32_t diag_w                       : 1;  /**< COMWAKE detected. */
	uint32_t diag_i                       : 1;  /**< PHY internal error. */
	uint32_t diag_n                       : 1;  /**< PHY ready change. */
	uint32_t reserved_12_15               : 4;
	uint32_t err_e                        : 1;  /**< Internal error. */
	uint32_t err_p                        : 1;  /**< Protocol error. */
	uint32_t err_c                        : 1;  /**< Non-recovered persistent communication error. */
	uint32_t err_t                        : 1;  /**< Non-recovered transient data integrity error. */
	uint32_t reserved_2_7                 : 6;
	uint32_t err_m                        : 1;  /**< Recovered communication error. */
	uint32_t err_i                        : 1;  /**< Recovered data integrity. */
#else
	uint32_t err_i                        : 1;
	uint32_t err_m                        : 1;
	uint32_t reserved_2_7                 : 6;
	uint32_t err_t                        : 1;
	uint32_t err_c                        : 1;
	uint32_t err_p                        : 1;
	uint32_t err_e                        : 1;
	uint32_t reserved_12_15               : 4;
	uint32_t diag_n                       : 1;
	uint32_t diag_i                       : 1;
	uint32_t diag_w                       : 1;
	uint32_t diag_b                       : 1;
	uint32_t diag_d                       : 1;
	uint32_t diag_c                       : 1;
	uint32_t diag_h                       : 1;
	uint32_t diag_s                       : 1;
	uint32_t diag_t                       : 1;
	uint32_t diag_f                       : 1;
	uint32_t diag_x                       : 1;
	uint32_t reserved_27_31               : 5;
#endif
	} s;
	struct cvmx_sata_uahc_px_serr_s       cn70xx;
	struct cvmx_sata_uahc_px_serr_s       cn70xxp1;
	struct cvmx_sata_uahc_px_serr_s       cn73xx;
};
typedef union cvmx_sata_uahc_px_serr cvmx_sata_uahc_px_serr_t;

/**
 * cvmx_sata_uahc_p#_sig
 */
union cvmx_sata_uahc_px_sig {
	uint32_t u32;
	struct cvmx_sata_uahc_px_sig_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t sig                          : 32; /**< Signature. */
#else
	uint32_t sig                          : 32;
#endif
	} s;
	struct cvmx_sata_uahc_px_sig_s        cn70xx;
	struct cvmx_sata_uahc_px_sig_s        cn70xxp1;
	struct cvmx_sata_uahc_px_sig_s        cn73xx;
};
typedef union cvmx_sata_uahc_px_sig cvmx_sata_uahc_px_sig_t;

/**
 * cvmx_sata_uahc_p#_sntf
 */
union cvmx_sata_uahc_px_sntf {
	uint32_t u32;
	struct cvmx_sata_uahc_px_sntf_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t pmn                          : 16; /**< PM notify. */
#else
	uint32_t pmn                          : 16;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_sata_uahc_px_sntf_s       cn70xx;
	struct cvmx_sata_uahc_px_sntf_s       cn70xxp1;
	struct cvmx_sata_uahc_px_sntf_s       cn73xx;
};
typedef union cvmx_sata_uahc_px_sntf cvmx_sata_uahc_px_sntf_t;

/**
 * cvmx_sata_uahc_p#_ssts
 */
union cvmx_sata_uahc_px_ssts {
	uint32_t u32;
	struct cvmx_sata_uahc_px_ssts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_12_31               : 20;
	uint32_t ipm                          : 4;  /**< Interface power management. */
	uint32_t spd                          : 4;  /**< Current interface speed. */
	uint32_t det                          : 4;  /**< Device detection. */
#else
	uint32_t det                          : 4;
	uint32_t spd                          : 4;
	uint32_t ipm                          : 4;
	uint32_t reserved_12_31               : 20;
#endif
	} s;
	struct cvmx_sata_uahc_px_ssts_s       cn70xx;
	struct cvmx_sata_uahc_px_ssts_s       cn70xxp1;
	struct cvmx_sata_uahc_px_ssts_s       cn73xx;
};
typedef union cvmx_sata_uahc_px_ssts cvmx_sata_uahc_px_ssts_t;

/**
 * cvmx_sata_uahc_p#_tfd
 */
union cvmx_sata_uahc_px_tfd {
	uint32_t u32;
	struct cvmx_sata_uahc_px_tfd_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t tferr                        : 8;  /**< Copy of task-file error register. */
	uint32_t sts                          : 8;  /**< Copy of task-file status register.
                                                         <7> = BSY: Indicates the interface is busy.
                                                         <6:4> = Command specific.
                                                         <3> = DRQ: Indicates a data transfer is requested.
                                                         <2:1> = Command specific.
                                                         <0> = ERR: Indicates an error during the transfer. */
#else
	uint32_t sts                          : 8;
	uint32_t tferr                        : 8;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_sata_uahc_px_tfd_s        cn70xx;
	struct cvmx_sata_uahc_px_tfd_s        cn70xxp1;
	struct cvmx_sata_uahc_px_tfd_s        cn73xx;
};
typedef union cvmx_sata_uahc_px_tfd cvmx_sata_uahc_px_tfd_t;

/**
 * cvmx_sata_uctl_bist_status
 *
 * Results from BIST runs of SATA's memories.
 * Wait for NDONE==0, then look at defect indication.
 *
 * Accessible always.
 *
 * Reset by IOI reset.
 */
union cvmx_sata_uctl_bist_status {
	uint64_t u64;
	struct cvmx_sata_uctl_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t uctl_xm_r_bist_ndone         : 1;  /**< BIST is not complete for the UCTL AxiMaster read-data FIFO. */
	uint64_t uctl_xm_w_bist_ndone         : 1;  /**< BIST is not complete for the UCTL AxiMaster write-data FIFO. */
	uint64_t reserved_36_39               : 4;
	uint64_t uahc_p0_rxram_bist_ndone     : 1;  /**< BIST is not complete for the UAHC Port 0 RxFIFO RAM. */
	uint64_t uahc_p1_rxram_bist_ndone     : 1;  /**< BIST is not complete for the UAHC Port 1 RxFIFO RAM. */
	uint64_t uahc_p0_txram_bist_ndone     : 1;  /**< BIST is not complete for the UAHC Port 0 TxFIFO RAM. */
	uint64_t uahc_p1_txram_bist_ndone     : 1;  /**< BIST is not complete for the UAHC Port 1 TxFIFO RAM. */
	uint64_t reserved_10_31               : 22;
	uint64_t uctl_xm_r_bist_status        : 1;  /**< BIST status of the UCTL AxiMaster read-data FIFO. */
	uint64_t uctl_xm_w_bist_status        : 1;  /**< BIST status of the UCTL AxiMaster write-data FIFO. */
	uint64_t reserved_4_7                 : 4;
	uint64_t uahc_p0_rxram_bist_status    : 1;  /**< BIST status of the UAHC Port0 RxFIFO RAM. */
	uint64_t uahc_p1_rxram_bist_status    : 1;  /**< BIST status of the UAHC Port1 RxFIFO RAM. */
	uint64_t uahc_p0_txram_bist_status    : 1;  /**< BIST status of the UAHC Port0 TxFIFO RAM. */
	uint64_t uahc_p1_txram_bist_status    : 1;  /**< BIST status of the UAHC Port1 TxFIFO RAM. */
#else
	uint64_t uahc_p1_txram_bist_status    : 1;
	uint64_t uahc_p0_txram_bist_status    : 1;
	uint64_t uahc_p1_rxram_bist_status    : 1;
	uint64_t uahc_p0_rxram_bist_status    : 1;
	uint64_t reserved_4_7                 : 4;
	uint64_t uctl_xm_w_bist_status        : 1;
	uint64_t uctl_xm_r_bist_status        : 1;
	uint64_t reserved_10_31               : 22;
	uint64_t uahc_p1_txram_bist_ndone     : 1;
	uint64_t uahc_p0_txram_bist_ndone     : 1;
	uint64_t uahc_p1_rxram_bist_ndone     : 1;
	uint64_t uahc_p0_rxram_bist_ndone     : 1;
	uint64_t reserved_36_39               : 4;
	uint64_t uctl_xm_w_bist_ndone         : 1;
	uint64_t uctl_xm_r_bist_ndone         : 1;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_sata_uctl_bist_status_s   cn70xx;
	struct cvmx_sata_uctl_bist_status_s   cn70xxp1;
	struct cvmx_sata_uctl_bist_status_s   cn73xx;
};
typedef union cvmx_sata_uctl_bist_status cvmx_sata_uctl_bist_status_t;

/**
 * cvmx_sata_uctl_ctl
 *
 * This register controls clocks, resets, power, and BIST for the SATA.
 *
 * Accessible always.
 *
 * Reset by IOI reset.
 */
union cvmx_sata_uctl_ctl {
	uint64_t u64;
	struct cvmx_sata_uctl_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t clear_bist                   : 1;  /**< BIST fast-clear mode select. There are two major modes of BIST: FULL and CLEAR.
                                                         0 = FULL BIST is run by the BIST state machine.
                                                         1 = CLEAR BIST is run by the BIST state machine. A clear-BIST run clears all entries in
                                                         SATA RAMs to 0x0.
                                                         To avoid race conditions, software must first perform a CSR write operation that puts
                                                         CLEAR_BIST into the correct state and then perform another CSR write operation to set
                                                         START_BIST (keeping CLEAR_BIST constant). CLEAR BIST completion is indicated by
                                                         SATA_UCTL_BIST_STATUS[NDONE].
                                                         A BIST clear operation takes almost 2,000 host-controller clock cycles for the largest
                                                         RAM. */
	uint64_t start_bist                   : 1;  /**< Start BIST. The rising edge starts BIST on the memories in SATA. To run BIST, the host-
                                                         controller clock must be both configured and enabled, and should be configured to the
                                                         maximum available frequency given the available coprocessor clock and dividers.
                                                         Refer to Cold Reset for clock initialization procedures. BIST defect status can
                                                         be checked after FULL BIST completion, both of which are indicated in
                                                         SATA()_UCTL_BIST_STATUS. The FULL BIST run takes almost 80,000 host-controller
                                                         clock cycles for the largest RAM. */
	uint64_t reserved_31_61               : 31;
	uint64_t a_clk_en                     : 1;  /**< Host-controller clock enable. When set to one, the host-controller clock is generated. This
                                                         also enables access to UCTL registers 0x30-0xF8. */
	uint64_t a_clk_byp_sel                : 1;  /**< Select the bypass input to the host-controller clock divider.
                                                         0 = Use the divided coprocessor clock from the A_CLKDIV divider.
                                                         1 = use the bypass clock from the GPIO pins (generally bypass is only used for scan
                                                         purposes).
                                                         This signal is a multiplexer-select signal; it does not enable the host-controller clock.
                                                         You must set A_CLK_EN separately. A_CLK_BYP_SEL select should not be changed unless
                                                         A_CLK_EN is disabled. The bypass clock can be selected and running even if the host-
                                                         controller clock dividers are not running. */
	uint64_t a_clkdiv_rst                 : 1;  /**< Host-controller-clock divider reset. Divided clocks are not generated while the divider is
                                                         being reset.
                                                         This also resets the suspend-clock divider. */
	uint64_t reserved_27_27               : 1;
	uint64_t a_clkdiv_sel                 : 3;  /**< The host-controller clock frequency is the coprocessor-clock frequency divided by
                                                         A_CLKDIV_SEL. The host-controller clock frequency must be at or below the minimum
                                                         ACLK requirements.
                                                         This field can be changed only when A_CLKDIV_RST = 1. The divider values are the
                                                         following:
                                                         0x0 = divide by 1.
                                                         0x1 = divide by 2.
                                                         0x2 = divide by 3.
                                                         0x3 = divide by 4.
                                                         0x4 = divide by 6.
                                                         0x5 = divide by 8.
                                                         0x6 = divide by 16.
                                                         0x7 = divide by 24. */
	uint64_t reserved_5_23                : 19;
	uint64_t csclk_en                     : 1;  /**< Turns on the SATA UCTL interface clock (coprocessor clock). This enables access to UAHC
                                                         registers via the IOI, as well as UCTL registers starting from 0x30 via the RSL bus. */
	uint64_t reserved_2_3                 : 2;
	uint64_t sata_uahc_rst                : 1;  /**< Software reset; resets UAHC; active-high. */
	uint64_t sata_uctl_rst                : 1;  /**< Software reset; resets UCTL; active-high. Resets UAHC DMA and register shims and the UCTL
                                                         RSL registers 0x30-0xF8.
                                                         It does not reset UCTL RSL registers 0x0-0x28.
                                                         The UCTL RSL registers starting from 0x30 can be accessed only after the host-controller
                                                         clock is active and UCTL_RST is deasserted. */
#else
	uint64_t sata_uctl_rst                : 1;
	uint64_t sata_uahc_rst                : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t csclk_en                     : 1;
	uint64_t reserved_5_23                : 19;
	uint64_t a_clkdiv_sel                 : 3;
	uint64_t reserved_27_27               : 1;
	uint64_t a_clkdiv_rst                 : 1;
	uint64_t a_clk_byp_sel                : 1;
	uint64_t a_clk_en                     : 1;
	uint64_t reserved_31_61               : 31;
	uint64_t start_bist                   : 1;
	uint64_t clear_bist                   : 1;
#endif
	} s;
	struct cvmx_sata_uctl_ctl_s           cn70xx;
	struct cvmx_sata_uctl_ctl_s           cn70xxp1;
	struct cvmx_sata_uctl_ctl_s           cn73xx;
};
typedef union cvmx_sata_uctl_ctl cvmx_sata_uctl_ctl_t;

/**
 * cvmx_sata_uctl_ecc
 *
 * This register can be used to disable ECC correction, insert ECC errors, and debug ECC
 * failures.
 *
 * Fields ECC_ERR* are captured when there are no outstanding ECC errors indicated in INTSTAT
 * and a new ECC error arrives. Prioritization for multiple events occurring on the same cycle is
 * indicated by the ECC_ERR_SOURCE enumeration: highest encoded value has highest priority.
 *
 * Fields *ECC_DIS: Disables ECC correction, SBE and DBE errors are still reported.
 * If ECC_DIS is 0x1, then no data-correction occurs.
 *
 * Fields *ECC_FLIP_SYND:  Flip the syndrom[1:0] bits to generate 1-bit/2-bits error for testing.
 *
 * Accessible only when SATA_UCTL_CTL[A_CLK_EN].
 *
 * Reset by IOI reset or SATA_UCTL_CTL[SATA_UCTL_RST].
 */
union cvmx_sata_uctl_ecc {
	uint64_t u64;
	struct cvmx_sata_uctl_ecc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_63_63               : 1;
	uint64_t ecc_err_source               : 5;  /**< Source of ECC error, see SATA_UCTL_ECC_ERR_SOURCE_E. */
	uint64_t ecc_err_syndrome             : 18; /**< Syndrome bits of the ECC error. */
	uint64_t ecc_err_address              : 8;  /**< RAM address of the ECC error. */
	uint64_t reserved_24_31               : 8;
	uint64_t uctl_xm_r_ecc_flip_synd      : 2;  /**< Insert ECC error for testing purposes. */
	uint64_t uctl_xm_r_ecc_cor_dis        : 1;  /**< Enables ECC correction on UCTL AxiMaster read-data FIFO. */
	uint64_t uctl_xm_w_ecc_flip_synd      : 2;  /**< Insert ECC error for testing purposes. */
	uint64_t uctl_xm_w_ecc_cor_dis        : 1;  /**< Enables ECC correction on UCTL AxiMaster write-data FIFO. */
	uint64_t uahc_rx_ecc_flip_synd_p0     : 2;  /**< Insert ECC error for testing purposes for the UAHC RX RAM on Port 0. */
	uint64_t uahc_rx_ecc_cor_dis_p0       : 1;  /**< Enables ECC correction on the UAHC RX RAM on Port 0. */
	uint64_t uahc_tx_ecc_flip_synd_p0     : 2;  /**< Insert ECC error for testing purposes for the UAHC TX RAM on Port 0. */
	uint64_t uahc_tx_ecc_cor_dis_p0       : 1;  /**< Enables ECC correction on the UAHC TX RAM on Port 0. */
	uint64_t uahc_fb_ecc_flip_synd_p0     : 2;  /**< Insert ECC error for testing purposes for the UAHC FB RAM on Port 0. */
	uint64_t uahc_fb_ecc_cor_dis_p0       : 1;  /**< Enables ECC correction on the UAHC FB RAM on Port 0. */
	uint64_t uahc_rx_ecc_flip_synd_p1     : 2;  /**< Insert ECC error for testing purposes for the UAHC RX RAM on Port 1. */
	uint64_t uahc_rx_ecc_cor_dis_p1       : 1;  /**< Enables ECC correction on the UAHC RX RAM on Port 1. */
	uint64_t uahc_tx_ecc_flip_synd_p1     : 2;  /**< Insert ECC error for testing purposes for the UAHC TX RAM on Port 1. */
	uint64_t uahc_tx_ecc_cor_dis_p1       : 1;  /**< Enables ECC correction on the UAHC TX RAM on Port 1. */
	uint64_t uahc_fb_ecc_flip_synd_p1     : 2;  /**< Insert ECC error for testing purposes for the UAHC FB RAM on Port 1. */
	uint64_t uahc_fb_ecc_cor_dis_p1       : 1;  /**< Enables ECC correction on the UAHC FB RAM on Port 1. */
#else
	uint64_t uahc_fb_ecc_cor_dis_p1       : 1;
	uint64_t uahc_fb_ecc_flip_synd_p1     : 2;
	uint64_t uahc_tx_ecc_cor_dis_p1       : 1;
	uint64_t uahc_tx_ecc_flip_synd_p1     : 2;
	uint64_t uahc_rx_ecc_cor_dis_p1       : 1;
	uint64_t uahc_rx_ecc_flip_synd_p1     : 2;
	uint64_t uahc_fb_ecc_cor_dis_p0       : 1;
	uint64_t uahc_fb_ecc_flip_synd_p0     : 2;
	uint64_t uahc_tx_ecc_cor_dis_p0       : 1;
	uint64_t uahc_tx_ecc_flip_synd_p0     : 2;
	uint64_t uahc_rx_ecc_cor_dis_p0       : 1;
	uint64_t uahc_rx_ecc_flip_synd_p0     : 2;
	uint64_t uctl_xm_w_ecc_cor_dis        : 1;
	uint64_t uctl_xm_w_ecc_flip_synd      : 2;
	uint64_t uctl_xm_r_ecc_cor_dis        : 1;
	uint64_t uctl_xm_r_ecc_flip_synd      : 2;
	uint64_t reserved_24_31               : 8;
	uint64_t ecc_err_address              : 8;
	uint64_t ecc_err_syndrome             : 18;
	uint64_t ecc_err_source               : 5;
	uint64_t reserved_63_63               : 1;
#endif
	} s;
	struct cvmx_sata_uctl_ecc_s           cn73xx;
};
typedef union cvmx_sata_uctl_ecc cvmx_sata_uctl_ecc_t;

/**
 * cvmx_sata_uctl_intstat
 *
 * Summary of different bits of interrupts.
 *
 * Accessible always.
 *
 * Reset by IOI reset.
 */
union cvmx_sata_uctl_intstat {
	uint64_t u64;
	struct cvmx_sata_uctl_intstat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t fb_dbe_p0                    : 1;  /**< Detected double-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t rx_dbe_p0                    : 1;  /**< Detected double-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t tx_dbe_p0                    : 1;  /**< Detected double-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t fb_dbe_p1                    : 1;  /**< Detected double-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t rx_dbe_p1                    : 1;  /**< Detected double-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t tx_dbe_p1                    : 1;  /**< Detected double-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t fb_sbe_p0                    : 1;  /**< Detected double-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t rx_sbe_p0                    : 1;  /**< Detected double-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t tx_sbe_p0                    : 1;  /**< Detected double-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t fb_sbe_p1                    : 1;  /**< Detected double-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t rx_sbe_p1                    : 1;  /**< Detected double-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t tx_sbe_p1                    : 1;  /**< Detected double-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t xm_r_dbe                     : 1;  /**< Detected double-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t xm_r_sbe                     : 1;  /**< Detected single-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t xm_w_dbe                     : 1;  /**< Detected double-bit error on the UCTL AxiMaster write-data FIFO. */
	uint64_t xm_w_sbe                     : 1;  /**< Detected single-bit error on the UCTL AxiMaster write-data FIFO. */
	uint64_t reserved_3_25                : 23;
	uint64_t xm_bad_dma                   : 1;  /**< Detected bad DMA access from UAHC to IOI. The error information is logged in
                                                         SATA_UCTL_SHIM_CFG[XM_BAD_DMA_*]. Received a DMA request from UAHC that violates the
                                                         assumptions made by the AXI-to-IOI shim. Such scenarios include: illegal length/size
                                                         combinations and address out-of-bounds.
                                                         For more information on exact failures, see description in
                                                         SATA_UCTL_SHIM_CFG[XM_BAD_DMA_TYPE].
                                                         The hardware does not translate the request correctly and results may violate IOI
                                                         protocols. */
	uint64_t xs_ncb_oob                   : 1;  /**< Detected out-of-bound register access to UAHC over IOI. The UAHC defines 1MB of register
                                                         space, starting at offset 0x0. Any accesses outside of this register space cause this bit
                                                         to be set to 1. The error information is logged in SATA_UCTL_SHIM_CFG[XS_NCB_OOB_*]. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t xs_ncb_oob                   : 1;
	uint64_t xm_bad_dma                   : 1;
	uint64_t reserved_3_25                : 23;
	uint64_t xm_w_sbe                     : 1;
	uint64_t xm_w_dbe                     : 1;
	uint64_t xm_r_sbe                     : 1;
	uint64_t xm_r_dbe                     : 1;
	uint64_t tx_sbe_p1                    : 1;
	uint64_t rx_sbe_p1                    : 1;
	uint64_t fb_sbe_p1                    : 1;
	uint64_t tx_sbe_p0                    : 1;
	uint64_t rx_sbe_p0                    : 1;
	uint64_t fb_sbe_p0                    : 1;
	uint64_t tx_dbe_p1                    : 1;
	uint64_t rx_dbe_p1                    : 1;
	uint64_t fb_dbe_p1                    : 1;
	uint64_t tx_dbe_p0                    : 1;
	uint64_t rx_dbe_p0                    : 1;
	uint64_t fb_dbe_p0                    : 1;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_sata_uctl_intstat_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t xm_bad_dma                   : 1;  /**< Detected bad DMA access from UAHC to IOI. The error information is logged in
                                                         SATA_UCTL_SHIM_CFG[XM_BAD_DMA_*]. Received a DMA request from UAHC that violates the
                                                         assumptions made by the AXI-to-IOI shim. Such scenarios include: illegal length/size
                                                         combinations and address out-of-bounds.
                                                         For more information on exact failures, see description in
                                                         SATA_UCTL_SHIM_CFG[XM_BAD_DMA_TYPE].
                                                         The hardware does not translate the request correctly and results may violate IOI
                                                         protocols. */
	uint64_t xs_ncb_oob                   : 1;  /**< Detected out-of-bound register access to UAHC over IOI. The UAHC defines 1MB of register
                                                         space, starting at offset 0x0. Any accesses outside of this register space cause this bit
                                                         to be set to 1. The error information is logged in SATA_UCTL_SHIM_CFG[XS_NCB_OOB_*]. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t xs_ncb_oob                   : 1;
	uint64_t xm_bad_dma                   : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} cn70xx;
	struct cvmx_sata_uctl_intstat_cn70xx  cn70xxp1;
	struct cvmx_sata_uctl_intstat_s       cn73xx;
};
typedef union cvmx_sata_uctl_intstat cvmx_sata_uctl_intstat_t;

/**
 * cvmx_sata_uctl_shim_cfg
 *
 * This register allows configuration of various shim (UCTL) features.
 *
 * Fields XS_NCB_OOB_* are captured when there are no outstanding OOB errors indicated in INTSTAT
 * and a new OOB error arrives.
 *
 * Fields XS_BAD_DMA_* are captured when there are no outstanding DMA errors indicated in INTSTAT
 * and a new DMA error arrives.
 *
 * Accessible only when SATA_UCTL_CTL[A_CLK_EN].
 *
 * Reset by IOI reset or SATA_UCTL_CTL[SATA_UCTL_RST].
 */
union cvmx_sata_uctl_shim_cfg {
	uint64_t u64;
	struct cvmx_sata_uctl_shim_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t xs_ncb_oob_wrn               : 1;  /**< Read/write error log for out-of-bound UAHC register access.
                                                         0 = read, 1 = write. */
	uint64_t reserved_60_62               : 3;
	uint64_t xs_ncb_oob_osrc              : 12; /**< SRCID error log for out-of-bound UAHC register access. The IOI outbound SRCID for the OOB error. */
	uint64_t xm_bad_dma_wrn               : 1;  /**< Read/write error log for bad DMA access from UAHC.
                                                         0 = read error log, 1 = write error log. */
	uint64_t reserved_44_46               : 3;
	uint64_t xm_bad_dma_type              : 4;  /**< ErrType error log for bad DMA access from UAHC. Encodes the type of error encountered
                                                         (error largest encoded value has priority). See SATA_UCTL_XM_BAD_DMA_TYPE_E. */
	uint64_t reserved_14_39               : 26;
	uint64_t dma_read_cmd                 : 2;  /**< Selects the IOI read command used by DMA accesses. See SATA_UCTL_DMA_READ_CMD_E. */
	uint64_t reserved_11_11               : 1;
	uint64_t dma_write_cmd                : 1;  /**< Selects the NCB write command used by DMA accesses. See UCTL_DMA_WRITE_CMD_E. */
	uint64_t dma_endian_mode              : 2;  /**< Selects the endian format for DMA accesses to the L2C. See SATA_UCTL_ENDIAN_MODE_E. */
	uint64_t reserved_2_7                 : 6;
	uint64_t csr_endian_mode              : 2;  /**< Selects the endian format for IOI CSR accesses to the UAHC. Note that when UAHC CSRs are
                                                         accessed via RSL, they are returned as big-endian. See SATA_UCTL_ENDIAN_MODE_E. */
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
	struct cvmx_sata_uctl_shim_cfg_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t xs_ncb_oob_wrn               : 1;  /**< Read/write error log for out-of-bound UAHC register access.
                                                         0 = read, 1 = write. */
	uint64_t reserved_57_62               : 6;
	uint64_t xs_ncb_oob_osrc              : 9;  /**< SRCID error log for out-of-bound UAHC register access. The IOI outbound SRCID for the OOB error. */
	uint64_t xm_bad_dma_wrn               : 1;  /**< Read/write error log for bad DMA access from UAHC.
                                                         0 = read error log, 1 = write error log. */
	uint64_t reserved_44_46               : 3;
	uint64_t xm_bad_dma_type              : 4;  /**< ErrType error log for bad DMA access from UAHC. Encodes the type of error encountered
                                                         (error largest encoded value has priority). See SATA_UCTL_XM_BAD_DMA_TYPE_E. */
	uint64_t reserved_13_39               : 27;
	uint64_t dma_read_cmd                 : 1;  /**< Selects the IOI read command used by DMA accesses. See SATA_UCTL_DMA_READ_CMD_E. */
	uint64_t reserved_10_11               : 2;
	uint64_t dma_endian_mode              : 2;  /**< Selects the endian format for DMA accesses to the L2C. See SATA_UCTL_ENDIAN_MODE_E. */
	uint64_t reserved_2_7                 : 6;
	uint64_t csr_endian_mode              : 2;  /**< Selects the endian format for IOI CSR accesses to the UAHC. Note that when UAHC CSRs are
                                                         accessed via RSL, they are returned as big-endian. See SATA_UCTL_ENDIAN_MODE_E. */
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
	uint64_t xs_ncb_oob_osrc              : 9;
	uint64_t reserved_57_62               : 6;
	uint64_t xs_ncb_oob_wrn               : 1;
#endif
	} cn70xx;
	struct cvmx_sata_uctl_shim_cfg_cn70xx cn70xxp1;
	struct cvmx_sata_uctl_shim_cfg_s      cn73xx;
};
typedef union cvmx_sata_uctl_shim_cfg cvmx_sata_uctl_shim_cfg_t;

/**
 * cvmx_sata_uctl_spare0
 *
 * This register is spare.
 *
 * Accessible only when SATA_UCTL_CTL[A_CLK_EN].
 *
 * Reset by IOI reset or SATA_UCTL_CTL[SATA_UCTL_RST].
 */
union cvmx_sata_uctl_spare0 {
	uint64_t u64;
	struct cvmx_sata_uctl_spare0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_sata_uctl_spare0_s        cn70xx;
	struct cvmx_sata_uctl_spare0_s        cn70xxp1;
};
typedef union cvmx_sata_uctl_spare0 cvmx_sata_uctl_spare0_t;

/**
 * cvmx_sata_uctl_spare0_eco
 *
 * This register is spare.
 *
 * Accessible always.
 *
 * Reset by IOI reset.
 */
union cvmx_sata_uctl_spare0_eco {
	uint64_t u64;
	struct cvmx_sata_uctl_spare0_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t eco_rw                       : 32; /**< Spare. */
#else
	uint64_t eco_rw                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sata_uctl_spare0_eco_s    cn73xx;
};
typedef union cvmx_sata_uctl_spare0_eco cvmx_sata_uctl_spare0_eco_t;

/**
 * cvmx_sata_uctl_spare1
 *
 * This register is spare.
 *
 * Accessible only when SATA_UCTL_CTL[A_CLK_EN].
 *
 * Reset by IOI reset or SATA_UCTL_CTL[SATA_UCTL_RST].
 */
union cvmx_sata_uctl_spare1 {
	uint64_t u64;
	struct cvmx_sata_uctl_spare1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_sata_uctl_spare1_s        cn70xx;
	struct cvmx_sata_uctl_spare1_s        cn70xxp1;
};
typedef union cvmx_sata_uctl_spare1 cvmx_sata_uctl_spare1_t;

/**
 * cvmx_sata_uctl_spare1_eco
 *
 * This register is spare.
 *
 * Accessible only when SATA_UCTL_CTL[A_CLK_EN].
 *
 * Reset by IOI reset or SATA_UCTL_CTL[SATA_UCTL_RST].
 */
union cvmx_sata_uctl_spare1_eco {
	uint64_t u64;
	struct cvmx_sata_uctl_spare1_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t eco_rw                       : 32; /**< N/A */
#else
	uint64_t eco_rw                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sata_uctl_spare1_eco_s    cn73xx;
};
typedef union cvmx_sata_uctl_spare1_eco cvmx_sata_uctl_spare1_eco_t;

#endif
