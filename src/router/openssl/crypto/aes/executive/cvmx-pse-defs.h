/***********************license start***************
 * Copyright (c) 2003-2012  Cavium Inc. (support@cavium.com). All rights
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
 * cvmx-pse-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon pse.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_PSE_DEFS_H__
#define __CVMX_PSE_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_DQ_ECC_CTL0 CVMX_PSE_DQ_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PSE_DQ_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_DQ_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000100ull);
}
#else
#define CVMX_PSE_DQ_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000000100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_DQ_ECC_DBE_STS0 CVMX_PSE_DQ_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PSE_DQ_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_DQ_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000118ull);
}
#else
#define CVMX_PSE_DQ_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000000118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_DQ_ECC_DBE_STS_CMB0 CVMX_PSE_DQ_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PSE_DQ_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_DQ_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000120ull);
}
#else
#define CVMX_PSE_DQ_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000000120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_DQ_ECC_SBE_STS0 CVMX_PSE_DQ_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PSE_DQ_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_DQ_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000108ull);
}
#else
#define CVMX_PSE_DQ_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000000108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_DQ_ECC_SBE_STS_CMB0 CVMX_PSE_DQ_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PSE_DQ_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_DQ_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000110ull);
}
#else
#define CVMX_PSE_DQ_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000000110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_PQ_ECC_CTL0 CVMX_PSE_PQ_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PSE_PQ_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_PQ_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000300100ull);
}
#else
#define CVMX_PSE_PQ_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000300100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_PQ_ECC_DBE_STS0 CVMX_PSE_PQ_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PSE_PQ_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_PQ_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000300118ull);
}
#else
#define CVMX_PSE_PQ_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000300118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_PQ_ECC_DBE_STS_CMB0 CVMX_PSE_PQ_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PSE_PQ_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_PQ_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000300120ull);
}
#else
#define CVMX_PSE_PQ_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000300120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_PQ_ECC_SBE_STS0 CVMX_PSE_PQ_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PSE_PQ_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_PQ_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000300108ull);
}
#else
#define CVMX_PSE_PQ_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000300108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_PQ_ECC_SBE_STS_CMB0 CVMX_PSE_PQ_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PSE_PQ_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_PQ_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000300110ull);
}
#else
#define CVMX_PSE_PQ_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000300110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ1_ECC_CTL0 CVMX_PSE_SQ1_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PSE_SQ1_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ1_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000080100ull);
}
#else
#define CVMX_PSE_SQ1_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000080100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ1_ECC_DBE_STS0 CVMX_PSE_SQ1_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PSE_SQ1_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ1_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000080118ull);
}
#else
#define CVMX_PSE_SQ1_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000080118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ1_ECC_DBE_STS_CMB0 CVMX_PSE_SQ1_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PSE_SQ1_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ1_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000080120ull);
}
#else
#define CVMX_PSE_SQ1_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000080120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ1_ECC_SBE_STS0 CVMX_PSE_SQ1_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PSE_SQ1_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ1_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000080108ull);
}
#else
#define CVMX_PSE_SQ1_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000080108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ1_ECC_SBE_STS_CMB0 CVMX_PSE_SQ1_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PSE_SQ1_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ1_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000080110ull);
}
#else
#define CVMX_PSE_SQ1_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000080110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ2_ECC_CTL0 CVMX_PSE_SQ2_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PSE_SQ2_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ2_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000100100ull);
}
#else
#define CVMX_PSE_SQ2_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000100100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ2_ECC_DBE_STS0 CVMX_PSE_SQ2_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PSE_SQ2_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ2_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000100118ull);
}
#else
#define CVMX_PSE_SQ2_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000100118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ2_ECC_DBE_STS_CMB0 CVMX_PSE_SQ2_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PSE_SQ2_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ2_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000100120ull);
}
#else
#define CVMX_PSE_SQ2_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000100120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ2_ECC_SBE_STS0 CVMX_PSE_SQ2_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PSE_SQ2_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ2_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000100108ull);
}
#else
#define CVMX_PSE_SQ2_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000100108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ2_ECC_SBE_STS_CMB0 CVMX_PSE_SQ2_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PSE_SQ2_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ2_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000100110ull);
}
#else
#define CVMX_PSE_SQ2_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000100110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ3_ECC_CTL0 CVMX_PSE_SQ3_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PSE_SQ3_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ3_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000180100ull);
}
#else
#define CVMX_PSE_SQ3_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000180100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ3_ECC_DBE_STS0 CVMX_PSE_SQ3_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PSE_SQ3_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ3_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000180118ull);
}
#else
#define CVMX_PSE_SQ3_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000180118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ3_ECC_DBE_STS_CMB0 CVMX_PSE_SQ3_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PSE_SQ3_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ3_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000180120ull);
}
#else
#define CVMX_PSE_SQ3_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000180120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ3_ECC_SBE_STS0 CVMX_PSE_SQ3_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PSE_SQ3_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ3_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000180108ull);
}
#else
#define CVMX_PSE_SQ3_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000180108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ3_ECC_SBE_STS_CMB0 CVMX_PSE_SQ3_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PSE_SQ3_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ3_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000180110ull);
}
#else
#define CVMX_PSE_SQ3_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000180110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ4_ECC_CTL0 CVMX_PSE_SQ4_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PSE_SQ4_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ4_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000200100ull);
}
#else
#define CVMX_PSE_SQ4_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000200100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ4_ECC_DBE_STS0 CVMX_PSE_SQ4_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PSE_SQ4_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ4_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000200118ull);
}
#else
#define CVMX_PSE_SQ4_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000200118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ4_ECC_DBE_STS_CMB0 CVMX_PSE_SQ4_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PSE_SQ4_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ4_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000200120ull);
}
#else
#define CVMX_PSE_SQ4_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000200120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ4_ECC_SBE_STS0 CVMX_PSE_SQ4_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PSE_SQ4_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ4_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000200108ull);
}
#else
#define CVMX_PSE_SQ4_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000200108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ4_ECC_SBE_STS_CMB0 CVMX_PSE_SQ4_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PSE_SQ4_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ4_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000200110ull);
}
#else
#define CVMX_PSE_SQ4_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000200110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ5_ECC_CTL0 CVMX_PSE_SQ5_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PSE_SQ5_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ5_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000280100ull);
}
#else
#define CVMX_PSE_SQ5_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000280100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ5_ECC_DBE_STS0 CVMX_PSE_SQ5_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PSE_SQ5_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ5_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000280118ull);
}
#else
#define CVMX_PSE_SQ5_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000280118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ5_ECC_DBE_STS_CMB0 CVMX_PSE_SQ5_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PSE_SQ5_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ5_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000280120ull);
}
#else
#define CVMX_PSE_SQ5_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000280120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ5_ECC_SBE_STS0 CVMX_PSE_SQ5_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PSE_SQ5_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ5_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000280108ull);
}
#else
#define CVMX_PSE_SQ5_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000280108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PSE_SQ5_ECC_SBE_STS_CMB0 CVMX_PSE_SQ5_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PSE_SQ5_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PSE_SQ5_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000280110ull);
}
#else
#define CVMX_PSE_SQ5_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000280110ull))
#endif

/**
 * cvmx_pse_dq_ecc_ctl0
 */
union cvmx_pse_dq_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pse_dq_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dq_wt_ram_flip               : 2;  /**< DQ_WT_RAM flip syndrome bits on write. */
	uint64_t dq_wt_ram_cdis               : 1;  /**< DQ_WT_RAM ECC correction disable. */
	uint64_t dq_rt0_1_flip                : 2;  /**< DQ_RT0_1 flip syndrome bits on write. */
	uint64_t dq_rt0_1_cdis                : 1;  /**< DQ_RT0_1 ECC correction disable. */
	uint64_t dq_rt0_0_flip                : 2;  /**< DQ_RT0_0 flip syndrome bits on write. */
	uint64_t dq_rt0_0_cdis                : 1;  /**< DQ_RT0_0 ECC correction disable. */
	uint64_t reserved_0_54                : 55;
#else
	uint64_t reserved_0_54                : 55;
	uint64_t dq_rt0_0_cdis                : 1;
	uint64_t dq_rt0_0_flip                : 2;
	uint64_t dq_rt0_1_cdis                : 1;
	uint64_t dq_rt0_1_flip                : 2;
	uint64_t dq_wt_ram_cdis               : 1;
	uint64_t dq_wt_ram_flip               : 2;
#endif
	} s;
	struct cvmx_pse_dq_ecc_ctl0_s         cn78xx;
};
typedef union cvmx_pse_dq_ecc_ctl0 cvmx_pse_dq_ecc_ctl0_t;

/**
 * cvmx_pse_dq_ecc_dbe_sts0
 */
union cvmx_pse_dq_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pse_dq_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dq_wt_ram_dbe                : 1;  /**< Double-bit error for DQ_WT_RAM. */
	uint64_t dq_rt0_1_dbe                 : 1;  /**< Double-bit error for DQ_RT0_1. */
	uint64_t dq_rt0_0_dbe                 : 1;  /**< Double-bit error for DQ_RT0_0. */
	uint64_t reserved_0_60                : 61;
#else
	uint64_t reserved_0_60                : 61;
	uint64_t dq_rt0_0_dbe                 : 1;
	uint64_t dq_rt0_1_dbe                 : 1;
	uint64_t dq_wt_ram_dbe                : 1;
#endif
	} s;
	struct cvmx_pse_dq_ecc_dbe_sts0_s     cn78xx;
};
typedef union cvmx_pse_dq_ecc_dbe_sts0 cvmx_pse_dq_ecc_dbe_sts0_t;

/**
 * cvmx_pse_dq_ecc_dbe_sts_cmb0
 */
union cvmx_pse_dq_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pse_dq_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_dq_dbe_cmb0              : 1;  /**< Double-bit error for DQ_WT_RAM. Throws PKO_INTSN_E::PSE_DQ_DBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_dq_dbe_cmb0              : 1;
#endif
	} s;
	struct cvmx_pse_dq_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pse_dq_ecc_dbe_sts_cmb0 cvmx_pse_dq_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pse_dq_ecc_sbe_sts0
 */
union cvmx_pse_dq_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pse_dq_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dq_wt_ram_sbe                : 1;  /**< Single-bit error for DQ_WT_RAM. */
	uint64_t dq_rt0_1_sbe                 : 1;  /**< Single-bit error for DQ_RT0_1. */
	uint64_t dq_rt0_0_sbe                 : 1;  /**< Single-bit error for DQ_RT0_0. */
	uint64_t reserved_0_60                : 61;
#else
	uint64_t reserved_0_60                : 61;
	uint64_t dq_rt0_0_sbe                 : 1;
	uint64_t dq_rt0_1_sbe                 : 1;
	uint64_t dq_wt_ram_sbe                : 1;
#endif
	} s;
	struct cvmx_pse_dq_ecc_sbe_sts0_s     cn78xx;
};
typedef union cvmx_pse_dq_ecc_sbe_sts0 cvmx_pse_dq_ecc_sbe_sts0_t;

/**
 * cvmx_pse_dq_ecc_sbe_sts_cmb0
 */
