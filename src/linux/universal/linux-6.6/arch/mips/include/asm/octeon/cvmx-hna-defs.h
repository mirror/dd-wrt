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
 * cvmx-hna-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon hna.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_HNA_DEFS_H__
#define __CVMX_HNA_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_BIST0 CVMX_HNA_BIST0_FUNC()
static inline uint64_t CVMX_HNA_BIST0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_BIST0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800470007F0ull);
}
#else
#define CVMX_HNA_BIST0 (CVMX_ADD_IO_SEG(0x00011800470007F0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_BIST1 CVMX_HNA_BIST1_FUNC()
static inline uint64_t CVMX_HNA_BIST1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_BIST1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800470007F8ull);
}
#else
#define CVMX_HNA_BIST1 (CVMX_ADD_IO_SEG(0x00011800470007F8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_CONFIG CVMX_HNA_CONFIG_FUNC()
static inline uint64_t CVMX_HNA_CONFIG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_CONFIG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000000ull);
}
#else
#define CVMX_HNA_CONFIG (CVMX_ADD_IO_SEG(0x0001180047000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_CONTROL CVMX_HNA_CONTROL_FUNC()
static inline uint64_t CVMX_HNA_CONTROL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_CONTROL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000020ull);
}
#else
#define CVMX_HNA_CONTROL (CVMX_ADD_IO_SEG(0x0001180047000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_DBELL CVMX_HNA_DBELL_FUNC()
static inline uint64_t CVMX_HNA_DBELL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_DBELL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001470000000000ull);
}
#else
#define CVMX_HNA_DBELL (CVMX_ADD_IO_SEG(0x0001470000000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_DIFCTL CVMX_HNA_DIFCTL_FUNC()
static inline uint64_t CVMX_HNA_DIFCTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_DIFCTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001470600000000ull);
}
#else
#define CVMX_HNA_DIFCTL (CVMX_ADD_IO_SEG(0x0001470600000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_DIFRDPTR CVMX_HNA_DIFRDPTR_FUNC()
static inline uint64_t CVMX_HNA_DIFRDPTR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_DIFRDPTR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001470200000000ull);
}
#else
#define CVMX_HNA_DIFRDPTR (CVMX_ADD_IO_SEG(0x0001470200000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_ECO CVMX_HNA_ECO_FUNC()
static inline uint64_t CVMX_HNA_ECO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_ECO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800470000D0ull);
}
#else
#define CVMX_HNA_ECO (CVMX_ADD_IO_SEG(0x00011800470000D0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_ERROR CVMX_HNA_ERROR_FUNC()
static inline uint64_t CVMX_HNA_ERROR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_ERROR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000028ull);
}
#else
#define CVMX_HNA_ERROR (CVMX_ADD_IO_SEG(0x0001180047000028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_ERROR_CAPTURE_DATA CVMX_HNA_ERROR_CAPTURE_DATA_FUNC()
static inline uint64_t CVMX_HNA_ERROR_CAPTURE_DATA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_ERROR_CAPTURE_DATA not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000038ull);
}
#else
#define CVMX_HNA_ERROR_CAPTURE_DATA (CVMX_ADD_IO_SEG(0x0001180047000038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_ERROR_CAPTURE_INFO CVMX_HNA_ERROR_CAPTURE_INFO_FUNC()
static inline uint64_t CVMX_HNA_ERROR_CAPTURE_INFO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_ERROR_CAPTURE_INFO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000030ull);
}
#else
#define CVMX_HNA_ERROR_CAPTURE_INFO (CVMX_ADD_IO_SEG(0x0001180047000030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_HNA_HNC0_RAM1X(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63)))))
		cvmx_warn("CVMX_HNA_HNC0_RAM1X(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001470400000000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_HNA_HNC0_RAM1X(offset) (CVMX_ADD_IO_SEG(0x0001470400000000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_HNA_HNC0_RAM2X(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63)))))
		cvmx_warn("CVMX_HNA_HNC0_RAM2X(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001470400040000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_HNA_HNC0_RAM2X(offset) (CVMX_ADD_IO_SEG(0x0001470400040000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_HNA_HNC1_RAM1X(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63)))))
		cvmx_warn("CVMX_HNA_HNC1_RAM1X(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001470400400000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_HNA_HNC1_RAM1X(offset) (CVMX_ADD_IO_SEG(0x0001470400400000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_HNA_HNC1_RAM2X(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63)))))
		cvmx_warn("CVMX_HNA_HNC1_RAM2X(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001470400440000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_HNA_HNC1_RAM2X(offset) (CVMX_ADD_IO_SEG(0x0001470400440000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_HPU_CSR CVMX_HNA_HPU_CSR_FUNC()
static inline uint64_t CVMX_HNA_HPU_CSR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_HPU_CSR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000010ull);
}
#else
#define CVMX_HNA_HPU_CSR (CVMX_ADD_IO_SEG(0x0001180047000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_HPU_DBG CVMX_HNA_HPU_DBG_FUNC()
static inline uint64_t CVMX_HNA_HPU_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_HPU_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000008ull);
}
#else
#define CVMX_HNA_HPU_DBG (CVMX_ADD_IO_SEG(0x0001180047000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_HPU_EIR CVMX_HNA_HPU_EIR_FUNC()
static inline uint64_t CVMX_HNA_HPU_EIR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_HPU_EIR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000018ull);
}
#else
#define CVMX_HNA_HPU_EIR (CVMX_ADD_IO_SEG(0x0001180047000018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC0_CNT CVMX_HNA_PFC0_CNT_FUNC()
static inline uint64_t CVMX_HNA_PFC0_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_PFC0_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000090ull);
}
#else
#define CVMX_HNA_PFC0_CNT (CVMX_ADD_IO_SEG(0x0001180047000090ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC0_CTL CVMX_HNA_PFC0_CTL_FUNC()
static inline uint64_t CVMX_HNA_PFC0_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_PFC0_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000088ull);
}
#else
#define CVMX_HNA_PFC0_CTL (CVMX_ADD_IO_SEG(0x0001180047000088ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC1_CNT CVMX_HNA_PFC1_CNT_FUNC()
static inline uint64_t CVMX_HNA_PFC1_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_PFC1_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800470000A0ull);
}
#else
#define CVMX_HNA_PFC1_CNT (CVMX_ADD_IO_SEG(0x00011800470000A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC1_CTL CVMX_HNA_PFC1_CTL_FUNC()
static inline uint64_t CVMX_HNA_PFC1_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_PFC1_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000098ull);
}
#else
#define CVMX_HNA_PFC1_CTL (CVMX_ADD_IO_SEG(0x0001180047000098ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC2_CNT CVMX_HNA_PFC2_CNT_FUNC()
static inline uint64_t CVMX_HNA_PFC2_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_PFC2_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800470000B0ull);
}
#else
#define CVMX_HNA_PFC2_CNT (CVMX_ADD_IO_SEG(0x00011800470000B0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC2_CTL CVMX_HNA_PFC2_CTL_FUNC()
static inline uint64_t CVMX_HNA_PFC2_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_PFC2_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800470000A8ull);
}
#else
#define CVMX_HNA_PFC2_CTL (CVMX_ADD_IO_SEG(0x00011800470000A8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC3_CNT CVMX_HNA_PFC3_CNT_FUNC()
static inline uint64_t CVMX_HNA_PFC3_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_PFC3_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800470000C0ull);
}
#else
#define CVMX_HNA_PFC3_CNT (CVMX_ADD_IO_SEG(0x00011800470000C0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC3_CTL CVMX_HNA_PFC3_CTL_FUNC()
static inline uint64_t CVMX_HNA_PFC3_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_PFC3_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800470000B8ull);
}
#else
#define CVMX_HNA_PFC3_CTL (CVMX_ADD_IO_SEG(0x00011800470000B8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC_GCTL CVMX_HNA_PFC_GCTL_FUNC()
static inline uint64_t CVMX_HNA_PFC_GCTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_PFC_GCTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000080ull);
}
#else
#define CVMX_HNA_PFC_GCTL (CVMX_ADD_IO_SEG(0x0001180047000080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_SBD_DBG0 CVMX_HNA_SBD_DBG0_FUNC()
static inline uint64_t CVMX_HNA_SBD_DBG0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_SBD_DBG0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000040ull);
}
#else
#define CVMX_HNA_SBD_DBG0 (CVMX_ADD_IO_SEG(0x0001180047000040ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_SBD_DBG1 CVMX_HNA_SBD_DBG1_FUNC()
static inline uint64_t CVMX_HNA_SBD_DBG1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_SBD_DBG1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000048ull);
}
#else
#define CVMX_HNA_SBD_DBG1 (CVMX_ADD_IO_SEG(0x0001180047000048ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_SBD_DBG2 CVMX_HNA_SBD_DBG2_FUNC()
static inline uint64_t CVMX_HNA_SBD_DBG2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_SBD_DBG2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000050ull);
}
#else
#define CVMX_HNA_SBD_DBG2 (CVMX_ADD_IO_SEG(0x0001180047000050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_SBD_DBG3 CVMX_HNA_SBD_DBG3_FUNC()
static inline uint64_t CVMX_HNA_SBD_DBG3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_HNA_SBD_DBG3 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000058ull);
}
#else
#define CVMX_HNA_SBD_DBG3 (CVMX_ADD_IO_SEG(0x0001180047000058ull))
#endif

