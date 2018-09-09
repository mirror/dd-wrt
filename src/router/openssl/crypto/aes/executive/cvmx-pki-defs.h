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
 * cvmx-pki-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon pki.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_PKI_DEFS_H__
#define __CVMX_PKI_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_ACTIVE0 CVMX_PKI_ACTIVE0_FUNC()
static inline uint64_t CVMX_PKI_ACTIVE0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_ACTIVE0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000220ull);
}
#else
#define CVMX_PKI_ACTIVE0 (CVMX_ADD_IO_SEG(0x0001180044000220ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_ACTIVE1 CVMX_PKI_ACTIVE1_FUNC()
static inline uint64_t CVMX_PKI_ACTIVE1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_ACTIVE1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000230ull);
}
#else
#define CVMX_PKI_ACTIVE1 (CVMX_ADD_IO_SEG(0x0001180044000230ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_ACTIVE2 CVMX_PKI_ACTIVE2_FUNC()
static inline uint64_t CVMX_PKI_ACTIVE2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_ACTIVE2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000240ull);
}
#else
#define CVMX_PKI_ACTIVE2 (CVMX_ADD_IO_SEG(0x0001180044000240ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_AURAX_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKI_AURAX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044900000ull) + ((offset) & 1023) * 8;
}
#else
#define CVMX_PKI_AURAX_CFG(offset) (CVMX_ADD_IO_SEG(0x0001180044900000ull) + ((offset) & 1023) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_BIST_STATUS0 CVMX_PKI_BIST_STATUS0_FUNC()
static inline uint64_t CVMX_PKI_BIST_STATUS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_BIST_STATUS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000080ull);
}
#else
#define CVMX_PKI_BIST_STATUS0 (CVMX_ADD_IO_SEG(0x0001180044000080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_BIST_STATUS1 CVMX_PKI_BIST_STATUS1_FUNC()
static inline uint64_t CVMX_PKI_BIST_STATUS1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_BIST_STATUS1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000088ull);
}
#else
#define CVMX_PKI_BIST_STATUS1 (CVMX_ADD_IO_SEG(0x0001180044000088ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_BIST_STATUS2 CVMX_PKI_BIST_STATUS2_FUNC()
static inline uint64_t CVMX_PKI_BIST_STATUS2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_BIST_STATUS2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000090ull);
}
#else
#define CVMX_PKI_BIST_STATUS2 (CVMX_ADD_IO_SEG(0x0001180044000090ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_BPIDX_STATE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKI_BPIDX_STATE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044B00000ull) + ((offset) & 1023) * 8;
}
#else
#define CVMX_PKI_BPIDX_STATE(offset) (CVMX_ADD_IO_SEG(0x0001180044B00000ull) + ((offset) & 1023) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_BUF_CTL CVMX_PKI_BUF_CTL_FUNC()
static inline uint64_t CVMX_PKI_BUF_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_BUF_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000100ull);
}
#else
#define CVMX_PKI_BUF_CTL (CVMX_ADD_IO_SEG(0x0001180044000100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CHANX_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 4095)))))
		cvmx_warn("CVMX_PKI_CHANX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044A00000ull) + ((offset) & 4095) * 8;
}
#else
#define CVMX_PKI_CHANX_CFG(offset) (CVMX_ADD_IO_SEG(0x0001180044A00000ull) + ((offset) & 4095) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_CLKEN CVMX_PKI_CLKEN_FUNC()
static inline uint64_t CVMX_PKI_CLKEN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_CLKEN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000410ull);
}
#else
#define CVMX_PKI_CLKEN (CVMX_ADD_IO_SEG(0x0001180044000410ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_ECC_CTL(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PKI_CLX_ECC_CTL(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x000118004400C020ull) + ((block_id) & 3) * 0x10000ull;
}
#else
#define CVMX_PKI_CLX_ECC_CTL(block_id) (CVMX_ADD_IO_SEG(0x000118004400C020ull) + ((block_id) & 3) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_ECC_INT(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PKI_CLX_ECC_INT(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x000118004400C010ull) + ((block_id) & 3) * 0x10000ull;
}
#else
#define CVMX_PKI_CLX_ECC_INT(block_id) (CVMX_ADD_IO_SEG(0x000118004400C010ull) + ((block_id) & 3) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_INT(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PKI_CLX_INT(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x000118004400C000ull) + ((block_id) & 3) * 0x10000ull;
}
#else
#define CVMX_PKI_CLX_INT(block_id) (CVMX_ADD_IO_SEG(0x000118004400C000ull) + ((block_id) & 3) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_PCAMX_ACTIONX(unsigned long a, unsigned long b, unsigned long c)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((a <= 3)) && ((b <= 1)) && ((c <= 191))))))
		cvmx_warn("CVMX_PKI_CLX_PCAMX_ACTIONX(%lu,%lu,%lu) is invalid on this chip\n", a, b, c);
	return CVMX_ADD_IO_SEG(0x0001180044708000ull) + ((a) << 16) + ((b) << 12) + ((c) << 3);
}
#else
#define CVMX_PKI_CLX_PCAMX_ACTIONX(a, b, c) (CVMX_ADD_IO_SEG(0x0001180044708000ull) + ((a) << 16) + ((b) << 12) + ((c) << 3))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_PCAMX_MATCHX(unsigned long a, unsigned long b, unsigned long c)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((a <= 3)) && ((b <= 1)) && ((c <= 191))))))
		cvmx_warn("CVMX_PKI_CLX_PCAMX_MATCHX(%lu,%lu,%lu) is invalid on this chip\n", a, b, c);
	return CVMX_ADD_IO_SEG(0x0001180044704000ull) + ((a) << 16) + ((b) << 12) + ((c) << 3);
}
#else
#define CVMX_PKI_CLX_PCAMX_MATCHX(a, b, c) (CVMX_ADD_IO_SEG(0x0001180044704000ull) + ((a) << 16) + ((b) << 12) + ((c) << 3))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_PCAMX_TERMX(unsigned long a, unsigned long b, unsigned long c)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((a <= 3)) && ((b <= 1)) && ((c <= 191))))))
		cvmx_warn("CVMX_PKI_CLX_PCAMX_TERMX(%lu,%lu,%lu) is invalid on this chip\n", a, b, c);
	return CVMX_ADD_IO_SEG(0x0001180044700000ull) + ((a) << 16) + ((b) << 12) + ((c) << 3);
}
#else
#define CVMX_PKI_CLX_PCAMX_TERMX(a, b, c) (CVMX_ADD_IO_SEG(0x0001180044700000ull) + ((a) << 16) + ((b) << 12) + ((c) << 3))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_PKINDX_CFG(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_PKI_CLX_PKINDX_CFG(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180044300040ull) + (((offset) & 63) + ((block_id) & 3) * 0x100ull) * 256;
}
#else
#define CVMX_PKI_CLX_PKINDX_CFG(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180044300040ull) + (((offset) & 63) + ((block_id) & 3) * 0x100ull) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_PKINDX_KMEMX(unsigned long a, unsigned long b, unsigned long c)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((a <= 3)) && ((b <= 63)) && ((c <= 15))))))
		cvmx_warn("CVMX_PKI_CLX_PKINDX_KMEMX(%lu,%lu,%lu) is invalid on this chip\n", a, b, c);
	return CVMX_ADD_IO_SEG(0x0001180044200000ull) + ((a) << 16) + ((b) << 8) + ((c) << 3);
}
#else
#define CVMX_PKI_CLX_PKINDX_KMEMX(a, b, c) (CVMX_ADD_IO_SEG(0x0001180044200000ull) + ((a) << 16) + ((b) << 8) + ((c) << 3))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_PKINDX_L2_CUSTOM(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_PKI_CLX_PKINDX_L2_CUSTOM(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180044300058ull) + (((offset) & 63) + ((block_id) & 3) * 0x100ull) * 256;
}
#else
#define CVMX_PKI_CLX_PKINDX_L2_CUSTOM(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180044300058ull) + (((offset) & 63) + ((block_id) & 3) * 0x100ull) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_PKINDX_LG_CUSTOM(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_PKI_CLX_PKINDX_LG_CUSTOM(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180044300060ull) + (((offset) & 63) + ((block_id) & 3) * 0x100ull) * 256;
}
#else
#define CVMX_PKI_CLX_PKINDX_LG_CUSTOM(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180044300060ull) + (((offset) & 63) + ((block_id) & 3) * 0x100ull) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_PKINDX_SKIP(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_PKI_CLX_PKINDX_SKIP(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180044300050ull) + (((offset) & 63) + ((block_id) & 3) * 0x100ull) * 256;
}
#else
#define CVMX_PKI_CLX_PKINDX_SKIP(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180044300050ull) + (((offset) & 63) + ((block_id) & 3) * 0x100ull) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_PKINDX_STYLE(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_PKI_CLX_PKINDX_STYLE(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180044300048ull) + (((offset) & 63) + ((block_id) & 3) * 0x100ull) * 256;
}
#else
#define CVMX_PKI_CLX_PKINDX_STYLE(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180044300048ull) + (((offset) & 63) + ((block_id) & 3) * 0x100ull) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_SMEMX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 2047)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_PKI_CLX_SMEMX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180044400000ull) + (((offset) & 2047) + ((block_id) & 3) * 0x2000ull) * 8;
}
#else
#define CVMX_PKI_CLX_SMEMX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180044400000ull) + (((offset) & 2047) + ((block_id) & 3) * 0x2000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_START(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PKI_CLX_START(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x000118004400C030ull) + ((block_id) & 3) * 0x10000ull;
}
#else
#define CVMX_PKI_CLX_START(block_id) (CVMX_ADD_IO_SEG(0x000118004400C030ull) + ((block_id) & 3) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_STYLEX_ALG(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_PKI_CLX_STYLEX_ALG(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180044501000ull) + (((offset) & 63) + ((block_id) & 3) * 0x2000ull) * 8;
}
#else
#define CVMX_PKI_CLX_STYLEX_ALG(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180044501000ull) + (((offset) & 63) + ((block_id) & 3) * 0x2000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_STYLEX_CFG(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_PKI_CLX_STYLEX_CFG(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180044500000ull) + (((offset) & 63) + ((block_id) & 3) * 0x2000ull) * 8;
}
#else
#define CVMX_PKI_CLX_STYLEX_CFG(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180044500000ull) + (((offset) & 63) + ((block_id) & 3) * 0x2000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_STYLEX_CFG2(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_PKI_CLX_STYLEX_CFG2(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180044500800ull) + (((offset) & 63) + ((block_id) & 3) * 0x2000ull) * 8;
}
#else
#define CVMX_PKI_CLX_STYLEX_CFG2(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180044500800ull) + (((offset) & 63) + ((block_id) & 3) * 0x2000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_ECC_CTL0 CVMX_PKI_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PKI_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000060ull);
}
#else
#define CVMX_PKI_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001180044000060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_ECC_CTL1 CVMX_PKI_ECC_CTL1_FUNC()
static inline uint64_t CVMX_PKI_ECC_CTL1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_ECC_CTL1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000068ull);
}
#else
#define CVMX_PKI_ECC_CTL1 (CVMX_ADD_IO_SEG(0x0001180044000068ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_ECC_CTL2 CVMX_PKI_ECC_CTL2_FUNC()
static inline uint64_t CVMX_PKI_ECC_CTL2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_ECC_CTL2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000070ull);
}
#else
#define CVMX_PKI_ECC_CTL2 (CVMX_ADD_IO_SEG(0x0001180044000070ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_ECC_INT0 CVMX_PKI_ECC_INT0_FUNC()
static inline uint64_t CVMX_PKI_ECC_INT0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_ECC_INT0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000040ull);
}
#else
#define CVMX_PKI_ECC_INT0 (CVMX_ADD_IO_SEG(0x0001180044000040ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_ECC_INT1 CVMX_PKI_ECC_INT1_FUNC()
static inline uint64_t CVMX_PKI_ECC_INT1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_ECC_INT1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000048ull);
}
#else
#define CVMX_PKI_ECC_INT1 (CVMX_ADD_IO_SEG(0x0001180044000048ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_ECC_INT2 CVMX_PKI_ECC_INT2_FUNC()
static inline uint64_t CVMX_PKI_ECC_INT2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_ECC_INT2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000050ull);
}
#else
#define CVMX_PKI_ECC_INT2 (CVMX_ADD_IO_SEG(0x0001180044000050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_FRM_LEN_CHKX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PKI_FRM_LEN_CHKX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044004000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_PKI_FRM_LEN_CHKX(offset) (CVMX_ADD_IO_SEG(0x0001180044004000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_GBL_PEN CVMX_PKI_GBL_PEN_FUNC()
static inline uint64_t CVMX_PKI_GBL_PEN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_GBL_PEN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000200ull);
}
#else
#define CVMX_PKI_GBL_PEN (CVMX_ADD_IO_SEG(0x0001180044000200ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_GEN_INT CVMX_PKI_GEN_INT_FUNC()
static inline uint64_t CVMX_PKI_GEN_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_GEN_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000020ull);
}
#else
#define CVMX_PKI_GEN_INT (CVMX_ADD_IO_SEG(0x0001180044000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_ICGX_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_PKI_ICGX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118004400A000ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_PKI_ICGX_CFG(offset) (CVMX_ADD_IO_SEG(0x000118004400A000ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_IMEMX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2047)))))
		cvmx_warn("CVMX_PKI_IMEMX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044100000ull) + ((offset) & 2047) * 8;
}
#else
#define CVMX_PKI_IMEMX(offset) (CVMX_ADD_IO_SEG(0x0001180044100000ull) + ((offset) & 2047) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_LTYPEX_MAP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKI_LTYPEX_MAP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044005000ull) + ((offset) & 31) * 8;
}
#else
#define CVMX_PKI_LTYPEX_MAP(offset) (CVMX_ADD_IO_SEG(0x0001180044005000ull) + ((offset) & 31) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_PCAM_LOOKUP CVMX_PKI_PCAM_LOOKUP_FUNC()
static inline uint64_t CVMX_PKI_PCAM_LOOKUP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_PCAM_LOOKUP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000500ull);
}
#else
#define CVMX_PKI_PCAM_LOOKUP (CVMX_ADD_IO_SEG(0x0001180044000500ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_PCAM_RESULT CVMX_PKI_PCAM_RESULT_FUNC()
static inline uint64_t CVMX_PKI_PCAM_RESULT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_PCAM_RESULT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000510ull);
}
#else
#define CVMX_PKI_PCAM_RESULT (CVMX_ADD_IO_SEG(0x0001180044000510ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_PFE_DIAG CVMX_PKI_PFE_DIAG_FUNC()
static inline uint64_t CVMX_PKI_PFE_DIAG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_PFE_DIAG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000560ull);
}
#else
#define CVMX_PKI_PFE_DIAG (CVMX_ADD_IO_SEG(0x0001180044000560ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_PIX_DIAG CVMX_PKI_PIX_DIAG_FUNC()
static inline uint64_t CVMX_PKI_PIX_DIAG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_PIX_DIAG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000580ull);
}
#else
#define CVMX_PKI_PIX_DIAG (CVMX_ADD_IO_SEG(0x0001180044000580ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_PKINDX_ICGSEL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_PKINDX_ICGSEL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044010000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_PKI_PKINDX_ICGSEL(offset) (CVMX_ADD_IO_SEG(0x0001180044010000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_PKNDX_INB_STAT0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_PKNDX_INB_STAT0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044F00000ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_PKNDX_INB_STAT0(offset) (CVMX_ADD_IO_SEG(0x0001180044F00000ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_PKNDX_INB_STAT1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_PKNDX_INB_STAT1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044F00008ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_PKNDX_INB_STAT1(offset) (CVMX_ADD_IO_SEG(0x0001180044F00008ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_PKNDX_INB_STAT2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_PKNDX_INB_STAT2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044F00010ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_PKNDX_INB_STAT2(offset) (CVMX_ADD_IO_SEG(0x0001180044F00010ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_PKT_ERR CVMX_PKI_PKT_ERR_FUNC()
static inline uint64_t CVMX_PKI_PKT_ERR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_PKT_ERR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000030ull);
}
#else
#define CVMX_PKI_PKT_ERR (CVMX_ADD_IO_SEG(0x0001180044000030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_QPG_TBLX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2047)))))
		cvmx_warn("CVMX_PKI_QPG_TBLX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044800000ull) + ((offset) & 2047) * 8;
}
#else
#define CVMX_PKI_QPG_TBLX(offset) (CVMX_ADD_IO_SEG(0x0001180044800000ull) + ((offset) & 2047) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_REASM_SOPX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PKI_REASM_SOPX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044006000ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_PKI_REASM_SOPX(offset) (CVMX_ADD_IO_SEG(0x0001180044006000ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_REQ_WGT CVMX_PKI_REQ_WGT_FUNC()
static inline uint64_t CVMX_PKI_REQ_WGT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_REQ_WGT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000120ull);
}
#else
#define CVMX_PKI_REQ_WGT (CVMX_ADD_IO_SEG(0x0001180044000120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_SFT_RST CVMX_PKI_SFT_RST_FUNC()
static inline uint64_t CVMX_PKI_SFT_RST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_SFT_RST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000010ull);
}
#else
#define CVMX_PKI_SFT_RST (CVMX_ADD_IO_SEG(0x0001180044000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_HIST0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_HIST0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00000ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_HIST0(offset) (CVMX_ADD_IO_SEG(0x0001180044E00000ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_HIST1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_HIST1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00008ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_HIST1(offset) (CVMX_ADD_IO_SEG(0x0001180044E00008ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_HIST2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_HIST2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00010ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_HIST2(offset) (CVMX_ADD_IO_SEG(0x0001180044E00010ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_HIST3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_HIST3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00018ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_HIST3(offset) (CVMX_ADD_IO_SEG(0x0001180044E00018ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_HIST4(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_HIST4(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00020ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_HIST4(offset) (CVMX_ADD_IO_SEG(0x0001180044E00020ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_HIST5(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_HIST5(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00028ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_HIST5(offset) (CVMX_ADD_IO_SEG(0x0001180044E00028ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_HIST6(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_HIST6(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00030ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_HIST6(offset) (CVMX_ADD_IO_SEG(0x0001180044E00030ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00038ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT0(offset) (CVMX_ADD_IO_SEG(0x0001180044E00038ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00040ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT1(offset) (CVMX_ADD_IO_SEG(0x0001180044E00040ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT10(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT10(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00088ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT10(offset) (CVMX_ADD_IO_SEG(0x0001180044E00088ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT11(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT11(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00090ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT11(offset) (CVMX_ADD_IO_SEG(0x0001180044E00090ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT12(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT12(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00098ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT12(offset) (CVMX_ADD_IO_SEG(0x0001180044E00098ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT13(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT13(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E000A0ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT13(offset) (CVMX_ADD_IO_SEG(0x0001180044E000A0ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT14(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT14(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E000A8ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT14(offset) (CVMX_ADD_IO_SEG(0x0001180044E000A8ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT15(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT15(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E000B0ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT15(offset) (CVMX_ADD_IO_SEG(0x0001180044E000B0ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT16(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT16(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E000B8ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT16(offset) (CVMX_ADD_IO_SEG(0x0001180044E000B8ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT17(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT17(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E000C0ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT17(offset) (CVMX_ADD_IO_SEG(0x0001180044E000C0ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT18(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT18(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E000C8ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT18(offset) (CVMX_ADD_IO_SEG(0x0001180044E000C8ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00048ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT2(offset) (CVMX_ADD_IO_SEG(0x0001180044E00048ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00050ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT3(offset) (CVMX_ADD_IO_SEG(0x0001180044E00050ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT4(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT4(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00058ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT4(offset) (CVMX_ADD_IO_SEG(0x0001180044E00058ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT5(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT5(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00060ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT5(offset) (CVMX_ADD_IO_SEG(0x0001180044E00060ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT6(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT6(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00068ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT6(offset) (CVMX_ADD_IO_SEG(0x0001180044E00068ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT7(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT7(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00070ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT7(offset) (CVMX_ADD_IO_SEG(0x0001180044E00070ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT8(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT8(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00078ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT8(offset) (CVMX_ADD_IO_SEG(0x0001180044E00078ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STATX_STAT9(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STATX_STAT9(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044E00080ull) + ((offset) & 63) * 256;
}
#else
#define CVMX_PKI_STATX_STAT9(offset) (CVMX_ADD_IO_SEG(0x0001180044E00080ull) + ((offset) & 63) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_STAT_CTL CVMX_PKI_STAT_CTL_FUNC()
static inline uint64_t CVMX_PKI_STAT_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_STAT_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000110ull);
}
#else
#define CVMX_PKI_STAT_CTL (CVMX_ADD_IO_SEG(0x0001180044000110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STYLEX_BUF(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STYLEX_BUF(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044024000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_PKI_STYLEX_BUF(offset) (CVMX_ADD_IO_SEG(0x0001180044024000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STYLEX_TAG_MASK(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STYLEX_TAG_MASK(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044021000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_PKI_STYLEX_TAG_MASK(offset) (CVMX_ADD_IO_SEG(0x0001180044021000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STYLEX_TAG_SEL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STYLEX_TAG_SEL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044020000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_PKI_STYLEX_TAG_SEL(offset) (CVMX_ADD_IO_SEG(0x0001180044020000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STYLEX_WQ2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STYLEX_WQ2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044022000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_PKI_STYLEX_WQ2(offset) (CVMX_ADD_IO_SEG(0x0001180044022000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_STYLEX_WQ4(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_PKI_STYLEX_WQ4(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044023000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_PKI_STYLEX_WQ4(offset) (CVMX_ADD_IO_SEG(0x0001180044023000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_TAG_INCX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKI_TAG_INCX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044007000ull) + ((offset) & 31) * 8;
}
#else
#define CVMX_PKI_TAG_INCX_CTL(offset) (CVMX_ADD_IO_SEG(0x0001180044007000ull) + ((offset) & 31) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_TAG_INCX_MASK(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKI_TAG_INCX_MASK(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044008000ull) + ((offset) & 31) * 8;
}
#else
#define CVMX_PKI_TAG_INCX_MASK(offset) (CVMX_ADD_IO_SEG(0x0001180044008000ull) + ((offset) & 31) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_TAG_SECRET CVMX_PKI_TAG_SECRET_FUNC()
static inline uint64_t CVMX_PKI_TAG_SECRET_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_TAG_SECRET not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000430ull);
}
#else
#define CVMX_PKI_TAG_SECRET (CVMX_ADD_IO_SEG(0x0001180044000430ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_X2P_REQ_OFL CVMX_PKI_X2P_REQ_OFL_FUNC()
static inline uint64_t CVMX_PKI_X2P_REQ_OFL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKI_X2P_REQ_OFL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000038ull);
}
#else
#define CVMX_PKI_X2P_REQ_OFL (CVMX_ADD_IO_SEG(0x0001180044000038ull))
#endif