union cvmx_pse_dq_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pse_dq_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_dq_sbe_cmb0              : 1;  /**< Single-bit error for DQ_WT_RAM. Throws PKO_INTSN_E::PSE_DQ_SBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_dq_sbe_cmb0              : 1;
#endif
	} s;
	struct cvmx_pse_dq_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pse_dq_ecc_sbe_sts_cmb0 cvmx_pse_dq_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pse_pq_ecc_ctl0
 */
union cvmx_pse_pq_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pse_pq_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pq_tw_0_cmd_fifo_ram_flip    : 2;  /**< PQ_TW_0_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t pq_tw_0_cmd_fifo_ram_cdis    : 1;  /**< PQ_TW_0_CMD_FIFO_RAM ECC correction disable. */
	uint64_t pq_dqs_ram_flip              : 2;  /**< PQ_DQS_RAM flip syndrome bits on write. */
	uint64_t pq_dqs_ram_cdis              : 1;  /**< PQ_DQS_RAM ECC correction disable. */
	uint64_t pq_dqd_ram_flip              : 2;  /**< PQ_DQD_RAM flip syndrome bits on write. */
	uint64_t pq_dqd_ram_cdis              : 1;  /**< PQ_DQD_RAM ECC correction disable. */
	uint64_t pq_pqy_ram_flip              : 2;  /**< PQ_PQY_RAM flip syndrome bits on write. */
	uint64_t pq_pqy_ram_cdis              : 1;  /**< PQ_PQY_RAM ECC correction disable. */
	uint64_t pq_pqr_ram_flip              : 2;  /**< PQ_PQR_RAM flip syndrome bits on write. */
	uint64_t pq_pqr_ram_cdis              : 1;  /**< PQ_PQR_RAM ECC correction disable. */
	uint64_t pq_pqg_ram_flip              : 2;  /**< PQ_PQG_RAM flip syndrome bits on write. */
	uint64_t pq_pqg_ram_cdis              : 1;  /**< PQ_PQG_RAM ECC correction disable. */
	uint64_t pq_pqd_ram_flip              : 2;  /**< PQ_PQD_RAM flip syndrome bits on write. */
	uint64_t pq_pqd_ram_cdis              : 1;  /**< PQ_PQD_RAM ECC correction disable. */
	uint64_t pq_std_ram_flip              : 2;  /**< PQ_STD_RAM flip syndrome bits on write. */
	uint64_t pq_std_ram_cdis              : 1;  /**< PQ_STD_RAM ECC correction disable. */
	uint64_t pq_st_ram_flip               : 2;  /**< PQ_ST_RAM flip syndrome bits on write. */
	uint64_t pq_st_ram_cdis               : 1;  /**< PQ_ST_RAM ECC correction disable. */
	uint64_t pq_wmd_ram_flip              : 2;  /**< PQ_WMD_RAM flip syndrome bits on write. */
	uint64_t pq_wmd_ram_cdis              : 1;  /**< PQ_WMD_RAM ECC correction disable. */
	uint64_t pq_wms_ram_flip              : 2;  /**< PQ_WMS_RAM flip syndrome bits on write. */
	uint64_t pq_wms_ram_cdis              : 1;  /**< PQ_WMS_RAM ECC correction disable. */
	uint64_t reserved_0_30                : 31;
#else
	uint64_t reserved_0_30                : 31;
	uint64_t pq_wms_ram_cdis              : 1;
	uint64_t pq_wms_ram_flip              : 2;
	uint64_t pq_wmd_ram_cdis              : 1;
	uint64_t pq_wmd_ram_flip              : 2;
	uint64_t pq_st_ram_cdis               : 1;
	uint64_t pq_st_ram_flip               : 2;
	uint64_t pq_std_ram_cdis              : 1;
	uint64_t pq_std_ram_flip              : 2;
	uint64_t pq_pqd_ram_cdis              : 1;
	uint64_t pq_pqd_ram_flip              : 2;
	uint64_t pq_pqg_ram_cdis              : 1;
	uint64_t pq_pqg_ram_flip              : 2;
	uint64_t pq_pqr_ram_cdis              : 1;
	uint64_t pq_pqr_ram_flip              : 2;
	uint64_t pq_pqy_ram_cdis              : 1;
	uint64_t pq_pqy_ram_flip              : 2;
	uint64_t pq_dqd_ram_cdis              : 1;
	uint64_t pq_dqd_ram_flip              : 2;
	uint64_t pq_dqs_ram_cdis              : 1;
	uint64_t pq_dqs_ram_flip              : 2;
	uint64_t pq_tw_0_cmd_fifo_ram_cdis    : 1;
	uint64_t pq_tw_0_cmd_fifo_ram_flip    : 2;
#endif
	} s;
	struct cvmx_pse_pq_ecc_ctl0_s         cn78xx;
};
typedef union cvmx_pse_pq_ecc_ctl0 cvmx_pse_pq_ecc_ctl0_t;

/**
 * cvmx_pse_pq_ecc_dbe_sts0
 */
union cvmx_pse_pq_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pse_pq_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pq_tw_0_cmd_fifo_ram_dbe     : 1;  /**< Double-bit error for PQ_TW_0_CMD_FIFO_RAM. */
	uint64_t pq_dqs_ram_dbe               : 1;  /**< Double-bit error for PQ_DQS_RAM. */
	uint64_t pq_dqd_ram_dbe               : 1;  /**< Double-bit error for PQ_DQD_RAM. */
	uint64_t pq_pqy_ram_dbe               : 1;  /**< Double-bit error for PQ_PQY_RAM. */
	uint64_t pq_pqr_ram_dbe               : 1;  /**< Double-bit error for PQ_PQR_RAM. */
	uint64_t pq_pqg_ram_dbe               : 1;  /**< Double-bit error for PQ_PQG_RAM. */
	uint64_t pq_pqd_ram_dbe               : 1;  /**< Double-bit error for PQ_PQD_RAM. */
	uint64_t pq_std_ram_dbe               : 1;  /**< Double-bit error for PQ_STD_RAM. */
	uint64_t pq_st_ram_dbe                : 1;  /**< Double-bit error for PQ_ST_RAM. */
	uint64_t pq_wmd_ram_dbe               : 1;  /**< Double-bit error for PQ_WMD_RAM. */
	uint64_t pq_wms_ram_dbe               : 1;  /**< Double-bit error for PQ_WMS_RAM. */
	uint64_t reserved_0_52                : 53;
#else
	uint64_t reserved_0_52                : 53;
	uint64_t pq_wms_ram_dbe               : 1;
	uint64_t pq_wmd_ram_dbe               : 1;
	uint64_t pq_st_ram_dbe                : 1;
	uint64_t pq_std_ram_dbe               : 1;
	uint64_t pq_pqd_ram_dbe               : 1;
	uint64_t pq_pqg_ram_dbe               : 1;
	uint64_t pq_pqr_ram_dbe               : 1;
	uint64_t pq_pqy_ram_dbe               : 1;
	uint64_t pq_dqd_ram_dbe               : 1;
	uint64_t pq_dqs_ram_dbe               : 1;
	uint64_t pq_tw_0_cmd_fifo_ram_dbe     : 1;
#endif
	} s;
	struct cvmx_pse_pq_ecc_dbe_sts0_s     cn78xx;
};
typedef union cvmx_pse_pq_ecc_dbe_sts0 cvmx_pse_pq_ecc_dbe_sts0_t;

/**
 * cvmx_pse_pq_ecc_dbe_sts_cmb0
 */
union cvmx_pse_pq_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pse_pq_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_pq_dbe_cmb0              : 1;  /**< Double-bit error for PQ_TW_0_CMD_FIFO_RAM. Throws PKO_INTSN_E::PSE_PQ_DBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_pq_dbe_cmb0              : 1;
#endif
	} s;
	struct cvmx_pse_pq_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pse_pq_ecc_dbe_sts_cmb0 cvmx_pse_pq_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pse_pq_ecc_sbe_sts0
 */
union cvmx_pse_pq_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pse_pq_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pq_tw_0_cmd_fifo_ram_sbe     : 1;  /**< Single-bit error for PQ_TW_0_CMD_FIFO_RAM. */
	uint64_t pq_dqs_ram_sbe               : 1;  /**< Single-bit error for PQ_DQS_RAM. */
	uint64_t pq_dqd_ram_sbe               : 1;  /**< Single-bit error for PQ_DQD_RAM. */
	uint64_t pq_pqy_ram_sbe               : 1;  /**< Single-bit error for PQ_PQY_RAM. */
	uint64_t pq_pqr_ram_sbe               : 1;  /**< Single-bit error for PQ_PQR_RAM. */
	uint64_t pq_pqg_ram_sbe               : 1;  /**< Single-bit error for PQ_PQG_RAM. */
	uint64_t pq_pqd_ram_sbe               : 1;  /**< Single-bit error for PQ_PQD_RAM. */
	uint64_t pq_std_ram_sbe               : 1;  /**< Single-bit error for PQ_STD_RAM. */
	uint64_t pq_st_ram_sbe                : 1;  /**< Single-bit error for PQ_ST_RAM. */
	uint64_t pq_wmd_ram_sbe               : 1;  /**< Single-bit error for PQ_WMD_RAM. */
	uint64_t pq_wms_ram_sbe               : 1;  /**< Single-bit error for PQ_WMS_RAM. */
	uint64_t reserved_0_52                : 53;
#else
	uint64_t reserved_0_52                : 53;
	uint64_t pq_wms_ram_sbe               : 1;
	uint64_t pq_wmd_ram_sbe               : 1;
	uint64_t pq_st_ram_sbe                : 1;
	uint64_t pq_std_ram_sbe               : 1;
	uint64_t pq_pqd_ram_sbe               : 1;
	uint64_t pq_pqg_ram_sbe               : 1;
	uint64_t pq_pqr_ram_sbe               : 1;
	uint64_t pq_pqy_ram_sbe               : 1;
	uint64_t pq_dqd_ram_sbe               : 1;
	uint64_t pq_dqs_ram_sbe               : 1;
	uint64_t pq_tw_0_cmd_fifo_ram_sbe     : 1;
#endif
	} s;
	struct cvmx_pse_pq_ecc_sbe_sts0_s     cn78xx;
};
typedef union cvmx_pse_pq_ecc_sbe_sts0 cvmx_pse_pq_ecc_sbe_sts0_t;

/**
 * cvmx_pse_pq_ecc_sbe_sts_cmb0
 */
union cvmx_pse_pq_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pse_pq_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_pq_sbe_cmb0              : 1;  /**< Single-bit error for PQ_TW_0_CMD_FIFO_RAM. Throws PKO_INTSN_E::PSE_PQ_SBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_pq_sbe_cmb0              : 1;
#endif
	} s;
	struct cvmx_pse_pq_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pse_pq_ecc_sbe_sts_cmb0 cvmx_pse_pq_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pse_sq1_ecc_ctl0
 */