/**
 * cvmx_hna_bist0
 *
 * This register shows the result of the BIST run on the HNA (per-HPU).
 * 1 = BIST error, 0 = BIST passed, is in progress, or never ran.
 */
union cvmx_hna_bist0 {
	uint64_t u64;
	struct cvmx_hna_bist0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_58_63               : 6;
	uint64_t hpc3                         : 10; /**< BIST results for HPC3 RAM(s) (per-HPU). */
	uint64_t reserved_42_47               : 6;
	uint64_t hpc2                         : 10; /**< BIST results for HPC2 RAM(s) (per-HPU). */
	uint64_t reserved_26_31               : 6;
	uint64_t hpc1                         : 10; /**< BIST results for HPC1 RAM(s) (per-HPU). */
	uint64_t reserved_10_15               : 6;
	uint64_t hpc0                         : 10; /**< BIST results for HPC0 RAM(s) (per-HPU). */
#else
	uint64_t hpc0                         : 10;
	uint64_t reserved_10_15               : 6;
	uint64_t hpc1                         : 10;
	uint64_t reserved_26_31               : 6;
	uint64_t hpc2                         : 10;
	uint64_t reserved_42_47               : 6;
	uint64_t hpc3                         : 10;
	uint64_t reserved_58_63               : 6;
#endif
	} s;
	struct cvmx_hna_bist0_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t hpc1                         : 10; /**< BIST results for HPC1 RAM(s) (per-HPU). */
	uint64_t reserved_10_15               : 6;
	uint64_t hpc0                         : 10; /**< BIST results for HPC0 RAM(s) (per-HPU). */
#else
	uint64_t hpc0                         : 10;
	uint64_t reserved_10_15               : 6;
	uint64_t hpc1                         : 10;
	uint64_t reserved_26_63               : 38;
#endif
	} cn73xx;
	struct cvmx_hna_bist0_s               cn78xx;
	struct cvmx_hna_bist0_s               cn78xxp1;
};
typedef union cvmx_hna_bist0 cvmx_hna_bist0_t;

/**
 * cvmx_hna_bist1
 *
 * This register shows the result of the BIST run on the HNA (globals).
 * 1 = BIST error, 0 = BIST passed, is in progress, or never ran.
 */
union cvmx_hna_bist1 {
	uint64_t u64;
	struct cvmx_hna_bist1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t hnc1                         : 1;  /**< SC1 BIST results for cumulative HNC1 RAMs. */
	uint64_t hnc0                         : 1;  /**< SC0 BIST results for cumulative HNC0 RAMs. */
	uint64_t mrp1                         : 1;  /**< BIST results for DSM-DLC:MRP1 RAM. */
	uint64_t mrp0                         : 1;  /**< BIST results for DSM-DLC:MRP0 RAM. */
	uint64_t reserved_1_2                 : 2;
	uint64_t gib                          : 1;  /**< BIST results for GIB RAM. */
#else
	uint64_t gib                          : 1;
	uint64_t reserved_1_2                 : 2;
	uint64_t mrp0                         : 1;
	uint64_t mrp1                         : 1;
	uint64_t hnc0                         : 1;
	uint64_t hnc1                         : 1;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_hna_bist1_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t hnc0                         : 1;  /**< SC0 BIST results for cumulative HNC0 RAMs. */
	uint64_t mrp1                         : 1;  /**< BIST results for DSM-DLC:MRP1 RAM. */
	uint64_t mrp0                         : 1;  /**< BIST results for DSM-DLC:MRP0 RAM. */
	uint64_t reserved_1_2                 : 2;
	uint64_t gib                          : 1;  /**< BIST results for GIB RAM. */
#else
	uint64_t gib                          : 1;
	uint64_t reserved_1_2                 : 2;
	uint64_t mrp0                         : 1;
	uint64_t mrp1                         : 1;
	uint64_t hnc0                         : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} cn73xx;
	struct cvmx_hna_bist1_s               cn78xx;
	struct cvmx_hna_bist1_s               cn78xxp1;
};
typedef union cvmx_hna_bist1 cvmx_hna_bist1_t;

/**
 * cvmx_hna_config
 *
 * This register specifies the HNA HPU programmable controls.
 *
 */