/**
 * cvmx_pki_active0
 */
union cvmx_pki_active0 {
	uint64_t u64;
	struct cvmx_pki_active0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t pfe_active                   : 1;  /**< PFE active. For internal use; software should use PKI_SFT_RST[ACTIVE]. */
#else
	uint64_t pfe_active                   : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_pki_active0_s             cn78xx;
};
typedef union cvmx_pki_active0 cvmx_pki_active0_t;

/**
 * cvmx_pki_active1
 */
union cvmx_pki_active1 {
	uint64_t u64;
	struct cvmx_pki_active1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t fpc_active                   : 1;  /**< PBE FPC and FPA bus active. For internal use; software should use PKI_SFT_RST[ACTIVE]. */
	uint64_t iobp_active                  : 1;  /**< PBE PMW and IOBP bus active. For internal use; software should use PKI_SFT_RST[ACTIVE]. */
	uint64_t sws_active                   : 1;  /**< PBE SWS active. For internal use; software should use PKI_SFT_RST[ACTIVE]. */
	uint64_t pbtag_active                 : 1;  /**< PBE pbtags active. For internal use; software should use PKI_SFT_RST[ACTIVE]. */
#else
	uint64_t pbtag_active                 : 1;
	uint64_t sws_active                   : 1;
	uint64_t iobp_active                  : 1;
	uint64_t fpc_active                   : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_pki_active1_s             cn78xx;
};
typedef union cvmx_pki_active1 cvmx_pki_active1_t;

/**
 * cvmx_pki_active2
 */
union cvmx_pki_active2 {
	uint64_t u64;
	struct cvmx_pki_active2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t pix_active                   : 5;  /**< PIX control and ICG active. For internal use; software should use PKI_SFT_RST[ACTIVE]. */
#else
	uint64_t pix_active                   : 5;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_pki_active2_s             cn78xx;
};
typedef union cvmx_pki_active2 cvmx_pki_active2_t;

/**
 * cvmx_pki_aura#_cfg
 *
 * This register configures aura backpressure, etc; see Backpressure.
 *
 */
union cvmx_pki_aurax_cfg {
	uint64_t u64;
	struct cvmx_pki_aurax_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t pkt_add                      : 2;  /**< Specifies what to add to FPA_AURA(0..1023)_CNT when PKI enqueues a packet:
                                                         0 = zero.
                                                         1 = one.
                                                         2 = The number of FPA buffers allocated; i.e. if PKI_STYLE(0..63)_BUF[DIS_WQ_DAT] is set,
                                                         WQE[BUFS]+1, else WQE[BUFS].
                                                         3 = WQE[LEN] (i.e. the packet length). */
	uint64_t reserved_19_29               : 11;
	uint64_t ena_red                      : 1;  /**< Enable RED drop between PASS and DROP levels. */
	uint64_t ena_drop                     : 1;  /**< Enable tail drop when maximum DROP level exceeded. */
	uint64_t ena_bp                       : 1;  /**< Enable asserting backpressure on BPID when maximum DROP level exceeded. */
	uint64_t reserved_10_15               : 6;
	uint64_t bpid                         : 10; /**< Bpid to assert backpressure on. */
#else
	uint64_t bpid                         : 10;
	uint64_t reserved_10_15               : 6;
	uint64_t ena_bp                       : 1;
	uint64_t ena_drop                     : 1;
	uint64_t ena_red                      : 1;
	uint64_t reserved_19_29               : 11;
	uint64_t pkt_add                      : 2;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pki_aurax_cfg_s           cn78xx;
};
typedef union cvmx_pki_aurax_cfg cvmx_pki_aurax_cfg_t;

/**
 * cvmx_pki_bist_status0
 *
 * BIST status register.
 *
 */
union cvmx_pki_bist_status0 {
	uint64_t u64;
	struct cvmx_pki_bist_status0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t bist                         : 33; /**< BIST results. Hardware sets a bit in BIST for memory that fails BIST. INTERNAL: This
                                                         register collects status for PKI_PFE.
                                                         <32> - INB_ERRS
                                                         <31> - INB OCTS
                                                         <30> - INB PKTS
                                                         <29> - LD FIF
                                                         <28..27> - RD FIF
                                                         <26> - PBE STATE
                                                         <25> - WADR STATE
                                                         <24> - NXT PTAG
                                                         <23> - CUR PTAG
                                                         <22> - X2P FIF
                                                         <21> - DROP FIF
                                                         <20> - NXT BLK
                                                         <19..16> - KMEM
                                                         <15..0> - ASM BUFF */
#else
	uint64_t bist                         : 33;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_pki_bist_status0_s        cn78xx;
};
typedef union cvmx_pki_bist_status0 cvmx_pki_bist_status0_t;

/**
 * cvmx_pki_bist_status1
 *
 * BIST status register.
 *
 */
union cvmx_pki_bist_status1 {
	uint64_t u64;
	struct cvmx_pki_bist_status1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bist                         : 20; /**< BIST results. Hardware sets a bit in BIST for memory that fails BIST. INTERNAL: This
                                                         register collects status for PKI_PBE. */
#else
	uint64_t bist                         : 20;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pki_bist_status1_s        cn78xx;
};
typedef union cvmx_pki_bist_status1 cvmx_pki_bist_status1_t;

/**
 * cvmx_pki_bist_status2
 *
 * BIST status register.
 *
 */
union cvmx_pki_bist_status2 {
	uint64_t u64;
	struct cvmx_pki_bist_status2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t bist                         : 25; /**< BIST results. Hardware sets a bit in BIST for memory that fails BIST. INTERNAL: This
                                                         register collects status for PKI_PIX (verif/vkits/pki/pki_mem_info_table.sv).
                                                         - 24: IMEM
                                                         - 23: IPEC3 / IPEs 10 .. 19 (RegFile + DMEM)
                                                         - 22: IPEC3 / IPEs  0 ..  9 (RegFile + DMEM)
                                                         - 21: IPEC2 / IPEs 10 .. 19 (RegFile + DMEM)
                                                         - 20: IPEC2 / IPEs  0 ..  9 (RegFile + DMEM)
                                                         - 19: IPEC1 / IPEs 10 .. 19 (RegFile + DMEM)
                                                         - 18: IPEC1 / IPEs  0 ..  9 (RegFile + DMEM)
                                                         - 17: IPEC0 / IPEs 10 .. 19 (RegFile + DMEM)
                                                         - 16: IPEC0 / IPEs  0 ..  9 (RegFile + DMEM)
                                                         15..12: IPEC SMEM
                                                         11..8: IPEC PCAM ECC
                                                         7..4: IPEC PCAM RES
                                                         3..0: IPEC PCAM CAM */
#else
	uint64_t bist                         : 25;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_pki_bist_status2_s        cn78xx;
};
typedef union cvmx_pki_bist_status2 cvmx_pki_bist_status2_t;

/**
 * cvmx_pki_bpid#_state
 *
 * This register shows the current bpid state for diagnostics.
 *
 */
union cvmx_pki_bpidx_state {
	uint64_t u64;
	struct cvmx_pki_bpidx_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t xoff                         : 1;  /**< The corresponding bpid is being backpressured. */
#else
	uint64_t xoff                         : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_pki_bpidx_state_s         cn78xx;
};
typedef union cvmx_pki_bpidx_state cvmx_pki_bpidx_state_t;

/**
 * cvmx_pki_buf_ctl
 */
union cvmx_pki_buf_ctl {
	uint64_t u64;
	struct cvmx_pki_buf_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t fpa_wait                     : 1;  /**< Policy when FPA runs out of buffers:
                                                         0 = Drop the remainder of the packet requesting the buffer, and set WQE[OPCODE] to
                                                         RE_MEMOUT.
                                                         1 = Wait until buffers become available, only dropping packets if buffering ahead of PKI
                                                         fills. This may lead to head-of-line blocking of packets on other Auras. */
	uint64_t fpa_cac_dis                  : 1;  /**< When set, disable caching any FPA buffers, and immediately return any cached buffers to the FPA. */
	uint64_t reserved_6_8                 : 3;
	uint64_t pkt_off                      : 1;  /**< Packet buffer off. When this bit is set to 1, the PKI does not buffer the received packet
                                                         data; when it is clear to 0, the PKI works normally, buffering the received packet data. */
	uint64_t reserved_3_4                 : 2;
	uint64_t pbp_en                       : 1;  /**< Bpid enable. When set, enables the sending of bpid level backpressure to the input
                                                         interface.
                                                         The application should not de-assert this bit after asserting it. The receivers of this
                                                         bit may have been put into backpressure mode and can only be released by PKI informing
                                                         them that the backpressure has been released.
                                                         INTERNAL: Must be one for PKI HW to assert any output backpressure wires. */
	uint64_t reserved_1_1                 : 1;
	uint64_t pki_en                       : 1;  /**< PKI enable. When set to 1, enables the operation of the PKI. When clear to 0, the PKI
                                                         asserts backpressure on all ports. INTERNAL: Suppresses grants to X2P, not BPID
                                                         backpressure. */
#else
	uint64_t pki_en                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t pbp_en                       : 1;
	uint64_t reserved_3_4                 : 2;
	uint64_t pkt_off                      : 1;
	uint64_t reserved_6_8                 : 3;
	uint64_t fpa_cac_dis                  : 1;
	uint64_t fpa_wait                     : 1;
	uint64_t reserved_11_63               : 53;
#endif
	} s;
	struct cvmx_pki_buf_ctl_s             cn78xx;
};
typedef union cvmx_pki_buf_ctl cvmx_pki_buf_ctl_t;

/**
 * cvmx_pki_chan#_cfg
 *
 * This register configures each channel.
 *
 */
union cvmx_pki_chanx_cfg {
	uint64_t u64;
	struct cvmx_pki_chanx_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t imp                          : 1;  /**< Implemented. This register is sparse (only indexes with values in PKI_CHAN_E are
                                                         implemented).
                                                         0 = this index is read only.
                                                         1 = this index is read-write.
                                                         INTERNAL: Write to a non-implemented channel is ignored but returns write commit. Reading
                                                         non-implemented channel returns all zero data. */
	uint64_t reserved_10_15               : 6;
	uint64_t bpid                         : 10; /**< Bpid to receive backpressure from. */
#else
	uint64_t bpid                         : 10;
	uint64_t reserved_10_15               : 6;
	uint64_t imp                          : 1;
	uint64_t reserved_17_63               : 47;
#endif
	} s;
	struct cvmx_pki_chanx_cfg_s           cn78xx;
};
typedef union cvmx_pki_chanx_cfg cvmx_pki_chanx_cfg_t;