union cvmx_pse_sq1_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pse_sq1_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq1_cxs_ram_flip             : 2;  /**< SQ1_CXS_RAM flip syndrome bits on write. */
	uint64_t sq1_cxs_ram_cdis             : 1;  /**< SQ1_CXS_RAM ECC correction disable. */
	uint64_t sq1_cxd_ram_flip             : 2;  /**< SQ1_CXD_RAM flip syndrome bits on write. */
	uint64_t sq1_cxd_ram_cdis             : 1;  /**< SQ1_CXD_RAM ECC correction disable. */
	uint64_t sq1_pt_ram_flip              : 2;  /**< SQ1_PT_RAM flip syndrome bits on write. */
	uint64_t sq1_pt_ram_cdis              : 1;  /**< SQ1_PT_RAM ECC correction disable. */
	uint64_t sq1_nt_ram_flip              : 2;  /**< SQ1_NT_RAM flip syndrome bits on write. */
	uint64_t sq1_nt_ram_cdis              : 1;  /**< SQ1_NT_RAM ECC correction disable. */
	uint64_t sq1_rt_ram_flip              : 2;  /**< SQ1_RT_RAM flip syndrome bits on write. */
	uint64_t sq1_rt_ram_cdis              : 1;  /**< SQ1_RT_RAM ECC correction disable. */
	uint64_t sq1_tw_1_cmd_fifo_ram_flip   : 2;  /**< SQ1_TW_1_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq1_tw_1_cmd_fifo_ram_cdis   : 1;  /**< SQ1_TW_1_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq1_tw_0_cmd_fifo_ram_flip   : 2;  /**< SQ1_TW_0_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq1_tw_0_cmd_fifo_ram_cdis   : 1;  /**< SQ1_TW_0_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq1_sts1_ram_flip            : 2;  /**< SQ1_STS1_RAM flip syndrome bits on write. */
	uint64_t sq1_sts1_ram_cdis            : 1;  /**< SQ1_STS1_RAM ECC correction disable. */
	uint64_t sq1_sts0_ram_flip            : 2;  /**< SQ1_STS0_RAM flip syndrome bits on write. */
	uint64_t sq1_sts0_ram_cdis            : 1;  /**< SQ1_STS0_RAM ECC correction disable. */
	uint64_t sq1_std1_ram_flip            : 2;  /**< SQ1_STD1_RAM flip syndrome bits on write. */
	uint64_t sq1_std1_ram_cdis            : 1;  /**< SQ1_STD1_RAM ECC correction disable. */
	uint64_t sq1_std0_ram_flip            : 2;  /**< SQ1_STD0_RAM flip syndrome bits on write. */
	uint64_t sq1_std0_ram_cdis            : 1;  /**< SQ1_STD0_RAM ECC correction disable. */
	uint64_t sq1_wt_ram_flip              : 2;  /**< SQ1_WT_RAM flip syndrome bits on write. */
	uint64_t sq1_wt_ram_cdis              : 1;  /**< SQ1_WT_RAM ECC correction disable. */
	uint64_t reserved_0_27                : 28;
#else
	uint64_t reserved_0_27                : 28;
	uint64_t sq1_wt_ram_cdis              : 1;
	uint64_t sq1_wt_ram_flip              : 2;
	uint64_t sq1_std0_ram_cdis            : 1;
	uint64_t sq1_std0_ram_flip            : 2;
	uint64_t sq1_std1_ram_cdis            : 1;
	uint64_t sq1_std1_ram_flip            : 2;
	uint64_t sq1_sts0_ram_cdis            : 1;
	uint64_t sq1_sts0_ram_flip            : 2;
	uint64_t sq1_sts1_ram_cdis            : 1;
	uint64_t sq1_sts1_ram_flip            : 2;
	uint64_t sq1_tw_0_cmd_fifo_ram_cdis   : 1;
	uint64_t sq1_tw_0_cmd_fifo_ram_flip   : 2;
	uint64_t sq1_tw_1_cmd_fifo_ram_cdis   : 1;
	uint64_t sq1_tw_1_cmd_fifo_ram_flip   : 2;
	uint64_t sq1_rt_ram_cdis              : 1;
	uint64_t sq1_rt_ram_flip              : 2;
	uint64_t sq1_nt_ram_cdis              : 1;
	uint64_t sq1_nt_ram_flip              : 2;
	uint64_t sq1_pt_ram_cdis              : 1;
	uint64_t sq1_pt_ram_flip              : 2;
	uint64_t sq1_cxd_ram_cdis             : 1;
	uint64_t sq1_cxd_ram_flip             : 2;
	uint64_t sq1_cxs_ram_cdis             : 1;
	uint64_t sq1_cxs_ram_flip             : 2;
#endif
	} s;
	struct cvmx_pse_sq1_ecc_ctl0_s        cn78xx;
};
typedef union cvmx_pse_sq1_ecc_ctl0 cvmx_pse_sq1_ecc_ctl0_t;

/**
 * cvmx_pse_sq1_ecc_dbe_sts0
 */
union cvmx_pse_sq1_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pse_sq1_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq1_cxs_ram_dbe              : 1;  /**< Double-bit error for SQ1_CXS_RAM. */
	uint64_t sq1_cxd_ram_dbe              : 1;  /**< Double-bit error for SQ1_CXD_RAM. */
	uint64_t sq1_pt_ram_dbe               : 1;  /**< Double-bit error for SQ1_PT_RAM. */
	uint64_t sq1_nt_ram_dbe               : 1;  /**< Double-bit error for SQ1_NT_RAM. */
	uint64_t sq1_rt_ram_dbe               : 1;  /**< Double-bit error for SQ1_RT_RAM. */
	uint64_t sq1_tw_1_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ1_TW_1_CMD_FIFO_RAM. */
	uint64_t sq1_tw_0_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ1_TW_0_CMD_FIFO_RAM. */
	uint64_t sq1_sts1_ram_dbe             : 1;  /**< Double-bit error for SQ1_STS1_RAM. */
	uint64_t sq1_sts0_ram_dbe             : 1;  /**< Double-bit error for SQ1_STS0_RAM. */
	uint64_t sq1_std1_ram_dbe             : 1;  /**< Double-bit error for SQ1_STD1_RAM. */
	uint64_t sq1_std0_ram_dbe             : 1;  /**< Double-bit error for SQ1_STD0_RAM. */
	uint64_t sq1_wt_ram_dbe               : 1;  /**< Double-bit error for SQ1_WT_RAM. */
	uint64_t reserved_0_51                : 52;
#else
	uint64_t reserved_0_51                : 52;
	uint64_t sq1_wt_ram_dbe               : 1;
	uint64_t sq1_std0_ram_dbe             : 1;
	uint64_t sq1_std1_ram_dbe             : 1;
	uint64_t sq1_sts0_ram_dbe             : 1;
	uint64_t sq1_sts1_ram_dbe             : 1;
	uint64_t sq1_tw_0_cmd_fifo_ram_dbe    : 1;
	uint64_t sq1_tw_1_cmd_fifo_ram_dbe    : 1;
	uint64_t sq1_rt_ram_dbe               : 1;
	uint64_t sq1_nt_ram_dbe               : 1;
	uint64_t sq1_pt_ram_dbe               : 1;
	uint64_t sq1_cxd_ram_dbe              : 1;
	uint64_t sq1_cxs_ram_dbe              : 1;
#endif
	} s;
	struct cvmx_pse_sq1_ecc_dbe_sts0_s    cn78xx;
};
typedef union cvmx_pse_sq1_ecc_dbe_sts0 cvmx_pse_sq1_ecc_dbe_sts0_t;

/**
 * cvmx_pse_sq1_ecc_dbe_sts_cmb0
 */
union cvmx_pse_sq1_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pse_sq1_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq1_dbe_cmb0             : 1;  /**< Double-bit error for SQ1_CXS_RAM. Throws PKO_INTSN_E::PSE_SQ1_DBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq1_dbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pse_sq1_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pse_sq1_ecc_dbe_sts_cmb0 cvmx_pse_sq1_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pse_sq1_ecc_sbe_sts0
 */
union cvmx_pse_sq1_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pse_sq1_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq1_cxs_ram_sbe              : 1;  /**< Single-bit error for SQ1_CXS_RAM. */
	uint64_t sq1_cxd_ram_sbe              : 1;  /**< Single-bit error for SQ1_CXD_RAM. */
	uint64_t sq1_pt_ram_sbe               : 1;  /**< Single-bit error for SQ1_PT_RAM. */
	uint64_t sq1_nt_ram_sbe               : 1;  /**< Single-bit error for SQ1_NT_RAM. */
	uint64_t sq1_rt_ram_sbe               : 1;  /**< Single-bit error for SQ1_RT_RAM. */
	uint64_t sq1_tw_1_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ1_TW_1_CMD_FIFO_RAM. */
	uint64_t sq1_tw_0_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ1_TW_0_CMD_FIFO_RAM. */
	uint64_t sq1_sts1_ram_sbe             : 1;  /**< Single-bit error for SQ1_STS1_RAM. */
	uint64_t sq1_sts0_ram_sbe             : 1;  /**< Single-bit error for SQ1_STS0_RAM. */
	uint64_t sq1_std1_ram_sbe             : 1;  /**< Single-bit error for SQ1_STD1_RAM. */
	uint64_t sq1_std0_ram_sbe             : 1;  /**< Single-bit error for SQ1_STD0_RAM. */
	uint64_t sq1_wt_ram_sbe               : 1;  /**< Single-bit error for SQ1_WT_RAM. */
	uint64_t reserved_0_51                : 52;
#else
	uint64_t reserved_0_51                : 52;
	uint64_t sq1_wt_ram_sbe               : 1;
	uint64_t sq1_std0_ram_sbe             : 1;
	uint64_t sq1_std1_ram_sbe             : 1;
	uint64_t sq1_sts0_ram_sbe             : 1;
	uint64_t sq1_sts1_ram_sbe             : 1;
	uint64_t sq1_tw_0_cmd_fifo_ram_sbe    : 1;
	uint64_t sq1_tw_1_cmd_fifo_ram_sbe    : 1;
	uint64_t sq1_rt_ram_sbe               : 1;
	uint64_t sq1_nt_ram_sbe               : 1;
	uint64_t sq1_pt_ram_sbe               : 1;
	uint64_t sq1_cxd_ram_sbe              : 1;
	uint64_t sq1_cxs_ram_sbe              : 1;
#endif
	} s;
	struct cvmx_pse_sq1_ecc_sbe_sts0_s    cn78xx;
};
typedef union cvmx_pse_sq1_ecc_sbe_sts0 cvmx_pse_sq1_ecc_sbe_sts0_t;

/**
 * cvmx_pse_sq1_ecc_sbe_sts_cmb0
 */
union cvmx_pse_sq1_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pse_sq1_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq1_sbe_cmb0             : 1;  /**< Single-bit error for SQ1_CXS_RAM. Throws PKO_INTSN_E::PSE_SQ1_SBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq1_sbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pse_sq1_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pse_sq1_ecc_sbe_sts_cmb0 cvmx_pse_sq1_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pse_sq2_ecc_ctl0
 */