union cvmx_hna_config {
	uint64_t u64;
	struct cvmx_hna_config_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t stk_ll_dis                   : 1;  /**< Stack linked-list disable. When set, the linked-list mechanism for run stack and save
                                                         stack structures is disabled. In this mode, the linked-list chunk boundary checking is not
                                                         done, and therefore the previous/next pointers are non-existent. The stacks are
                                                         effectively in an infinite linear buffer, bounded only by the maximum sizes provided in
                                                         the instruction (IWORD3[RUNSTACKSZ] and IWORD6[SVSTACKSZ]). There is no space reserved for
                                                         the previous and next pointers, and [STK_CHKSZ] is ignored.
                                                         When the STK_LL_DIS is cleared, the stack linked-list mechanism operates as per spec. */
	uint64_t reserved_23_23               : 1;
	uint64_t stk_chksz                    : 3;  /**< Stack chunk size. This encoded value specifies the chunk size for both the RNSTK/SVSTK
                                                         data structures. The RNSTK/SVSTK use a doubly linked list where each chunk's first two
                                                         64-bit entries contain the previous and next chunk pointers.
                                                         0x0 = 32 entries or 256 bytes.
                                                         0x1 = 64 entries or 512 bytes.
                                                         0x2 = 128 entries or 1K bytes.
                                                         0x3 = 256 entries or 2K bytes.
                                                         0x4 = 512 entries or 4K bytes.
                                                         0x5 = 1024 entries or 8K bytes.
                                                         0x6 = 2048 entries or 16K bytes.
                                                         0x7 = 4096 entries or 32K bytes.
                                                         NOTE: This field can only be changed at initialization/power on time before the HNA is fed
                                                         instructions. */
	uint64_t rnstk_lwm                    : 4;  /**< RNSTK low watermark. This field specifies the low watermark for the run stack. Valid
                                                         range: 0-15.
                                                         Once the run stack goes below the low watermark, HNA fills entries from the global run
                                                         stack head to the local run stack tail. The granularity of this field is represented as
                                                         number of 128B cachelines.
                                                         NOTE: This field can only be changed at initialization/power on time before the HNA is fed
                                                         instructions. */
	uint64_t rnstk_hwm                    : 4;  /**< RNSTK high watermark. This field specifies the high watermark for the run stack. Valid
                                                         range: 0-15.
                                                         Once the local run stack level goes above the high watermark, the HNA spills entries from
                                                         the local run stack tail to the global run stack head (in DDR memory). The granularity of
                                                         this field is represented as number of 128B cachelines.
                                                         NOTE: This field can only be changed at initialization/power on time before the HNA is fed
                                                         instructions. */
	uint64_t reserved_9_11                : 3;
	uint64_t ecccordis                    : 1;  /**< ECC correction disable. When set, all HNA ECC protected data structures disable their ECC
                                                         correction logic. When clear (default) ECC correction is always enabled. */
	uint64_t clmskcrip                    : 4;  /**< Cluster cripple mask. A one in each bit of the mask represents which HPC cluster to
                                                         cripple.
                                                         MIO_FUS_DAT3[HNA_INFO_CLM] fuse bits are forced into this
                                                         register at reset. Any fuse bits that contain 1 are disallowed during a write and are
                                                         always read as 1. */
	uint64_t hpu_clcrip                   : 3;  /**< HPU cluster cripple. Encoding which represents number of HPUs to cripple for each
                                                         cluster. Typically HPU_CLCRIP=0x0, which enables all HPUs within each cluster. However,
                                                         when the HNA performance counters are used, software may want to limit the number of HPUs
                                                         per cluster available, as there are only 4 parallel performance counters.
                                                         0x0 = HPU[9:0]:ON, All engines enabled.
                                                         0x1 = HPU[9]:OFF /HPU[8:0]:ON, (n-1) engines enabled.
                                                         0x2 = HPU[9:8]:OFF /HPU[7:0]:ON, (n-2) engines enabled.
                                                         0x2 = HPU[9:7]:OFF /HPU[6:0]:ON, (n-3) engines enabled.
                                                         0x3 = HPU[9:6]:OFF /HPU[5:0]:ON, (n-4) engines enabled.
                                                         0x4 = HPU[9:5]:OFF /HPU[4:0]:ON, (n-5) engines enabled.
                                                         0x5 = HPU[9:4]:OFF /HPU[3:0]:ON, (n-6) engines enabled.
                                                         0x6 = HPU[9:2]:OFF /HPU[1:0]:ON, (n-8) engines enabled.
                                                         0x7 = HPU[9:1]:OFF /HPU[0]:ON, (n-9) 1 engine enabled.
                                                         NOTE: Higher numbered HPUs are crippled first. For instance, on CN78XX (with 10
                                                         HPUs/cluster), if HPU_CLCRIP=0x1, then HPU numbers [9] within the cluster are
                                                         crippled and only HPU numbers 0-8 are available.
                                                         Software NOTE: The MIO_FUS___HNA_NUMHPU_CRIPPLE[2:0] fuse bits are forced into this
                                                         register at reset. Any fuse bits that contain 1 are disallowed during a write and are
                                                         always read as 1. */
	uint64_t hpuclkdis                    : 1;  /**< HNA clock disable source. When set, the HNA clocks for HPU (thread engine) operation are
                                                         disabled (to conserve overall chip clocking power when the HNA function is not used).
                                                         NOTE: When set, software must never issue NCB-direct CSR operations to the HNA (will
                                                         result in NCB bus timeout errors).
                                                         NOTE: This should only be written to a different value during power-on software
                                                         initialization.
                                                         Software NOTE: The MIO_FUS___HNA_HPU_DISABLE fuse bit is forced into this register at
                                                         reset. If the fuse bit contains 1, writes to HPUCLKDIS are disallowed and are always read
                                                         as 1. */
#else
	uint64_t hpuclkdis                    : 1;
	uint64_t hpu_clcrip                   : 3;
	uint64_t clmskcrip                    : 4;
	uint64_t ecccordis                    : 1;
	uint64_t reserved_9_11                : 3;
	uint64_t rnstk_hwm                    : 4;
	uint64_t rnstk_lwm                    : 4;
	uint64_t stk_chksz                    : 3;
	uint64_t reserved_23_23               : 1;
	uint64_t stk_ll_dis                   : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_hna_config_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t stk_ll_dis                   : 1;  /**< Stack linked-list disable. When set, the linked-list mechanism for run stack and save
                                                         stack structures is disabled. In this mode, the linked-list chunk boundary checking is not
                                                         done, and therefore the previous/next pointers are non-existent. The stacks are
                                                         effectively in an infinite linear buffer, bounded only by the maximum sizes provided in
                                                         the instruction (IWORD3[RUNSTACKSZ] and IWORD6[SVSTACKSZ]). There is no space reserved for
                                                         the previous and next pointers, and [STK_CHKSZ] is ignored.
                                                         When the STK_LL_DIS is cleared, the stack linked-list mechanism operates as per spec. */
	uint64_t reserved_23_23               : 1;
	uint64_t stk_chksz                    : 3;  /**< Stack chunk size. This encoded value specifies the chunk size for both the RNSTK/SVSTK
                                                         data structures. The RNSTK/SVSTK use a doubly linked list where each chunk's first two
                                                         64-bit entries contain the previous and next chunk pointers.
                                                         0x0 = 32 entries or 256 bytes.
                                                         0x1 = 64 entries or 512 bytes.
                                                         0x2 = 128 entries or 1K bytes.
                                                         0x3 = 256 entries or 2K bytes.
                                                         0x4 = 512 entries or 4K bytes.
                                                         0x5 = 1024 entries or 8K bytes.
                                                         0x6 = 2048 entries or 16K bytes.
                                                         0x7 = 4096 entries or 32K bytes.
                                                         NOTE: This field can only be changed at initialization/power on time before the HNA is fed
                                                         instructions. */
	uint64_t rnstk_lwm                    : 4;  /**< RNSTK low watermark. This field specifies the low watermark for the run stack. Valid
                                                         range: 0-15.
                                                         Once the run stack goes below the low watermark, HNA fills entries from the global run
                                                         stack head to the local run stack tail. The granularity of this field is represented as
                                                         number of 128B cachelines.
                                                         NOTE: This field can only be changed at initialization/power on time before the HNA is fed
                                                         instructions. */
	uint64_t rnstk_hwm                    : 4;  /**< RNSTK high watermark. This field specifies the high watermark for the run stack. Valid
                                                         range: 0-15.
                                                         Once the local run stack level goes above the high watermark, the HNA spills entries from
                                                         the local run stack tail to the global run stack head (in DDR memory). The granularity of
                                                         this field is represented as number of 128B cachelines.
                                                         NOTE: This field can only be changed at initialization/power on time before the HNA is fed
                                                         instructions. */
	uint64_t reserved_9_11                : 3;
	uint64_t ecccordis                    : 1;  /**< ECC correction disable. When set, all HNA ECC protected data structures disable their ECC
                                                         correction logic. When clear (default) ECC correction is always enabled. */
	uint64_t reserved_6_7                 : 2;
	uint64_t clmskcrip                    : 2;  /**< Cluster cripple mask. A one in each bit of the mask represents which HPC cluster to
                                                         cripple. CN73XX HNA has 2 clusters, where all CLMSKCRIP mask bits are used.
                                                         MIO_FUS_DAT3[HNA_INFO_CLM] fuse bits are forced into this
                                                         register at reset. Any fuse bits that contain 1 are disallowed during a write and are
                                                         always read as 1. */
	uint64_t hpu_clcrip                   : 3;  /**< HPU cluster cripple. Encoding which represents number of HPUs to cripple for each
                                                         cluster. Typically HPU_CLCRIP=0x0, which enables all HPUs within each cluster. However,
                                                         when the HNA performance counters are used, software may want to limit the number of HPUs
                                                         per cluster available, as there are only 4 parallel performance counters.
                                                         0x0 = HPU[9:0]:ON, All engines enabled.
                                                         0x1 = HPU[9]:OFF /HPU[8:0]:ON, (n-1) engines enabled.
                                                         0x2 = HPU[9:8]:OFF /HPU[7:0]:ON, (n-2) engines enabled.
                                                         0x2 = HPU[9:7]:OFF /HPU[6:0]:ON, (n-3) engines enabled.
                                                         0x3 = HPU[9:6]:OFF /HPU[5:0]:ON, (n-4) engines enabled.
                                                         0x4 = HPU[9:5]:OFF /HPU[4:0]:ON, (n-5) engines enabled.
                                                         0x5 = HPU[9:4]:OFF /HPU[3:0]:ON, (n-6) engines enabled.
                                                         0x6 = HPU[9:2]:OFF /HPU[1:0]:ON, (n-8) engines enabled.
                                                         0x7 = HPU[9:1]:OFF /HPU[0]:ON, (n-9) 1 engine enabled.
                                                         NOTE: Higher numbered HPUs are crippled first. For instance, on CN78XX (with 10
                                                         HPUs/cluster), if HPU_CLCRIP=0x1, then HPU numbers [9] within the cluster are
                                                         crippled and only HPU numbers 0-8 are available.
                                                         Software NOTE: The MIO_FUS___HNA_NUMHPU_CRIPPLE[2:0] fuse bits are forced into this
                                                         register at reset. Any fuse bits that contain 1 are disallowed during a write and are
                                                         always read as 1. */
	uint64_t hpuclkdis                    : 1;  /**< HNA clock disable source. When set, the HNA clocks for HPU (thread engine) operation are
                                                         disabled (to conserve overall chip clocking power when the HNA function is not used).
                                                         NOTE: When set, software must never issue NCB-direct CSR operations to the HNA (will
                                                         result in NCB bus timeout errors).
                                                         NOTE: This should only be written to a different value during power-on software
                                                         initialization.
                                                         Software NOTE: The MIO_FUS___HNA_HPU_DISABLE fuse bit is forced into this register at
                                                         reset. If the fuse bit contains 1, writes to HPUCLKDIS are disallowed and are always read
                                                         as 1. */
#else
	uint64_t hpuclkdis                    : 1;
	uint64_t hpu_clcrip                   : 3;
	uint64_t clmskcrip                    : 2;
	uint64_t reserved_6_7                 : 2;
	uint64_t ecccordis                    : 1;
	uint64_t reserved_9_11                : 3;
	uint64_t rnstk_hwm                    : 4;
	uint64_t rnstk_lwm                    : 4;
	uint64_t stk_chksz                    : 3;
	uint64_t reserved_23_23               : 1;
	uint64_t stk_ll_dis                   : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} cn73xx;
	struct cvmx_hna_config_s              cn78xx;
	struct cvmx_hna_config_s              cn78xxp1;
};
typedef union cvmx_hna_config cvmx_hna_config_t;