/**
 * cvmx_pki_cl#_ecc_ctl
 */
union cvmx_pki_clx_ecc_ctl {
	uint64_t u64;
	struct cvmx_pki_clx_ecc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pcam_en                      : 1;  /**< PCAM ECC checking enable. INTERNAL: This enables the PCAM scrubber. */
	uint64_t reserved_24_62               : 39;
	uint64_t pcam1_flip                   : 2;  /**< PCAM1 flip syndrome bits on write. */
	uint64_t pcam0_flip                   : 2;  /**< PCAM  flip syndrome bits on write. */
	uint64_t smem_flip                    : 2;  /**< SMEM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the KMEM ram to
                                                         test single-bit or double-bit error handling. */
	uint64_t dmem_flip                    : 1;  /**< DMEM flip parity. */
	uint64_t rf_flip                      : 1;  /**< RF flip parity. */
	uint64_t reserved_5_15                : 11;
	uint64_t pcam1_cdis                   : 1;  /**< PCAM1 ECC correction disable. */
	uint64_t pcam0_cdis                   : 1;  /**< PCAM0 ECC correction disable. */
	uint64_t smem_cdis                    : 1;  /**< SMEM ECC correction disable. */
	uint64_t dmem_cdis                    : 1;  /**< DMEM parity poising disable. */
	uint64_t rf_cdis                      : 1;  /**< RF RAM parity poising disable. */
#else
	uint64_t rf_cdis                      : 1;
	uint64_t dmem_cdis                    : 1;
	uint64_t smem_cdis                    : 1;
	uint64_t pcam0_cdis                   : 1;
	uint64_t pcam1_cdis                   : 1;
	uint64_t reserved_5_15                : 11;
	uint64_t rf_flip                      : 1;
	uint64_t dmem_flip                    : 1;
	uint64_t smem_flip                    : 2;
	uint64_t pcam0_flip                   : 2;
	uint64_t pcam1_flip                   : 2;
	uint64_t reserved_24_62               : 39;
	uint64_t pcam_en                      : 1;
#endif
	} s;
	struct cvmx_pki_clx_ecc_ctl_s         cn78xx;
};
typedef union cvmx_pki_clx_ecc_ctl cvmx_pki_clx_ecc_ctl_t;

/**
 * cvmx_pki_cl#_ecc_int
 */
union cvmx_pki_clx_ecc_int {
	uint64_t u64;
	struct cvmx_pki_clx_ecc_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t pcam1_dbe                    : 1;  /**< PCAM1 ECC double bit error. Throws PKI_INTSN_E::PKI_CL(0..3)_ECC_PCAM1_DBE. */
	uint64_t pcam1_sbe                    : 1;  /**< PCAM1 ECC single bit error. Throws PKI_INTSN_E::PKI_CL(0..3)_ECC_PCAM1_SBE. */
	uint64_t pcam0_dbe                    : 1;  /**< PCAM0 ECC double bit error. Throws PKI_INTSN_E::PKI_CL(0..3)_ECC_PCAM0_DBE. */
	uint64_t pcam0_sbe                    : 1;  /**< PCAM0 ECC single bit error. Throws PKI_INTSN_E::PKI_CL(0..3)_ECC_PCAM0_SBE. */
	uint64_t smem_dbe                     : 1;  /**< SMEM ECC double bit error. Throws PKI_INTSN_E::PKI_CL(0..3)_ECC_SMEM_DBE. */
	uint64_t smem_sbe                     : 1;  /**< SMEM ECC single bit error. Throws PKI_INTSN_E::PKI_CL(0..3)_ECC_SMEM_SBE. */
	uint64_t dmem_perr                    : 1;  /**< DMEM parity error. Throws PKI_INTSN_E::PKI_CL(0..3)_ECC_DMEM_PERR. */
	uint64_t rf_perr                      : 1;  /**< RF RAM parity error. Throws PKI_INTSN_E::PKI_CL(0..3)_ECC_RF_PERR. */
#else
	uint64_t rf_perr                      : 1;
	uint64_t dmem_perr                    : 1;
	uint64_t smem_sbe                     : 1;
	uint64_t smem_dbe                     : 1;
	uint64_t pcam0_sbe                    : 1;
	uint64_t pcam0_dbe                    : 1;
	uint64_t pcam1_sbe                    : 1;
	uint64_t pcam1_dbe                    : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_pki_clx_ecc_int_s         cn78xx;
};
typedef union cvmx_pki_clx_ecc_int cvmx_pki_clx_ecc_int_t;

/**
 * cvmx_pki_cl#_int
 */
union cvmx_pki_clx_int {
	uint64_t u64;
	struct cvmx_pki_clx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t trapz                        : 1;  /**< PCAM sequencer trapz interrupt. Throws PKI_INTSN_E::PKI_CL(0..3)_INT_TRAPZ. INTERNAL:
                                                         Caused by TRAP sequence state, may indicate PKI enabled without proper sequencer code
                                                         loaded in PKI_IMEM(0..2047). */
	uint64_t iptint                       : 1;  /**< PCAM sequencer debug interrupt. Throws PKI_INTSN_E::PKI_CL(0..3)_INT_IPTINT. INTERNAL:
                                                         Caused by TRAP or INTR sequence state. */
	uint64_t sched_conf                   : 1;  /**< PCAM/SMEM internal port conflict. Internal error, should not occur. Throws
                                                         PKI_INTSN_E::PKI_CL(0..3)_INT_SCHED_CONF. INTERNAL: Sequencer mis-scheduled PCAM or SMEM
                                                         ops with overlapping accesses. */
	uint64_t pcam_conf                    : 2;  /**< PCAM(0..1) match hit multiple rows, indicating a programming error in
                                                         PKI_CL(0..3)_PCAM(0..1)_MATCH(0..191) or related registers. Throws
                                                         PKI_INTSN_E::PKI_CL(0..3)_INT_PCAM_CONF(0..1). */
#else
	uint64_t pcam_conf                    : 2;
	uint64_t sched_conf                   : 1;
	uint64_t iptint                       : 1;
	uint64_t trapz                        : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_pki_clx_int_s             cn78xx;
};
typedef union cvmx_pki_clx_int cvmx_pki_clx_int_t;

/**
 * cvmx_pki_cl#_pcam#_action#
 *
 * If multiple PCAM entries hit, the PKI_GEN_INT[PCAMERR] error interrupt is signaled, and it is
 * unpredictable which PCAM action register will be used.
 */
union cvmx_pki_clx_pcamx_actionx {
	uint64_t u64;
	struct cvmx_pki_clx_pcamx_actionx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t pmc                          : 7;  /**< Parse Mode Change. Where to resume parsing after applying the scan offset (if any) as bit
                                                         mask of which sequence steps to no longer process:
                                                         <0> = LA (L2)
                                                         <1> = LB (Custom)
                                                         <2> = LC (L3)
                                                         <3> = LD (Virt)
                                                         <4> = LE (IL3)
                                                         <5> = LF (L4)
                                                         <6> = LG (Custom/Application)
                                                         The legal values are:
                                                         0x0 = no change in parsing
                                                         0x1 = Skip further LA parsing; start LB parsing.
                                                         For TERM==L2_CUSTOM only)
                                                         0x3 = Skip further LA/LB parsing; start LC parsing.
                                                         For TERMs through Ethertypes only)
                                                         0x7 = Skip further LA-LC parsing; start LD parsing.
                                                         For TERMs through L3FLAGS only)
                                                         0x3F = Skip further LA-LF parsing; start LG parsing.
                                                         For TERMs through IL3FLAGS only)
                                                         0x7F = Skip all parsing; no further packet inspection.
                                                         For TERMs through L3FLAGS only)
                                                         For example an Ethertype match action that wishes to resume with additional Ethertype
                                                         matches would use a zero PMC to indicate no parse mode change. An Ethertype match action
                                                         that wishes to not parse any additional Ethertypes and resume at LC would use 0x3.
                                                         Must be zero for invalid entries, or for TERMs that do not allow a parse mode change as
                                                         specified in the PKI_PCAM_TERM_E table.
                                                         See also Parse Mode. */
	uint64_t style_add                    : 8;  /**< Resulting interim style adder. If this CAM entry matches, the value to add to the current
                                                         style (may wrap around through 256). See Styles. Must be zero for invalid entries. */
	uint64_t pf                           : 3;  /**< Parse flag to set. Specifies the parse flag to set when entry matches, see PCAM actions
                                                         may also specify setting one of four user flags in the generated work queue entry,
                                                         WQE[PF1] through WQE[PF4]. These flags are not used by hardware; they indicate to software
                                                         that exceptional handling may be required, such as the presence of an encrypted packet.:
                                                         0x0 = no change.
                                                         0x1 = Set WQE[PF1].
                                                         0x2 = Set WQE[PF2].
                                                         0x3 = Set WQE[PF3].
                                                         0x4 = Set WQE[PF4].
                                                         else = reserved.
                                                         Must be zero for invalid entries. */
	uint64_t setty                        : 5;  /**< Set pointer type. If non-zero, indicates the layer type to be set as described under
                                                         PKI_PCAM_TERM_E. Values are enumerated in PKI_LTYPE_E. Must be zero for invalid entries. */
	uint64_t advance                      : 8;  /**< Relative number of bytes to advance scan pointer when entry matches. See Parser Skip and
                                                         Advancing. Must be even. Must be zero for invalid entries and for TERMs that do not allow
                                                         an advance as specified in the PKI_PCAM_TERM_E table. */
#else
	uint64_t advance                      : 8;
	uint64_t setty                        : 5;
	uint64_t pf                           : 3;
	uint64_t style_add                    : 8;
	uint64_t pmc                          : 7;
	uint64_t reserved_31_63               : 33;
#endif
	} s;
	struct cvmx_pki_clx_pcamx_actionx_s   cn78xx;
};
typedef union cvmx_pki_clx_pcamx_actionx cvmx_pki_clx_pcamx_actionx_t;

/**
 * cvmx_pki_cl#_pcam#_match#
 */
union cvmx_pki_clx_pcamx_matchx {
	uint64_t u64;
	struct cvmx_pki_clx_pcamx_matchx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data1                        : 32; /**< See DATA0. */
	uint64_t data0                        : 32; /**< The packet data to compare against. Bits may be ignored in comparison based on [DATA0] and
                                                         [DATA1]. If the entry matches, PKI_CL(0..3)_PCAM(0..1)_ACTION(0..191) will determine the
                                                         action to be taken. See PKI_PCAM_TERM_E for comparison bit definitions.
                                                         The field value is ternary, where each bit matches as follows:
                                                         DATA1<n> DATA0<n>
                                                         0 0 Always match; data<n> don't care.
                                                         0 1 Match when data<n> == 0.
                                                         1 0 Match when data<n> == 1.
                                                         1 1 Reserved. */
#else
	uint64_t data0                        : 32;
	uint64_t data1                        : 32;
#endif
	} s;
	struct cvmx_pki_clx_pcamx_matchx_s    cn78xx;
};
typedef union cvmx_pki_clx_pcamx_matchx cvmx_pki_clx_pcamx_matchx_t;

/**
 * cvmx_pki_cl#_pcam#_term#
 */
union cvmx_pki_clx_pcamx_termx {
	uint64_t u64;
	struct cvmx_pki_clx_pcamx_termx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t valid                        : 1;  /**< Valid. */
	uint64_t reserved_48_62               : 15;
	uint64_t term1                        : 8;  /**< See TERM0. */
	uint64_t style1                       : 8;  /**< See STYLE0. */
	uint64_t reserved_16_31               : 16;
	uint64_t term0                        : 8;  /**< Comparison type. Enumerated with PKI_PCAM_TERM_E. The field value is ternary, where each
                                                         bit matches as follows:
                                                         TERM1<n> TERM0<n>
                                                         0 0 Always match; data<n> don't care.
                                                         0 1 Match when data<n> == 0.
                                                         1 0 Match when data<n> == 1.
                                                         1 1 Reserved. */
	uint64_t style0                       : 8;  /**< Previous interim style. The style that must have been calculated by the port
                                                         PKI_CL(0..3)_PKIND(0..63)_STYLE[STYLE] or as modified by previous CAM hits's
                                                         PKI_CL(0..3)_PCAM(0..1)_ACTION(0..191)[STYLE]. This is used to form AND style matches; see
                                                         Styles.
                                                         The field value is ternary, where each bit matches as follows:
                                                         STYLE1<n> STYLE0<n>
                                                         0 0 Always match; data<n> don't care.
                                                         0 1 Match when data<n> == 0.
                                                         1 0 Match when data<n> == 1.
                                                         1 1 Reserved. */
#else
	uint64_t style0                       : 8;
	uint64_t term0                        : 8;
	uint64_t reserved_16_31               : 16;
	uint64_t style1                       : 8;
	uint64_t term1                        : 8;
	uint64_t reserved_48_62               : 15;
	uint64_t valid                        : 1;
#endif
	} s;
	struct cvmx_pki_clx_pcamx_termx_s     cn78xx;
};
typedef union cvmx_pki_clx_pcamx_termx cvmx_pki_clx_pcamx_termx_t;

/**
 * cvmx_pki_cl#_pkind#_cfg
 */
union cvmx_pki_clx_pkindx_cfg {
	uint64_t u64;
	struct cvmx_pki_clx_pkindx_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t fcs_pres                     : 1;  /**< FCS present.
                                                         0 = FCS not present. FCS may not be checked nor stripped.
                                                         1 = FCS present; the last four bytes of the packet are part of the FCS and may not be
                                                         considered part of a IP, TCP or other header for length error checks.
                                                         PKI_CL(0..3)_STYLE(0..63)_CFG[FCS_CHK or FCS_STRIP] may optionally be set. */
	uint64_t mpls_en                      : 1;  /**< Enable MPLS parsing.
                                                         0 = Any MPLS labels are ignored, but may be handled by custom Ethertype PCAM matchers.
                                                         1 = MPLS label stacks are parsed and skipped over. PKI_GBL_PEN[MPLS_PEN] must be set. */
	uint64_t inst_hdr                     : 1;  /**< INST header. When set, the eight-byte INST_HDR is present on all packets (except incoming
                                                         packets on the DPI ports). */
	uint64_t lg_custom                    : 1;  /**< Layer G Custom Match Enable.
                                                         0 = Disable custom LG header extraction
                                                         1 = Enable custom LG header extraction.
                                                         PKI_GBL_PEN[CLG_PEN] must be set. */
	uint64_t fulc_en                      : 1;  /**< Enable Fulcrum tag parsing.
                                                         0 = Any Fulcrum header is ignored.
                                                         1 = Fulcrum header is parsed. PKI_GBL_PEN[FULC_PEN] must be set.
                                                         At most one of FULC_EN, DSA_EN or HG_EN may be set. */
	uint64_t dsa_en                       : 1;  /**< Enable DSA parsing. This field should not be set for DPI ports.
                                                         0 = Any DSA header is ignored.
                                                         1 = DSA is parsed. PKI_GBL_PEN[DSA_PEN] must be set.
                                                         At most one of FULC_EN, DSA_EN or HG_EN may be set. */
	uint64_t hg2_en                       : 1;  /**< Enable HiGig 2 parsing. This field should not be set for DPI ports.
                                                         0 = Any HiGig2 header is ignored.
                                                         1 = HiGig2 is parsed. PKI_GBL_PEN[HG_PEN] must be set. */
	uint64_t hg_en                        : 1;  /**< Enable HiGig parsing. This field should not be set for DPI ports.
                                                         0 = Any HiGig header is ignored.
                                                         1 = HiGig is parsed. PKI_GBL_PEN[HG_PEN] must be set.
                                                         At most one of FULC_EN, DSA_EN or HG_EN may be set. */
#else
	uint64_t hg_en                        : 1;
	uint64_t hg2_en                       : 1;
	uint64_t dsa_en                       : 1;
	uint64_t fulc_en                      : 1;
	uint64_t lg_custom                    : 1;
	uint64_t inst_hdr                     : 1;
	uint64_t mpls_en                      : 1;
	uint64_t fcs_pres                     : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_pki_clx_pkindx_cfg_s      cn78xx;
};
typedef union cvmx_pki_clx_pkindx_cfg cvmx_pki_clx_pkindx_cfg_t;

/**
 * cvmx_pki_cl#_pkind#_kmem#
 *
 * The register initialization value for each PKIND and register number (plus 32 or 48 based on
 * PKI_ICG(0..3)_CFG[MLO]). The other PKI_PKND* registers alias inside regions of
 * PKI_CL(0..3)_PKIND(0..63)_KMEM(0..15). To avoid confusing tools, these aliases have address
 * bit 20 set; the PKI address decoder ignores bit 20 when accessing
 * PKI_CL(0..3)_PKIND(0..63)_KMEM(0..15).
 */