union cvmx_pse_sq2_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pse_sq2_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pq_cxs_ram_flip              : 2;  /**< PQ_CXS_RAM flip syndrome bits on write. */
	uint64_t pq_cxs_ram_cdis              : 1;  /**< PQ_CXS_RAM ECC correction disable. */
	uint64_t sq2_cxs_ram_flip             : 2;  /**< SQ2_CXS_RAM flip syndrome bits on write. */
	uint64_t sq2_cxs_ram_cdis             : 1;  /**< SQ2_CXS_RAM ECC correction disable. */
	uint64_t pq_cxd_ram_flip              : 2;  /**< PQ_CXD_RAM flip syndrome bits on write. */
	uint64_t pq_cxd_ram_cdis              : 1;  /**< PQ_CXD_RAM ECC correction disable. */
	uint64_t sq2_cxd_ram_flip             : 2;  /**< SQ2_CXD_RAM flip syndrome bits on write. */
	uint64_t sq2_cxd_ram_cdis             : 1;  /**< SQ2_CXD_RAM ECC correction disable. */
	uint64_t sq2_pt_ram_flip              : 2;  /**< SQ2_PT_RAM flip syndrome bits on write. */
	uint64_t sq2_pt_ram_cdis              : 1;  /**< SQ2_PT_RAM ECC correction disable. */
	uint64_t sq2_nt_ram_flip              : 2;  /**< SQ2_NT_RAM flip syndrome bits on write. */
	uint64_t sq2_nt_ram_cdis              : 1;  /**< SQ2_NT_RAM ECC correction disable. */
	uint64_t sq2_rt_ram_flip              : 2;  /**< SQ2_RT_RAM flip syndrome bits on write. */
	uint64_t sq2_rt_ram_cdis              : 1;  /**< SQ2_RT_RAM ECC correction disable. */
	uint64_t sq2_tw_1_cmd_fifo_ram_flip   : 2;  /**< SQ2_TW_1_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq2_tw_1_cmd_fifo_ram_cdis   : 1;  /**< SQ2_TW_1_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq2_tw_0_cmd_fifo_ram_flip   : 2;  /**< SQ2_TW_0_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq2_tw_0_cmd_fifo_ram_cdis   : 1;  /**< SQ2_TW_0_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq2_sts1_ram_flip            : 2;  /**< SQ2_STS1_RAM flip syndrome bits on write. */
	uint64_t sq2_sts1_ram_cdis            : 1;  /**< SQ2_STS1_RAM ECC correction disable. */
	uint64_t sq2_sts0_ram_flip            : 2;  /**< SQ2_STS0_RAM flip syndrome bits on write. */
	uint64_t sq2_sts0_ram_cdis            : 1;  /**< SQ2_STS0_RAM ECC correction disable. */
	uint64_t sq2_std1_ram_flip            : 2;  /**< SQ2_STD1_RAM flip syndrome bits on write. */
	uint64_t sq2_std1_ram_cdis            : 1;  /**< SQ2_STD1_RAM ECC correction disable. */
	uint64_t sq2_std0_ram_flip            : 2;  /**< SQ2_STD0_RAM flip syndrome bits on write. */
	uint64_t sq2_std0_ram_cdis            : 1;  /**< SQ2_STD0_RAM ECC correction disable. */
	uint64_t sq2_wt_ram_flip              : 2;  /**< SQ2_WT_RAM flip syndrome bits on write. */
	uint64_t sq2_wt_ram_cdis              : 1;  /**< SQ2_WT_RAM ECC correction disable. */
	uint64_t reserved_0_21                : 22;
#else
	uint64_t reserved_0_21                : 22;
	uint64_t sq2_wt_ram_cdis              : 1;
	uint64_t sq2_wt_ram_flip              : 2;
	uint64_t sq2_std0_ram_cdis            : 1;
	uint64_t sq2_std0_ram_flip            : 2;
	uint64_t sq2_std1_ram_cdis            : 1;
	uint64_t sq2_std1_ram_flip            : 2;
	uint64_t sq2_sts0_ram_cdis            : 1;
	uint64_t sq2_sts0_ram_flip            : 2;
	uint64_t sq2_sts1_ram_cdis            : 1;
	uint64_t sq2_sts1_ram_flip            : 2;
	uint64_t sq2_tw_0_cmd_fifo_ram_cdis   : 1;
	uint64_t sq2_tw_0_cmd_fifo_ram_flip   : 2;
	uint64_t sq2_tw_1_cmd_fifo_ram_cdis   : 1;
	uint64_t sq2_tw_1_cmd_fifo_ram_flip   : 2;
	uint64_t sq2_rt_ram_cdis              : 1;
	uint64_t sq2_rt_ram_flip              : 2;
	uint64_t sq2_nt_ram_cdis              : 1;
	uint64_t sq2_nt_ram_flip              : 2;
	uint64_t sq2_pt_ram_cdis              : 1;
	uint64_t sq2_pt_ram_flip              : 2;
	uint64_t sq2_cxd_ram_cdis             : 1;
	uint64_t sq2_cxd_ram_flip             : 2;
	uint64_t pq_cxd_ram_cdis              : 1;
	uint64_t pq_cxd_ram_flip              : 2;
	uint64_t sq2_cxs_ram_cdis             : 1;
	uint64_t sq2_cxs_ram_flip             : 2;
	uint64_t pq_cxs_ram_cdis              : 1;
	uint64_t pq_cxs_ram_flip              : 2;
#endif
	} s;
	struct cvmx_pse_sq2_ecc_ctl0_s        cn78xx;
};
typedef union cvmx_pse_sq2_ecc_ctl0 cvmx_pse_sq2_ecc_ctl0_t;

/**
 * cvmx_pse_sq2_ecc_dbe_sts0
 */
union cvmx_pse_sq2_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pse_sq2_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pq_cxs_ram_dbe               : 1;  /**< Double-bit error for PQ_CXS_RAM. */
	uint64_t sq2_cxs_ram_dbe              : 1;  /**< Double-bit error for SQ2_CXS_RAM. */
	uint64_t pq_cxd_ram_dbe               : 1;  /**< Double-bit error for PQ_CXD_RAM. */
	uint64_t sq2_cxd_ram_dbe              : 1;  /**< Double-bit error for SQ2_CXD_RAM. */
	uint64_t sq2_pt_ram_dbe               : 1;  /**< Double-bit error for SQ2_PT_RAM. */
	uint64_t sq2_nt_ram_dbe               : 1;  /**< Double-bit error for SQ2_NT_RAM. */
	uint64_t sq2_rt_ram_dbe               : 1;  /**< Double-bit error for SQ2_RT_RAM. */
	uint64_t sq2_tw_1_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ2_TW_1_CMD_FIFO_RAM. */
	uint64_t sq2_tw_0_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ2_TW_0_CMD_FIFO_RAM. */
	uint64_t sq2_sts1_ram_dbe             : 1;  /**< Double-bit error for SQ2_STS1_RAM. */
	uint64_t sq2_sts0_ram_dbe             : 1;  /**< Double-bit error for SQ2_STS0_RAM. */
	uint64_t sq2_std1_ram_dbe             : 1;  /**< Double-bit error for SQ2_STD1_RAM. */
	uint64_t sq2_std0_ram_dbe             : 1;  /**< Double-bit error for SQ2_STD0_RAM. */
	uint64_t sq2_wt_ram_dbe               : 1;  /**< Double-bit error for SQ2_WT_RAM. */
	uint64_t reserved_0_49                : 50;
#else
	uint64_t reserved_0_49                : 50;
	uint64_t sq2_wt_ram_dbe               : 1;
	uint64_t sq2_std0_ram_dbe             : 1;
	uint64_t sq2_std1_ram_dbe             : 1;
	uint64_t sq2_sts0_ram_dbe             : 1;
	uint64_t sq2_sts1_ram_dbe             : 1;
	uint64_t sq2_tw_0_cmd_fifo_ram_dbe    : 1;
	uint64_t sq2_tw_1_cmd_fifo_ram_dbe    : 1;
	uint64_t sq2_rt_ram_dbe               : 1;
	uint64_t sq2_nt_ram_dbe               : 1;
	uint64_t sq2_pt_ram_dbe               : 1;
	uint64_t sq2_cxd_ram_dbe              : 1;
	uint64_t pq_cxd_ram_dbe               : 1;
	uint64_t sq2_cxs_ram_dbe              : 1;
	uint64_t pq_cxs_ram_dbe               : 1;
#endif
	} s;
	struct cvmx_pse_sq2_ecc_dbe_sts0_s    cn78xx;
};
typedef union cvmx_pse_sq2_ecc_dbe_sts0 cvmx_pse_sq2_ecc_dbe_sts0_t;

/**
 * cvmx_pse_sq2_ecc_dbe_sts_cmb0
 */
union cvmx_pse_sq2_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pse_sq2_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq2_dbe_cmb0             : 1;  /**< Double-bit error for PQ_CXS_RAM. Throws PKO_INTSN_E::PSE_SQ2_DBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq2_dbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pse_sq2_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pse_sq2_ecc_dbe_sts_cmb0 cvmx_pse_sq2_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pse_sq2_ecc_sbe_sts0
 */
union cvmx_pse_sq2_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pse_sq2_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pq_cxs_ram_sbe               : 1;  /**< Single-bit error for PQ_CXS_RAM. */
	uint64_t sq2_cxs_ram_sbe              : 1;  /**< Single-bit error for SQ2_CXS_RAM. */
	uint64_t pq_cxd_ram_sbe               : 1;  /**< Single-bit error for PQ_CXD_RAM. */
	uint64_t sq2_cxd_ram_sbe              : 1;  /**< Single-bit error for SQ2_CXD_RAM. */
	uint64_t sq2_pt_ram_sbe               : 1;  /**< Single-bit error for SQ2_PT_RAM. */
	uint64_t sq2_nt_ram_sbe               : 1;  /**< Single-bit error for SQ2_NT_RAM. */
	uint64_t sq2_rt_ram_sbe               : 1;  /**< Single-bit error for SQ2_RT_RAM. */
	uint64_t sq2_tw_1_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ2_TW_1_CMD_FIFO_RAM. */
	uint64_t sq2_tw_0_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ2_TW_0_CMD_FIFO_RAM. */
	uint64_t sq2_sts1_ram_sbe             : 1;  /**< Single-bit error for SQ2_STS1_RAM. */
	uint64_t sq2_sts0_ram_sbe             : 1;  /**< Single-bit error for SQ2_STS0_RAM. */
	uint64_t sq2_std1_ram_sbe             : 1;  /**< Single-bit error for SQ2_STD1_RAM. */
	uint64_t sq2_std0_ram_sbe             : 1;  /**< Single-bit error for SQ2_STD0_RAM. */
	uint64_t sq2_wt_ram_sbe               : 1;  /**< Single-bit error for SQ2_WT_RAM. */
	uint64_t reserved_0_49                : 50;