/**
 * cvmx_hna_control
 *
 * This register specifies the HNA CTL/HNC programmable controls.
 *
 */
union cvmx_hna_control {
	uint64_t u64;
	struct cvmx_hna_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t frcperr                      : 1;  /**< Force parity error during an OCM load. When set, a parity error is forced during the OCM
                                                         load instruction. Software can force a single line to contain a parity error by setting
                                                         this bit and performance a OCM load for a single line (DLEN=32), then clearing the bit. */
	uint64_t sbdnum                       : 6;  /**< Reserved. SBD debug entry number. */
	uint64_t sbdlck                       : 1;  /**< Reserved. HNA scoreboard lock strobe. */
	uint64_t reserved_3_4                 : 2;
	uint64_t pmode                        : 1;  /**< Reserved. NCB-NRP arbiter mode. (0=fixed priority [LP=DFF,HP=RGF]/1=RR
                                                         NOTE: This should only be written to a different value during power-on software
                                                         initialization. */
	uint64_t qmode                        : 1;  /**< Reserved. NCB-NRQ arbiter mode. 0 = Fixed priority (LP = NPF, IRF, WRF, PRF, RSRF, HP =
                                                         SLL); 1 = Round robin.
                                                         NOTE: This should only be written to a different value during power-on software
                                                         initialization. */
	uint64_t imode                        : 1;  /**< Reserved. NCB-inbound arbiter. 0 = Fixed priority (LP = NRQ, HP = NRP); 1 = Round robin).
                                                         NOTE: This should only be written to a different value during power-on software
                                                         initialization. */
#else
	uint64_t imode                        : 1;
	uint64_t qmode                        : 1;
	uint64_t pmode                        : 1;
	uint64_t reserved_3_4                 : 2;
	uint64_t sbdlck                       : 1;
	uint64_t sbdnum                       : 6;
	uint64_t frcperr                      : 1;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_hna_control_s             cn73xx;
	struct cvmx_hna_control_s             cn78xx;
	struct cvmx_hna_control_s             cn78xxp1;
};
typedef union cvmx_hna_control cvmx_hna_control_t;

/**
 * cvmx_hna_dbell
 *
 * To write to the HNA_DBELL register, a device issues an IOBST directed at the HNA with
 * addr[34:32] = 0x0 or 0x1. To read the HNA_DBELL register, a device issues an IOBLD64 directed
 * at the HNA with addr[34:32] = 0x0 or 0x1.
 *
 * If HNA_CONFIG[HPUCLKDIS]=1 (HNA-HPU clocks disabled), reads/writes to the HNA_DBELL register
 * do not take effect. If the HNA-disable fuse is blown, reads/writes to the HNA_DBELL register
 * do not take effect.
 */
union cvmx_hna_dbell {
	uint64_t u64;
	struct cvmx_hna_dbell_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t dbell                        : 20; /**< Represents the cumulative total of pending HNA instructions that software has previously
                                                         written into the HNA instruction FIFO (DIF) in main memory. Each HNA instruction contains
                                                         a fixed size 64B instruction word which is executed by the HNA hardware. The DBELL field
                                                         can hold up to 1M-1 (2^20-1) pending HNA instruction requests. During a software read, the
                                                         most recent contents of HNA_DBELL are returned at the time the NCB-INB bus is driven.
                                                         NOTE: Since HNA hardware updates this register, its contents are unpredictable in
                                                         software. */
#else
	uint64_t dbell                        : 20;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_hna_dbell_s               cn73xx;
	struct cvmx_hna_dbell_s               cn78xx;
	struct cvmx_hna_dbell_s               cn78xxp1;
};
typedef union cvmx_hna_dbell cvmx_hna_dbell_t;

/**
 * cvmx_hna_difctl
 *
 * To write to the HNA_DIFCTL register, a device issues an IOBST directed at the HNA with
 * addr[34:32]=0x6. To read the HNA_DIFCTL register, a device issues an IOBLD64 directed at the
 * HNA with addr[34:32]=0x6.
 *
 * This register is intended to only be written once (at power-up). Any future writes could cause
 * the HNA and FPA hardware to become unpredictable. If HNA_CONFIG[HPUCLKDIS]=1 (HNA-HPU clocks
 * disabled), reads/writes to HNA_DIFCTL do not take effect. If the HNA-disable FUSE is blown,
 * reads/writes to HNA_DIFCTL do not take effect.
 */
union cvmx_hna_difctl {
	uint64_t u64;
	struct cvmx_hna_difctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t aura                         : 16; /**< Represents the 16-bit Aura ID used by HNA hardware when the HNA instruction chunk is
                                                         recycled back to the Free Page List maintained by FPA (once the HNA instruction has been
                                                         issued). */
	uint64_t reserved_13_25               : 13;
	uint64_t ldwb                         : 1;  /**< Load don't write back. When set, the hardware issues LDWB command towards the cache when
                                                         fetching last word of instructions; as a result the line is not written back when
                                                         replaced. When clear, the hardware issues regular load towards cache which will cause the
                                                         line to be written back before being replaced. */
	uint64_t reserved_9_11                : 3;
	uint64_t size                         : 9;  /**< Represents the number of 64B instructions contained within each HNA instruction chunk. At
                                                         power-on, software seeds the SIZE register with a fixed chunk size (must be at least 3).
                                                         HNA hardware uses this field to determine the size of each HNA instruction chunk, in order
                                                         to:
                                                         a) determine when to read the next HNA instruction chunk pointer which is written by
                                                         software at the end of the current HNA instruction chunk (see HNA description of next
                                                         chunk buffer Ptr for format).
                                                         b) determine when a HNA instruction chunk can be returned to the free page list maintained
                                                         by FPA. */
#else
	uint64_t size                         : 9;
	uint64_t reserved_9_11                : 3;
	uint64_t ldwb                         : 1;
	uint64_t reserved_13_25               : 13;
	uint64_t aura                         : 16;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_hna_difctl_s              cn73xx;
	struct cvmx_hna_difctl_s              cn78xx;
	struct cvmx_hna_difctl_s              cn78xxp1;
};
typedef union cvmx_hna_difctl cvmx_hna_difctl_t;

/**
 * cvmx_hna_difrdptr
 *
 * To write to the HNA_DIFRDPTR register, a device issues an IOBST directed at the HNA with
 * addr[34:32] = 0x2 or 0x3. To read the HNA_DIFRDPTR register, a device issues an IOBLD64
 * directed at the HNA with addr[34:32] = 0x2 or 0x3.
 *
 * If HNA_CONFIG[HPUCLKDIS]=1 (HNA-HPU clocks disabled), reads/writes to HNA_DIFRDPTR do not take
 * effect. If the HNA-disable fuse is blown, reads/writes to HNA_DIFRDPTR do not take effect.
 */
union cvmx_hna_difrdptr {
	uint64_t u64;
	struct cvmx_hna_difrdptr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t rdptr                        : 36; /**< Represents the 64B-aligned address of the current instruction in the HNA Instruction FIFO
                                                         in main memory. The RDPTR must be seeded by software at boot time, and is then maintained
                                                         thereafter by HNA hardware. During the software seed write, [RDPTR][6]=0, since HNA
                                                         instruction chunks must be 128B aligned. During a software read, the most recent contents
                                                         of [RDPTR] are returned at the time the NCB-INB bus is driven.
                                                         NOTE: Since HNA hardware updates this register, its contents are unpredictable in software
                                                         (unless it is guaranteed that no new doorbell register writes have occurred and HNA_DBELL
                                                         is read as zero). */
	uint64_t reserved_0_5                 : 6;
#else
	uint64_t reserved_0_5                 : 6;
	uint64_t rdptr                        : 36;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_hna_difrdptr_s            cn73xx;
	struct cvmx_hna_difrdptr_s            cn78xx;
	struct cvmx_hna_difrdptr_s            cn78xxp1;
};
typedef union cvmx_hna_difrdptr cvmx_hna_difrdptr_t;

/**
 * cvmx_hna_eco
 */
union cvmx_hna_eco {
	uint64_t u64;
	struct cvmx_hna_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t eco_rw                       : 32; /**< Reserved. */
#else
	uint64_t eco_rw                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_hna_eco_s                 cn73xx;
	struct cvmx_hna_eco_s                 cn78xx;
};
typedef union cvmx_hna_eco cvmx_hna_eco_t;

/**
 * cvmx_hna_error
 *
 * This register contains error status information.
 *
 */