union cvmx_pki_clx_pkindx_kmemx {
	uint64_t u64;
	struct cvmx_pki_clx_pkindx_kmemx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t data                         : 16; /**< RAM data at given address. */
#else
	uint64_t data                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_pki_clx_pkindx_kmemx_s    cn78xx;
};
typedef union cvmx_pki_clx_pkindx_kmemx cvmx_pki_clx_pkindx_kmemx_t;

/**
 * cvmx_pki_cl#_pkind#_l2_custom
 */
union cvmx_pki_clx_pkindx_l2_custom {
	uint64_t u64;
	struct cvmx_pki_clx_pkindx_l2_custom_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t valid                        : 1;  /**< Valid.
                                                         0 = Disable custom L2 header extraction.
                                                         1 = Enable custom L2 header extraction.
                                                         PKI_GBL_PEN[CLG_PEN] must be set. */
	uint64_t reserved_8_14                : 7;
	uint64_t offset                       : 8;  /**< Scan offset. Pointer to first byte of 32-bit custom extraction header, as absolute number
                                                         of bytes from beginning of packet. If PTP_MODE, the 8-byte timestamp is prepended to the
                                                         packet, and must be included in counting offset bytes. */
#else
	uint64_t offset                       : 8;
	uint64_t reserved_8_14                : 7;
	uint64_t valid                        : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_pki_clx_pkindx_l2_custom_s cn78xx;
};
typedef union cvmx_pki_clx_pkindx_l2_custom cvmx_pki_clx_pkindx_l2_custom_t;

/**
 * cvmx_pki_cl#_pkind#_lg_custom
 */
union cvmx_pki_clx_pkindx_lg_custom {
	uint64_t u64;
	struct cvmx_pki_clx_pkindx_lg_custom_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t offset                       : 8;  /**< Scan offset. Pointer to first byte of 32-bit custom extraction header, as relative number
                                                         of bytes from WQE[LFPTR]. */
#else
	uint64_t offset                       : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_pki_clx_pkindx_lg_custom_s cn78xx;
};
typedef union cvmx_pki_clx_pkindx_lg_custom cvmx_pki_clx_pkindx_lg_custom_t;

/**
 * cvmx_pki_cl#_pkind#_skip
 */
union cvmx_pki_clx_pkindx_skip {
	uint64_t u64;
	struct cvmx_pki_clx_pkindx_skip_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t fcs_skip                     : 8;  /**< Skip amount from front of packet to first byte covered by FCS start. The skip must be
                                                         even. If PTP_MODE, the 8-byte timestamp is prepended to the packet, and FCS_SKIP must be
                                                         at least 8. */
	uint64_t inst_skip                    : 8;  /**< Skip amount from front of packet to begin parsing at. If
                                                         PKI_CL(0..3)_PKIND(0..63)_CFG[INST_HDR] is set, points at the first byte of the
                                                         instruction header. If INST_HDR is clear, points at the first byte to begin parsing at.
                                                         The skip must be even. If PTP_MODE, the 8-byte timestamp is prepended to the packet, and
                                                         INST_SKIP must be at least 8. */
#else
	uint64_t inst_skip                    : 8;
	uint64_t fcs_skip                     : 8;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_pki_clx_pkindx_skip_s     cn78xx;
};
typedef union cvmx_pki_clx_pkindx_skip cvmx_pki_clx_pkindx_skip_t;

/**
 * cvmx_pki_cl#_pkind#_style
 */
union cvmx_pki_clx_pkindx_style {
	uint64_t u64;
	struct cvmx_pki_clx_pkindx_style_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t pm                           : 7;  /**< Initial parse mode. Bit mask of which sequence steps to perform, refer to Parse Mode:
                                                         <0> = LA (L2)
                                                         <1> = LB (Custom)
                                                         <2> = LC (L3)
                                                         <3> = LD (L4 Virt)
                                                         <4> = LE (IL3)
                                                         <5> = LF (L4)
                                                         <6> = LG (Custom/Application)
                                                         The legal values are:
                                                         0x0 =   Parse LA..LG
                                                         0x1 =   Parse LB..LG
                                                         0x3 =   Parse LC..LG
                                                         0x3F = Parse LG
                                                         0x7F = Parse nothing
                                                         else = Reserved */
	uint64_t style                        : 8;  /**< Initial style. Initial style number for packets on this port, will remain as final style
                                                         if no PCAM entries match the packet. Note only 64 final styles exist, the upper two bits
                                                         will only be used for PCAM matching. */
#else
	uint64_t style                        : 8;
	uint64_t pm                           : 7;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_pki_clx_pkindx_style_s    cn78xx;
};
typedef union cvmx_pki_clx_pkindx_style cvmx_pki_clx_pkindx_style_t;

/**
 * cvmx_pki_cl#_smem#
 *
 * PKI_STYLE* registers alias inside regions of PKI_CL(0..3)_SMEM(0..2047). To avoid confusing
 * tools, these aliases have address bit 20 set; the PKI address decoder ignores bit 20 when
 * accessing PKI_CL(0..3)_SMEM(0..2047).
 */
union cvmx_pki_clx_smemx {
	uint64_t u64;
	struct cvmx_pki_clx_smemx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t data                         : 32; /**< RAM data at given address. */
#else
	uint64_t data                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pki_clx_smemx_s           cn78xx;
};
typedef union cvmx_pki_clx_smemx cvmx_pki_clx_smemx_t;

/**
 * cvmx_pki_cl#_start
 */
union cvmx_pki_clx_start {
	uint64_t u64;
	struct cvmx_pki_clx_start_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t start                        : 11; /**< Cluster start offset. <1:0> must be zero. For diagnostic use only. */
#else
	uint64_t start                        : 11;
	uint64_t reserved_11_63               : 53;
#endif
	} s;
	struct cvmx_pki_clx_start_s           cn78xx;
};
typedef union cvmx_pki_clx_start cvmx_pki_clx_start_t;

/**
 * cvmx_pki_cl#_style#_alg
 */
union cvmx_pki_clx_stylex_alg {
	uint64_t u64;
	struct cvmx_pki_clx_stylex_alg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t tt                           : 2;  /**< SSO tag type to schedule to, enumerated with SSO_TT_E. */
	uint64_t apad_nip                     : 3;  /**< Value for WQE[APAD] when packet is not IP. */
	uint64_t qpg_qos                      : 3;  /**< Algorithm to select QoS field in QPG calculation. Enumerated with PKI_QPGQOS_E. See QPG. */
	uint64_t qpg_port_sh                  : 3;  /**< Number of bits to shift port number in QPG calculation. See QPG. */
	uint64_t qpg_port_msb                 : 4;  /**< MSB to take from port number in QPG calculation. See QPG.
                                                         0 = Exclude port number from QPG.
                                                         4 = Include port<3:0>.
                                                         8 = Include port<7:0>.
                                                         else Reserved. */
	uint64_t reserved_11_16               : 6;
	uint64_t tag_vni                      : 1;  /**< When VXLAN is found, include VNI in tag generation. When NVGRE is found, include TNI. */
	uint64_t tag_gtp                      : 1;  /**< When GTP is parsed, include GTP's TEID in tag generation. */
	uint64_t tag_spi                      : 1;  /**< Include AH/GRE in tag generation.
                                                         0 = Exclude AH/GRE in tag generation.
                                                         1 = If IP SEC AH is parsed, include AH SPI field in tag generation, or if GRE found,
                                                         include GRE call number in tag generation. */
	uint64_t tag_syn                      : 1;  /**< Exclude source address for TCP SYN packets.
                                                         0 = Include source address if TAG_SRC_* used.
                                                         1 = Exclude source address for TCP packets with SYN & !ACK, even if TAG_SRC_* set. */
	uint64_t tag_pctl                     : 1;  /**< Include final IPv4 protocol or IPv6 next header in tag generation. */
	uint64_t tag_vs1                      : 1;  /**< When VLAN stacking is parsed, include second (network order) VLAN in tuple tag generation. */
	uint64_t tag_vs0                      : 1;  /**< When VLAN stacking is parsed, include first (network order) VLAN in tuple tag generation. */
	uint64_t tag_vlan                     : 1;  /**< Reserved. */
	uint64_t tag_mpls0                    : 1;  /**< When a MPLS label is parsed, include the top label in tag generation. */
	uint64_t tag_prt                      : 1;  /**< Include interface port in tag hash. */
	uint64_t wqe_vs                       : 1;  /**< Which VLAN to put into WQE[VLPTR] when VLAN stacking.
                                                         0 = Use the first (in network order) VLAN or DSA VID.
                                                         1 = Use the second (in network order) VLAN. */
#else
	uint64_t wqe_vs                       : 1;
	uint64_t tag_prt                      : 1;
	uint64_t tag_mpls0                    : 1;
	uint64_t tag_vlan                     : 1;
	uint64_t tag_vs0                      : 1;
	uint64_t tag_vs1                      : 1;
	uint64_t tag_pctl                     : 1;
	uint64_t tag_syn                      : 1;
	uint64_t tag_spi                      : 1;
	uint64_t tag_gtp                      : 1;
	uint64_t tag_vni                      : 1;
	uint64_t reserved_11_16               : 6;
	uint64_t qpg_port_msb                 : 4;
	uint64_t qpg_port_sh                  : 3;
	uint64_t qpg_qos                      : 3;
	uint64_t apad_nip                     : 3;
	uint64_t tt                           : 2;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pki_clx_stylex_alg_s      cn78xx;
};
typedef union cvmx_pki_clx_stylex_alg cvmx_pki_clx_stylex_alg_t;

/**
 * cvmx_pki_cl#_style#_cfg
 */
union cvmx_pki_clx_stylex_cfg {
	uint64_t u64;
	struct cvmx_pki_clx_stylex_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t ip6_udp_opt                  : 1;  /**< IPv6/UDP checksum is optional. IPv4 allows an optional UDP checksum by sending the all-0s
                                                         patterns. IPv6 outlaws this and the spec says to always check UDP checksum.
                                                         0 = Spec compliant, do not allow optional code.
                                                         1 = Treat IPv6 as IPv4; the all-0s pattern will cause a UDP checksum pass. */
	uint64_t lenerr_en                    : 1;  /**< L2 length error check enable. Check if frame was received with L2 length error. This check
                                                         is typically not enabled for incoming packets on the DPI ports. INTERNAL: Sequencer clears
                                                         this bit for PKI_BE when SNAP length checks are not appropriate. */
	uint64_t lenerr_eqpad                 : 1;  /**< L2 length checks exact pad size.
                                                         0 = Length check uses greater then or equal comparison. Packets must have at least minimum
                                                         padding, but may have more. This mode must be used when there may be extra Etherypes
                                                         including VLAN tags.
                                                         1 = Length check uses equal comparison. Packets must have the exact padding necessary to
                                                         insure a minimum frame size and no more. */
	uint64_t minmax_sel                   : 1;  /**< Selects which PKI_FRM_LEN_CHK(0..1) register is used for this pkind for MINERR and MAXERR
                                                         checks.
                                                         0 = use PKI_FRM_LEN_CHK0
                                                         1 = use PKI_FRM_LEN_CHK1 */
	uint64_t maxerr_en                    : 1;  /**< Max frame error check enable. */
	uint64_t minerr_en                    : 1;  /**< Min frame error check enable. This check is typically not enabled for incoming packets on
                                                         the DPI ports. */
	uint64_t qpg_dis_grptag               : 1;  /**< Disable computing group using WQE[TAG]. */
	uint64_t fcs_strip                    : 1;  /**< Strip L2 FCS bytes from packet, decrease WQE[LEN] by 4 bytes.
                                                         PKI_CL(0..3)_PKIND(0..63)_CFG[FCS_PRES] must be set. */
	uint64_t fcs_chk                      : 1;  /**< FCS checking enabled. PKI_CL(0..3)_PKIND(0..63)_CFG[FCS_PRES] must be set. */
	uint64_t rawdrp                       : 1;  /**< Allow RAW packet drop.
                                                         0 = Never drop packets with WQE[RAW] set.
                                                         1 = Allow the PKI to drop RAW packets based on PKI_AURA(0..1023)_CFG[ENA_RED/ENA_DROP]. */
	uint64_t drop                         : 1;  /**< Force packet dropping.
                                                         0 = Drop packet based on PKI_AURA(0..1023)_CFG[ENA_RED/ENA_DROP].
                                                         1 = Always drop the packet. Overrides NODROP, RAWDRP. */
	uint64_t nodrop                       : 1;  /**< Disable QoS packet drop.
                                                         0 = Allowed to drop packet based on PKI_AURA(0..1023)_CFG[ENA_RED/ENA_DROP].
                                                         1 = Never drop the packet. Overrides RAWDRP. */
	uint64_t qpg_dis_padd                 : 1;  /**< Disable computing port adder by QPG algorithm. */
	uint64_t qpg_dis_grp                  : 1;  /**< Disable computing group by QPG algorithm. */
	uint64_t qpg_dis_aura                 : 1;  /**< Disable computing aura by QPG algorithm. */
	uint64_t reserved_11_15               : 5;
	uint64_t qpg_base                     : 11; /**< Base index into PKI_QPG_TBL(0..2047). INTERNAL: Sequencer starts with QPG_BASE, performs
                                                         the QPG calculation and packs the resulting QPG index back into this field for PKI_BE_S. */
#else
	uint64_t qpg_base                     : 11;
	uint64_t reserved_11_15               : 5;
	uint64_t qpg_dis_aura                 : 1;
	uint64_t qpg_dis_grp                  : 1;
	uint64_t qpg_dis_padd                 : 1;
	uint64_t nodrop                       : 1;
	uint64_t drop                         : 1;
	uint64_t rawdrp                       : 1;
	uint64_t fcs_chk                      : 1;
	uint64_t fcs_strip                    : 1;
	uint64_t qpg_dis_grptag               : 1;
	uint64_t minerr_en                    : 1;
	uint64_t maxerr_en                    : 1;
	uint64_t minmax_sel                   : 1;
	uint64_t lenerr_eqpad                 : 1;
	uint64_t lenerr_en                    : 1;
	uint64_t ip6_udp_opt                  : 1;
	uint64_t reserved_31_63               : 33;
#endif
	} s;
	struct cvmx_pki_clx_stylex_cfg_s      cn78xx;
};
typedef union cvmx_pki_clx_stylex_cfg cvmx_pki_clx_stylex_cfg_t;

/**
 * cvmx_pki_cl#_style#_cfg2
 */