#else
	uint64_t reserved_0_49                : 50;
	uint64_t sq2_wt_ram_sbe               : 1;
	uint64_t sq2_std0_ram_sbe             : 1;
	uint64_t sq2_std1_ram_sbe             : 1;
	uint64_t sq2_sts0_ram_sbe             : 1;
	uint64_t sq2_sts1_ram_sbe             : 1;
	uint64_t sq2_tw_0_cmd_fifo_ram_sbe    : 1;
	uint64_t sq2_tw_1_cmd_fifo_ram_sbe    : 1;
	uint64_t sq2_rt_ram_sbe               : 1;
	uint64_t sq2_nt_ram_sbe               : 1;
	uint64_t sq2_pt_ram_sbe               : 1;
	uint64_t sq2_cxd_ram_sbe              : 1;
	uint64_t pq_cxd_ram_sbe               : 1;
	uint64_t sq2_cxs_ram_sbe              : 1;
	uint64_t pq_cxs_ram_sbe               : 1;
#endif
	} s;
	struct cvmx_pse_sq2_ecc_sbe_sts0_s    cn78xx;
};
typedef union cvmx_pse_sq2_ecc_sbe_sts0 cvmx_pse_sq2_ecc_sbe_sts0_t;

/**
 * cvmx_pse_sq2_ecc_sbe_sts_cmb0
 */
union cvmx_pse_sq2_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pse_sq2_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq2_sbe_cmb0             : 1;  /**< Single-bit error for PQ_CXS_RAM. Throws PKO_INTSN_E::PSE_SQ2_SBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq2_sbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pse_sq2_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pse_sq2_ecc_sbe_sts_cmb0 cvmx_pse_sq2_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pse_sq3_ecc_ctl0
 */
union cvmx_pse_sq3_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pse_sq3_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq3_pt_ram_flip              : 2;  /**< SQ3_PT_RAM flip syndrome bits on write. */
	uint64_t sq3_pt_ram_cdis              : 1;  /**< SQ3_PT_RAM ECC correction disable. */
	uint64_t sq3_nt_ram_flip              : 2;  /**< SQ3_NT_RAM flip syndrome bits on write. */
	uint64_t sq3_nt_ram_cdis              : 1;  /**< SQ3_NT_RAM ECC correction disable. */
	uint64_t sq3_rt_ram_flip              : 2;  /**< SQ3_RT_RAM flip syndrome bits on write. */
	uint64_t sq3_rt_ram_cdis              : 1;  /**< SQ3_RT_RAM ECC correction disable. */
	uint64_t sq3_tw_3_cmd_fifo_ram_flip   : 2;  /**< SQ3_TW_3_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq3_tw_3_cmd_fifo_ram_cdis   : 1;  /**< SQ3_TW_3_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq3_tw_2_cmd_fifo_ram_flip   : 2;  /**< SQ3_TW_2_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq3_tw_2_cmd_fifo_ram_cdis   : 1;  /**< SQ3_TW_2_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq3_tw_1_cmd_fifo_ram_flip   : 2;  /**< SQ3_TW_1_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq3_tw_1_cmd_fifo_ram_cdis   : 1;  /**< SQ3_TW_1_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq3_tw_0_cmd_fifo_ram_flip   : 2;  /**< SQ3_TW_0_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq3_tw_0_cmd_fifo_ram_cdis   : 1;  /**< SQ3_TW_0_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq3_sts3_ram_flip            : 2;  /**< SQ3_STS3_RAM flip syndrome bits on write. */
	uint64_t sq3_sts3_ram_cdis            : 1;  /**< SQ3_STS3_RAM ECC correction disable. */
	uint64_t sq3_sts2_ram_flip            : 2;  /**< SQ3_STS2_RAM flip syndrome bits on write. */
	uint64_t sq3_sts2_ram_cdis            : 1;  /**< SQ3_STS2_RAM ECC correction disable. */
	uint64_t sq3_sts1_ram_flip            : 2;  /**< SQ3_STS1_RAM flip syndrome bits on write. */
	uint64_t sq3_sts1_ram_cdis            : 1;  /**< SQ3_STS1_RAM ECC correction disable. */
	uint64_t sq3_sts0_ram_flip            : 2;  /**< SQ3_STS0_RAM flip syndrome bits on write. */
	uint64_t sq3_sts0_ram_cdis            : 1;  /**< SQ3_STS0_RAM ECC correction disable. */
	uint64_t sq3_std3_ram_flip            : 2;  /**< SQ3_STD3_RAM flip syndrome bits on write. */
	uint64_t sq3_std3_ram_cdis            : 1;  /**< SQ3_STD3_RAM ECC correction disable. */
	uint64_t sq3_std2_ram_flip            : 2;  /**< SQ3_STD2_RAM flip syndrome bits on write. */
	uint64_t sq3_std2_ram_cdis            : 1;  /**< SQ3_STD2_RAM ECC correction disable. */
	uint64_t sq3_std1_ram_flip            : 2;  /**< SQ3_STD1_RAM flip syndrome bits on write. */
	uint64_t sq3_std1_ram_cdis            : 1;  /**< SQ3_STD1_RAM ECC correction disable. */
	uint64_t sq3_std0_ram_flip            : 2;  /**< SQ3_STD0_RAM flip syndrome bits on write. */
	uint64_t sq3_std0_ram_cdis            : 1;  /**< SQ3_STD0_RAM ECC correction disable. */
	uint64_t sq3_wt_ram_flip              : 2;  /**< SQ3_WT_RAM flip syndrome bits on write. */
	uint64_t sq3_wt_ram_cdis              : 1;  /**< SQ3_WT_RAM ECC correction disable. */
	uint64_t reserved_0_15                : 16;
#else
	uint64_t reserved_0_15                : 16;
	uint64_t sq3_wt_ram_cdis              : 1;
	uint64_t sq3_wt_ram_flip              : 2;
	uint64_t sq3_std0_ram_cdis            : 1;
	uint64_t sq3_std0_ram_flip            : 2;
	uint64_t sq3_std1_ram_cdis            : 1;
	uint64_t sq3_std1_ram_flip            : 2;
	uint64_t sq3_std2_ram_cdis            : 1;
	uint64_t sq3_std2_ram_flip            : 2;
	uint64_t sq3_std3_ram_cdis            : 1;
	uint64_t sq3_std3_ram_flip            : 2;
	uint64_t sq3_sts0_ram_cdis            : 1;
	uint64_t sq3_sts0_ram_flip            : 2;
	uint64_t sq3_sts1_ram_cdis            : 1;
	uint64_t sq3_sts1_ram_flip            : 2;
	uint64_t sq3_sts2_ram_cdis            : 1;
	uint64_t sq3_sts2_ram_flip            : 2;
	uint64_t sq3_sts3_ram_cdis            : 1;
	uint64_t sq3_sts3_ram_flip            : 2;
	uint64_t sq3_tw_0_cmd_fifo_ram_cdis   : 1;
	uint64_t sq3_tw_0_cmd_fifo_ram_flip   : 2;
	uint64_t sq3_tw_1_cmd_fifo_ram_cdis   : 1;
	uint64_t sq3_tw_1_cmd_fifo_ram_flip   : 2;
	uint64_t sq3_tw_2_cmd_fifo_ram_cdis   : 1;
	uint64_t sq3_tw_2_cmd_fifo_ram_flip   : 2;
	uint64_t sq3_tw_3_cmd_fifo_ram_cdis   : 1;
	uint64_t sq3_tw_3_cmd_fifo_ram_flip   : 2;
	uint64_t sq3_rt_ram_cdis              : 1;
	uint64_t sq3_rt_ram_flip              : 2;
	uint64_t sq3_nt_ram_cdis              : 1;
	uint64_t sq3_nt_ram_flip              : 2;
	uint64_t sq3_pt_ram_cdis              : 1;
	uint64_t sq3_pt_ram_flip              : 2;
#endif
	} s;
	struct cvmx_pse_sq3_ecc_ctl0_s        cn78xx;
};
typedef union cvmx_pse_sq3_ecc_ctl0 cvmx_pse_sq3_ecc_ctl0_t;

/**
 * cvmx_pse_sq3_ecc_dbe_sts0
 */
union cvmx_pse_sq3_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pse_sq3_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq3_pt_ram_dbe               : 1;  /**< Double-bit error for SQ3_PT_RAM. */
	uint64_t sq3_nt_ram_dbe               : 1;  /**< Double-bit error for SQ3_NT_RAM. */
	uint64_t sq3_rt_ram_dbe               : 1;  /**< Double-bit error for SQ3_RT_RAM. */
	uint64_t sq3_tw_3_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ3_TW_3_CMD_FIFO_RAM. */
	uint64_t sq3_tw_2_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ3_TW_2_CMD_FIFO_RAM. */
	uint64_t sq3_tw_1_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ3_TW_1_CMD_FIFO_RAM. */
	uint64_t sq3_tw_0_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ3_TW_0_CMD_FIFO_RAM. */
	uint64_t sq3_sts3_ram_dbe             : 1;  /**< Double-bit error for SQ3_STS3_RAM. */
	uint64_t sq3_sts2_ram_dbe             : 1;  /**< Double-bit error for SQ3_STS2_RAM. */
	uint64_t sq3_sts1_ram_dbe             : 1;  /**< Double-bit error for SQ3_STS1_RAM. */
	uint64_t sq3_sts0_ram_dbe             : 1;  /**< Double-bit error for SQ3_STS0_RAM. */
	uint64_t sq3_std3_ram_dbe             : 1;  /**< Double-bit error for SQ3_STD3_RAM. */
	uint64_t sq3_std2_ram_dbe             : 1;  /**< Double-bit error for SQ3_STD2_RAM. */
	uint64_t sq3_std1_ram_dbe             : 1;  /**< Double-bit error for SQ3_STD1_RAM. */
	uint64_t sq3_std0_ram_dbe             : 1;  /**< Double-bit error for SQ3_STD0_RAM. */
	uint64_t sq3_wt_ram_dbe               : 1;  /**< Double-bit error for SQ3_WT_RAM. */
	uint64_t reserved_0_47                : 48;
#else
	uint64_t reserved_0_47                : 48;
	uint64_t sq3_wt_ram_dbe               : 1;
	uint64_t sq3_std0_ram_dbe             : 1;
	uint64_t sq3_std1_ram_dbe             : 1;
	uint64_t sq3_std2_ram_dbe             : 1;
	uint64_t sq3_std3_ram_dbe             : 1;
	uint64_t sq3_sts0_ram_dbe             : 1;
	uint64_t sq3_sts1_ram_dbe             : 1;
	uint64_t sq3_sts2_ram_dbe             : 1;
	uint64_t sq3_sts3_ram_dbe             : 1;
	uint64_t sq3_tw_0_cmd_fifo_ram_dbe    : 1;
	uint64_t sq3_tw_1_cmd_fifo_ram_dbe    : 1;
	uint64_t sq3_tw_2_cmd_fifo_ram_dbe    : 1;
	uint64_t sq3_tw_3_cmd_fifo_ram_dbe    : 1;
	uint64_t sq3_rt_ram_dbe               : 1;
	uint64_t sq3_nt_ram_dbe               : 1;
	uint64_t sq3_pt_ram_dbe               : 1;