union cvmx_hna_error {
	uint64_t u64;
	struct cvmx_hna_error_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_21_63               : 43;
	uint64_t hnc_parerr                   : 1;  /**< HNC reported parity error with the response data. */
	uint64_t osmerr                       : 1;  /**< OSM reported an error with the response data. */
	uint64_t replerr                      : 1;  /**< HNA illegal replication factor error. HNA only supports 1*, 2*, and 4* port replication.
                                                         Legal configurations for memory are 2-port or 4-port configurations. The REPLERR interrupt
                                                         is set in the following illegal configuration cases:
                                                         1) An 8* replication factor is detected for any memory reference.
                                                         2) A 4* replication factor is detected for any memory reference when only 2 memory ports
                                                         are enabled.
                                                         NOTE: If [REPLERR] is set during a HNA graph walk operation, the walk will prematurely
                                                         terminate with RWORD0[REA] = ERR. */
	uint64_t hnanxm                       : 1;  /**< HNA non-existent memory access. HPUs (and backdoor CSR HNA memory region reads) have
                                                         access to the following 40-bit L2/DRAM address space which maps to a 38-bit physical
                                                         DDR3/4 SDRAM address space (256 GB max).
                                                         DR0: 0x0 0000 0000 0000 to 0x0 0000 0FFF FFFF
                                                         maps to lower 256MB of physical DDR3/4 SDRAM.
                                                         DR1: 0x0 0000 2000 0000 to 0x0 0020 0FFF FFFF
                                                         maps to upper 127.75GB of DDR3/4 SDRAM.
                                                         NOTE: the 2nd 256MB hole maps to I/O and is unused (nonexistent) for memory.
                                                         In the event the HNA generates a reference to the L2/DRAM address hole (0x0000.0FFF.FFFF -
                                                         0x0000.1FFF.FFFF), the HNANXM programmable interrupt bit will be set.
                                                         Software Note:
                                                         The HNA graph compiler must avoid making references to this 2nd 256MB HOLE which is a non-
                                                         existent memory region.
                                                         NOTE: If [HNANXM] is set during a HNA graph walk operation, then the walk will prematurely
                                                         terminate with RWORD0[REAS]=MEMERR. */
	uint64_t dlc1_dbe                     : 1;  /**< A DLC1 response indicated a double-bit ECC error (DBE). */
	uint64_t dlc0_dbe                     : 1;  /**< A DLC0 response indicated a double-bit ECC error (DBE). */
	uint64_t dlc1_ovferr                  : 1;  /**< DLC1 FIFO overflow error detected. This condition should never architecturally occur, and
                                                         is here in case hardware credit/debit scheme is not working. */
	uint64_t dlc0_ovferr                  : 1;  /**< DLC0 FIFO overflow error detected. This condition should never architecturally occur, and
                                                         is here in case hardware credit/debit scheme is not working. */
	uint64_t hnc_ovferr                   : 1;  /**< HNC (RAM1) address overflow error detected.
                                                         This condition is signaled by the HPU when an node access is made to RAM1 outside the size
                                                         of the RAM. */
	uint64_t hna_inst_err                 : 1;  /**< Instruction field ITYPE is not GWALK nor OSM Load. */
	uint64_t reserved_6_10                : 5;
	uint64_t hpu_pdb_par_err              : 1;  /**< A parity error was detected in a packet data buffer in one of the HNA engines. This is not
                                                         self-correcting, and the HPU behavior is undefined. */
	uint64_t hpu_rnstck_dbe               : 1;  /**< A double-bit ECC (DBE) error was detected in the run stack of an HPU. This is not self-
                                                         correcting, and the HPU behavior is undefined. */
	uint64_t hpu_rnstck_sbe               : 1;  /**< A single-bit ECC (SBE) error was detected in the run stack of an HPU. This is self-
                                                         correcting, and the HPU should continue to operate normally. */
	uint64_t hpu_svstck_dbe               : 1;  /**< A double-bit ECC (DBE) error was detected in the save stack / result buffer of an HPU.
                                                         This is not self-correcting, and the HPU behavior is undefined. */
	uint64_t hpu_svstck_sbe               : 1;  /**< A single-bit ECC (SBE) error was detected in the save stack / result buffer of an HPU.
                                                         This is self-correcting, and the HPU should continue to operate normally. */
	uint64_t dblovf                       : 1;  /**< Doorbell overflow detected - status bit. When set, the 20-bit accumulated doorbell
                                                         register had overflowed (software wrote too many doorbell requests).
                                                         NOTE: Detection of a doorbell register overflow is a catastrophic error which may leave
                                                         the HNA hardware in an unrecoverable state. */
#else
	uint64_t dblovf                       : 1;
	uint64_t hpu_svstck_sbe               : 1;
	uint64_t hpu_svstck_dbe               : 1;
	uint64_t hpu_rnstck_sbe               : 1;
	uint64_t hpu_rnstck_dbe               : 1;
	uint64_t hpu_pdb_par_err              : 1;
	uint64_t reserved_6_10                : 5;
	uint64_t hna_inst_err                 : 1;
	uint64_t hnc_ovferr                   : 1;
	uint64_t dlc0_ovferr                  : 1;
	uint64_t dlc1_ovferr                  : 1;
	uint64_t dlc0_dbe                     : 1;
	uint64_t dlc1_dbe                     : 1;
	uint64_t hnanxm                       : 1;
	uint64_t replerr                      : 1;
	uint64_t osmerr                       : 1;
	uint64_t hnc_parerr                   : 1;
	uint64_t reserved_21_63               : 43;
#endif
	} s;
	struct cvmx_hna_error_s               cn73xx;
	struct cvmx_hna_error_s               cn78xx;
	struct cvmx_hna_error_s               cn78xxp1;
};
typedef union cvmx_hna_error cvmx_hna_error_t;

/**
 * cvmx_hna_error_capture_data
 *
 * This register holds the HPU_STATUS data captured during some HNA_ERROR events.
 *
 */
union cvmx_hna_error_capture_data {
	uint64_t u64;
	struct cvmx_hna_error_capture_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t hpu_stat                     : 64; /**< HPU_STATUS data captured during some HNA_ERROR events. It will preserve the HPU_STATUS
                                                         from the
                                                         first such event that is detected, until the VLD bit in the HNA_ERROR_CAPTURE_INFO
                                                         register is reset by clearing the corresponding interrupt. If multiple error events occur
                                                         before the VLD bit is cleared, the
                                                         subsequent HPU_STATUS information will be lost. The error capture is triggered by the
                                                         following error conditions:
                                                         * HPU memory errors (SBE or DBE on svstk/rnstk/rwb, or parity error on PDB).
                                                         * HNC parity error.
                                                         * RAM1 address overflow.
                                                         The HPU STATUS format is as follows:
                                                         <pre>
                                                         [63:60]   0x0
                                                         [60:40]   failing address              Last fetched node
                                                         [39:36]   0x0
                                                         [35:28]   svstck_synd
                                                         [27]      svstck_dbe                   Interrupt
                                                         [26]      svstck_sbe                   Interrupt
                                                         [25:18]   rnstck_synd
                                                         [17]      rnstck_dbe                   Interrupt
                                                         [16]      rnstck_sbe                   Interrupt
                                                         [15:11]   Current opcode
                                                         [10]      1'b0
                                                         [9]       OSM or DLC Response Error    REASON MEMERR
                                                         [8]       Bad node                     REASON BADNODE
                                                         [7]       Node in RAM2 / bad entry ID  REASON BADADR
                                                         [6]       Bad stack entry              REASON BAD STACKENTRY
                                                         [5]       Svstck full                  REASON SVSTACK FULL
                                                         [4]       Rwb full                     REASON RWB FULL
                                                         [3]       Rnstck full                  REASON RUN STACK FULL
                                                         [2]       Packet data parity error     Interrupt
                                                         [1]       RAM1 or RAM2 parity error    REASON MEMERR
                                                         [0]       Ram1 overflow error          REASON MEMERR
                                                         </pre> */
#else
	uint64_t hpu_stat                     : 64;
#endif
	} s;
	struct cvmx_hna_error_capture_data_s  cn73xx;
	struct cvmx_hna_error_capture_data_s  cn78xx;
	struct cvmx_hna_error_capture_data_s  cn78xxp1;
};
typedef union cvmx_hna_error_capture_data cvmx_hna_error_capture_data_t;

/**
 * cvmx_hna_error_capture_info
 *
 * This register holds the meta-data for the HPU_STATUS captured during some HNA_ERROR events.
 * Upon the first occurrence of select HPU error events, the HPU status is captured in the
 * HNA_ERROR_CAPTURE_DATA register.  The cluster and engine ID are captured in this register,
 * along with a valid bit.  The valid bit is reset by clearing the corresponding interrupt.  Any
 * errors that occur while the valid bit is set will be lost.  The HPU errors that are captured
 * are memory errors (DBE, SBE, parity) in the HPU internal RAMs, HNC parity errors, and HNC
 * overflow errors.
 */