union cvmx_pki_clx_stylex_cfg2 {
	uint64_t u64;
	struct cvmx_pki_clx_stylex_cfg2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t tag_inc                      : 4;  /**< Include masked tags using PKI_TAG_INC(0..31)_MASK. Each bit indicates to include the
                                                         corresponding PKI_TAG_INC_MASK range, see PKI_INST_HDR_S. */
	uint64_t reserved_25_27               : 3;
	uint64_t tag_masken                   : 1;  /**< Apply PKI_STYLE(0..63)_TAG_MASK to computed tag. INTERNAL: Sequencer must clear for PKI BE
                                                         when the tag comes from the PKI_INST_HDR_S. */
	uint64_t tag_src_lg                   : 1;  /**< Include Layer G source address in tuple tag generation. */
	uint64_t tag_src_lf                   : 1;  /**< Include Layer F source address in tuple tag generation. */
	uint64_t tag_src_le                   : 1;  /**< Include Layer E source address in tuple tag generation. */
	uint64_t tag_src_ld                   : 1;  /**< Include Layer D source address in tuple tag generation. */
	uint64_t tag_src_lc                   : 1;  /**< Include Layer C source address in tuple tag generation. */
	uint64_t tag_src_lb                   : 1;  /**< Include Layer B source address in tuple tag generation. INTERNAL: Sequencer must clear
                                                         TAG_SRC_L* for PKI BE when TCP SYNs are not tagged, or when the tag comes from the
                                                         PKI_INST_HDR_S. */
	uint64_t tag_dst_lg                   : 1;  /**< Include Layer G destination address in tuple tag generation. */
	uint64_t tag_dst_lf                   : 1;  /**< Include Layer F destination address in tuple tag generation. */
	uint64_t tag_dst_le                   : 1;  /**< Include Layer E destination address in tuple tag generation. */
	uint64_t tag_dst_ld                   : 1;  /**< Include Layer D destination address in tuple tag generation. */
	uint64_t tag_dst_lc                   : 1;  /**< Include Layer C destination address in tuple tag generation. */
	uint64_t tag_dst_lb                   : 1;  /**< Include Layer B destination address in tuple tag generation. INTERNAL: Sequencer must
                                                         clear TAG_SRC_L* for PKI BE when the tag comes from the PKI_INST_HDR_S. */
	uint64_t len_lg                       : 1;  /**< Check length of Layer G. */
	uint64_t len_lf                       : 1;  /**< Check length of Layer F. */
	uint64_t len_le                       : 1;  /**< Check length of Layer E. */
	uint64_t len_ld                       : 1;  /**< Check length of Layer D. */
	uint64_t len_lc                       : 1;  /**< Check length of Layer C. */
	uint64_t len_lb                       : 1;  /**< Check length of Layer B. */
	uint64_t csum_lg                      : 1;  /**< Compute checksum on Layer G. */
	uint64_t csum_lf                      : 1;  /**< Compute checksum on Layer F. */
	uint64_t csum_le                      : 1;  /**< Compute checksum on Layer E. */
	uint64_t csum_ld                      : 1;  /**< Compute checksum on Layer D. */
	uint64_t csum_lc                      : 1;  /**< Compute checksum on Layer C. */
	uint64_t csum_lb                      : 1;  /**< Compute checksum on Layer B. */
#else
	uint64_t csum_lb                      : 1;
	uint64_t csum_lc                      : 1;
	uint64_t csum_ld                      : 1;
	uint64_t csum_le                      : 1;
	uint64_t csum_lf                      : 1;
	uint64_t csum_lg                      : 1;
	uint64_t len_lb                       : 1;
	uint64_t len_lc                       : 1;
	uint64_t len_ld                       : 1;
	uint64_t len_le                       : 1;
	uint64_t len_lf                       : 1;
	uint64_t len_lg                       : 1;
	uint64_t tag_dst_lb                   : 1;
	uint64_t tag_dst_lc                   : 1;
	uint64_t tag_dst_ld                   : 1;
	uint64_t tag_dst_le                   : 1;
	uint64_t tag_dst_lf                   : 1;
	uint64_t tag_dst_lg                   : 1;
	uint64_t tag_src_lb                   : 1;
	uint64_t tag_src_lc                   : 1;
	uint64_t tag_src_ld                   : 1;
	uint64_t tag_src_le                   : 1;
	uint64_t tag_src_lf                   : 1;
	uint64_t tag_src_lg                   : 1;
	uint64_t tag_masken                   : 1;
	uint64_t reserved_25_27               : 3;
	uint64_t tag_inc                      : 4;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pki_clx_stylex_cfg2_s     cn78xx;
};
typedef union cvmx_pki_clx_stylex_cfg2 cvmx_pki_clx_stylex_cfg2_t;

/**
 * cvmx_pki_clken
 */
union cvmx_pki_clken {
	uint64_t u64;
	struct cvmx_pki_clken_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t clken                        : 1;  /**< Controls the conditional clocking within PKI
                                                         0 = Allow hardware to control the clocks.
                                                         1 = Force the clocks to be always on. */
#else
	uint64_t clken                        : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_pki_clken_s               cn78xx;
};
typedef union cvmx_pki_clken cvmx_pki_clken_t;

/**
 * cvmx_pki_ecc_ctl0
 *
 * This register allows inserting ECC errors for testing.
 *
 */
union cvmx_pki_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pki_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t ldfif_flip                   : 2;  /**< LDFIF RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the KMEM ram
                                                         to test single-bit or double-bit error handling. */
	uint64_t ldfif_cdis                   : 1;  /**< LDFIF RAM ECC correction disable. */
	uint64_t rdfif_flip                   : 2;  /**< RDFIF RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the KMEM ram
                                                         to test single-bit or double-bit error handling. */
	uint64_t rdfif_cdis                   : 1;  /**< RDFIF RAM ECC correction disable. */
	uint64_t wadr_flip                    : 2;  /**< WADR RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the KMEM ram
                                                         to test single-bit or double-bit error handling. */
	uint64_t wadr_cdis                    : 1;  /**< WADR RAM ECC correction disable. */
	uint64_t nxtptag_flip                 : 2;  /**< NXTPTAG RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the KMEM
                                                         ram to test single-bit or double-bit error handling. */
	uint64_t nxtptag_cdis                 : 1;  /**< NXTPTAG RAM ECC correction disable. */
	uint64_t curptag_flip                 : 2;  /**< CURPTAG RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the KMEM
                                                         ram to test single-bit or double-bit error handling. */
	uint64_t curptag_cdis                 : 1;  /**< CURPTAG RAM ECC correction disable. */
	uint64_t nxtblk_flip                  : 2;  /**< NXTBLK RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the KMEM ram
                                                         to test single-bit or double-bit error handling. */
	uint64_t nxtblk_cdis                  : 1;  /**< NXTBLK RAM ECC correction disable. */
	uint64_t kmem_flip                    : 2;  /**< KMEM RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the KMEM ram
                                                         to test single-bit or double-bit error handling. */
	uint64_t kmem_cdis                    : 1;  /**< KMEM RAM ECC correction disable. */
	uint64_t asm_flip                     : 2;  /**< ASM RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the KMEM ram to
                                                         test single-bit or double-bit error handling. */
	uint64_t asm_cdis                     : 1;  /**< ASM RAM ECC correction disable. */
#else
	uint64_t asm_cdis                     : 1;
	uint64_t asm_flip                     : 2;
	uint64_t kmem_cdis                    : 1;
	uint64_t kmem_flip                    : 2;
	uint64_t nxtblk_cdis                  : 1;
	uint64_t nxtblk_flip                  : 2;
	uint64_t curptag_cdis                 : 1;
	uint64_t curptag_flip                 : 2;
	uint64_t nxtptag_cdis                 : 1;
	uint64_t nxtptag_flip                 : 2;
	uint64_t wadr_cdis                    : 1;
	uint64_t wadr_flip                    : 2;
	uint64_t rdfif_cdis                   : 1;
	uint64_t rdfif_flip                   : 2;
	uint64_t ldfif_cdis                   : 1;
	uint64_t ldfif_flip                   : 2;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_pki_ecc_ctl0_s            cn78xx;
};
typedef union cvmx_pki_ecc_ctl0 cvmx_pki_ecc_ctl0_t;

/**
 * cvmx_pki_ecc_ctl1
 *
 * This register allows inserting ECC errors for testing.
 *
 */
union cvmx_pki_ecc_ctl1 {
	uint64_t u64;
	struct cvmx_pki_ecc_ctl1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t pktwq_flip                   : 2;  /**< PKTWQ flip syndrome bits on write. */
	uint64_t pktwq_cdis                   : 1;  /**< PKTWQ ECC correction disable. */
	uint64_t reserved_18_23               : 6;
	uint64_t tag_flip                     : 2;  /**< TAG flip syndrome bits on write. */
	uint64_t tag_cdis                     : 1;  /**< TAG ECC correction disable. */
	uint64_t aura_flip                    : 2;  /**< AURA flip syndrome bits on write. */
	uint64_t aura_cdis                    : 1;  /**< AURA ECC correction disable. */
	uint64_t chan_flip                    : 2;  /**< CHAN flip syndrome bits on write. */
	uint64_t chan_cdis                    : 1;  /**< CHAN ECC correction disable. */
	uint64_t pbtag_flip                   : 2;  /**< PBTAG flip syndrome bits on write. */
	uint64_t pbtag_cdis                   : 1;  /**< PBTAG ECC correction disable. */
	uint64_t stylewq_flip                 : 2;  /**< STYLEWQ flip syndrome bits on write. */
	uint64_t stylewq_cdis                 : 1;  /**< STYLEWQ ECC correction disable. */
	uint64_t qpg_flip                     : 2;  /**< QPG flip syndrome bits on write. */
	uint64_t qpg_cdis                     : 1;  /**< QPG ECC correction disable. */
#else
	uint64_t qpg_cdis                     : 1;
	uint64_t qpg_flip                     : 2;
	uint64_t stylewq_cdis                 : 1;
	uint64_t stylewq_flip                 : 2;
	uint64_t pbtag_cdis                   : 1;
	uint64_t pbtag_flip                   : 2;
	uint64_t chan_cdis                    : 1;
	uint64_t chan_flip                    : 2;
	uint64_t aura_cdis                    : 1;
	uint64_t aura_flip                    : 2;
	uint64_t tag_cdis                     : 1;
	uint64_t tag_flip                     : 2;
	uint64_t reserved_18_23               : 6;
	uint64_t pktwq_cdis                   : 1;
	uint64_t pktwq_flip                   : 2;
	uint64_t reserved_27_63               : 37;
#endif
	} s;
	struct cvmx_pki_ecc_ctl1_s            cn78xx;
};
typedef union cvmx_pki_ecc_ctl1 cvmx_pki_ecc_ctl1_t;

/**
 * cvmx_pki_ecc_ctl2
 *
 * This register allows inserting ECC errors for testing.
 *
 */
union cvmx_pki_ecc_ctl2 {
	uint64_t u64;
	struct cvmx_pki_ecc_ctl2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t imem_flip                    : 2;  /**< KMEM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the KMEM ram to
                                                         test single-bit or double-bit error handling. */
	uint64_t imem_cdis                    : 1;  /**< IMEM ECC correction disable. */
#else
	uint64_t imem_cdis                    : 1;
	uint64_t imem_flip                    : 2;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_pki_ecc_ctl2_s            cn78xx;
};
typedef union cvmx_pki_ecc_ctl2 cvmx_pki_ecc_ctl2_t;

/**
 * cvmx_pki_ecc_int0
 */
union cvmx_pki_ecc_int0 {
	uint64_t u64;
	struct cvmx_pki_ecc_int0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t ldfif_dbe                    : 1;  /**< LDFIF ECC double bit error. Throws PKI_INTSN_E::PKI_ECC0_LDFIF_DBE. */
	uint64_t ldfif_sbe                    : 1;  /**< LDFIF ECC single bit error. Throws PKI_INTSN_E::PKI_ECC0_LDFIF_SBE. */
	uint64_t rdfif_dbe                    : 1;  /**< RDFIF ECC double bit error. Throws PKI_INTSN_E::PKI_ECC0_RDFIF_DBE. */
	uint64_t rdfif_sbe                    : 1;  /**< RDFIF ECC single bit error. Throws PKI_INTSN_E::PKI_ECC0_RDFIF_SBE. */
	uint64_t wadr_dbe                     : 1;  /**< WADR ECC double bit error. Throws PKI_INTSN_E::PKI_ECC0_WADR_DBE. */
	uint64_t wadr_sbe                     : 1;  /**< WADR ECC single bit error. Throws PKI_INTSN_E::PKI_ECC0_WADR_SBE. */
	uint64_t nxtptag_dbe                  : 1;  /**< NXTPTAG ECC double bit error. Throws PKI_INTSN_E::PKI_ECC0_NXTPTAG_DBE. */
	uint64_t nxtptag_sbe                  : 1;  /**< NXTPTAG ECC single bit error. Throws PKI_INTSN_E::PKI_ECC0_NXTPTAG_SBE. */
	uint64_t curptag_dbe                  : 1;  /**< CURPTAG ECC double bit error. Throws PKI_INTSN_E::PKI_ECC0_CURPTAG_DBE. */
	uint64_t curptag_sbe                  : 1;  /**< CURPTAG ECC single bit error. Throws PKI_INTSN_E::PKI_ECC0_CURPTAG_SBE. */
	uint64_t nxtblk_dbe                   : 1;  /**< NXTBLK ECC double bit error. Throws PKI_INTSN_E::PKI_ECC0_NXTBLK_DBE. */
	uint64_t nxtblk_sbe                   : 1;  /**< NXTBLK ECC single bit error. Throws PKI_INTSN_E::PKI_ECC0_NXTBLK_SBE. */
	uint64_t kmem_dbe                     : 1;  /**< KMEM ECC double bit error. Throws PKI_INTSN_E::PKI_ECC0_KMEM_DBE. */
	uint64_t kmem_sbe                     : 1;  /**< KMEM ECC single bit error. Throws PKI_INTSN_E::PKI_ECC0_KMEM_SBE. */
	uint64_t asm_dbe                      : 1;  /**< ASM ECC double bit error. Throws PKI_INTSN_E::PKI_ECC0_ASM_DBE. */
	uint64_t asm_sbe                      : 1;  /**< ASM ECC single bit error. Throws PKI_INTSN_E::PKI_ECC0_ASM_SBE. */
#else
	uint64_t asm_sbe                      : 1;
	uint64_t asm_dbe                      : 1;
	uint64_t kmem_sbe                     : 1;
	uint64_t kmem_dbe                     : 1;
	uint64_t nxtblk_sbe                   : 1;
	uint64_t nxtblk_dbe                   : 1;
	uint64_t curptag_sbe                  : 1;
	uint64_t curptag_dbe                  : 1;
	uint64_t nxtptag_sbe                  : 1;
	uint64_t nxtptag_dbe                  : 1;
	uint64_t wadr_sbe                     : 1;
	uint64_t wadr_dbe                     : 1;
	uint64_t rdfif_sbe                    : 1;
	uint64_t rdfif_dbe                    : 1;
	uint64_t ldfif_sbe                    : 1;
	uint64_t ldfif_dbe                    : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_pki_ecc_int0_s            cn78xx;
};
typedef union cvmx_pki_ecc_int0 cvmx_pki_ecc_int0_t;

/**
 * cvmx_pki_ecc_int1
 */
union cvmx_pki_ecc_int1 {
	uint64_t u64;
	struct cvmx_pki_ecc_int1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t pktwq_dbe                    : 1;  /**< PKTWQ ECC double bit error. Throws PKI_INTSN_E::PKI_ECC1_PKTWQ_DBE. */
	uint64_t pktwq_sbe                    : 1;  /**< PKTWQ ECC single bit error. Throws PKI_INTSN_E::PKI_ECC1_PKTWQ_SBE. */
	uint64_t reserved_12_15               : 4;
	uint64_t tag_dbe                      : 1;  /**< TAG ECC double bit error. Throws PKI_INTSN_E::PKI_ECC1_TAG_DBE. */
	uint64_t tag_sbe                      : 1;  /**< TAG ECC single bit error. Throws PKI_INTSN_E::PKI_ECC1_TAG_SBE. */
	uint64_t aura_dbe                     : 1;  /**< AURA ECC double bit error. Throws PKI_INTSN_E::PKI_ECC1_AURA_DBE. */
	uint64_t aura_sbe                     : 1;  /**< AURA ECC single bit error. Throws PKI_INTSN_E::PKI_ECC1_AURA_SBE. */
	uint64_t chan_dbe                     : 1;  /**< CHAN ECC double bit error. Throws PKI_INTSN_E::PKI_ECC1_CHAN_DBE. */
	uint64_t chan_sbe                     : 1;  /**< CHAN ECC single bit error. Throws PKI_INTSN_E::PKI_ECC1_CHAN_SBE. */
	uint64_t pbtag_dbe                    : 1;  /**< PBTAG ECC double bit error. Throws PKI_INTSN_E::PKI_ECC1_PBTAG_DBE. */
	uint64_t pbtag_sbe                    : 1;  /**< PBTAG ECC single bit error. Throws PKI_INTSN_E::PKI_ECC1_PBTAG_SBE. */
	uint64_t stylewq_dbe                  : 1;  /**< STYLEWQ ECC double bit error. Throws PKI_INTSN_E::PKI_ECC1_STYLEWQ_DBE. */
	uint64_t stylewq_sbe                  : 1;  /**< STYLEWQ ECC single bit error. Throws PKI_INTSN_E::PKI_ECC1_STYLEWQ_SBE. */
	uint64_t qpg_dbe                      : 1;  /**< QPG ECC double bit error. Throws PKI_INTSN_E::PKI_ECC1_QPG_DBE. */
	uint64_t qpg_sbe                      : 1;  /**< QPG ECC single bit error. Throws PKI_INTSN_E::PKI_ECC1_QPG_SBE. */
#else
	uint64_t qpg_sbe                      : 1;
	uint64_t qpg_dbe                      : 1;
	uint64_t stylewq_sbe                  : 1;
	uint64_t stylewq_dbe                  : 1;
	uint64_t pbtag_sbe                    : 1;
	uint64_t pbtag_dbe                    : 1;
	uint64_t chan_sbe                     : 1;
	uint64_t chan_dbe                     : 1;
	uint64_t aura_sbe                     : 1;
	uint64_t aura_dbe                     : 1;
	uint64_t tag_sbe                      : 1;
	uint64_t tag_dbe                      : 1;
	uint64_t reserved_12_15               : 4;
	uint64_t pktwq_sbe                    : 1;
	uint64_t pktwq_dbe                    : 1;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_pki_ecc_int1_s            cn78xx;
};
typedef union cvmx_pki_ecc_int1 cvmx_pki_ecc_int1_t;