#endif
	} s;
	struct cvmx_pse_sq3_ecc_dbe_sts0_s    cn78xx;
};
typedef union cvmx_pse_sq3_ecc_dbe_sts0 cvmx_pse_sq3_ecc_dbe_sts0_t;

/**
 * cvmx_pse_sq3_ecc_dbe_sts_cmb0
 */
union cvmx_pse_sq3_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pse_sq3_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq3_dbe_cmb0             : 1;  /**< Double-bit error for SQ3_PT_RAM. Throws PKO_INTSN_E::PSE_SQ3_DBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq3_dbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pse_sq3_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pse_sq3_ecc_dbe_sts_cmb0 cvmx_pse_sq3_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pse_sq3_ecc_sbe_sts0
 */
union cvmx_pse_sq3_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pse_sq3_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq3_pt_ram_sbe               : 1;  /**< Single-bit error for SQ3_PT_RAM. */
	uint64_t sq3_nt_ram_sbe               : 1;  /**< Single-bit error for SQ3_NT_RAM. */
	uint64_t sq3_rt_ram_sbe               : 1;  /**< Single-bit error for SQ3_RT_RAM. */
	uint64_t sq3_tw_3_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ3_TW_3_CMD_FIFO_RAM. */
	uint64_t sq3_tw_2_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ3_TW_2_CMD_FIFO_RAM. */
	uint64_t sq3_tw_1_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ3_TW_1_CMD_FIFO_RAM. */
	uint64_t sq3_tw_0_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ3_TW_0_CMD_FIFO_RAM. */
	uint64_t sq3_sts3_ram_sbe             : 1;  /**< Single-bit error for SQ3_STS3_RAM. */
	uint64_t sq3_sts2_ram_sbe             : 1;  /**< Single-bit error for SQ3_STS2_RAM. */
	uint64_t sq3_sts1_ram_sbe             : 1;  /**< Single-bit error for SQ3_STS1_RAM. */
	uint64_t sq3_sts0_ram_sbe             : 1;  /**< Single-bit error for SQ3_STS0_RAM. */
	uint64_t sq3_std3_ram_sbe             : 1;  /**< Single-bit error for SQ3_STD3_RAM. */
	uint64_t sq3_std2_ram_sbe             : 1;  /**< Single-bit error for SQ3_STD2_RAM. */
	uint64_t sq3_std1_ram_sbe             : 1;  /**< Single-bit error for SQ3_STD1_RAM. */
	uint64_t sq3_std0_ram_sbe             : 1;  /**< Single-bit error for SQ3_STD0_RAM. */
	uint64_t sq3_wt_ram_sbe               : 1;  /**< Single-bit error for SQ3_WT_RAM. */
	uint64_t reserved_0_47                : 48;
#else
	uint64_t reserved_0_47                : 48;
	uint64_t sq3_wt_ram_sbe               : 1;
	uint64_t sq3_std0_ram_sbe             : 1;
	uint64_t sq3_std1_ram_sbe             : 1;
	uint64_t sq3_std2_ram_sbe             : 1;
	uint64_t sq3_std3_ram_sbe             : 1;
	uint64_t sq3_sts0_ram_sbe             : 1;
	uint64_t sq3_sts1_ram_sbe             : 1;
	uint64_t sq3_sts2_ram_sbe             : 1;
	uint64_t sq3_sts3_ram_sbe             : 1;
	uint64_t sq3_tw_0_cmd_fifo_ram_sbe    : 1;
	uint64_t sq3_tw_1_cmd_fifo_ram_sbe    : 1;
	uint64_t sq3_tw_2_cmd_fifo_ram_sbe    : 1;
	uint64_t sq3_tw_3_cmd_fifo_ram_sbe    : 1;
	uint64_t sq3_rt_ram_sbe               : 1;
	uint64_t sq3_nt_ram_sbe               : 1;
	uint64_t sq3_pt_ram_sbe               : 1;
#endif
	} s;
	struct cvmx_pse_sq3_ecc_sbe_sts0_s    cn78xx;
};
typedef union cvmx_pse_sq3_ecc_sbe_sts0 cvmx_pse_sq3_ecc_sbe_sts0_t;

/**
 * cvmx_pse_sq3_ecc_sbe_sts_cmb0
 */
union cvmx_pse_sq3_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pse_sq3_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq3_sbe_cmb0             : 1;  /**< Single-bit error for SQ3_PT_RAM. Throws PKO_INTSN_E::PSE_SQ3_SBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq3_sbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pse_sq3_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pse_sq3_ecc_sbe_sts_cmb0 cvmx_pse_sq3_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pse_sq4_ecc_ctl0
 */
union cvmx_pse_sq4_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pse_sq4_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq4_pt_ram_flip              : 2;  /**< SQ4_PT_RAM flip syndrome bits on write. */
	uint64_t sq4_pt_ram_cdis              : 1;  /**< SQ4_PT_RAM ECC correction disable. */
	uint64_t sq4_nt_ram_flip              : 2;  /**< SQ4_NT_RAM flip syndrome bits on write. */
	uint64_t sq4_nt_ram_cdis              : 1;  /**< SQ4_NT_RAM ECC correction disable. */
	uint64_t sq4_rt_ram_flip              : 2;  /**< SQ4_RT_RAM flip syndrome bits on write. */
	uint64_t sq4_rt_ram_cdis              : 1;  /**< SQ4_RT_RAM ECC correction disable. */
	uint64_t sq4_tw_3_cmd_fifo_ram_flip   : 2;  /**< SQ4_TW_3_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq4_tw_3_cmd_fifo_ram_cdis   : 1;  /**< SQ4_TW_3_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq4_tw_2_cmd_fifo_ram_flip   : 2;  /**< SQ4_TW_2_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq4_tw_2_cmd_fifo_ram_cdis   : 1;  /**< SQ4_TW_2_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq4_tw_1_cmd_fifo_ram_flip   : 2;  /**< SQ4_TW_1_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq4_tw_1_cmd_fifo_ram_cdis   : 1;  /**< SQ4_TW_1_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq4_tw_0_cmd_fifo_ram_flip   : 2;  /**< SQ4_TW_0_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq4_tw_0_cmd_fifo_ram_cdis   : 1;  /**< SQ4_TW_0_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq4_sts3_ram_flip            : 2;  /**< SQ4_STS3_RAM flip syndrome bits on write. */
	uint64_t sq4_sts3_ram_cdis            : 1;  /**< SQ4_STS3_RAM ECC correction disable. */
	uint64_t sq4_sts2_ram_flip            : 2;  /**< SQ4_STS2_RAM flip syndrome bits on write. */
	uint64_t sq4_sts2_ram_cdis            : 1;  /**< SQ4_STS2_RAM ECC correction disable. */
	uint64_t sq4_sts1_ram_flip            : 2;  /**< SQ4_STS1_RAM flip syndrome bits on write. */
	uint64_t sq4_sts1_ram_cdis            : 1;  /**< SQ4_STS1_RAM ECC correction disable. */
	uint64_t sq4_sts0_ram_flip            : 2;  /**< SQ4_STS0_RAM flip syndrome bits on write. */
	uint64_t sq4_sts0_ram_cdis            : 1;  /**< SQ4_STS0_RAM ECC correction disable. */
	uint64_t sq4_std3_ram_flip            : 2;  /**< SQ4_STD3_RAM flip syndrome bits on write. */
	uint64_t sq4_std3_ram_cdis            : 1;  /**< SQ4_STD3_RAM ECC correction disable. */
	uint64_t sq4_std2_ram_flip            : 2;  /**< SQ4_STD2_RAM flip syndrome bits on write. */
	uint64_t sq4_std2_ram_cdis            : 1;  /**< SQ4_STD2_RAM ECC correction disable. */
	uint64_t sq4_std1_ram_flip            : 2;  /**< SQ4_STD1_RAM flip syndrome bits on write. */
	uint64_t sq4_std1_ram_cdis            : 1;  /**< SQ4_STD1_RAM ECC correction disable. */
	uint64_t sq4_std0_ram_flip            : 2;  /**< SQ4_STD0_RAM flip syndrome bits on write. */
	uint64_t sq4_std0_ram_cdis            : 1;  /**< SQ4_STD0_RAM ECC correction disable. */
	uint64_t sq4_wt_ram_flip              : 2;  /**< SQ4_WT_RAM flip syndrome bits on write. */
	uint64_t sq4_wt_ram_cdis              : 1;  /**< SQ4_WT_RAM ECC correction disable. */
	uint64_t reserved_0_15                : 16;
#else
	uint64_t reserved_0_15                : 16;
	uint64_t sq4_wt_ram_cdis              : 1;
	uint64_t sq4_wt_ram_flip              : 2;
	uint64_t sq4_std0_ram_cdis            : 1;
	uint64_t sq4_std0_ram_flip            : 2;
	uint64_t sq4_std1_ram_cdis            : 1;
	uint64_t sq4_std1_ram_flip            : 2;
	uint64_t sq4_std2_ram_cdis            : 1;
	uint64_t sq4_std2_ram_flip            : 2;
	uint64_t sq4_std3_ram_cdis            : 1;
	uint64_t sq4_std3_ram_flip            : 2;
	uint64_t sq4_sts0_ram_cdis            : 1;
	uint64_t sq4_sts0_ram_flip            : 2;
	uint64_t sq4_sts1_ram_cdis            : 1;
	uint64_t sq4_sts1_ram_flip            : 2;
	uint64_t sq4_sts2_ram_cdis            : 1;
	uint64_t sq4_sts2_ram_flip            : 2;
	uint64_t sq4_sts3_ram_cdis            : 1;
	uint64_t sq4_sts3_ram_flip            : 2;
	uint64_t sq4_tw_0_cmd_fifo_ram_cdis   : 1;
	uint64_t sq4_tw_0_cmd_fifo_ram_flip   : 2;
	uint64_t sq4_tw_1_cmd_fifo_ram_cdis   : 1;
	uint64_t sq4_tw_1_cmd_fifo_ram_flip   : 2;
	uint64_t sq4_tw_2_cmd_fifo_ram_cdis   : 1;
	uint64_t sq4_tw_2_cmd_fifo_ram_flip   : 2;
	uint64_t sq4_tw_3_cmd_fifo_ram_cdis   : 1;
	uint64_t sq4_tw_3_cmd_fifo_ram_flip   : 2;
	uint64_t sq4_rt_ram_cdis              : 1;
	uint64_t sq4_rt_ram_flip              : 2;
	uint64_t sq4_nt_ram_cdis              : 1;
	uint64_t sq4_nt_ram_flip              : 2;
	uint64_t sq4_pt_ram_cdis              : 1;
	uint64_t sq4_pt_ram_flip              : 2;
#endif
	} s;
	struct cvmx_pse_sq4_ecc_ctl0_s        cn78xx;
};
typedef union cvmx_pse_sq4_ecc_ctl0 cvmx_pse_sq4_ecc_ctl0_t;