union cvmx_hna_error_capture_info {
	uint64_t u64;
	struct cvmx_hna_error_capture_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t cl_id                        : 2;  /**< Cluster which was the source of the HPU_STATUS. */
	uint64_t hpu_id                       : 4;  /**< HPU which was the source of the HPU_STATUS. */
	uint64_t reserved_2_3                 : 2;
	uint64_t ovf                          : 1;  /**< Reserved. */
	uint64_t vld                          : 1;  /**< The VLD bit indicates that an HNA_ERROR has occurred and the HPU_STATUS has been captured
                                                         in the HNA_ERROR_CAPTURE_DATA register. The first such HPU_STATUS will be stored until
                                                         this bit is reset by clearing one of these interrupts:
                                                         * HNA_ERROR_SVSTCK_SBE.
                                                         * HNA_ERROR_SVSTCK_DBE.
                                                         * HNA_ERROR_RNSTCK_SBE.
                                                         * HNA_ERROR_RNSTCK_DBE.
                                                         * HNA_ERROR_PDB_PAR_ERR.
                                                         * HNA_ERROR_HNC_OVFERR.
                                                         * HNA_ERROR_HNC_PARERR. */
#else
	uint64_t vld                          : 1;
	uint64_t ovf                          : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t hpu_id                       : 4;
	uint64_t cl_id                        : 2;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_hna_error_capture_info_s  cn73xx;
	struct cvmx_hna_error_capture_info_s  cn78xx;
	struct cvmx_hna_error_capture_info_s  cn78xxp1;
};
typedef union cvmx_hna_error_capture_info cvmx_hna_error_capture_info_t;

/**
 * cvmx_hna_hnc0_ram1#
 *
 * This address space allows read-only access to HNC0 RAM1 for diagnostic use. It is defined here
 * to contain 64 words (16 RAM lines) for basic DV testing, but the actual address space spans
 * the entire RAM1, 8192 lines.
 */
union cvmx_hna_hnc0_ram1x {
	uint64_t u64;
	struct cvmx_hna_hnc0_ram1x_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ram1_data                    : 64; /**< 64-bit chunk of RAM1. */
#else
	uint64_t ram1_data                    : 64;
#endif
	} s;
	struct cvmx_hna_hnc0_ram1x_s          cn73xx;
	struct cvmx_hna_hnc0_ram1x_s          cn78xx;
	struct cvmx_hna_hnc0_ram1x_s          cn78xxp1;
};
typedef union cvmx_hna_hnc0_ram1x cvmx_hna_hnc0_ram1x_t;

/**
 * cvmx_hna_hnc0_ram2#
 *
 * This address space allows read-only access to HNC0 RAM2 for diagnostic use. It is defined here
 * to contain 64 words (16 RAM lines) for basic DV testing, but the actual address space spans
 * the entire RAM2, 512 lines.
 */
union cvmx_hna_hnc0_ram2x {
	uint64_t u64;
	struct cvmx_hna_hnc0_ram2x_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ram2_data                    : 64; /**< 64-bit chunk of RAM2. */
#else
	uint64_t ram2_data                    : 64;
#endif
	} s;
	struct cvmx_hna_hnc0_ram2x_s          cn73xx;
	struct cvmx_hna_hnc0_ram2x_s          cn78xx;
	struct cvmx_hna_hnc0_ram2x_s          cn78xxp1;
};
typedef union cvmx_hna_hnc0_ram2x cvmx_hna_hnc0_ram2x_t;

/**
 * cvmx_hna_hnc1_ram1#
 *
 * This address space allows read-only access to HNC1 RAM1 for diagnostic use. It is defined here
 * to contain 64 words (16 RAM lines) for basic DV testing, but the actual address space spans
 * the entire RAM1, 8192 lines.
 */
union cvmx_hna_hnc1_ram1x {
	uint64_t u64;
	struct cvmx_hna_hnc1_ram1x_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ram1_data                    : 64; /**< 64-bit chunk of RAM1. */
#else
	uint64_t ram1_data                    : 64;
#endif
	} s;
	struct cvmx_hna_hnc1_ram1x_s          cn73xx;
	struct cvmx_hna_hnc1_ram1x_s          cn78xx;
	struct cvmx_hna_hnc1_ram1x_s          cn78xxp1;
};
typedef union cvmx_hna_hnc1_ram1x cvmx_hna_hnc1_ram1x_t;

/**
 * cvmx_hna_hnc1_ram2#
 *
 * This address space allows read-only access to HNC1 RAM2 for diagnostic use. It is defined here
 * to contain 64 words (16 RAM lines) for basic DV testing, but the actual address space spans
 * the entire RAM1, 512 lines.
 */
union cvmx_hna_hnc1_ram2x {
	uint64_t u64;
	struct cvmx_hna_hnc1_ram2x_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ram2_data                    : 64; /**< 64-bit chunk of RAM2. */
#else
	uint64_t ram2_data                    : 64;
#endif
	} s;
	struct cvmx_hna_hnc1_ram2x_s          cn73xx;
	struct cvmx_hna_hnc1_ram2x_s          cn78xx;
	struct cvmx_hna_hnc1_ram2x_s          cn78xxp1;
};
typedef union cvmx_hna_hnc1_ram2x cvmx_hna_hnc1_ram2x_t;

/**
 * cvmx_hna_hpu_csr
 */
union cvmx_hna_hpu_csr {
	uint64_t u64;
	struct cvmx_hna_hpu_csr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t csrdat                       : 64;
#else
	uint64_t csrdat                       : 64;
#endif
	} s;
	struct cvmx_hna_hpu_csr_s             cn73xx;
	struct cvmx_hna_hpu_csr_s             cn78xx;
	struct cvmx_hna_hpu_csr_s             cn78xxp1;
};
typedef union cvmx_hna_hpu_csr cvmx_hna_hpu_csr_t;

/**
 * cvmx_hna_hpu_dbg
 *
 * This register specifies the HPU CSR number, cluster number = CLID and HPU number = HPUID used
 * during a a CSR read of the HNA_HPU_CSR register.
 */
union cvmx_hna_hpu_dbg {
	uint64_t u64;
	struct cvmx_hna_hpu_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t csrnum                       : 2;  /**< HPU CSR number:
                                                         0x0 = HPU_STATUS.
                                                         0x1 = DBG_CURSTK.
                                                         0x2 = DBG_GENERAL. */
	uint64_t clid                         : 2;  /**< Cluster number. Valid range is 0-3. */
	uint64_t hpuid                        : 4;  /**< HPU engine ID. Valid range 0-11. */
#else
	uint64_t hpuid                        : 4;
	uint64_t clid                         : 2;
	uint64_t csrnum                       : 2;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_hna_hpu_dbg_s             cn73xx;
	struct cvmx_hna_hpu_dbg_s             cn78xx;
	struct cvmx_hna_hpu_dbg_s             cn78xxp1;
};
typedef union cvmx_hna_hpu_dbg cvmx_hna_hpu_dbg_t;

/**
 * cvmx_hna_hpu_eir
 *
 * Used by software to force parity or ECC errors on some internal HPU data structures. A CSR
 * write of this register forces either a parity or ECC error on the next access at cluster
 * number = [CLID], HPU number = [HPUID].
 */
union cvmx_hna_hpu_eir {
	uint64_t u64;
	struct cvmx_hna_hpu_eir_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t wrdone                       : 1;  /**< When HNA_HPU_EIR is written, this bit is cleared by hardware. When the targeted HPU has
                                                         received the error injection command (i.e. error injection armed), [WRDONE] is set.
                                                         Software first writes HNA_HPU_EIR, then does a polling read of [WRDONE] (until it
                                                         becomes 1), before issuing an HNA instruction to the targeted HPU which will inject the
                                                         intended error type for a single occurrence (one-shot). */
	uint64_t pdperr                       : 1;  /**< Packet data buffer parity error. Forces parity error on next packet data buffer read. */
	uint64_t svflipsyn                    : 2;  /**< Save stack flip syndrome control. Forces 1-bit/2-bit errors on next save stack read. */
	uint64_t rsflipsyn                    : 2;  /**< Run stack flip syndrome control. Forces 1-bit/2-bit errors on next run stack read. */
	uint64_t clid                         : 2;  /**< HPC cluster number. Valid range is 0-3. */
	uint64_t hpuid                        : 4;  /**< HPU engine ID. Valid range is 0-11. */
#else
	uint64_t hpuid                        : 4;
	uint64_t clid                         : 2;
	uint64_t rsflipsyn                    : 2;
	uint64_t svflipsyn                    : 2;
	uint64_t pdperr                       : 1;
	uint64_t wrdone                       : 1;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_hna_hpu_eir_s             cn73xx;
	struct cvmx_hna_hpu_eir_s             cn78xx;
	struct cvmx_hna_hpu_eir_s             cn78xxp1;
};
typedef union cvmx_hna_hpu_eir cvmx_hna_hpu_eir_t;