/**
 * cvmx_pki_ecc_int2
 */
union cvmx_pki_ecc_int2 {
	uint64_t u64;
	struct cvmx_pki_ecc_int2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t imem_dbe                     : 1;  /**< IMEM ECC double bit error. Throws PKI_INTSN_E::PKI_ECC2_IMEM_DBE. */
	uint64_t imem_sbe                     : 1;  /**< IMEM ECC single bit error. Throws PKI_INTSN_E::PKI_ECC2_IMEM_SBE. */
#else
	uint64_t imem_sbe                     : 1;
	uint64_t imem_dbe                     : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_pki_ecc_int2_s            cn78xx;
};
typedef union cvmx_pki_ecc_int2 cvmx_pki_ecc_int2_t;

/**
 * cvmx_pki_frm_len_chk#
 */
union cvmx_pki_frm_len_chkx {
	uint64_t u64;
	struct cvmx_pki_frm_len_chkx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t maxlen                       : 16; /**< Byte count for max-sized frame check. */
	uint64_t minlen                       : 16; /**< Byte count for min-sized frame check. */
#else
	uint64_t minlen                       : 16;
	uint64_t maxlen                       : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pki_frm_len_chkx_s        cn78xx;
};
typedef union cvmx_pki_frm_len_chkx cvmx_pki_frm_len_chkx_t;

/**
 * cvmx_pki_gbl_pen
 */
union cvmx_pki_gbl_pen {
	uint64_t u64;
	struct cvmx_pki_gbl_pen_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t virt_pen                     : 1;  /**< Virtualization parsing enable.
                                                         0 = VXLAN/NVGRE is never used in any style. This enables internal power and latency
                                                         reductions.
                                                         1 = VXLAN/NVGRE parsing may be used. */
	uint64_t clg_pen                      : 1;  /**< Custom LG parsing enable.
                                                         0 = Custom LG is never used in any style; i.e. PKI_CL(0..3)_PKIND(0..63)_CFG[LG_CUSTOM] is
                                                         zero for all indices. This enables internal power and latency reductions.
                                                         1 = Custom LG parsing may be used. */
	uint64_t cl2_pen                      : 1;  /**< Custom L2 parsing enable.
                                                         0 = Custom L2 is never used in any style; i.e. PKI_CL(0..3)_PKIND(0..63)_L2_CUSTOM[VALID]
                                                         is zero for all indices. This enables internal power and latency reductions.
                                                         1 = Custom L2 parsing may be used. */
	uint64_t l4_pen                       : 1;  /**< L4 parsing enable.
                                                         0 = L4 parsing is never used in any style. This enables internal power and latency
                                                         reductions.
                                                         1 = L4 parsing may be used. */
	uint64_t il3_pen                      : 1;  /**< L3 inner parsing enable. Must be zero if L3_PEN is zero.
                                                         0 = L3 inner parsing is never used in any style. This enables internal power and latency
                                                         reductions.
                                                         1 = L3 inner (IP-in-IP) parsing may be used. */
	uint64_t l3_pen                       : 1;  /**< L3 parsing enable.
                                                         0 = L3 parsing is never used in any style. This enables internal power and latency
                                                         reductions.
                                                         1 = L3 parsing may be used. */
	uint64_t mpls_pen                     : 1;  /**< MPLS parsing enable.
                                                         0 = MPLS parsing is never used in any style; i.e. PKI_CL(0..3)_PKIND(0..63)_CFG[MPLS_EN]
                                                         is zero for all indices. This enables internal power and latency reductions.
                                                         1 = MPLS parsing may be used. */
	uint64_t fulc_pen                     : 1;  /**< Fulcrum parsing enable.
                                                         0 = Fulcrum parsing is never used in any style; i.e.
                                                         PKI_CL(0..3)_PKIND(0..63)_CFG[FULC_EN] is zero for all indices. This enables internal
                                                         power and latency reductions.
                                                         1 = Fulcrum parsing may be used. */
	uint64_t dsa_pen                      : 1;  /**< DSA parsing enable. Must be zero if HG_PEN is set.
                                                         0 = DSA parsing is never used in any style; i.e. PKI_CL(0..3)_PKIND(0..63)_CFG[DSA_EN] is
                                                         zero for all indices. This enables internal power and latency reductions.
                                                         1 = DSA parsing may be used. */
	uint64_t hg_pen                       : 1;  /**< HiGig parsing enable. Must be zero if DSA_PEN is set.
                                                         0 = HiGig parsing is never used in any style; i.e. PKI_CL(0..3)_PKIND(0..63)_CFG[HG2_EN,
                                                         HG_EN] are zero for all indices. This enables internal power and latency reductions.
                                                         1 = HiGig parsing may be used. */
#else
	uint64_t hg_pen                       : 1;
	uint64_t dsa_pen                      : 1;
	uint64_t fulc_pen                     : 1;
	uint64_t mpls_pen                     : 1;
	uint64_t l3_pen                       : 1;
	uint64_t il3_pen                      : 1;
	uint64_t l4_pen                       : 1;
	uint64_t cl2_pen                      : 1;
	uint64_t clg_pen                      : 1;
	uint64_t virt_pen                     : 1;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_pki_gbl_pen_s             cn78xx;
};
typedef union cvmx_pki_gbl_pen cvmx_pki_gbl_pen_t;

/**
 * cvmx_pki_gen_int
 */
union cvmx_pki_gen_int {
	uint64_t u64;
	struct cvmx_pki_gen_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t x2p_req_ofl                  : 1;  /**< Set when a device attempts to have more than the allocated requests outstanding to PKI.
                                                         Throws PKI_INTSN_E::PKI_GEN_X2P_REQ_OFL. */
	uint64_t drp_noavail                  : 1;  /**< Set when packet dropped due to no FPA pointers available for the aura the packet
                                                         requested. Throws PKI_INTSN_E::PKI_GEN_DRP_NOAVAIL. */
	uint64_t dat                          : 1;  /**< Set when data arrives before a SOP for the same reasm-id for a packet. The first detected
                                                         error associated with bits [DAT,SOP,EOP] of this register is only set here. A new bit can
                                                         be set when the previous reported bit is cleared. Throws PKI_INTSN_E::PKI_GEN_DAT. */
	uint64_t eop                          : 1;  /**< Set when an EOP is followed by an EOP for the same reasm-id for a packet. The first
                                                         detected error associated with bits [DAT,EOP,SOP] of this register is only set here. A new
                                                         bit can be set when the previous reported bit is cleared. Also see PKI_PKT_ERR. Throws
                                                         PKI_INTSN_E::PKI_GEN_EOP. */
	uint64_t sop                          : 1;  /**< Set when a SOP is followed by an SOP for the same reasm-id for a packet. The first
                                                         detected error associated with bits [DAT,EOP,SOP] of this register is only set here. A new
                                                         bit can be set when the previous reported bit is cleared. Also see PKI_PKT_ERR. Throws
                                                         PKI_INTSN_E::PKI_GEN_SOP. */
	uint64_t bckprs                       : 1;  /**< PKI asserted backpressure. Set when PKI was unable to accept the next valid data from
                                                         BGX/DPI/ILK etc. over X2P due to all internal resources being used up, and PKI will
                                                         backpressure X2P. Throws PKI_INTSN_E::PKI_GEN_BCKPRS. */
	uint64_t crcerr                       : 1;  /**< PKI calculated bad CRC. If the packet arrived via a BGX interface, the packet had an FCS
                                                         error. If the packet arrived via PKO internal loopback, the packet had one or more parity
                                                         errors. Not applicable when the packet arrived via the DPI interface. For ILK interfaces,
                                                         the following ILK errors can cause packets to terminate with this error code:
                                                         SERDES_LOCK_LOSS, BDRY_SYNC_LOSS, SCRM_SYNC_LOSS, LANE_ALIGN_FAIL, DESKEW_FIFO_OVFL,
                                                         CRC24_ERR, UKWN_CNTL_WORD, and BAD_64B67B. Throws PKI_INTSN_E::PKI_GEN_CRCERR. */
	uint64_t pktdrp                       : 1;  /**< Packet dropped due to QOS. If the QOS algorithm decides to drop a packet, PKI asserts this
                                                         interrupt. Throws PKI_INTSN_E::PKI_GEN_PKTDRP. */
#else
	uint64_t pktdrp                       : 1;
	uint64_t crcerr                       : 1;
	uint64_t bckprs                       : 1;
	uint64_t sop                          : 1;
	uint64_t eop                          : 1;
	uint64_t dat                          : 1;
	uint64_t drp_noavail                  : 1;
	uint64_t x2p_req_ofl                  : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_pki_gen_int_s             cn78xx;
};
typedef union cvmx_pki_gen_int cvmx_pki_gen_int_t;

/**
 * cvmx_pki_icg#_cfg
 *
 * Configures a cluster group.
 *
 */
union cvmx_pki_icgx_cfg {
	uint64_t u64;
	struct cvmx_pki_icgx_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_53_63               : 11;
	uint64_t maxipe_use                   : 5;  /**< Maximum number of IPEs to use in each cluster for this ICG. For diagnostic use only.
                                                         INTERNAL: Allows reducing the number of IPEs available for debug, characterization,
                                                         repair, etc. Must be 1-20. Normally, PKI will have all 20 IPEs available in a cluster for
                                                         packet processing, other values will decrease performance. */
	uint64_t reserved_36_47               : 12;
	uint64_t clusters                     : 4;  /**< Bit-mask of clusters in this cluster group. A given cluster can only be enabled in a
                                                         single cluster group. Behavior is undefined for an ICG which receives traffic with a
                                                         [CLUSTERS] of 0x0. IGC(0)'s entry resets to 0xF, all other entries to 0x0. */
	uint64_t reserved_27_31               : 5;
	uint64_t release_rqd                  : 1;  /**< Release required. For diagnostic use only. INTERNAL:
                                                         0 = Release of r64 to r95 will occur immediately, no release microop is needed.
                                                         1 = Release will wait until release microop executes. */
	uint64_t mlo                          : 1;  /**< Memory low bypass enable. For diagnostic use only. INTERNAL:
                                                         0 = KMEM specifies contents of r48 to r63. The sequencer code expects this setting.
                                                         1 = KMEM specifies contents of r32 to r47. This may be desirable when PKIENA=0 to allow
                                                         direct control over the back end. */
	uint64_t pena                         : 1;  /**< Parse enable. Must be set after PKI has been initialized.
                                                         INTERNAL: Software should set after the IMEM and associated state is initialized.
                                                         0 = IPT transitions from start directly to done without executing a sequence, and the KMEM
                                                         bits effectively are copied through to the WQ.
                                                         1 = Normal sequencer operation. */
	uint64_t timer                        : 12; /**< Current hold-off timer. Enables even spreading of cluster utilization over time; while
                                                         TIMER is non-zero, a cluster in this group will not start parsing. When a cluster in this
                                                         group starts parsing, TIMER is set to DELAY, and decrements every sclk cycle. TIMER is
                                                         zeroed if all clusters in this group are idle. */
	uint64_t delay                        : 12; /**< Delay between cluster starts, as described under TIMER. If zero, a cluster can start at
                                                         any time relative to other clusters. DELAY should be typically selected to minimize the
                                                         average observed parser latency by loading with the parsing delay divided by the number of
                                                         clusters in this cluster group. The smallest useful non-zero value is 0xA0, corresponding
                                                         to the minimum number of cycles needed to fill one cluster with packets. */
#else
	uint64_t delay                        : 12;
	uint64_t timer                        : 12;
	uint64_t pena                         : 1;
	uint64_t mlo                          : 1;
	uint64_t release_rqd                  : 1;
	uint64_t reserved_27_31               : 5;
	uint64_t clusters                     : 4;
	uint64_t reserved_36_47               : 12;
	uint64_t maxipe_use                   : 5;
	uint64_t reserved_53_63               : 11;
#endif
	} s;
	struct cvmx_pki_icgx_cfg_s            cn78xx;
};
typedef union cvmx_pki_icgx_cfg cvmx_pki_icgx_cfg_t;

/**
 * cvmx_pki_imem#
 */
union cvmx_pki_imemx {
	uint64_t u64;
	struct cvmx_pki_imemx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Instruction word at given address. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_pki_imemx_s               cn78xx;
};
typedef union cvmx_pki_imemx cvmx_pki_imemx_t;

/**
 * cvmx_pki_ltype#_map
 */
union cvmx_pki_ltypex_map {
	uint64_t u64;
	struct cvmx_pki_ltypex_map_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t beltype                      : 3;  /**< For each given PKI_LTYPE_E, the protocol type backend hardware should assume this layer
                                                         type corresponds to. Enumerated by PKI_BELTYPE_E. The recommended settings for each
                                                         PKI_LTYPE_E are shown in the PKI_LTYPE_E table. */
#else
	uint64_t beltype                      : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_pki_ltypex_map_s          cn78xx;
};
typedef union cvmx_pki_ltypex_map cvmx_pki_ltypex_map_t;

/**
 * cvmx_pki_pcam_lookup
 *
 * For diagnostic use only, perform a PCAM lookup against the provided cluster and PCAM instance
 * and loads results into PKI_PCAM_RESULT.
 */
union cvmx_pki_pcam_lookup {
	uint64_t u64;
	struct cvmx_pki_pcam_lookup_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t cl                           : 2;  /**< Cluster number to lookup within. */
	uint64_t reserved_49_51               : 3;
	uint64_t pcam                         : 1;  /**< PCAM number to lookup within. */
	uint64_t term                         : 8;  /**< Term value to lookup. */
	uint64_t style                        : 8;  /**< Style value to lookup. */
	uint64_t data                         : 32; /**< Data to lookup. */
#else
	uint64_t data                         : 32;
	uint64_t style                        : 8;
	uint64_t term                         : 8;
	uint64_t pcam                         : 1;
	uint64_t reserved_49_51               : 3;
	uint64_t cl                           : 2;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_pki_pcam_lookup_s         cn78xx;
};
typedef union cvmx_pki_pcam_lookup cvmx_pki_pcam_lookup_t;

/**
 * cvmx_pki_pcam_result
 */
union cvmx_pki_pcam_result {
	uint64_t u64;
	struct cvmx_pki_pcam_result_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t match                        : 1;  /**< Resulting match. */
	uint64_t entry                        : 8;  /**< Resulting matching entry number, unpredictable unless [MATCH] set. */
	uint64_t result                       : 32; /**< Resulting data from matching line's PKI_CL(0..3)_PCAM(0..1)_ACTION(0..191), or zero if no match. */
#else
	uint64_t result                       : 32;
	uint64_t entry                        : 8;
	uint64_t match                        : 1;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pki_pcam_result_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t conflict                     : 1;  /**< Conflict. The lookup resulted in multiple entries matching PKI_PCAM_LOOKUP[DATA], [TERM]
                                                         and [STYLE], or zero if no match. */
	uint64_t reserved_41_62               : 22;
	uint64_t match                        : 1;  /**< Resulting match. */
	uint64_t entry                        : 8;  /**< Resulting matching entry number, unpredictable unless [MATCH] set. */
	uint64_t result                       : 32; /**< Resulting data from matching line's PKI_CL(0..3)_PCAM(0..1)_ACTION(0..191), or zero if no match. */
#else
	uint64_t result                       : 32;
	uint64_t entry                        : 8;
	uint64_t match                        : 1;
	uint64_t reserved_41_62               : 22;
	uint64_t conflict                     : 1;
#endif
	} cn78xx;
};
typedef union cvmx_pki_pcam_result cvmx_pki_pcam_result_t;

/**
 * cvmx_pki_pfe_diag
 */
union cvmx_pki_pfe_diag {
	uint64_t u64;
	struct cvmx_pki_pfe_diag_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t bad_rid                      : 1;  /**< Asserted when PFE sees and drops an X2P transaction with a RID > 95. */
#else
	uint64_t bad_rid                      : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_pki_pfe_diag_s            cn78xx;
};
typedef union cvmx_pki_pfe_diag cvmx_pki_pfe_diag_t;

/**
 * cvmx_pki_pix_diag
 */
union cvmx_pki_pix_diag {
	uint64_t u64;
	struct cvmx_pki_pix_diag_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t nosched                      : 4;  /**< Asserted when PFE requests an ICG with no enabled CLUSTERS. */
#else
	uint64_t nosched                      : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_pki_pix_diag_s            cn78xx;
};
typedef union cvmx_pki_pix_diag cvmx_pki_pix_diag_t;

/**
 * cvmx_pki_pkind#_icgsel
 */