/**
 * cvmx_pse_sq4_ecc_dbe_sts0
 */
union cvmx_pse_sq4_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pse_sq4_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq4_pt_ram_dbe               : 1;  /**< Double-bit error for SQ4_PT_RAM. */
	uint64_t sq4_nt_ram_dbe               : 1;  /**< Double-bit error for SQ4_NT_RAM. */
	uint64_t sq4_rt_ram_dbe               : 1;  /**< Double-bit error for SQ4_RT_RAM. */
	uint64_t sq4_tw_3_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ4_TW_3_CMD_FIFO_RAM. */
	uint64_t sq4_tw_2_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ4_TW_2_CMD_FIFO_RAM. */
	uint64_t sq4_tw_1_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ4_TW_1_CMD_FIFO_RAM. */
	uint64_t sq4_tw_0_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ4_TW_0_CMD_FIFO_RAM. */
	uint64_t sq4_sts3_ram_dbe             : 1;  /**< Double-bit error for SQ4_STS3_RAM. */
	uint64_t sq4_sts2_ram_dbe             : 1;  /**< Double-bit error for SQ4_STS2_RAM. */
	uint64_t sq4_sts1_ram_dbe             : 1;  /**< Double-bit error for SQ4_STS1_RAM. */
	uint64_t sq4_sts0_ram_dbe             : 1;  /**< Double-bit error for SQ4_STS0_RAM. */
	uint64_t sq4_std3_ram_dbe             : 1;  /**< Double-bit error for SQ4_STD3_RAM. */
	uint64_t sq4_std2_ram_dbe             : 1;  /**< Double-bit error for SQ4_STD2_RAM. */
	uint64_t sq4_std1_ram_dbe             : 1;  /**< Double-bit error for SQ4_STD1_RAM. */
	uint64_t sq4_std0_ram_dbe             : 1;  /**< Double-bit error for SQ4_STD0_RAM. */
	uint64_t sq4_wt_ram_dbe               : 1;  /**< Double-bit error for SQ4_WT_RAM. */
	uint64_t reserved_0_47                : 48;
#else
	uint64_t reserved_0_47                : 48;
	uint64_t sq4_wt_ram_dbe               : 1;
	uint64_t sq4_std0_ram_dbe             : 1;
	uint64_t sq4_std1_ram_dbe             : 1;
	uint64_t sq4_std2_ram_dbe             : 1;
	uint64_t sq4_std3_ram_dbe             : 1;
	uint64_t sq4_sts0_ram_dbe             : 1;
	uint64_t sq4_sts1_ram_dbe             : 1;
	uint64_t sq4_sts2_ram_dbe             : 1;
	uint64_t sq4_sts3_ram_dbe             : 1;
	uint64_t sq4_tw_0_cmd_fifo_ram_dbe    : 1;
	uint64_t sq4_tw_1_cmd_fifo_ram_dbe    : 1;
	uint64_t sq4_tw_2_cmd_fifo_ram_dbe    : 1;
	uint64_t sq4_tw_3_cmd_fifo_ram_dbe    : 1;
	uint64_t sq4_rt_ram_dbe               : 1;
	uint64_t sq4_nt_ram_dbe               : 1;
	uint64_t sq4_pt_ram_dbe               : 1;
#endif
	} s;
	struct cvmx_pse_sq4_ecc_dbe_sts0_s    cn78xx;
};
typedef union cvmx_pse_sq4_ecc_dbe_sts0 cvmx_pse_sq4_ecc_dbe_sts0_t;

/**
 * cvmx_pse_sq4_ecc_dbe_sts_cmb0
 */
union cvmx_pse_sq4_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pse_sq4_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq4_dbe_cmb0             : 1;  /**< Double-bit error for SQ4_PT_RAM. Throws PKO_INTSN_E::PSE_SQ4_DBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq4_dbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pse_sq4_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pse_sq4_ecc_dbe_sts_cmb0 cvmx_pse_sq4_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pse_sq4_ecc_sbe_sts0
 */
union cvmx_pse_sq4_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pse_sq4_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq4_pt_ram_sbe               : 1;  /**< Single-bit error for SQ4_PT_RAM. */
	uint64_t sq4_nt_ram_sbe               : 1;  /**< Single-bit error for SQ4_NT_RAM. */
	uint64_t sq4_rt_ram_sbe               : 1;  /**< Single-bit error for SQ4_RT_RAM. */
	uint64_t sq4_tw_3_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ4_TW_3_CMD_FIFO_RAM. */
	uint64_t sq4_tw_2_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ4_TW_2_CMD_FIFO_RAM. */
	uint64_t sq4_tw_1_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ4_TW_1_CMD_FIFO_RAM. */
	uint64_t sq4_tw_0_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ4_TW_0_CMD_FIFO_RAM. */
	uint64_t sq4_sts3_ram_sbe             : 1;  /**< Single-bit error for SQ4_STS3_RAM. */
	uint64_t sq4_sts2_ram_sbe             : 1;  /**< Single-bit error for SQ4_STS2_RAM. */
	uint64_t sq4_sts1_ram_sbe             : 1;  /**< Single-bit error for SQ4_STS1_RAM. */
	uint64_t sq4_sts0_ram_sbe             : 1;  /**< Single-bit error for SQ4_STS0_RAM. */
	uint64_t sq4_std3_ram_sbe             : 1;  /**< Single-bit error for SQ4_STD3_RAM. */
	uint64_t sq4_std2_ram_sbe             : 1;  /**< Single-bit error for SQ4_STD2_RAM. */
	uint64_t sq4_std1_ram_sbe             : 1;  /**< Single-bit error for SQ4_STD1_RAM. */
	uint64_t sq4_std0_ram_sbe             : 1;  /**< Single-bit error for SQ4_STD0_RAM. */
	uint64_t sq4_wt_ram_sbe               : 1;  /**< Single-bit error for SQ4_WT_RAM. */
	uint64_t reserved_0_47                : 48;
#else
	uint64_t reserved_0_47                : 48;
	uint64_t sq4_wt_ram_sbe               : 1;
	uint64_t sq4_std0_ram_sbe             : 1;
	uint64_t sq4_std1_ram_sbe             : 1;
	uint64_t sq4_std2_ram_sbe             : 1;
	uint64_t sq4_std3_ram_sbe             : 1;
	uint64_t sq4_sts0_ram_sbe             : 1;
	uint64_t sq4_sts1_ram_sbe             : 1;
	uint64_t sq4_sts2_ram_sbe             : 1;
	uint64_t sq4_sts3_ram_sbe             : 1;
	uint64_t sq4_tw_0_cmd_fifo_ram_sbe    : 1;
	uint64_t sq4_tw_1_cmd_fifo_ram_sbe    : 1;
	uint64_t sq4_tw_2_cmd_fifo_ram_sbe    : 1;
	uint64_t sq4_tw_3_cmd_fifo_ram_sbe    : 1;
	uint64_t sq4_rt_ram_sbe               : 1;
	uint64_t sq4_nt_ram_sbe               : 1;
	uint64_t sq4_pt_ram_sbe               : 1;
#endif
	} s;
	struct cvmx_pse_sq4_ecc_sbe_sts0_s    cn78xx;
};
typedef union cvmx_pse_sq4_ecc_sbe_sts0 cvmx_pse_sq4_ecc_sbe_sts0_t;

/**
 * cvmx_pse_sq4_ecc_sbe_sts_cmb0
 */
union cvmx_pse_sq4_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pse_sq4_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq4_sbe_cmb0             : 1;  /**< Single-bit error for SQ4_PT_RAM. Throws PKO_INTSN_E::PSE_SQ4_SBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq4_sbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pse_sq4_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pse_sq4_ecc_sbe_sts_cmb0 cvmx_pse_sq4_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pse_sq5_ecc_ctl0
 */
union cvmx_pse_sq5_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pse_sq5_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq5_pt_ram_flip              : 2;  /**< SQ5_PT_RAM flip syndrome bits on write. */
	uint64_t sq5_pt_ram_cdis              : 1;  /**< SQ5_PT_RAM ECC correction disable. */
	uint64_t sq5_nt_ram_flip              : 2;  /**< SQ5_NT_RAM flip syndrome bits on write. */
	uint64_t sq5_nt_ram_cdis              : 1;  /**< SQ5_NT_RAM ECC correction disable. */
	uint64_t sq5_rt_ram_flip              : 2;  /**< SQ5_RT_RAM flip syndrome bits on write. */
	uint64_t sq5_rt_ram_cdis              : 1;  /**< SQ5_RT_RAM ECC correction disable. */
	uint64_t sq5_tw_3_cmd_fifo_ram_flip   : 2;  /**< SQ5_TW_3_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq5_tw_3_cmd_fifo_ram_cdis   : 1;  /**< SQ5_TW_3_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq5_tw_2_cmd_fifo_ram_flip   : 2;  /**< SQ5_TW_2_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq5_tw_2_cmd_fifo_ram_cdis   : 1;  /**< SQ5_TW_2_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq5_tw_1_cmd_fifo_ram_flip   : 2;  /**< SQ5_TW_1_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq5_tw_1_cmd_fifo_ram_cdis   : 1;  /**< SQ5_TW_1_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq5_tw_0_cmd_fifo_ram_flip   : 2;  /**< SQ5_TW_0_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t sq5_tw_0_cmd_fifo_ram_cdis   : 1;  /**< SQ5_TW_0_CMD_FIFO_RAM ECC correction disable. */
	uint64_t sq5_sts3_ram_flip            : 2;  /**< SQ5_STS3_RAM flip syndrome bits on write. */
	uint64_t sq5_sts3_ram_cdis            : 1;  /**< SQ5_STS3_RAM ECC correction disable. */
	uint64_t sq5_sts2_ram_flip            : 2;  /**< SQ5_STS2_RAM flip syndrome bits on write. */
	uint64_t sq5_sts2_ram_cdis            : 1;  /**< SQ5_STS2_RAM ECC correction disable. */
	uint64_t sq5_sts1_ram_flip            : 2;  /**< SQ5_STS1_RAM flip syndrome bits on write. */
	uint64_t sq5_sts1_ram_cdis            : 1;  /**< SQ5_STS1_RAM ECC correction disable. */
	uint64_t sq5_sts0_ram_flip            : 2;  /**< SQ5_STS0_RAM flip syndrome bits on write. */
	uint64_t sq5_sts0_ram_cdis            : 1;  /**< SQ5_STS0_RAM ECC correction disable. */
	uint64_t sq5_std3_ram_flip            : 2;  /**< SQ5_STD3_RAM flip syndrome bits on write. */
	uint64_t sq5_std3_ram_cdis            : 1;  /**< SQ5_STD3_RAM ECC correction disable. */
	uint64_t sq5_std2_ram_flip            : 2;  /**< SQ5_STD2_RAM flip syndrome bits on write. */
	uint64_t sq5_std2_ram_cdis            : 1;  /**< SQ5_STD2_RAM ECC correction disable. */
	uint64_t sq5_std1_ram_flip            : 2;  /**< SQ5_STD1_RAM flip syndrome bits on write. */
	uint64_t sq5_std1_ram_cdis            : 1;  /**< SQ5_STD1_RAM ECC correction disable. */
	uint64_t sq5_std0_ram_flip            : 2;  /**< SQ5_STD0_RAM flip syndrome bits on write. */
	uint64_t sq5_std0_ram_cdis            : 1;  /**< SQ5_STD0_RAM ECC correction disable. */
	uint64_t sq5_wt_ram_flip              : 2;  /**< SQ5_WT_RAM flip syndrome bits on write. */
	uint64_t sq5_wt_ram_cdis              : 1;  /**< SQ5_WT_RAM ECC correction disable. */
	uint64_t reserved_0_15                : 16;