/**
 * cvmx_hna_pfc0_cnt
 */
union cvmx_hna_pfc0_cnt {
	uint64_t u64;
	struct cvmx_hna_pfc0_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pfcnt                        : 64; /**< HNA performance counter 0. When HNA_PFC_GCTL[CNT0ENA]=1, the event selected by
                                                         HNA_PFC0_CTL[EVSEL] is counted. See also HNA_PFC_GCTL[CNT0WCLR] and HNA_PFC_GCTL
                                                         [CNT0RCLR] for special clear count cases available for software data collection. */
#else
	uint64_t pfcnt                        : 64;
#endif
	} s;
	struct cvmx_hna_pfc0_cnt_s            cn73xx;
	struct cvmx_hna_pfc0_cnt_s            cn78xx;
	struct cvmx_hna_pfc0_cnt_s            cn78xxp1;
};
typedef union cvmx_hna_pfc0_cnt cvmx_hna_pfc0_cnt_t;

/**
 * cvmx_hna_pfc0_ctl
 */
union cvmx_hna_pfc0_ctl {
	uint64_t u64;
	struct cvmx_hna_pfc0_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t evsel                        : 6;  /**< Performance counter 0 event selector (64 total). Enumerated by HNA_PFC_SEL_E. */
	uint64_t reserved_6_7                 : 2;
	uint64_t clhpu                        : 4;  /**< "Performance counter 0 cluster HPU selector. When HNA_PFC_GCTL[PMODE]=0 (per-cluster HPU),
                                                         this field
                                                         is used to select/monitor the cluster's HPU# for all events
                                                         associated with performance counter 0." */
	uint64_t clnum                        : 2;  /**< "Performance counter 0 cluster selector. When HNA_PFC_GCTL[PMODE]=0 (per-cluster HPU),
                                                         this field is used to select/monitor the cluster# for all events associated with
                                                         performance counter 0." */
#else
	uint64_t clnum                        : 2;
	uint64_t clhpu                        : 4;
	uint64_t reserved_6_7                 : 2;
	uint64_t evsel                        : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_hna_pfc0_ctl_s            cn73xx;
	struct cvmx_hna_pfc0_ctl_s            cn78xx;
	struct cvmx_hna_pfc0_ctl_s            cn78xxp1;
};
typedef union cvmx_hna_pfc0_ctl cvmx_hna_pfc0_ctl_t;

/**
 * cvmx_hna_pfc1_cnt
 */
union cvmx_hna_pfc1_cnt {
	uint64_t u64;
	struct cvmx_hna_pfc1_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pfcnt                        : 64; /**< HNA performance counter 1. When HNA_PFC_GCTL[CNT1ENA]=1, the event selected by
                                                         HNA_PFC1_CTL[EVSEL] is counted. See also HNA_PFC_GCTL[CNT1WCLR] and HNA_PFC_GCTL
                                                         [CNT1RCLR] for special clear count cases available for software data collection. */
#else
	uint64_t pfcnt                        : 64;
#endif
	} s;
	struct cvmx_hna_pfc1_cnt_s            cn73xx;
	struct cvmx_hna_pfc1_cnt_s            cn78xx;
	struct cvmx_hna_pfc1_cnt_s            cn78xxp1;
};
typedef union cvmx_hna_pfc1_cnt cvmx_hna_pfc1_cnt_t;

/**
 * cvmx_hna_pfc1_ctl
 */
union cvmx_hna_pfc1_ctl {
	uint64_t u64;
	struct cvmx_hna_pfc1_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t evsel                        : 6;  /**< Performance counter 1 event selector (64 total). Enumerated by HNA_PFC_SEL_E. */
	uint64_t reserved_6_7                 : 2;
	uint64_t clhpu                        : 4;  /**< "Performance counter 1 cluster HPU selector. When HNA_PFC_GCTL[PMODE]=0 (per-cluster HPU),
                                                         this field is used to select/monitor the cluster's HPU# for all events associated with
                                                         performance counter 1." */
	uint64_t clnum                        : 2;  /**< "Performance counter 1 cluster selector. When HNA_PFC_GCTL[PMODE]=0 (per-cluster HPU),
                                                         this field is used to select/monitor the cluster# for all events associated with
                                                         performance counter 1." */
#else
	uint64_t clnum                        : 2;
	uint64_t clhpu                        : 4;
	uint64_t reserved_6_7                 : 2;
	uint64_t evsel                        : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_hna_pfc1_ctl_s            cn73xx;
	struct cvmx_hna_pfc1_ctl_s            cn78xx;
	struct cvmx_hna_pfc1_ctl_s            cn78xxp1;
};
typedef union cvmx_hna_pfc1_ctl cvmx_hna_pfc1_ctl_t;

/**
 * cvmx_hna_pfc2_cnt
 */
union cvmx_hna_pfc2_cnt {
	uint64_t u64;
	struct cvmx_hna_pfc2_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pfcnt                        : 64; /**< HNA performance counter 2. When HNA_PFC_GCTL[CNT2ENA]=1, the event selected by
                                                         HNA_PFC2_CTL[EVSEL] is counted. See also HNA_PFC_GCTL[CNT2WCLR] and HNA_PFC_GCTL
                                                         [CNT2RCLR] for special clear count cases available for software data collection. */
#else
	uint64_t pfcnt                        : 64;
#endif
	} s;
	struct cvmx_hna_pfc2_cnt_s            cn73xx;
	struct cvmx_hna_pfc2_cnt_s            cn78xx;
	struct cvmx_hna_pfc2_cnt_s            cn78xxp1;
};
typedef union cvmx_hna_pfc2_cnt cvmx_hna_pfc2_cnt_t;

/**
 * cvmx_hna_pfc2_ctl
 */
union cvmx_hna_pfc2_ctl {
	uint64_t u64;
	struct cvmx_hna_pfc2_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t evsel                        : 6;  /**< Performance counter 2 event selector (64 total). Enumerated by HNA_PFC_SEL_E. */
	uint64_t reserved_6_7                 : 2;
	uint64_t clhpu                        : 4;  /**< "Performance counter 2 cluster HPU Selector. When HNA_PFC_GCTL[PMODE]=0 (per-cluster
                                                         HPU), this field is used to select/monitor the cluster's HPU# for all events associated
                                                         with performance counter 2." */
	uint64_t clnum                        : 2;  /**< "Performance counter 2 cluster selector. When HNA_PFC_GCTL[PMODE]=0 (per-cluster HPU),
                                                         this field is used to select/monitor the cluster# for all events associated with
                                                         performance counter 2." */
#else
	uint64_t clnum                        : 2;
	uint64_t clhpu                        : 4;
	uint64_t reserved_6_7                 : 2;
	uint64_t evsel                        : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_hna_pfc2_ctl_s            cn73xx;
	struct cvmx_hna_pfc2_ctl_s            cn78xx;
	struct cvmx_hna_pfc2_ctl_s            cn78xxp1;
};
typedef union cvmx_hna_pfc2_ctl cvmx_hna_pfc2_ctl_t;

/**
 * cvmx_hna_pfc3_cnt
 */
union cvmx_hna_pfc3_cnt {
	uint64_t u64;
	struct cvmx_hna_pfc3_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pfcnt                        : 64; /**< HNA performance counter 3. When HNA_PFC_GCTL[CNT3ENA]=1, the event selected by
                                                         HNA_PFC3_CTL[EVSEL] is counted. See also HNA_PFC_GCTL[CNT3WCLR] and HNA_PFC_GCTL
                                                         [CNT3RCLR] for special clear count cases available for software data collection. */
#else
	uint64_t pfcnt                        : 64;
#endif
	} s;
	struct cvmx_hna_pfc3_cnt_s            cn73xx;
	struct cvmx_hna_pfc3_cnt_s            cn78xx;
	struct cvmx_hna_pfc3_cnt_s            cn78xxp1;
};
typedef union cvmx_hna_pfc3_cnt cvmx_hna_pfc3_cnt_t;

/**
 * cvmx_hna_pfc3_ctl
 */