union cvmx_pki_pkindx_icgsel {
	uint64_t u64;
	struct cvmx_pki_pkindx_icgsel_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t icg                          : 2;  /**< Cluster group that will service traffic on this pkind. See also PKI_ICG(0..3)_CFG, the
                                                         register to which this field indexes. */
#else
	uint64_t icg                          : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_pki_pkindx_icgsel_s       cn78xx;
};
typedef union cvmx_pki_pkindx_icgsel cvmx_pki_pkindx_icgsel_t;

/**
 * cvmx_pki_pknd#_inb_stat0
 *
 * Inbound packets received by PKI per pkind.
 *
 */
union cvmx_pki_pkndx_inb_stat0 {
	uint64_t u64;
	struct cvmx_pki_pkndx_inb_stat0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t pkts                         : 48; /**< Number of packets without errors received by PKI. */
#else
	uint64_t pkts                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_pkndx_inb_stat0_s     cn78xx;
};
typedef union cvmx_pki_pkndx_inb_stat0 cvmx_pki_pkndx_inb_stat0_t;

/**
 * cvmx_pki_pknd#_inb_stat1
 *
 * Inbound octets received by PKI per pkind.
 *
 */
union cvmx_pki_pkndx_inb_stat1 {
	uint64_t u64;
	struct cvmx_pki_pkndx_inb_stat1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t octs                         : 48; /**< Total number of octets from all packets received by PKI. */
#else
	uint64_t octs                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_pkndx_inb_stat1_s     cn78xx;
};
typedef union cvmx_pki_pkndx_inb_stat1 cvmx_pki_pkndx_inb_stat1_t;

/**
 * cvmx_pki_pknd#_inb_stat2
 *
 * Inbound error packets received by PKI per pkind.
 *
 */
union cvmx_pki_pkndx_inb_stat2 {
	uint64_t u64;
	struct cvmx_pki_pkndx_inb_stat2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t errs                         : 48; /**< Number of packets with errors received by PKI. */
#else
	uint64_t errs                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_pkndx_inb_stat2_s     cn78xx;
};
typedef union cvmx_pki_pkndx_inb_stat2 cvmx_pki_pkndx_inb_stat2_t;

/**
 * cvmx_pki_pkt_err
 */
union cvmx_pki_pkt_err {
	uint64_t u64;
	struct cvmx_pki_pkt_err_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t reasm                        : 7;  /**< When PKI_GEN_INT[DAT,SOP,EOP] is set, this field latches the failing reassembly number
                                                         associated with the error. */
#else
	uint64_t reasm                        : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_pki_pkt_err_s             cn78xx;
};
typedef union cvmx_pki_pkt_err cvmx_pki_pkt_err_t;

/**
 * cvmx_pki_qpg_tbl#
 *
 * The QPG table is used to indirectly calculate the Portadd/Aura/Group from the Diffsrv, HiGig
 * or VLAN information as described in QPG.
 */
union cvmx_pki_qpg_tblx {
	uint64_t u64;
	struct cvmx_pki_qpg_tblx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t padd                         : 12; /**< Port to channel adder for calculating WQE[CHAN]. */
	uint64_t grptag_ok                    : 3;  /**< Number of WQE[TAG] bits to add into WQE[GRP] if no error is detected. */
	uint64_t reserved_42_44               : 3;
	uint64_t grp_ok                       : 10; /**< SSO group to schedule packet to and to load WQE[GRP] with if no error is detected. */
	uint64_t grptag_bad                   : 3;  /**< Number of WQE[TAG] bits to add into WQE[GRP] if an error is detected. */
	uint64_t reserved_26_28               : 3;
	uint64_t grp_bad                      : 10; /**< SSO group to schedule packet to and to load WQE[GRP] with if an error is detected. */
	uint64_t reserved_12_15               : 4;
	uint64_t aura_node                    : 2;  /**< Aura node number. The node number is part of the upper aura bits, however PKI can only
                                                         allocate from auras on the local node, therefore these bits are hardcoded to the node
                                                         number. */
	uint64_t laura                        : 10; /**< Aura on local node for QOS calculations and loading into WQE[AURA]WQE[AURA]. */
#else
	uint64_t laura                        : 10;
	uint64_t aura_node                    : 2;
	uint64_t reserved_12_15               : 4;
	uint64_t grp_bad                      : 10;
	uint64_t reserved_26_28               : 3;
	uint64_t grptag_bad                   : 3;
	uint64_t grp_ok                       : 10;
	uint64_t reserved_42_44               : 3;
	uint64_t grptag_ok                    : 3;
	uint64_t padd                         : 12;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_pki_qpg_tblx_s            cn78xx;
};
typedef union cvmx_pki_qpg_tblx cvmx_pki_qpg_tblx_t;

/**
 * cvmx_pki_reasm_sop#
 *
 * Set when a SOP is detected on a Reasm-Id, where the Reasm-ID value sets the bit vector of this
 * register.
 */
union cvmx_pki_reasm_sopx {
	uint64_t u64;
	struct cvmx_pki_reasm_sopx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sop                          : 64; /**< When set, a SOP was detected on a reasm-Id. When clear, a SOP has not yet been received,
                                                         or an EOP was received on the Reasm-Id. */
#else
	uint64_t sop                          : 64;
#endif
	} s;
	struct cvmx_pki_reasm_sopx_s          cn78xx;
};
typedef union cvmx_pki_reasm_sopx cvmx_pki_reasm_sopx_t;

/**
 * cvmx_pki_req_wgt
 *
 * Controls the round-robin weights between each PKI requestor. Intended for diagnostic tuning only.
 *
 */
union cvmx_pki_req_wgt {
	uint64_t u64;
	struct cvmx_pki_req_wgt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t wgt8                         : 4;  /**< Weight for ILK0. */
	uint64_t wgt7                         : 4;  /**< Weight for LBK. */
	uint64_t wgt6                         : 4;  /**< Weight for DPI. */
	uint64_t wgt5                         : 4;  /**< Weight for BGX5. */
	uint64_t wgt4                         : 4;  /**< Weight for BGX4. */
	uint64_t wgt3                         : 4;  /**< Weight for BGX3. */
	uint64_t wgt2                         : 4;  /**< Weight for BGX2. */
	uint64_t wgt1                         : 4;  /**< Weight for BGX1. */
	uint64_t wgt0                         : 4;  /**< Weight for BGX0. */
#else
	uint64_t wgt0                         : 4;
	uint64_t wgt1                         : 4;
	uint64_t wgt2                         : 4;
	uint64_t wgt3                         : 4;
	uint64_t wgt4                         : 4;
	uint64_t wgt5                         : 4;
	uint64_t wgt6                         : 4;
	uint64_t wgt7                         : 4;
	uint64_t wgt8                         : 4;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_pki_req_wgt_s             cn78xx;
};
typedef union cvmx_pki_req_wgt cvmx_pki_req_wgt_t;

/**
 * cvmx_pki_sft_rst
 */
union cvmx_pki_sft_rst {
	uint64_t u64;
	struct cvmx_pki_sft_rst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t busy                         : 1;  /**< When set, PKI is busy completing reset. No access except the reading of this bit should
                                                         occur to the PKI until this is clear. INTERNAL: The BUSY bit for this implementation is a
                                                         placeholder and is not required to be implemented in HW. The soft reset pulse is short
                                                         enough that we can guarantee that reset will complete below a subsequent RSL reference can
                                                         be made. It is still useful for this bit to exist in case that property every changes and
                                                         the reset requires a longer duration. For this implementation, SW will check the bit which
                                                         will always report not BUSY allowing SW to proceed with its flow. */
	uint64_t reserved_33_62               : 30;
	uint64_t active                       : 1;  /**< When set, PKI is actively processing packet traffic. It is recommenced that software wait
                                                         until ACTIVE is clear before setting RST. INTERNAL: ACTIVE is an OR of PKI_ACTIVE0..2. */
	uint64_t reserved_1_31                : 31;
	uint64_t rst                          : 1;  /**< Reset. When set to 1 by software, PKI will produce an internal reset pulse. */
#else
	uint64_t rst                          : 1;
	uint64_t reserved_1_31                : 31;
	uint64_t active                       : 1;
	uint64_t reserved_33_62               : 30;
	uint64_t busy                         : 1;
#endif
	} s;
	struct cvmx_pki_sft_rst_s             cn78xx;
};
typedef union cvmx_pki_sft_rst cvmx_pki_sft_rst_t;

/**
 * cvmx_pki_stat#_hist0
 */
union cvmx_pki_statx_hist0 {
	uint64_t u64;
	struct cvmx_pki_statx_hist0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t h1to63                       : 48; /**< Number of non-dropped 1 to 63 byte packets. Packet length includes FCS and any prepended
                                                         PTP timestamp. */
#else
	uint64_t h1to63                       : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_hist0_s         cn78xx;
};
typedef union cvmx_pki_statx_hist0 cvmx_pki_statx_hist0_t;

/**
 * cvmx_pki_stat#_hist1
 */
union cvmx_pki_statx_hist1 {
	uint64_t u64;
	struct cvmx_pki_statx_hist1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t h64to127                     : 48; /**< Number of non-dropped 64 to 127 byte packets. */
#else
	uint64_t h64to127                     : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_hist1_s         cn78xx;
};
typedef union cvmx_pki_statx_hist1 cvmx_pki_statx_hist1_t;

/**
 * cvmx_pki_stat#_hist2
 */
union cvmx_pki_statx_hist2 {
	uint64_t u64;
	struct cvmx_pki_statx_hist2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t h128to255                    : 48; /**< Number of non-dropped 128 to 255 byte packets. */
#else
	uint64_t h128to255                    : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_hist2_s         cn78xx;
};
typedef union cvmx_pki_statx_hist2 cvmx_pki_statx_hist2_t;

/**
 * cvmx_pki_stat#_hist3
 */
union cvmx_pki_statx_hist3 {
	uint64_t u64;
	struct cvmx_pki_statx_hist3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t h256to511                    : 48; /**< Number of non-dropped 256 to 511 byte packets. */
#else
	uint64_t h256to511                    : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_hist3_s         cn78xx;
};
typedef union cvmx_pki_statx_hist3 cvmx_pki_statx_hist3_t;

/**
 * cvmx_pki_stat#_hist4
 */
union cvmx_pki_statx_hist4 {
	uint64_t u64;
	struct cvmx_pki_statx_hist4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t h512to1023                   : 48; /**< Number of non-dropped 512 to 1023 byte packets. */
#else
	uint64_t h512to1023                   : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_hist4_s         cn78xx;
};
typedef union cvmx_pki_statx_hist4 cvmx_pki_statx_hist4_t;

/**
 * cvmx_pki_stat#_hist5
 */
union cvmx_pki_statx_hist5 {
	uint64_t u64;
	struct cvmx_pki_statx_hist5_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t h1024to1518                  : 48; /**< Number of non-dropped 1024 to 1518 byte packets. */
#else
	uint64_t h1024to1518                  : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_hist5_s         cn78xx;
};
typedef union cvmx_pki_statx_hist5 cvmx_pki_statx_hist5_t;

/**
 * cvmx_pki_stat#_hist6
 */
union cvmx_pki_statx_hist6 {
	uint64_t u64;
	struct cvmx_pki_statx_hist6_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t h1519                        : 48; /**< Number of non-dropped 1519 byte and above packets. */
#else
	uint64_t h1519                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_hist6_s         cn78xx;
};
typedef union cvmx_pki_statx_hist6 cvmx_pki_statx_hist6_t;

/**
 * cvmx_pki_stat#_stat0
 */
union cvmx_pki_statx_stat0 {
	uint64_t u64;
	struct cvmx_pki_statx_stat0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t pkts                         : 48; /**< Number of non-dropped packets processed by PKI. */
#else
	uint64_t pkts                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat0_s         cn78xx;
};
typedef union cvmx_pki_statx_stat0 cvmx_pki_statx_stat0_t;

/**
 * cvmx_pki_stat#_stat1
 */
union cvmx_pki_statx_stat1 {
	uint64_t u64;
	struct cvmx_pki_statx_stat1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t octs                         : 48; /**< Number of non-dropped octets received by PKI (good and bad). */
#else
	uint64_t octs                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat1_s         cn78xx;
};
typedef union cvmx_pki_statx_stat1 cvmx_pki_statx_stat1_t;

/**
 * cvmx_pki_stat#_stat10
 */
union cvmx_pki_statx_stat10 {
	uint64_t u64;
	struct cvmx_pki_statx_stat10_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t jabber                       : 48; /**< Number of non-dropped packets with length > maximum and FCS error. */
#else
	uint64_t jabber                       : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat10_s        cn78xx;
};
typedef union cvmx_pki_statx_stat10 cvmx_pki_statx_stat10_t;

/**
 * cvmx_pki_stat#_stat11
 */
union cvmx_pki_statx_stat11 {
	uint64_t u64;
	struct cvmx_pki_statx_stat11_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t oversz                       : 48; /**< Number of non-dropped packets with length > maximum and no FCS error. */
#else
	uint64_t oversz                       : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat11_s        cn78xx;
};
typedef union cvmx_pki_statx_stat11 cvmx_pki_statx_stat11_t;

/**
 * cvmx_pki_stat#_stat12
 */
union cvmx_pki_statx_stat12 {
	uint64_t u64;
	struct cvmx_pki_statx_stat12_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t l2err                        : 48; /**< Number of non-dropped packets with receive errors (WQE[ERRLEV]==RE or L2) not covered by
                                                         more specific length or FCS statistic error registers. */
#else
	uint64_t l2err                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat12_s        cn78xx;
};
typedef union cvmx_pki_statx_stat12 cvmx_pki_statx_stat12_t;

/**
 * cvmx_pki_stat#_stat13
 */
union cvmx_pki_statx_stat13 {
	uint64_t u64;
	struct cvmx_pki_statx_stat13_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t spec                         : 48; /**< Number of packets, dropped or non-dropped, with special handling. For profiling and
                                                         diagnostic use only.
                                                         INTERNAL: Counts packets completing IPE processing with WQE[SH] set. */
#else
	uint64_t spec                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat13_s        cn78xx;
};
typedef union cvmx_pki_statx_stat13 cvmx_pki_statx_stat13_t;

/**
 * cvmx_pki_stat#_stat14
 */
union cvmx_pki_statx_stat14 {
	uint64_t u64;
	struct cvmx_pki_statx_stat14_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t drp_bcast                    : 48; /**< Number of packets with L2 broadcast DMAC that were dropped due to RED or buffer
                                                         exhaustion. See WQE[L2B] for the definition of L2 broadcast. */
#else
	uint64_t drp_bcast                    : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat14_s        cn78xx;
};
typedef union cvmx_pki_statx_stat14 cvmx_pki_statx_stat14_t;

/**
 * cvmx_pki_stat#_stat15
 */
union cvmx_pki_statx_stat15 {
	uint64_t u64;
	struct cvmx_pki_statx_stat15_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t drp_mcast                    : 48; /**< Number of packets with L2 multicast DMAC that were dropped due to RED or buffer
                                                         exhaustion. See WQE[L2M] for the definition of L2 multicast. */
#else
	uint64_t drp_mcast                    : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat15_s        cn78xx;
};
typedef union cvmx_pki_statx_stat15 cvmx_pki_statx_stat15_t;

/**
 * cvmx_pki_stat#_stat16
 */
union cvmx_pki_statx_stat16 {
	uint64_t u64;
	struct cvmx_pki_statx_stat16_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t drp_bcast                    : 48; /**< Number of packets with IPv4 L3 broadcast destination address that were dropped due to RED
                                                         or buffer exhaustion. See WQE[L3B] for the definition of L2 multicast. */
#else
	uint64_t drp_bcast                    : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat16_s        cn78xx;
};
typedef union cvmx_pki_statx_stat16 cvmx_pki_statx_stat16_t;

/**
 * cvmx_pki_stat#_stat17
 */
union cvmx_pki_statx_stat17 {
	uint64_t u64;
	struct cvmx_pki_statx_stat17_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t drp_mcast                    : 48; /**< Number of packets with IPv4 or IPv6 L3 multicast destination address that were dropped due
                                                         to RED or buffer exhaustion. See WQE[L3M] for the definition of L3 multicast. */
#else
	uint64_t drp_mcast                    : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat17_s        cn78xx;
};
typedef union cvmx_pki_statx_stat17 cvmx_pki_statx_stat17_t;

/**
 * cvmx_pki_stat#_stat18
 */