#else
	uint64_t reserved_0_15                : 16;
	uint64_t sq5_wt_ram_cdis              : 1;
	uint64_t sq5_wt_ram_flip              : 2;
	uint64_t sq5_std0_ram_cdis            : 1;
	uint64_t sq5_std0_ram_flip            : 2;
	uint64_t sq5_std1_ram_cdis            : 1;
	uint64_t sq5_std1_ram_flip            : 2;
	uint64_t sq5_std2_ram_cdis            : 1;
	uint64_t sq5_std2_ram_flip            : 2;
	uint64_t sq5_std3_ram_cdis            : 1;
	uint64_t sq5_std3_ram_flip            : 2;
	uint64_t sq5_sts0_ram_cdis            : 1;
	uint64_t sq5_sts0_ram_flip            : 2;
	uint64_t sq5_sts1_ram_cdis            : 1;
	uint64_t sq5_sts1_ram_flip            : 2;
	uint64_t sq5_sts2_ram_cdis            : 1;
	uint64_t sq5_sts2_ram_flip            : 2;
	uint64_t sq5_sts3_ram_cdis            : 1;
	uint64_t sq5_sts3_ram_flip            : 2;
	uint64_t sq5_tw_0_cmd_fifo_ram_cdis   : 1;
	uint64_t sq5_tw_0_cmd_fifo_ram_flip   : 2;
	uint64_t sq5_tw_1_cmd_fifo_ram_cdis   : 1;
	uint64_t sq5_tw_1_cmd_fifo_ram_flip   : 2;
	uint64_t sq5_tw_2_cmd_fifo_ram_cdis   : 1;
	uint64_t sq5_tw_2_cmd_fifo_ram_flip   : 2;
	uint64_t sq5_tw_3_cmd_fifo_ram_cdis   : 1;
	uint64_t sq5_tw_3_cmd_fifo_ram_flip   : 2;
	uint64_t sq5_rt_ram_cdis              : 1;
	uint64_t sq5_rt_ram_flip              : 2;
	uint64_t sq5_nt_ram_cdis              : 1;
	uint64_t sq5_nt_ram_flip              : 2;
	uint64_t sq5_pt_ram_cdis              : 1;
	uint64_t sq5_pt_ram_flip              : 2;
#endif
	} s;
	struct cvmx_pse_sq5_ecc_ctl0_s        cn78xx;
};
typedef union cvmx_pse_sq5_ecc_ctl0 cvmx_pse_sq5_ecc_ctl0_t;

/**
 * cvmx_pse_sq5_ecc_dbe_sts0
 */
union cvmx_pse_sq5_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pse_sq5_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq5_pt_ram_dbe               : 1;  /**< Double-bit error for SQ5_PT_RAM. */
	uint64_t sq5_nt_ram_dbe               : 1;  /**< Double-bit error for SQ5_NT_RAM. */
	uint64_t sq5_rt_ram_dbe               : 1;  /**< Double-bit error for SQ5_RT_RAM. */
	uint64_t sq5_tw_3_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ5_TW_3_CMD_FIFO_RAM. */
	uint64_t sq5_tw_2_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ5_TW_2_CMD_FIFO_RAM. */
	uint64_t sq5_tw_1_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ5_TW_1_CMD_FIFO_RAM. */
	uint64_t sq5_tw_0_cmd_fifo_ram_dbe    : 1;  /**< Double-bit error for SQ5_TW_0_CMD_FIFO_RAM. */
	uint64_t sq5_sts3_ram_dbe             : 1;  /**< Double-bit error for SQ5_STS3_RAM. */
	uint64_t sq5_sts2_ram_dbe             : 1;  /**< Double-bit error for SQ5_STS2_RAM. */
	uint64_t sq5_sts1_ram_dbe             : 1;  /**< Double-bit error for SQ5_STS1_RAM. */
	uint64_t sq5_sts0_ram_dbe             : 1;  /**< Double-bit error for SQ5_STS0_RAM. */
	uint64_t sq5_std3_ram_dbe             : 1;  /**< Double-bit error for SQ5_STD3_RAM. */
	uint64_t sq5_std2_ram_dbe             : 1;  /**< Double-bit error for SQ5_STD2_RAM. */
	uint64_t sq5_std1_ram_dbe             : 1;  /**< Double-bit error for SQ5_STD1_RAM. */
	uint64_t sq5_std0_ram_dbe             : 1;  /**< Double-bit error for SQ5_STD0_RAM. */
	uint64_t sq5_wt_ram_dbe               : 1;  /**< Double-bit error for SQ5_WT_RAM. */
	uint64_t reserved_0_47                : 48;
#else
	uint64_t reserved_0_47                : 48;
	uint64_t sq5_wt_ram_dbe               : 1;
	uint64_t sq5_std0_ram_dbe             : 1;
	uint64_t sq5_std1_ram_dbe             : 1;
	uint64_t sq5_std2_ram_dbe             : 1;
	uint64_t sq5_std3_ram_dbe             : 1;
	uint64_t sq5_sts0_ram_dbe             : 1;
	uint64_t sq5_sts1_ram_dbe             : 1;
	uint64_t sq5_sts2_ram_dbe             : 1;
	uint64_t sq5_sts3_ram_dbe             : 1;
	uint64_t sq5_tw_0_cmd_fifo_ram_dbe    : 1;
	uint64_t sq5_tw_1_cmd_fifo_ram_dbe    : 1;
	uint64_t sq5_tw_2_cmd_fifo_ram_dbe    : 1;
	uint64_t sq5_tw_3_cmd_fifo_ram_dbe    : 1;
	uint64_t sq5_rt_ram_dbe               : 1;
	uint64_t sq5_nt_ram_dbe               : 1;
	uint64_t sq5_pt_ram_dbe               : 1;
#endif
	} s;
	struct cvmx_pse_sq5_ecc_dbe_sts0_s    cn78xx;
};
typedef union cvmx_pse_sq5_ecc_dbe_sts0 cvmx_pse_sq5_ecc_dbe_sts0_t;

/**
 * cvmx_pse_sq5_ecc_dbe_sts_cmb0
 */
union cvmx_pse_sq5_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pse_sq5_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq5_dbe_cmb0             : 1;  /**< Double-bit error for SQ5_PT_RAM. Throws PKO_INTSN_E::PSE_SQ5_DBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq5_dbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pse_sq5_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pse_sq5_ecc_dbe_sts_cmb0 cvmx_pse_sq5_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pse_sq5_ecc_sbe_sts0
 */
union cvmx_pse_sq5_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pse_sq5_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq5_pt_ram_sbe               : 1;  /**< Single-bit error for SQ5_PT_RAM. */
	uint64_t sq5_nt_ram_sbe               : 1;  /**< Single-bit error for SQ5_NT_RAM. */
	uint64_t sq5_rt_ram_sbe               : 1;  /**< Single-bit error for SQ5_RT_RAM. */
	uint64_t sq5_tw_3_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ5_TW_3_CMD_FIFO_RAM. */
	uint64_t sq5_tw_2_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ5_TW_2_CMD_FIFO_RAM. */
	uint64_t sq5_tw_1_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ5_TW_1_CMD_FIFO_RAM. */
	uint64_t sq5_tw_0_cmd_fifo_ram_sbe    : 1;  /**< Single-bit error for SQ5_TW_0_CMD_FIFO_RAM. */
	uint64_t sq5_sts3_ram_sbe             : 1;  /**< Single-bit error for SQ5_STS3_RAM. */
	uint64_t sq5_sts2_ram_sbe             : 1;  /**< Single-bit error for SQ5_STS2_RAM. */
	uint64_t sq5_sts1_ram_sbe             : 1;  /**< Single-bit error for SQ5_STS1_RAM. */
	uint64_t sq5_sts0_ram_sbe             : 1;  /**< Single-bit error for SQ5_STS0_RAM. */
	uint64_t sq5_std3_ram_sbe             : 1;  /**< Single-bit error for SQ5_STD3_RAM. */
	uint64_t sq5_std2_ram_sbe             : 1;  /**< Single-bit error for SQ5_STD2_RAM. */
	uint64_t sq5_std1_ram_sbe             : 1;  /**< Single-bit error for SQ5_STD1_RAM. */
	uint64_t sq5_std0_ram_sbe             : 1;  /**< Single-bit error for SQ5_STD0_RAM. */
	uint64_t sq5_wt_ram_sbe               : 1;  /**< Single-bit error for SQ5_WT_RAM. */
	uint64_t reserved_0_47                : 48;
#else
	uint64_t reserved_0_47                : 48;
	uint64_t sq5_wt_ram_sbe               : 1;
	uint64_t sq5_std0_ram_sbe             : 1;
	uint64_t sq5_std1_ram_sbe             : 1;
	uint64_t sq5_std2_ram_sbe             : 1;
	uint64_t sq5_std3_ram_sbe             : 1;
	uint64_t sq5_sts0_ram_sbe             : 1;
	uint64_t sq5_sts1_ram_sbe             : 1;
	uint64_t sq5_sts2_ram_sbe             : 1;
	uint64_t sq5_sts3_ram_sbe             : 1;
	uint64_t sq5_tw_0_cmd_fifo_ram_sbe    : 1;
	uint64_t sq5_tw_1_cmd_fifo_ram_sbe    : 1;
	uint64_t sq5_tw_2_cmd_fifo_ram_sbe    : 1;
	uint64_t sq5_tw_3_cmd_fifo_ram_sbe    : 1;
	uint64_t sq5_rt_ram_sbe               : 1;
	uint64_t sq5_nt_ram_sbe               : 1;
	uint64_t sq5_pt_ram_sbe               : 1;
#endif
	} s;
	struct cvmx_pse_sq5_ecc_sbe_sts0_s    cn78xx;
};
typedef union cvmx_pse_sq5_ecc_sbe_sts0 cvmx_pse_sq5_ecc_sbe_sts0_t;

/**
 * cvmx_pse_sq5_ecc_sbe_sts_cmb0
 */
union cvmx_pse_sq5_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pse_sq5_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq5_sbe_cmb0             : 1;  /**< Single-bit error for SQ5_PT_RAM. Throws PKO_INTSN_E::PSE_SQ5_SBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq5_sbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pse_sq5_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pse_sq5_ecc_sbe_sts_cmb0 cvmx_pse_sq5_ecc_sbe_sts_cmb0_t;

#endif