union cvmx_hna_pfc3_ctl {
	uint64_t u64;
	struct cvmx_hna_pfc3_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t evsel                        : 6;  /**< Performance counter 3 event selector (64 total). Enumerated by HNA_PFC_SEL_E. */
	uint64_t reserved_6_7                 : 2;
	uint64_t clhpu                        : 4;  /**< "Performance counter 3 cluster HPU selector. When HNA_PFC_GCTL[PMODE]=0 (per-cluster HPU),
                                                         this field is used to select/monitor the cluster's HPU# for all events associated with
                                                         performance counter 3." */
	uint64_t clnum                        : 2;  /**< "Performance counter 3 cluster selector. When HNA_PFC_GCTL[PMODE]=0 (per-cluster HPU),
                                                         this field is used to select/monitor the cluster# for all events associated with
                                                         performance counter 3." */
#else
	uint64_t clnum                        : 2;
	uint64_t clhpu                        : 4;
	uint64_t reserved_6_7                 : 2;
	uint64_t evsel                        : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_hna_pfc3_ctl_s            cn73xx;
	struct cvmx_hna_pfc3_ctl_s            cn78xx;
	struct cvmx_hna_pfc3_ctl_s            cn78xxp1;
};
typedef union cvmx_hna_pfc3_ctl cvmx_hna_pfc3_ctl_t;

/**
 * cvmx_hna_pfc_gctl
 *
 * Global control across all performance counters.
 *
 */
union cvmx_hna_pfc_gctl {
	uint64_t u64;
	struct cvmx_hna_pfc_gctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t cnt3rclr                     : 1;  /**< Performance counter 3 read clear. If this bit is set, CSR reads to the HNA_PFC3_CNT will
                                                         clear the count value. This allows software to maintain 'cumulative' counters to avoid
                                                         hardware wraparound. */
	uint64_t cnt2rclr                     : 1;  /**< Performance counter 2 read clear. If this bit is set, CSR reads to the HNA_PFC2_CNT will
                                                         clear the count value. This allows software to maintain 'cumulative' counters to avoid
                                                         hardware wraparound. */
	uint64_t cnt1rclr                     : 1;  /**< Performance counter 1 read clear. If this bit is set, CSR reads to the HNA_PFC1_CNT will
                                                         clear the count value. This allows software to maintain 'cumulative' counters to avoid
                                                         hardware wraparound. */
	uint64_t cnt0rclr                     : 1;  /**< Performance counter 0 read clear. If this bit is set, CSR reads to the HNA_PFC0_CNT will
                                                         clear the count value. This allows software to maintain 'cumulative' counters to avoid
                                                         hardware wraparound. */
	uint64_t cnt3wclr                     : 1;  /**< Performance counter 3 write clear. If this bit is set, CSR writes to the HNA_PFC3_CNT will
                                                         clear the count value. If this bit is clear, CSR writes to the HNA_PFC3_CNT will continue
                                                         the count from the written value. */
	uint64_t cnt2wclr                     : 1;  /**< Performance counter 2 write clear. If this bit is set, CSR writes to the HNA_PFC2_CNT will
                                                         clear the count value. If this bit is clear, CSR writes to the HNA_PFC2_CNT will continue
                                                         the count from the written value. */
	uint64_t cnt1wclr                     : 1;  /**< Performance counter 1 write clear. If this bit is set, CSR writes to the HNA_PFC1_CNT will
                                                         clear the count value. If this bit is clear, CSR writes to the HNA_PFC1_CNT will continue
                                                         the count from the written value. */
	uint64_t cnt0wclr                     : 1;  /**< Performance counter 0 write clear. If this bit is set, CSR writes to the HNA_PFC0_CNT will
                                                         clear the count value. If this bit is clear, CSR writes to the HNA_PFC0_CNT will continue
                                                         the count from the written value. */
	uint64_t cnt3ena                      : 1;  /**< Performance counter 3 enable. When this bit is set, the performance counter 3 is enabled. */
	uint64_t cnt2ena                      : 1;  /**< Performance counter 2 enable. When this bit is set, the performance counter 2 is enabled. */
	uint64_t cnt1ena                      : 1;  /**< Performance counter 1 enable. When this bit is set, the performance counter 1 is enabled. */
	uint64_t cnt0ena                      : 1;  /**< Performance counter 0 enable. When this bit is set, the performance counter 0 is enabled. */
#else
	uint64_t cnt0ena                      : 1;
	uint64_t cnt1ena                      : 1;
	uint64_t cnt2ena                      : 1;
	uint64_t cnt3ena                      : 1;
	uint64_t cnt0wclr                     : 1;
	uint64_t cnt1wclr                     : 1;
	uint64_t cnt2wclr                     : 1;
	uint64_t cnt3wclr                     : 1;
	uint64_t cnt0rclr                     : 1;
	uint64_t cnt1rclr                     : 1;
	uint64_t cnt2rclr                     : 1;
	uint64_t cnt3rclr                     : 1;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_hna_pfc_gctl_s            cn73xx;
	struct cvmx_hna_pfc_gctl_s            cn78xx;
	struct cvmx_hna_pfc_gctl_s            cn78xxp1;
};
typedef union cvmx_hna_pfc_gctl cvmx_hna_pfc_gctl_t;

/**
 * cvmx_hna_sbd_dbg0
 *
 * When the HNA_CONTROL[SBDLCK] bit is written to 1, the contents of this register are locked
 * down. Otherwise, the contents of this register are the active contents of the HNA scoreboard
 * at the time of the CSR read.
 */
union cvmx_hna_sbd_dbg0 {
	uint64_t u64;
	struct cvmx_hna_sbd_dbg0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sbd                          : 64; /**< HNA Scoreboard 0 data. */
#else
	uint64_t sbd                          : 64;
#endif
	} s;
	struct cvmx_hna_sbd_dbg0_s            cn73xx;
	struct cvmx_hna_sbd_dbg0_s            cn78xx;
	struct cvmx_hna_sbd_dbg0_s            cn78xxp1;
};
typedef union cvmx_hna_sbd_dbg0 cvmx_hna_sbd_dbg0_t;

/**
 * cvmx_hna_sbd_dbg1
 *
 * When the HNA_CONTROL[SBDLCK] bit is written to 1, the contents of this register are locked
 * down. Otherwise, the contents of this register are the 'active' contents of the HNA scoreboard
 * at the time of the CSR read.
 */
union cvmx_hna_sbd_dbg1 {
	uint64_t u64;
	struct cvmx_hna_sbd_dbg1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sbd                          : 64; /**< Scoreboard 1 debug data.
                                                         [63:58] = Reserved.
                                                         [57:16] = Packet data pointer.
                                                         [15] = Unused.
                                                         [14:0] = Packet data counter. */
#else
	uint64_t sbd                          : 64;
#endif
	} s;
	struct cvmx_hna_sbd_dbg1_s            cn73xx;
	struct cvmx_hna_sbd_dbg1_s            cn78xx;
	struct cvmx_hna_sbd_dbg1_s            cn78xxp1;
};
typedef union cvmx_hna_sbd_dbg1 cvmx_hna_sbd_dbg1_t;

/**
 * cvmx_hna_sbd_dbg2
 *
 * When the HNA_CONTROL[SBDLCK] bit is written to 1, the contents of this register are locked
 * down. Otherwise, the contents of this register are the actives contents of the HNA scoreboard
 * at the time of the CSR read.
 */
union cvmx_hna_sbd_dbg2 {
	uint64_t u64;
	struct cvmx_hna_sbd_dbg2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sbd                          : 64; /**< HNA Scoreboard 2 data.
                                                         [63:48] = Reserved.
                                                         [47:45] = ITYPE.
                                                         [44:6] = Result write pointer.
                                                         [5:0] = Pending result write counter. */
#else
	uint64_t sbd                          : 64;
#endif
	} s;
	struct cvmx_hna_sbd_dbg2_s            cn73xx;
	struct cvmx_hna_sbd_dbg2_s            cn78xx;
	struct cvmx_hna_sbd_dbg2_s            cn78xxp1;
};
typedef union cvmx_hna_sbd_dbg2 cvmx_hna_sbd_dbg2_t;

/**
 * cvmx_hna_sbd_dbg3
 *
 * When the HNA_CONTROL[SBDLCK] bit is written 1, the contents of this register are locked down.
 * Otherwise, the contents of this register are the 'active' contents of the HNA Scoreboard at
 * the time of the CSR read.
 */
union cvmx_hna_sbd_dbg3 {
	uint64_t u64;
	struct cvmx_hna_sbd_dbg3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sbd                          : 64; /**< HNA Scoreboard 3 data. */
#else
	uint64_t sbd                          : 64;
#endif
	} s;
	struct cvmx_hna_sbd_dbg3_s            cn73xx;
	struct cvmx_hna_sbd_dbg3_s            cn78xx;
	struct cvmx_hna_sbd_dbg3_s            cn78xxp1;
};
typedef union cvmx_hna_sbd_dbg3 cvmx_hna_sbd_dbg3_t;

#endif