union cvmx_pki_statx_stat18 {
	uint64_t u64;
	struct cvmx_pki_statx_stat18_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t drp_spec                     : 48; /**< Number of packets dropped with special handling. For profiling and diagnostic use only.
                                                         INTERNAL: Counts packets with dropped after completing IPE processing with WQE[SH] set. */
#else
	uint64_t drp_spec                     : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat18_s        cn78xx;
};
typedef union cvmx_pki_statx_stat18 cvmx_pki_statx_stat18_t;

/**
 * cvmx_pki_stat#_stat2
 */
union cvmx_pki_statx_stat2 {
	uint64_t u64;
	struct cvmx_pki_statx_stat2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t raw                          : 48; /**< Number of non-dropped packets with WQE[RAW] set. */
#else
	uint64_t raw                          : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat2_s         cn78xx;
};
typedef union cvmx_pki_statx_stat2 cvmx_pki_statx_stat2_t;

/**
 * cvmx_pki_stat#_stat3
 */
union cvmx_pki_statx_stat3 {
	uint64_t u64;
	struct cvmx_pki_statx_stat3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t drp_pkts                     : 48; /**< Inbound packets dropped by RED or buffer exhaustion. */
#else
	uint64_t drp_pkts                     : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat3_s         cn78xx;
};
typedef union cvmx_pki_statx_stat3 cvmx_pki_statx_stat3_t;

/**
 * cvmx_pki_stat#_stat4
 */
union cvmx_pki_statx_stat4 {
	uint64_t u64;
	struct cvmx_pki_statx_stat4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t drp_octs                     : 48; /**< Inbound octets dropped by RED or buffer exhaustion. */
#else
	uint64_t drp_octs                     : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat4_s         cn78xx;
};
typedef union cvmx_pki_statx_stat4 cvmx_pki_statx_stat4_t;

/**
 * cvmx_pki_stat#_stat5
 */
union cvmx_pki_statx_stat5 {
	uint64_t u64;
	struct cvmx_pki_statx_stat5_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t bcast                        : 48; /**< Number of non-dropped L2 broadcast packets. Does not include multicast packets. See
                                                         WQE[L2B] for the definition of L2 broadcast. */
#else
	uint64_t bcast                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat5_s         cn78xx;
};
typedef union cvmx_pki_statx_stat5 cvmx_pki_statx_stat5_t;

/**
 * cvmx_pki_stat#_stat6
 */
union cvmx_pki_statx_stat6 {
	uint64_t u64;
	struct cvmx_pki_statx_stat6_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t mcast                        : 48; /**< Number of non-dropped L2 multicast packets. Does not include broadcast packets. See
                                                         WQE[L2M] for the definition of L2 multicast. */
#else
	uint64_t mcast                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat6_s         cn78xx;
};
typedef union cvmx_pki_statx_stat6 cvmx_pki_statx_stat6_t;

/**
 * cvmx_pki_stat#_stat7
 */
union cvmx_pki_statx_stat7 {
	uint64_t u64;
	struct cvmx_pki_statx_stat7_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t fcs                          : 48; /**< Number of non-dropped packets with an FCS opcode error, excluding fragments or overruns. */
#else
	uint64_t fcs                          : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat7_s         cn78xx;
};
typedef union cvmx_pki_statx_stat7 cvmx_pki_statx_stat7_t;

/**
 * cvmx_pki_stat#_stat8
 */
union cvmx_pki_statx_stat8 {
	uint64_t u64;
	struct cvmx_pki_statx_stat8_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t frag                         : 48; /**< Number of non-dropped packets with length < minimum and FCS error */
#else
	uint64_t frag                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat8_s         cn78xx;
};
typedef union cvmx_pki_statx_stat8 cvmx_pki_statx_stat8_t;

/**
 * cvmx_pki_stat#_stat9
 */
union cvmx_pki_statx_stat9 {
	uint64_t u64;
	struct cvmx_pki_statx_stat9_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t undersz                      : 48; /**< Number of non-dropped packets with length < minimum and no FCS error. */
#else
	uint64_t undersz                      : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat9_s         cn78xx;
};
typedef union cvmx_pki_statx_stat9 cvmx_pki_statx_stat9_t;

/**
 * cvmx_pki_stat_ctl
 *
 * Controls how the PKI statistics counters are handled.
 *
 */
union cvmx_pki_stat_ctl {
	uint64_t u64;
	struct cvmx_pki_stat_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t mode                         : 2;  /**< The PKI_STAT*_X registers can be indexed either by port kind (pkind), or final style.
                                                         (Does not apply to the PKI_STAT_INB* registers.)
                                                         0 = X represents the packet's pkind
                                                         1 = X represents the low 6-bits of packet's final style
                                                         2,3 = Reserved */
#else
	uint64_t mode                         : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_pki_stat_ctl_s            cn78xx;
};
typedef union cvmx_pki_stat_ctl cvmx_pki_stat_ctl_t;

/**
 * cvmx_pki_style#_buf
 */
union cvmx_pki_stylex_buf {
	uint64_t u64;
	struct cvmx_pki_stylex_buf_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t pkt_lend                     : 1;  /**< Packet little-endian. Changes write operations of packet data to L2C to be in little-
                                                         endian. Does not change the WQE header format, which is properly endian neutral. */
	uint64_t wqe_hsz                      : 2;  /**< Work queue header size:
                                                         0x0 = WORD0..4, standard WQE_S. Note FIRST_SKIP may be set to not include WORD4 in memory.
                                                         0x1 = WORD0..5
                                                         0x2 = WORD0..6
                                                         0x3 = WORD0..7
                                                         else = Reserved
                                                         INTERNAL: Selects which PIX words are transferred to the PKI BE. If a word is not
                                                         transferred and the word will reach memory (FIRST_SKIP is greater than that word number),
                                                         then the final WQE memory word will be zero, not the PIX register contents. */
	uint64_t wqe_skip                     : 2;  /**< WQE start offset. The number of 128-byte cache lines to skip between the buffer pointer
                                                         and WORD0 of the work-queue entry. See PKI Hardware Allocating Multiple Buffers. */
	uint64_t first_skip                   : 6;  /**< The number of eight-byte words from the top of the first MBUF that the PKI stores the next
                                                         pointer. If [DIS_WQ_DAT]=1, any value is legal. If [DIS_WQ_DAT]=0, legal values must
                                                         satisfy:
                                                         FIRST_SKIP <= PKI_STYLE(0..63)_BUF[MB_SIZE] - 18.
                                                         FIRST_SKIP must be at least 0x4, but 0x5 is recommended minimum. 0x4 will drop WQE WORD4,
                                                         for use in backward compatible applications.
                                                         WQE_SKIP * (128/8) + 4 <= FIRST_SKIP, to insure the minimum of four work-queue entry words
                                                         will fit within FIRST_SKIP. */
	uint64_t later_skip                   : 6;  /**< The number of eight-byte words from the top of any MBUF that is not the first MBUF that
                                                         PKI writes the next-pointer to. Legal values are 0 to PKI_STYLE(0..63)_BUF[MB_SIZE] - 18. */
	uint64_t opc_mode                     : 2;  /**< Select the style of write to the L2C.
                                                         0 = all packet data and next-buffer pointers are written through to memory.
                                                         1 = all packet data and next-buffer pointers are written into the cache.
                                                         2 = the first aligned cache block holding the WQE and/or front packet data are written to
                                                         the L2 cache. All remaining cache blocks are not written to the L2 cache.
                                                         3 = the first two aligned cache blocks holding the WQE and front packet data are written
                                                         to the L2 cache. All remaining cache blocks are not written to the L2 cache. */
	uint64_t dis_wq_dat                   : 1;  /**< Separate first data buffer from the work queue entry.
                                                         0 = The packet link pointer will be at word [FIRST_SKIP] immediately followed by packet
                                                         data, in the same buffer as the work queue entry.
                                                         1 = The packet link pointer will be at word [FIRST_SKIP] in a new buffer separate from the
                                                         work queue entry. Words following the WQE in the same cache line will be zeroed, other
                                                         lines in the buffer will not be modified and will retain stale data (from the buffer's
                                                         previous use). This setting may decrease the peak PKI performance by up to half on small
                                                         packets. */
	uint64_t mb_size                      : 13; /**< The number of eight-byte words to store into a buffer. This must be in the range of 32 to
                                                         4096. This must be less than or equal to the maximum size of every free page in every FPA
                                                         pool this style may use. */
#else
	uint64_t mb_size                      : 13;
	uint64_t dis_wq_dat                   : 1;
	uint64_t opc_mode                     : 2;
	uint64_t later_skip                   : 6;
	uint64_t first_skip                   : 6;
	uint64_t wqe_skip                     : 2;
	uint64_t wqe_hsz                      : 2;
	uint64_t pkt_lend                     : 1;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_pki_stylex_buf_s          cn78xx;
};
typedef union cvmx_pki_stylex_buf cvmx_pki_stylex_buf_t;

/**
 * cvmx_pki_style#_tag_mask
 */
union cvmx_pki_stylex_tag_mask {
	uint64_t u64;
	struct cvmx_pki_stylex_tag_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t mask                         : 16; /**< When set, each bit excludes corresponding bit of the tuple computed tag from being
                                                         included in the final tag. PKI_CL(0..3)_STYLE(0..63)_CFG2 [TAG_MASKEN] must be set. Does
                                                         not affect tags from packets with a PKI_INST_HDR_S when PKI_INST_HDR[UTAG] is set */
#else
	uint64_t mask                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_pki_stylex_tag_mask_s     cn78xx;
};
typedef union cvmx_pki_stylex_tag_mask cvmx_pki_stylex_tag_mask_t;

/**
 * cvmx_pki_style#_tag_sel
 */
union cvmx_pki_stylex_tag_sel {
	uint64_t u64;
	struct cvmx_pki_stylex_tag_sel_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t tag_idx3                     : 3;  /**< Index for TAG_INC<3>. */
	uint64_t reserved_19_23               : 5;
	uint64_t tag_idx2                     : 3;  /**< Index for TAG_INC<2>. */
	uint64_t reserved_11_15               : 5;
	uint64_t tag_idx1                     : 3;  /**< Index for TAG_INC<1>. */
	uint64_t reserved_3_7                 : 5;
	uint64_t tag_idx0                     : 3;  /**< Index for TAG_INC<0>. This value is multipled by 4 to index into PKI_TAG_INC(0..31)_MASK.
                                                         See WQE[TAG]. */
#else
	uint64_t tag_idx0                     : 3;
	uint64_t reserved_3_7                 : 5;
	uint64_t tag_idx1                     : 3;
	uint64_t reserved_11_15               : 5;
	uint64_t tag_idx2                     : 3;
	uint64_t reserved_19_23               : 5;
	uint64_t tag_idx3                     : 3;
	uint64_t reserved_27_63               : 37;
#endif
	} s;
	struct cvmx_pki_stylex_tag_sel_s      cn78xx;
};
typedef union cvmx_pki_stylex_tag_sel cvmx_pki_stylex_tag_sel_t;

/**
 * cvmx_pki_style#_wq2
 */
union cvmx_pki_stylex_wq2 {
	uint64_t u64;
	struct cvmx_pki_stylex_wq2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Data for WQ2<63:0>. This is ORed over any parser calculated WQ2<63:0> fields, and is used
                                                         to emulate as if the parser set a WQ field such as WQE[PF1]. PKI_INST_HDR_S packets may
                                                         also want to use this mode to set WQE[LCTY] to IP when PKI parsing IP is disabled. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_pki_stylex_wq2_s          cn78xx;
};
typedef union cvmx_pki_stylex_wq2 cvmx_pki_stylex_wq2_t;

/**
 * cvmx_pki_style#_wq4
 */
union cvmx_pki_stylex_wq4 {
	uint64_t u64;
	struct cvmx_pki_stylex_wq4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Data for WQ4<63:0>. This is ORed over any parser calculated WQ4<63:0> fields, and is used
                                                         to emulate as if the parser set a WQ pointer field. PKI_INST_HDR_S packets may also want
                                                         to use this mode to set WQE[LCPTR] to the start of IP when PKI parsing IP is disabled. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_pki_stylex_wq4_s          cn78xx;
};
typedef union cvmx_pki_stylex_wq4 cvmx_pki_stylex_wq4_t;

/**
 * cvmx_pki_tag_inc#_ctl
 */
union cvmx_pki_tag_incx_ctl {
	uint64_t u64;
	struct cvmx_pki_tag_incx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t ptr_sel                      : 4;  /**< Which pointer to use for the bitmask in PKI_TAG_INC(0..31)_MASK.
                                                         0 = Absolute from start of packet.
                                                         1-7 = Reserved.
                                                         8 = Relative to start of WQE[LAPTR]. LAPTR must be valid (see WQE[LAPTR]) or mask is
                                                         ignored.
                                                         9 = Relative to start of WQE[LBPTR]. LBPTR must be valid (see WQE[LBPTR]) or mask is
                                                         ignored.
                                                         10 = Relative to start of WQE[LCPTR]. LCPTR must be valid (see WQE[LCPTR]) or mask is
                                                         ignored.
                                                         11 = Relative to start of WQE[LDPTR]. LDPTR must be valid (see WQE[LDPTR]) or mask is
                                                         ignored.
                                                         12 = Relative to start of WQE[LEPTR]. LEPTR must be valid (see WQE[LEPTR]) or mask is
                                                         ignored.
                                                         13 = Relative to start of WQE[LFPTR]. LFPTR must be valid (see WQE[LFPTR]) or mask is
                                                         ignored.
                                                         14 = Relative to start of WQE[LGPTR]. LGPTR must be valid (see WQE[LGPTR]) or mask is
                                                         ignored.
                                                         15 = Relative to start of WQE[VLPTR]. VLPTR must be valid (see WQE[VLPTR]) or mask is
                                                         ignored.
                                                         INTERNAL: Note excluding 0, the encoding matches the byte number to read from WQE WORD4. */
	uint64_t offset                       : 8;  /**< Offset for PKI_TAG_INC(0..31)_MASK. Number of bytes to add to the selected pointer before
                                                         applying the mask. */
#else
	uint64_t offset                       : 8;
	uint64_t ptr_sel                      : 4;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_pki_tag_incx_ctl_s        cn78xx;
};
typedef union cvmx_pki_tag_incx_ctl cvmx_pki_tag_incx_ctl_t;

/**
 * cvmx_pki_tag_inc#_mask
 */
union cvmx_pki_tag_incx_mask {
	uint64_t u64;
	struct cvmx_pki_tag_incx_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t en                           : 64; /**< Include byte in mask-tag algorithm. Each EN bit corresponds to 64 consecutive bytes in the
                                                         data stream, as controlled by PKI_TAG_INC(0..31)_CTL as described in WQE[TAG]. */
#else
	uint64_t en                           : 64;
#endif
	} s;
	struct cvmx_pki_tag_incx_mask_s       cn78xx;
};
typedef union cvmx_pki_tag_incx_mask cvmx_pki_tag_incx_mask_t;

/**
 * cvmx_pki_tag_secret
 *
 * The source and destination initial values (IVs) in tag generation provide a mechanism for
 * seeding with a random initialization value to reduce cache collision attacks.
 */
union cvmx_pki_tag_secret {
	uint64_t u64;
	struct cvmx_pki_tag_secret_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dst6                         : 16; /**< Secret for the destination tuple IPv6 tag CRC calculation. Typically identical to SRC6 to
                                                         insure tagging is symmetric between source/destination flows. Typically different from DST
                                                         for maximum security. */
	uint64_t src6                         : 16; /**< Secret for the source tuple IPv6 tag CRC calculation. Typically different from SRC for
                                                         maximum security. */
	uint64_t dst                          : 16; /**< Secret for the destination tuple tag CRC calculation. Typically identical to DST6 to
                                                         insure tagging is symmetric between source/destination flows. */
	uint64_t src                          : 16; /**< Secret for the source tuple tag CRC calculation. */
#else
	uint64_t src                          : 16;
	uint64_t dst                          : 16;
	uint64_t src6                         : 16;
	uint64_t dst6                         : 16;
#endif
	} s;
	struct cvmx_pki_tag_secret_s          cn78xx;
};
typedef union cvmx_pki_tag_secret cvmx_pki_tag_secret_t;

/**
 * cvmx_pki_x2p_req_ofl
 */
union cvmx_pki_x2p_req_ofl {
	uint64_t u64;
	struct cvmx_pki_x2p_req_ofl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t x2p_did                      : 4;  /**< When PKI_GEN_INT[X2P_REQ_OFL] is set, this field latches the X2P device id number which
                                                         attempted to overflow the allowed outstanding requests to PKI. */
#else
	uint64_t x2p_did                      : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_pki_x2p_req_ofl_s         cn78xx;
};
typedef union cvmx_pki_x2p_req_ofl cvmx_pki_x2p_req_ofl_t;

#endif
