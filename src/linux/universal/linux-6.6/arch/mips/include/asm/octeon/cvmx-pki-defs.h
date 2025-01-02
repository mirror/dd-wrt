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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 511))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 511)))))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 511))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 511)))))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 4095))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 4095))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 4095))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 4095)))))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_PKI_CLKEN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000410ull);
}
#else
#define CVMX_PKI_CLKEN (CVMX_ADD_IO_SEG(0x0001180044000410ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_ECC_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PKI_CLX_ECC_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118004400C020ull) + ((offset) & 3) * 0x10000ull;
}
#else
#define CVMX_PKI_CLX_ECC_CTL(offset) (CVMX_ADD_IO_SEG(0x000118004400C020ull) + ((offset) & 3) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_ECC_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PKI_CLX_ECC_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118004400C010ull) + ((offset) & 3) * 0x10000ull;
}
#else
#define CVMX_PKI_CLX_ECC_INT(offset) (CVMX_ADD_IO_SEG(0x000118004400C010ull) + ((offset) & 3) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PKI_CLX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118004400C000ull) + ((offset) & 3) * 0x10000ull;
}
#else
#define CVMX_PKI_CLX_INT(offset) (CVMX_ADD_IO_SEG(0x000118004400C000ull) + ((offset) & 3) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_PCAMX_ACTIONX(unsigned long a, unsigned long b, unsigned long c)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((a <= 1)) && ((b <= 1)) && ((c <= 191)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((a <= 3)) && ((b <= 1)) && ((c <= 191)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((a <= 3)) && ((b <= 1)) && ((c <= 191)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((a <= 1)) && ((b <= 1)) && ((c <= 191))))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((a <= 1)) && ((b <= 1)) && ((c <= 191)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((a <= 3)) && ((b <= 1)) && ((c <= 191)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((a <= 3)) && ((b <= 1)) && ((c <= 191)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((a <= 1)) && ((b <= 1)) && ((c <= 191))))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((a <= 1)) && ((b <= 1)) && ((c <= 191)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((a <= 3)) && ((b <= 1)) && ((c <= 191)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((a <= 3)) && ((b <= 1)) && ((c <= 191)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((a <= 1)) && ((b <= 1)) && ((c <= 191))))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 63)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 63)) && ((block_id <= 1))))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((a <= 1)) && ((b <= 63)) && ((c <= 15)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((a <= 3)) && ((b <= 63)) && ((c <= 15)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((a <= 3)) && ((b <= 63)) && ((c <= 15)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((a <= 1)) && ((b <= 63)) && ((c <= 15))))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 63)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 63)) && ((block_id <= 1))))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 63)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 63)) && ((block_id <= 1))))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 63)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 63)) && ((block_id <= 1))))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 63)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 63)) && ((block_id <= 1))))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 2047)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 2047)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 2047)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 2047)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_PKI_CLX_SMEMX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180044400000ull) + (((offset) & 2047) + ((block_id) & 3) * 0x2000ull) * 8;
}
#else
#define CVMX_PKI_CLX_SMEMX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180044400000ull) + (((offset) & 2047) + ((block_id) & 3) * 0x2000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_START(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PKI_CLX_START(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118004400C030ull) + ((offset) & 3) * 0x10000ull;
}
#else
#define CVMX_PKI_CLX_START(offset) (CVMX_ADD_IO_SEG(0x000118004400C030ull) + ((offset) & 3) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_CLX_STYLEX_ALG(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 63)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 63)) && ((block_id <= 1))))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 63)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 63)) && ((block_id <= 1))))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 63)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 63)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 63)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_PKI_CLX_STYLEX_CFG2(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180044500800ull) + (((offset) & 63) + ((block_id) & 3) * 0x2000ull) * 8;
}
#else
#define CVMX_PKI_CLX_STYLEX_CFG2(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180044500800ull) + (((offset) & 63) + ((block_id) & 3) * 0x2000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_DSTATX_STAT0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKI_DSTATX_STAT0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044C00000ull) + ((offset) & 1023) * 64;
}
#else
#define CVMX_PKI_DSTATX_STAT0(offset) (CVMX_ADD_IO_SEG(0x0001180044C00000ull) + ((offset) & 1023) * 64)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_DSTATX_STAT1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKI_DSTATX_STAT1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044C00008ull) + ((offset) & 1023) * 64;
}
#else
#define CVMX_PKI_DSTATX_STAT1(offset) (CVMX_ADD_IO_SEG(0x0001180044C00008ull) + ((offset) & 1023) * 64)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_DSTATX_STAT2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKI_DSTATX_STAT2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044C00010ull) + ((offset) & 1023) * 64;
}
#else
#define CVMX_PKI_DSTATX_STAT2(offset) (CVMX_ADD_IO_SEG(0x0001180044C00010ull) + ((offset) & 1023) * 64)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_DSTATX_STAT3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKI_DSTATX_STAT3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044C00018ull) + ((offset) & 1023) * 64;
}
#else
#define CVMX_PKI_DSTATX_STAT3(offset) (CVMX_ADD_IO_SEG(0x0001180044C00018ull) + ((offset) & 1023) * 64)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_DSTATX_STAT4(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKI_DSTATX_STAT4(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044C00020ull) + ((offset) & 1023) * 64;
}
#else
#define CVMX_PKI_DSTATX_STAT4(offset) (CVMX_ADD_IO_SEG(0x0001180044C00020ull) + ((offset) & 1023) * 64)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_ECC_CTL0 CVMX_PKI_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PKI_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_PKI_ICGX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118004400A000ull);
}
#else
#define CVMX_PKI_ICGX_CFG(offset) (CVMX_ADD_IO_SEG(0x000118004400A000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_IMEMX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2047))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2047))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2047))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 2047)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKI_LTYPEX_MAP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044005000ull) + ((offset) & 31) * 8;
}
#else
#define CVMX_PKI_LTYPEX_MAP(offset) (CVMX_ADD_IO_SEG(0x0001180044005000ull) + ((offset) & 31) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_PBE_ECO CVMX_PKI_PBE_ECO_FUNC()
static inline uint64_t CVMX_PKI_PBE_ECO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_PKI_PBE_ECO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000710ull);
}
#else
#define CVMX_PKI_PBE_ECO (CVMX_ADD_IO_SEG(0x0001180044000710ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_PCAM_LOOKUP CVMX_PKI_PCAM_LOOKUP_FUNC()
static inline uint64_t CVMX_PKI_PCAM_LOOKUP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_PKI_PFE_DIAG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000560ull);
}
#else
#define CVMX_PKI_PFE_DIAG (CVMX_ADD_IO_SEG(0x0001180044000560ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_PFE_ECO CVMX_PKI_PFE_ECO_FUNC()
static inline uint64_t CVMX_PKI_PFE_ECO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_PKI_PFE_ECO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000720ull);
}
#else
#define CVMX_PKI_PFE_ECO (CVMX_ADD_IO_SEG(0x0001180044000720ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_PIX_CLKEN CVMX_PKI_PIX_CLKEN_FUNC()
static inline uint64_t CVMX_PKI_PIX_CLKEN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_PKI_PIX_CLKEN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000600ull);
}
#else
#define CVMX_PKI_PIX_CLKEN (CVMX_ADD_IO_SEG(0x0001180044000600ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_PIX_DIAG CVMX_PKI_PIX_DIAG_FUNC()
static inline uint64_t CVMX_PKI_PIX_DIAG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_PKI_PIX_DIAG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000580ull);
}
#else
#define CVMX_PKI_PIX_DIAG (CVMX_ADD_IO_SEG(0x0001180044000580ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_PIX_ECO CVMX_PKI_PIX_ECO_FUNC()
static inline uint64_t CVMX_PKI_PIX_ECO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_PKI_PIX_ECO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000700ull);
}
#else
#define CVMX_PKI_PIX_ECO (CVMX_ADD_IO_SEG(0x0001180044000700ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_PKINDX_ICGSEL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_PKI_PKT_ERR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000030ull);
}
#else
#define CVMX_PKI_PKT_ERR (CVMX_ADD_IO_SEG(0x0001180044000030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKI_PTAG_AVAIL CVMX_PKI_PTAG_AVAIL_FUNC()
static inline uint64_t CVMX_PKI_PTAG_AVAIL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_PKI_PTAG_AVAIL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180044000130ull);
}
#else
#define CVMX_PKI_PTAG_AVAIL (CVMX_ADD_IO_SEG(0x0001180044000130ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_QPG_TBLBX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2047))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2047))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 2047)))))
		cvmx_warn("CVMX_PKI_QPG_TBLBX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180044820000ull) + ((offset) & 2047) * 8;
}
#else
#define CVMX_PKI_QPG_TBLBX(offset) (CVMX_ADD_IO_SEG(0x0001180044820000ull) + ((offset) & 2047) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKI_QPG_TBLX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2047))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2047))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2047))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 2047)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 31)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 31)))))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
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
	struct cvmx_pki_active0_s             cn73xx;
	struct cvmx_pki_active0_s             cn78xx;
	struct cvmx_pki_active0_s             cn78xxp1;
	struct cvmx_pki_active0_s             cnf75xx;
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
	struct cvmx_pki_active1_s             cn73xx;
	struct cvmx_pki_active1_s             cn78xx;
	struct cvmx_pki_active1_s             cn78xxp1;
	struct cvmx_pki_active1_s             cnf75xx;
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
	struct cvmx_pki_active2_s             cn73xx;
	struct cvmx_pki_active2_s             cn78xx;
	struct cvmx_pki_active2_s             cn78xxp1;
	struct cvmx_pki_active2_s             cnf75xx;
};
typedef union cvmx_pki_active2 cvmx_pki_active2_t;

/**
 * cvmx_pki_aura#_cfg
 *
 * This register configures aura backpressure, etc.
 *
 */
union cvmx_pki_aurax_cfg {
	uint64_t u64;
	struct cvmx_pki_aurax_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t pkt_add                      : 2;  /**< Specifies what to add to FPA_AURA()_CNT when PKI enqueues a packet:
                                                         0x0 = zero.
                                                         0x1 = one.
                                                         0x2 = The number of FPA buffers allocated; i.e. if PKI_STYLE()_BUF[DIS_WQ_DAT]
                                                         is set, PKI_WQE_S[BUFS]+1, else PKI_WQE_S[BUFS].
                                                         0x3 = PKI_WQE_S[LEN] (i.e. the packet length). */
	uint64_t reserved_19_29               : 11;
	uint64_t ena_red                      : 1;  /**< Enable RED drop between PASS and DROP levels. See also
                                                         FPA_AURA()_POOL_LEVELS[RED_ENA] and FPA_AURA()_CNT_LEVELS[RED_ENA]. */
	uint64_t ena_drop                     : 1;  /**< Enable tail drop when maximum DROP level exceeded. See also
                                                         FPA_AURA()_POOL_LEVELS[DROP] and FPA_AURA()_CNT_LEVELS[DROP]. */
	uint64_t ena_bp                       : 1;  /**< Enable asserting backpressure on BPID when maximum DROP level exceeded. See also
                                                         FPA_AURA()_POOL_LEVELS[RED_ENA] and FPA_AURA()_CNT_LEVELS[RED_ENA]. */
	uint64_t reserved_10_15               : 6;
	uint64_t bpid                         : 10; /**< Bpid to assert backpressure on. Values must be 0 to 511. */
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
	struct cvmx_pki_aurax_cfg_s           cn73xx;
	struct cvmx_pki_aurax_cfg_s           cn78xx;
	struct cvmx_pki_aurax_cfg_s           cn78xxp1;
	struct cvmx_pki_aurax_cfg_s           cnf75xx;
};
typedef union cvmx_pki_aurax_cfg cvmx_pki_aurax_cfg_t;

/**
 * cvmx_pki_bist_status0
 *
 * This register indicates BIST status.
 *
 */
union cvmx_pki_bist_status0 {
	uint64_t u64;
	struct cvmx_pki_bist_status0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t bist                         : 31; /**< BIST results. Hardware sets a bit in BIST for memory that fails BIST. */
#else
	uint64_t bist                         : 31;
	uint64_t reserved_31_63               : 33;
#endif
	} s;
	struct cvmx_pki_bist_status0_s        cn73xx;
	struct cvmx_pki_bist_status0_s        cn78xx;
	struct cvmx_pki_bist_status0_s        cn78xxp1;
	struct cvmx_pki_bist_status0_s        cnf75xx;
};
typedef union cvmx_pki_bist_status0 cvmx_pki_bist_status0_t;

/**
 * cvmx_pki_bist_status1
 *
 * This register indicates BIST status.
 *
 */
union cvmx_pki_bist_status1 {
	uint64_t u64;
	struct cvmx_pki_bist_status1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t bist                         : 26; /**< BIST results. Hardware sets a bit in BIST for memory that fails BIST. */
#else
	uint64_t bist                         : 26;
	uint64_t reserved_26_63               : 38;
#endif
	} s;
	struct cvmx_pki_bist_status1_s        cn73xx;
	struct cvmx_pki_bist_status1_s        cn78xx;
	struct cvmx_pki_bist_status1_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_21_63               : 43;
	uint64_t bist                         : 21; /**< BIST results. Hardware sets a bit in BIST for memory that fails BIST. */
#else
	uint64_t bist                         : 21;
	uint64_t reserved_21_63               : 43;
#endif
	} cn78xxp1;
	struct cvmx_pki_bist_status1_s        cnf75xx;
};
typedef union cvmx_pki_bist_status1 cvmx_pki_bist_status1_t;

/**
 * cvmx_pki_bist_status2
 *
 * This register indicates BIST status.
 *
 */
union cvmx_pki_bist_status2 {
	uint64_t u64;
	struct cvmx_pki_bist_status2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t bist                         : 25; /**< BIST results. Hardware sets a bit in BIST for memory that fails BIST. */
#else
	uint64_t bist                         : 25;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_pki_bist_status2_s        cn73xx;
	struct cvmx_pki_bist_status2_s        cn78xx;
	struct cvmx_pki_bist_status2_s        cn78xxp1;
	struct cvmx_pki_bist_status2_s        cnf75xx;
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
	struct cvmx_pki_bpidx_state_s         cn73xx;
	struct cvmx_pki_bpidx_state_s         cn78xx;
	struct cvmx_pki_bpidx_state_s         cn78xxp1;
	struct cvmx_pki_bpidx_state_s         cnf75xx;
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
                                                         0 = Drop the remainder of the packet requesting the buffer, and set
                                                         PKI_WQE_S[ERRLEV,OPCODE] to PKI_ERRLEV_E::RE,PKI_OPCODE_E::RE_MEMOUT.
                                                         1 = Wait until buffers become available, only dropping packets if buffering ahead of PKI
                                                         fills. This may lead to head-of-line blocking of packets on other Auras. */
	uint64_t fpa_cac_dis                  : 1;  /**< Reserved. */
	uint64_t reserved_6_8                 : 3;
	uint64_t pkt_off                      : 1;  /**< Packet buffer off. When this bit is set to 1, the PKI does not buffer the received packet
                                                         data; when it is clear to 0, the PKI works normally, buffering the received packet data. */
	uint64_t reserved_3_4                 : 2;
	uint64_t pbp_en                       : 1;  /**< Bpid enable. When set, enables the sending of bpid level backpressure to the input
                                                         interface.
                                                         The application should not de-assert this bit after asserting it. The receivers of this
                                                         bit may have been put into backpressure mode and can only be released by PKI informing
                                                         them that the backpressure has been released. */
	uint64_t reserved_1_1                 : 1;
	uint64_t pki_en                       : 1;  /**< PKI enable. When set to 1, enables the operation of the PKI. When clear to 0, the PKI
                                                         asserts backpressure on all ports. */
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
	struct cvmx_pki_buf_ctl_s             cn73xx;
	struct cvmx_pki_buf_ctl_s             cn78xx;
	struct cvmx_pki_buf_ctl_s             cn78xxp1;
	struct cvmx_pki_buf_ctl_s             cnf75xx;
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
                                                         0 = This index is read only.
                                                         1 = This index is read-write.
                                                         Write to a non-implemented channel is ignored. Reading a non-implemented channel
                                                         returns all zero data. */
	uint64_t reserved_10_15               : 6;
	uint64_t bpid                         : 10; /**< Bpid from which to receive backpressure. Value must be 0 to 511. */
#else
	uint64_t bpid                         : 10;
	uint64_t reserved_10_15               : 6;
	uint64_t imp                          : 1;
	uint64_t reserved_17_63               : 47;
#endif
	} s;
	struct cvmx_pki_chanx_cfg_s           cn73xx;
	struct cvmx_pki_chanx_cfg_s           cn78xx;
	struct cvmx_pki_chanx_cfg_s           cn78xxp1;
	struct cvmx_pki_chanx_cfg_s           cnf75xx;
};
typedef union cvmx_pki_chanx_cfg cvmx_pki_chanx_cfg_t;

/**
 * cvmx_pki_cl#_ecc_ctl
 *
 * This register configures ECC. All of PKI_CL()_ECC_CTL must be configured identically.
 *
 */
union cvmx_pki_clx_ecc_ctl {
	uint64_t u64;
	struct cvmx_pki_clx_ecc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pcam_en                      : 1;  /**< PCAM ECC scrubber and checking enable. PCAM_EN must be clear when reading or
                                                         writing the PKI_PCAM_RESULT registers. */
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
	uint64_t dmem_cdis                    : 1;  /**< DMEM parity poisoning disable. */
	uint64_t rf_cdis                      : 1;  /**< RF RAM parity poisoning disable. */
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
	struct cvmx_pki_clx_ecc_ctl_s         cn73xx;
	struct cvmx_pki_clx_ecc_ctl_s         cn78xx;
	struct cvmx_pki_clx_ecc_ctl_s         cn78xxp1;
	struct cvmx_pki_clx_ecc_ctl_s         cnf75xx;
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
	uint64_t pcam1_dbe                    : 1;  /**< PCAM1 ECC double-bit error. Throws PKI_INTSN_E::PKI_CL()_ECC_PCAM1_DBE. */
	uint64_t pcam1_sbe                    : 1;  /**< PCAM1 ECC single-bit error. Throws PKI_INTSN_E::PKI_CL()_ECC_PCAM1_SBE. */
	uint64_t pcam0_dbe                    : 1;  /**< PCAM0 ECC double-bit error. Throws PKI_INTSN_E::PKI_CL()_ECC_PCAM0_DBE. */
	uint64_t pcam0_sbe                    : 1;  /**< PCAM0 ECC single-bit error. Throws PKI_INTSN_E::PKI_CL()_ECC_PCAM0_SBE. */
	uint64_t smem_dbe                     : 1;  /**< SMEM ECC double-bit error. Throws PKI_INTSN_E::PKI_CL()_ECC_SMEM_DBE. If
                                                         SMEM_DBE is thrown, software may scrub the error by reloading PKI_CL()_SMEM(). */
	uint64_t smem_sbe                     : 1;  /**< SMEM ECC single-bit error. Throws PKI_INTSN_E::PKI_CL()_ECC_SMEM_SBE. If
                                                         SMEM_SBE is thrown, software may scrub the error by reloading PKI_CL()_SMEM(). */
	uint64_t dmem_perr                    : 1;  /**< DMEM parity error. Throws PKI_INTSN_E::PKI_CL()_ECC_DMEM_PERR. */
	uint64_t rf_perr                      : 1;  /**< RF RAM parity error. Throws PKI_INTSN_E::PKI_CL()_ECC_RF_PERR. */
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
	struct cvmx_pki_clx_ecc_int_s         cn73xx;
	struct cvmx_pki_clx_ecc_int_s         cn78xx;
	struct cvmx_pki_clx_ecc_int_s         cn78xxp1;
	struct cvmx_pki_clx_ecc_int_s         cnf75xx;
};
typedef union cvmx_pki_clx_ecc_int cvmx_pki_clx_ecc_int_t;

/**
 * cvmx_pki_cl#_int
 */
union cvmx_pki_clx_int {
	uint64_t u64;
	struct cvmx_pki_clx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t iptint                       : 1;  /**< PCAM parse engine debug interrupt. Throws PKI_INTSN_E::PKI_CL()_INT_IPTINT. */
	uint64_t sched_conf                   : 1;  /**< PCAM/SMEM internal port conflict. Internal error, should not occur. Throws
                                                         PKI_INTSN_E::PKI_CL()_INT_SCHED_CONF. */
	uint64_t pcam_conf                    : 2;  /**< PCAM() match hit multiple rows, indicating either a soft error in the PCAM or a
                                                         programming error in PKI_CL()_PCAM()_MATCH() or related registers. Throws
                                                         PKI_INTSN_E::PKI_CL()_INT_PCAM_CONF(). Once a conflict is detected, the PCAM state
                                                         is unpredictable and is required to be fully reconfigured before further valid processing
                                                         can take place. */
#else
	uint64_t pcam_conf                    : 2;
	uint64_t sched_conf                   : 1;
	uint64_t iptint                       : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_pki_clx_int_s             cn73xx;
	struct cvmx_pki_clx_int_s             cn78xx;
	struct cvmx_pki_clx_int_s             cn78xxp1;
	struct cvmx_pki_clx_int_s             cnf75xx;
};
typedef union cvmx_pki_clx_int cvmx_pki_clx_int_t;

/**
 * cvmx_pki_cl#_pcam#_action#
 *
 * This register configures the result side of the PCAM. PKI hardware is opaque as to the use
 * of the 32 bits of CAM result.
 *
 * For each legal j and k, PKI_CL(i)_PCAM(j)_ACTION(k) must be configured identically for i=0..1.
 *
 * With the current parse engine code:
 *
 * Action performed based on PCAM lookup using the PKI_CL()_PCAM()_TERM() and
 * PKI_CL()_PCAM()_MATCH() registers.
 *
 * If lookup data matches no PCAM entries, then no action takes place. No matches indicates
 * normal parsing will continue.
 *
 * If data matches multiple PCAM entries, PKI_WQE_S[ERRLEV,OPCODE] of the processed packet may
 * be set to PKI_ERRLEV_E::RE,PKI_OPCODE_E::RE_PKIPCAM and the PKI_CL()_INT[PCAM_CONF] error
 * interrupt is signaled.  Once a conflict is detected, the PCAM state is unpredictable and is
 * required to be fully reconfigured before further valid processing can take place.
 */
union cvmx_pki_clx_pcamx_actionx {
	uint64_t u64;
	struct cvmx_pki_clx_pcamx_actionx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t pmc                          : 7;  /**< Parse mode change. Where to resume parsing after applying the scan offset (if any) as bit
                                                         mask of which parse engine steps to no longer process:
                                                         _ <0> = LA (L2).
                                                         _ <1> = LB (Custom).
                                                         _ <2> = LC (L3).
                                                         _ <3> = LD (Virt).
                                                         _ <4> = LE (IL3).
                                                         _ <5> = LF (L4).
                                                         _ <6> = LG (Custom/Application-specific relative to C, D, E, or F).
                                                         Typically PMC is 0x0 to indicate no parse mode change.  Must be zero for invalid
                                                         entries, or for TERMs that do not allow a parse mode change as specified in the
                                                         PMC column of the PKI_PCAM_TERM_E table.
                                                         Typical values for PMC<5:0> are:
                                                           0x0 = no change in parsing.
                                                           0x1 = Skip further LA parsing; start LB parsing.
                                                           0x3 = Skip further LA/LB parsing; start LC parsing.
                                                           0x7 = Skip further LA-LC parsing; start LD parsing.
                                                           0x38 = Skip LD, LE, LF parsing.
                                                           0x3F = Skip all parsing; no further packet inspection.
                                                         The typical use of PMC<5:0> being nonzero is for Ethertypes or custom headers
                                                         to indicate non-IP follows that Ethertype/custom header. This corresponds to use
                                                         only with PKI_PCAM_TERM_E::ETHTYPE0..3, and PKI_PCAM_TERM_E::L2_CUSTOM.
                                                         Independently PMC<6> may be set to disable LG (LG_CUSTOM) parsing. */
	uint64_t style_add                    : 8;  /**< Resulting interim style adder. If this CAM entry matches, the value to add to the current
                                                         style (may wrap around through 256). Must be zero for invalid entries. */
	uint64_t pf                           : 3;  /**< Parse flag to set. Specifies the parse flag to set when entry matches, see PCAM actions
                                                         may also specify setting one of four user flags in the generated work queue entry,
                                                         PKI_WQE_S[PF1] through PKI_WQE_S[PF4]. These flags are not used by hardware; they are
                                                         intended to
                                                         indicate to software that exceptional handling may be required, such as the presence of an
                                                         encrypted packet:
                                                         _ 0x0 = no change.
                                                         _ 0x1 = Set PKI_WQE_S[PF1].
                                                         _ 0x2 = Set PKI_WQE_S[PF2].
                                                         _ 0x3 = Set PKI_WQE_S[PF3].
                                                         _ 0x4 = Set PKI_WQE_S[PF4].
                                                         _ else = reserved.
                                                         Must be zero for invalid entries. */
	uint64_t setty                        : 5;  /**< Set pointer type. If nonzero, indicates the layer type to be set as described under
                                                         PKI_PCAM_TERM_E. Values are enumerated by PKI_LTYPE_E. Must be zero for invalid entries.
                                                         The PKI_PCAM_TERM_E table enumerates legal/common SETTY values. */
	uint64_t advance                      : 8;  /**< Relative number of bytes to advance scan pointer when entry matches.
                                                         Must be even. Must be zero for invalid entries and for TERMs that do not allow
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
	struct cvmx_pki_clx_pcamx_actionx_s   cn73xx;
	struct cvmx_pki_clx_pcamx_actionx_s   cn78xx;
	struct cvmx_pki_clx_pcamx_actionx_s   cn78xxp1;
	struct cvmx_pki_clx_pcamx_actionx_s   cnf75xx;
};
typedef union cvmx_pki_clx_pcamx_actionx cvmx_pki_clx_pcamx_actionx_t;

/**
 * cvmx_pki_cl#_pcam#_match#
 *
 * This register configures the match side of the PCAM. PKI hardware is opaque as to the use
 * of the 32 bits of CAM data.
 *
 * For each legal j and k, PKI_CL(i)_PCAM(j)_MATCH(k) must be configured identically for i=0..1.
 */
union cvmx_pki_clx_pcamx_matchx {
	uint64_t u64;
	struct cvmx_pki_clx_pcamx_matchx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data1                        : 32; /**< See [DATA0]. */
	uint64_t data0                        : 32; /**< The packet data to compare against. Bits may be ignored in comparison based on [DATA0] and
                                                         [DATA1]. If the entry matches, PKI_CL()_PCAM()_ACTION() will determine the
                                                         action to be taken. See PKI_PCAM_TERM_E for comparison bit definitions.
                                                         The field value is ternary, where each bit matches as follows:
                                                         _ [DATA1]<n>=0, [DATA0]<n>=0: Always match; data<n> don't care.
                                                         _ [DATA1]<n>=0, [DATA0]<n>=1: Match when data<n> == 0.
                                                         _ [DATA1]<n>=1, [DATA0]<n>=0: Match when data<n> == 1.
                                                         _ [DATA1]<n>=1, [DATA0]<n>=1: Reserved. */
#else
	uint64_t data0                        : 32;
	uint64_t data1                        : 32;
#endif
	} s;
	struct cvmx_pki_clx_pcamx_matchx_s    cn73xx;
	struct cvmx_pki_clx_pcamx_matchx_s    cn78xx;
	struct cvmx_pki_clx_pcamx_matchx_s    cn78xxp1;
	struct cvmx_pki_clx_pcamx_matchx_s    cnf75xx;
};
typedef union cvmx_pki_clx_pcamx_matchx cvmx_pki_clx_pcamx_matchx_t;

/**
 * cvmx_pki_cl#_pcam#_term#
 *
 * This register configures the match side of the PCAM. PKI hardware is opaque as to the use
 * of the 16 bits of CAM data; the split between TERM and STYLE is defined by the
 * parse engine.
 *
 * For each legal j and k, PKI_CL(i)_PCAM(j)_TERM(k) must be configured identically for i=0..1.
 */
union cvmx_pki_clx_pcamx_termx {
	uint64_t u64;
	struct cvmx_pki_clx_pcamx_termx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t valid                        : 1;  /**< Valid. */
	uint64_t reserved_48_62               : 15;
	uint64_t term1                        : 8;  /**< See [TERM0]. */
	uint64_t style1                       : 8;  /**< See [STYLE0]. */
	uint64_t reserved_16_31               : 16;
	uint64_t term0                        : 8;  /**< Comparison type. Enumerated by PKI_PCAM_TERM_E. The field value is ternary, where each
                                                         bit matches as follows:
                                                         _ [TERM1]<n>=0, [TERM0]<n>=0: Always match; data<n> don't care.
                                                         _ [TERM1]<n>=0, [TERM0]<n>=1: Match when data<n> == 0.
                                                         _ [TERM1]<n>=1, [TERM0]<n>=0: Match when data<n> == 1.
                                                         _ [TERM1]<n>=1, [TERM0]<n>=1: Reserved. */
	uint64_t style0                       : 8;  /**< Previous interim style. The style that must have been calculated by the port
                                                         PKI_CL()_PKIND()_STYLE[STYLE] or as modified by previous CAM hits'
                                                         PKI_CL()_PCAM()_ACTION()[STYLE_ADD]. This is used to form AND style matches.
                                                         The field value is ternary, where each bit matches as follows:
                                                         _ [STYLE1]<n>=0, [STYLE0]<n>=0: Always match; data<n> don't care.
                                                         _ [STYLE1]<n>=0, [STYLE0]<n>=1: Match when data<n> == 0.
                                                         _ [STYLE1]<n>=1, [STYLE0]<n>=0: Match when data<n> == 1.
                                                         _ [STYLE1]<n>=1, [STYLE0]<n>=1: Reserved. */
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
	struct cvmx_pki_clx_pcamx_termx_s     cn73xx;
	struct cvmx_pki_clx_pcamx_termx_s     cn78xx;
	struct cvmx_pki_clx_pcamx_termx_s     cn78xxp1;
	struct cvmx_pki_clx_pcamx_termx_s     cnf75xx;
};
typedef union cvmx_pki_clx_pcamx_termx cvmx_pki_clx_pcamx_termx_t;

/**
 * cvmx_pki_cl#_pkind#_cfg
 *
 * This register is inside PKI_CL()_PKIND()_KMEM(). These CSRs are used only by
 * the PKI parse engine.
 *
 * For each legal j, PKI_CL(i)_PKIND(j)_CFG must be configured identically for i=0..1.
 */
union cvmx_pki_clx_pkindx_cfg {
	uint64_t u64;
	struct cvmx_pki_clx_pkindx_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t lg_custom_layer              : 3;  /**< Layer G custom match enable.
                                                         0x0 = Disable custom LG header extraction
                                                         0x1 = Enable custom LG header extraction after layer C.
                                                         0x2 = Enable custom LG header extraction after layer D.
                                                         0x3 = Enable custom LG header extraction after layer E.
                                                         0x4 = Enable custom LG header extraction after layer F.
                                                         0x5-0x7 = Reserved. */
	uint64_t fcs_pres                     : 1;  /**< FCS present.
                                                         0 = FCS not present. FCS may not be checked nor stripped.
                                                         1 = FCS present; the last four bytes of the packet are part of the FCS and may not be
                                                         considered part of a IP, TCP or other header for length error checks.
                                                         When either PKI_CL()_STYLE()_CFG[FCS_CHK or FCS_STRIP] are set, the
                                                         corresponding [FCS_PRES] must be set. */
	uint64_t mpls_en                      : 1;  /**< Enable MPLS parsing.
                                                         0 = Any MPLS labels are ignored, but may be handled by custom Ethertype PCAM matchers.
                                                         1 = MPLS label stacks are parsed and skipped over. */
	uint64_t inst_hdr                     : 1;  /**< INST header. When set, a PKI_INST_HDR_S is present PKI_CL()_PKIND()_SKIP[INST_SKIP]
                                                         bytes into the packet received by PKI. */
	uint64_t lg_custom                    : 1;  /**< Enable parsing LG_CUSTOM layers. */
	uint64_t fulc_en                      : 1;  /**< Enable Fulcrum tag parsing.
                                                         0 = Any Fulcrum header is ignored.
                                                         1 = Fulcrum header is parsed.
                                                         [FULC_EN] must be clear when any of [HG_EN,HG2_EN,DSA_EN] are set. */
	uint64_t dsa_en                       : 1;  /**< Enable DSA parsing.
                                                         0 = Any DSA header is ignored.
                                                         1 = DSA is parsed.
                                                         [DSA_EN] must be clear when any of [HG_EN,HG2_EN,FULC_EN] are set. */
	uint64_t hg2_en                       : 1;  /**< Enable HiGig 2 parsing.
                                                         0 = Any HiGig2 header is ignored.
                                                         1 = HiGig2 is parsed.
                                                         [HG2_EN] must be clear when any of [HG_EN,FULC_EN,DSA_EN] is set. */
	uint64_t hg_en                        : 1;  /**< Enable HiGig parsing.
                                                         0 = Any HiGig header is ignored.
                                                         1 = HiGig is parsed.
                                                         [HG_EN] must be clear when any of [HG2_EN,FULC_EN,DSA_EN] is set. */
#else
	uint64_t hg_en                        : 1;
	uint64_t hg2_en                       : 1;
	uint64_t dsa_en                       : 1;
	uint64_t fulc_en                      : 1;
	uint64_t lg_custom                    : 1;
	uint64_t inst_hdr                     : 1;
	uint64_t mpls_en                      : 1;
	uint64_t fcs_pres                     : 1;
	uint64_t lg_custom_layer              : 3;
	uint64_t reserved_11_63               : 53;
#endif
	} s;
	struct cvmx_pki_clx_pkindx_cfg_s      cn73xx;
	struct cvmx_pki_clx_pkindx_cfg_s      cn78xx;
	struct cvmx_pki_clx_pkindx_cfg_s      cn78xxp1;
	struct cvmx_pki_clx_pkindx_cfg_s      cnf75xx;
};
typedef union cvmx_pki_clx_pkindx_cfg cvmx_pki_clx_pkindx_cfg_t;

/**
 * cvmx_pki_cl#_pkind#_kmem#
 *
 * This register initializes the KMEM, which initializes the parse engine state for each
 * pkind. These CSRs are used only by the PKI parse engine.
 *
 * Inside the KMEM are the following parse engine registers. These registers are the
 * preferred access method for software:
 * * PKI_CL()_PKIND()_CFG.
 * * PKI_CL()_PKIND()_STYLE.
 * * PKI_CL()_PKIND()_SKIP.
 * * PKI_CL()_PKIND()_L2_CUSTOM.
 * * PKI_CL()_PKIND()_LG_CUSTOM.
 *
 * To avoid overlapping addresses, these aliases have address bit 20 set in contrast to
 * this register; the PKI address decoder ignores bit 20 when accessing
 * PKI_CL()_PKIND()_KMEM().
 *
 * Software must reload the PKI_CL()_PKIND()_KMEM() registers upon the detection of
 * PKI_ECC_INT0[KMEM_SBE] or PKI_ECC_INT0[KMEM_DBE].
 *
 * For each legal j and k value, PKI_CL(i)_PKIND(j)_KMEM(k) must be configured
 * identically for i=0..1.
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
	struct cvmx_pki_clx_pkindx_kmemx_s    cn73xx;
	struct cvmx_pki_clx_pkindx_kmemx_s    cn78xx;
	struct cvmx_pki_clx_pkindx_kmemx_s    cn78xxp1;
	struct cvmx_pki_clx_pkindx_kmemx_s    cnf75xx;
};
typedef union cvmx_pki_clx_pkindx_kmemx cvmx_pki_clx_pkindx_kmemx_t;

/**
 * cvmx_pki_cl#_pkind#_l2_custom
 *
 * This register is inside PKI_CL()_PKIND()_KMEM(). These CSRs are used only by
 * the PKI parse engine.
 *
 * For each legal j, PKI_CL(i)_PKIND(j)_L2_CUSTOM must be configured identically for i=0..1.
 */
union cvmx_pki_clx_pkindx_l2_custom {
	uint64_t u64;
	struct cvmx_pki_clx_pkindx_l2_custom_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t valid                        : 1;  /**< Valid.
                                                         0 = Disable custom L2 header extraction.
                                                         1 = Enable custom L2 header extraction. */
	uint64_t reserved_8_14                : 7;
	uint64_t offset                       : 8;  /**< Scan offset. Pointer to first byte of 32-bit custom extraction header, as
                                                         absolute number of bytes from beginning of packet. Must be even. If PTP_MODE,
                                                         the 8-byte timestamp is prepended to the packet, and must be included in
                                                         counting offset bytes. */
#else
	uint64_t offset                       : 8;
	uint64_t reserved_8_14                : 7;
	uint64_t valid                        : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_pki_clx_pkindx_l2_custom_s cn73xx;
	struct cvmx_pki_clx_pkindx_l2_custom_s cn78xx;
	struct cvmx_pki_clx_pkindx_l2_custom_s cn78xxp1;
	struct cvmx_pki_clx_pkindx_l2_custom_s cnf75xx;
};
typedef union cvmx_pki_clx_pkindx_l2_custom cvmx_pki_clx_pkindx_l2_custom_t;

/**
 * cvmx_pki_cl#_pkind#_lg_custom
 *
 * This register is inside PKI_CL()_PKIND()_KMEM(). These CSRs are used only by
 * the PKI parse engine.
 *
 * For each legal j, PKI_CL(i)_PKIND(j)_LG_CUSTOM must be configured identically for i=0..1.
 */
union cvmx_pki_clx_pkindx_lg_custom {
	uint64_t u64;
	struct cvmx_pki_clx_pkindx_lg_custom_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t offset                       : 8;  /**< Scan offset. Pointer to first byte of 32-bit custom extraction header, as
                                                         relative number of bytes from PKI_WQE_S[LCPTR], PKI_WQE_S[LDPTR],
                                                         PKI_WQE_S[LEPTR], PKI_WQE_S[LFPTR], as selected by
                                                         PKI_CL()_PKIND()_CFG[LG_CUSTOM_LAYER]. Must be even. */
#else
	uint64_t offset                       : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_pki_clx_pkindx_lg_custom_s cn73xx;
	struct cvmx_pki_clx_pkindx_lg_custom_s cn78xx;
	struct cvmx_pki_clx_pkindx_lg_custom_s cn78xxp1;
	struct cvmx_pki_clx_pkindx_lg_custom_s cnf75xx;
};
typedef union cvmx_pki_clx_pkindx_lg_custom cvmx_pki_clx_pkindx_lg_custom_t;

/**
 * cvmx_pki_cl#_pkind#_skip
 *
 * This register is inside PKI_CL()_PKIND()_KMEM(). These CSRs are used only by
 * the PKI parse engine.
 *
 * For each legal j, PKI_CL(i)_PKIND(j)_SKIP must be configured identically for i=0..1.
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
                                                         PKI_CL()_PKIND()_CFG[INST_HDR] is set, points at the first byte of the
                                                         instruction header. If INST_HDR is clear, points at the first byte to begin parsing at.
                                                         The skip must be even. If PTP_MODE, the 8-byte timestamp is prepended to the packet, and
                                                         INST_SKIP must be at least 8. */
#else
	uint64_t inst_skip                    : 8;
	uint64_t fcs_skip                     : 8;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_pki_clx_pkindx_skip_s     cn73xx;
	struct cvmx_pki_clx_pkindx_skip_s     cn78xx;
	struct cvmx_pki_clx_pkindx_skip_s     cn78xxp1;
	struct cvmx_pki_clx_pkindx_skip_s     cnf75xx;
};
typedef union cvmx_pki_clx_pkindx_skip cvmx_pki_clx_pkindx_skip_t;

/**
 * cvmx_pki_cl#_pkind#_style
 *
 * This register is inside PKI_CL()_PKIND()_KMEM(). These CSRs are used only by
 * the PKI parse engine.
 *
 * For each legal j, PKI_CL(i)_PKIND(j)_STYLE must be configured identically for i=0..1.
 */
union cvmx_pki_clx_pkindx_style {
	uint64_t u64;
	struct cvmx_pki_clx_pkindx_style_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t pm                           : 7;  /**< Initial parse mode. Bit mask of which steps the parse engine should perform, refer
                                                         to Parse Mode:
                                                         _ <0> = LA - Layer A (Ethernet dest+src, Higig/Fulcrum/DSA).
                                                         _ <1> = LB - Layer B (Ethertype/VLAN, MPLS/ARP/REVARP/SNAP).
                                                         _ <2> = LC - Layer C (Outer IPv4 or IPv6).
                                                         _ <3> = LD - Layer D (Geneve/VXLAN/VXLANGPE/NVGRE).
                                                         _ <4> = LE - Layer E (Inner IPv4 or IPv6).
                                                         _ <5> = LF - Layer F (UDP, TCP, SCTP, IPCOMP, IPSEC ESP/AH, GRE).
                                                         _ <6> = LG - Layer G (Custom/Application).
                                                         The legal values are:
                                                         _ 0x0 = Parse LA..LG.
                                                         _ 0x1 = Parse LB..LG.
                                                         _ 0x3 = Parse LC..LG.
                                                         _ 0x7F = Parse none and error check none of LA..LG.
                                                         _ else = Reserved. */
	uint64_t style                        : 8;  /**< Initial style. Initial style number for packets on this port, will remain as final style
                                                         if no PCAM entries match the packet. Note only 64 final styles exist, the upper two bits
                                                         will only be used for PCAM matching. */
#else
	uint64_t style                        : 8;
	uint64_t pm                           : 7;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_pki_clx_pkindx_style_s    cn73xx;
	struct cvmx_pki_clx_pkindx_style_s    cn78xx;
	struct cvmx_pki_clx_pkindx_style_s    cn78xxp1;
	struct cvmx_pki_clx_pkindx_style_s    cnf75xx;
};
typedef union cvmx_pki_clx_pkindx_style cvmx_pki_clx_pkindx_style_t;

/**
 * cvmx_pki_cl#_smem#
 *
 * This register initializes the SMEM, which configures the parse engine. These CSRs
 * are used by the PKI parse engine and other PKI hardware.
 *
 * Inside the SMEM are the following parse engine registers. These registers are the
 * preferred access method for software:
 * * PKI_CL()_STYLE()_CFG
 * * PKI_CL()_STYLE()_CFG2
 * * PKI_CL()_STYLE()_ALG
 *
 * To avoid overlapping addresses, these aliases have address bit 20 set in contrast to
 * this register; the PKI address decoder ignores bit 20 when accessing
 * PKI_CL()_SMEM().
 *
 * Software must reload the PKI_CL()_SMEM() registers upon the detection of
 * PKI_CL()_ECC_INT[SMEM_SBE] or PKI_CL()_ECC_INT[SMEM_DBE].
 *
 * For each legal j, PKI_CL(i)_SMEM(j) must be configured identically for i=0..1.
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
	struct cvmx_pki_clx_smemx_s           cn73xx;
	struct cvmx_pki_clx_smemx_s           cn78xx;
	struct cvmx_pki_clx_smemx_s           cn78xxp1;
	struct cvmx_pki_clx_smemx_s           cnf75xx;
};
typedef union cvmx_pki_clx_smemx cvmx_pki_clx_smemx_t;

/**
 * cvmx_pki_cl#_start
 *
 * This register configures a cluster. All of PKI_CL()_START must be programmed identically.
 *
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
	struct cvmx_pki_clx_start_s           cn73xx;
	struct cvmx_pki_clx_start_s           cn78xx;
	struct cvmx_pki_clx_start_s           cn78xxp1;
	struct cvmx_pki_clx_start_s           cnf75xx;
};
typedef union cvmx_pki_clx_start cvmx_pki_clx_start_t;

/**
 * cvmx_pki_cl#_style#_alg
 *
 * This register is inside PKI_CL()_SMEM(). These CSRs are used only by
 * the PKI parse engine.
 *
 * For each legal j, PKI_CL(i)_STYLE(j)_ALG must be configured identically for i=0..1.
 */
union cvmx_pki_clx_stylex_alg {
	uint64_t u64;
	struct cvmx_pki_clx_stylex_alg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t tt                           : 2;  /**< SSO tag type to schedule to, enumerated by SSO_TT_E. */
	uint64_t apad_nip                     : 3;  /**< Value for PKI_WQE_S[APAD] when packet is not IP. */
	uint64_t qpg_qos                      : 3;  /**< Algorithm to select QoS field in QPG calculation. Enumerated by PKI_QPGQOS_E. */
	uint64_t qpg_port_sh                  : 3;  /**< Number of bits to shift port number in QPG calculation. */
	uint64_t qpg_port_msb                 : 4;  /**< MSB to take from port number in QPG calculation.
                                                         0 = Exclude port number from QPG.
                                                         4 = Include port<3:0>.
                                                         8 = Include port<7:0>.
                                                         _ All other values reserved. */
	uint64_t reserved_11_16               : 6;
	uint64_t tag_vni                      : 1;  /**< When NVGRE/VXLAN/VXLANGPE/GENEVE is found, include VNI in tag generation. When NVGRE is
                                                         found, include VSID. */
	uint64_t tag_gtp                      : 1;  /**< When GTP is parsed, include GTP's TEID in tag generation.
                                                         The GTP PKI_PCAM_TERM_E::L4_PORT PCAM entry must have
                                                         PKI_CL()_PCAM()_ACTION()[SETTY,ADVANCE]
                                                         be PKI_LTYPE_E::GTP,8 (ADVANCE needs to skip over the UDP header). */
	uint64_t tag_spi                      : 1;  /**< Include AH/ESP/GRE in tag generation.
                                                         0 = Exclude AH/ESP/GRE in tag generation.
                                                         1 = If IP SEC is parsed, include AH/ESP SPI field in tag generation, or if GRE
                                                         found, include GRE call number in tag generation. */
	uint64_t tag_syn                      : 1;  /**< Exclude source address for TCP SYN packets.
                                                         0 = Include source address if TAG_SRC_* used.
                                                         1 = Exclude source address for TCP packets with SYN & !ACK, even if TAG_SRC_* set. */
	uint64_t tag_pctl                     : 1;  /**< Include final IPv4 protocol or IPv6 next header in tag generation. */
	uint64_t tag_vs1                      : 1;  /**< When VLAN stacking is parsed, include second (network order) VLAN in tuple tag generation. */
	uint64_t tag_vs0                      : 1;  /**< When VLAN stacking is parsed, include first (network order) VLAN in tuple tag generation. */
	uint64_t tag_vlan                     : 1;  /**< Reserved. */
	uint64_t tag_mpls0                    : 1;  /**< Reserved. */
	uint64_t tag_prt                      : 1;  /**< Include interface port in tag hash. */
	uint64_t wqe_vs                       : 1;  /**< Which VLAN to put into PKI_WQE_S[VLPTR] when VLAN stacking.
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
	struct cvmx_pki_clx_stylex_alg_s      cn73xx;
	struct cvmx_pki_clx_stylex_alg_s      cn78xx;
	struct cvmx_pki_clx_stylex_alg_s      cn78xxp1;
	struct cvmx_pki_clx_stylex_alg_s      cnf75xx;
};
typedef union cvmx_pki_clx_stylex_alg cvmx_pki_clx_stylex_alg_t;

/**
 * cvmx_pki_cl#_style#_cfg
 *
 * This register is inside PKI_CL()_SMEM(). These CSRs are used by
 * the PKI parse engine and other PKI hardware.
 *
 * For each legal j, PKI_CL(i)_STYLE(j)_CFG must be configured identically for i=0..1.
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
	uint64_t lenerr_en                    : 1;  /**< L2 length error check enable. When set, the hardware checks that the number of packet
                                                         bytes following the L2 length field (excluding FCS) is at least as large as the L2
                                                         length field value whenever the Ethertype / L2 length field is <= 1535.
                                                         The PKI L2 length check assumes that the packet received by PKI contains a trailing FCS
                                                         when
                                                         the PKI_CL()_PKIND()_CFG[FCS_PRES] for the pkind of the packet is set. */
	uint64_t lenerr_eqpad                 : 1;  /**< Reserved. Must be zero. */
	uint64_t minmax_sel                   : 1;  /**< Selects which PKI_FRM_LEN_CHK() register is used for this pkind for MINERR and MAXERR
                                                         checks.
                                                         0 = use PKI_FRM_LEN_CHK(0).
                                                         1 = use PKI_FRM_LEN_CHK(1). */
	uint64_t maxerr_en                    : 1;  /**< Max frame error check enable. See PKI_FRM_LEN_CHK()[MAXLEN], [MINMAX_SEL],
                                                         PKI_OPCODE_E::L2_OVERSIZE,L2_OVERRUN, PKI_ERRLEV_E::LA, and PKI_WQE_S[ERRLEV,OPCODE]. */
	uint64_t minerr_en                    : 1;  /**< Min frame error check enable. See PKI_FRM_LEN_CHK()[MINLEN], [MINMAX_SEL],
                                                         PKI_OPCODE_E::L2_UNDERSIZE,L2_FRAGMENT, PKI_ERRLEV_E::LA, and PKI_WQE_S[ERRLEV,OPCODE]. */
	uint64_t qpg_dis_grptag               : 1;  /**< Disable computing group using PKI_WQE_S[TAG]. See PKI_WQE_S[GRP]. */
	uint64_t fcs_strip                    : 1;  /**< Strip L2 FCS bytes from packet, decrease PKI_WQE_S[LEN] by 4 bytes. Corresponding
                                                         PKI_CL()_PKIND()_CFG[FCS_PRES] must be set when [FCS_STRIP] is set.
                                                         See the PKI_OPCODE_E::L2_PUNY exception case. */
	uint64_t fcs_chk                      : 1;  /**< FCS checking enabled. Corresponding PKI_CL()_PKIND()_CFG[FCS_PRES] must be set
                                                         when [FCS_CHK] is set. See PKI_OPCODE_E::L2_PFCS,L2_FRAGMENT,L2_OVERRUN,
                                                         PKI_ERRLEV_E::LA, and PKI_WQE_S[ERRLEV,OPCODE]. */
	uint64_t rawdrp                       : 1;  /**< Allow RAW packet drop.
                                                         0 = Never drop packets with PKI_WQE_S[RAW] set.
                                                         1 = Allow the PKI to drop RAW packets based on PKI_AURA()_CFG[ENA_RED/ENA_DROP]. */
	uint64_t drop                         : 1;  /**< Force packet dropping.
                                                         0 = Drop packet based on PKI_AURA()_CFG[ENA_RED/ENA_DROP].
                                                         1 = Always drop the packet. Overrides [NODROP], [RAWDRP]. */
	uint64_t nodrop                       : 1;  /**< Disable QoS packet drop.
                                                         0 = Allowed to drop packet based on PKI_AURA()_CFG[ENA_RED/ENA_DROP].
                                                         1 = Never drop the packet. Overrides [RAWDRP]. */
	uint64_t qpg_dis_padd                 : 1;  /**< Disable computing port adder by QPG algorithm. See PKI_WQE_S[CHAN]. */
	uint64_t qpg_dis_grp                  : 1;  /**< Disable computing group by QPG algorithm.
                                                         [QPG_DIS_GRP] should normally always be clear. */
	uint64_t qpg_dis_aura                 : 1;  /**< Disable computing aura by QPG algorithm.
                                                         [QPG_DIS_AURA] should normally always be clear. */
	uint64_t reserved_11_15               : 5;
	uint64_t qpg_base                     : 11; /**< Base index into PKI_QPG_TBL(). */
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
	struct cvmx_pki_clx_stylex_cfg_s      cn73xx;
	struct cvmx_pki_clx_stylex_cfg_s      cn78xx;
	struct cvmx_pki_clx_stylex_cfg_s      cn78xxp1;
	struct cvmx_pki_clx_stylex_cfg_s      cnf75xx;
};
typedef union cvmx_pki_clx_stylex_cfg cvmx_pki_clx_stylex_cfg_t;

/**
 * cvmx_pki_cl#_style#_cfg2
 *
 * This register is inside PKI_CL()_SMEM(). These CSRs are used by
 * the PKI parse engine and other PKI hardware.
 *
 * For each legal j, PKI_CL(i)_STYLE(j)_CFG2 must be configured identically for i=0..1.
 */
union cvmx_pki_clx_stylex_cfg2 {
	uint64_t u64;
	struct cvmx_pki_clx_stylex_cfg2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t tag_inc                      : 4;  /**< Include masked tags using PKI_TAG_INC()_MASK. Each bit indicates to include the
                                                         corresponding PKI_TAG_INC()_MASK range. See also PKI_TAG_INC()_CTL
                                                         and PKI_WQE_S[TAG]. */
	uint64_t reserved_25_27               : 3;
	uint64_t tag_masken                   : 1;  /**< When set, apply PKI_STYLE()_TAG_MASK to computed tag. See PKI_WQE_S[TAG]. */
	uint64_t tag_src_lg                   : 1;  /**< Reserved. Must be zero. */
	uint64_t tag_src_lf                   : 1;  /**< When set, PKI hardware includes the (inner or outer) IPv4/IPv6 TCP/UDP/SCTP
                                                         source port in tuple tag generation when PKI_LTYPE(PKI_WQE_S[LFTY])_MAP[BELTYPE]
                                                         equals any of PKI_BELTYPE_E::TCP,UDP,SCTP. PKI_WQE_S[LFPTR] points to the
                                                         TCP/UDP/SCTP header in this case. In an IP-in-IP case, [TAG_SRC_LF] refers
                                                         to the inner IPv4/IPv6 TCP/UDP/SCTP, else the outer/only IPv4/IPv6 TCP/UDP/SCTP.
                                                         See PKI_WQE_S[TAG]. */
	uint64_t tag_src_le                   : 1;  /**< When set, PKI hardware includes the inner IPv4/IPv6 source address (in an
                                                         IP-in-IP case) in tuple tag generation when PKI_LTYPE(PKI_WQE_S[LETY])_MAP[BELTYPE]
                                                         equals either of PKI_BELTYPE_E::IP4,IP6. PKI_WQE_S[LEPTR] points to the inner
                                                         IPv4/IPv6 header in this case. See PKI_WQE_S[TAG]. */
	uint64_t tag_src_ld                   : 1;  /**< When set, PKI hardware includes the outer IPv4/IPv6 UDP source port (in an
                                                         IP-in-IP case) in tuple tag generation when PKI_LTYPE(PKI_WQE_S[LDTY])_MAP[BELTYPE]
                                                         equals PKI_BELTYPE_E::UDP. PKI_WQE_S[LDPTR] points to the UDP header in this case.
                                                         See PKI_WQE_S[TAG]. */
	uint64_t tag_src_lc                   : 1;  /**< When set, PKI hardware includes the outermost IPv4/IPv6 source address
                                                         in tuple tag generation when PKI_LTYPE(PKI_WQE_S[LCTY])_MAP[BELTYPE]
                                                         equals either of PKI_BELTYPE_E::IP4,IP6. PKI_WQE_S[LCPTR] points to the
                                                         IPv4/IPv6 header in this case. See PKI_WQE_S[TAG]. */
	uint64_t tag_src_lb                   : 1;  /**< Reserved. Must be zero. */
	uint64_t tag_dst_lg                   : 1;  /**< Reserved. Must be zero. */
	uint64_t tag_dst_lf                   : 1;  /**< When set, PKI hardware includes the (inner or outer) IPv4/IPv6 TCP/UDP/SCTP
                                                         destination port in tuple tag generation when PKI_LTYPE(PKI_WQE_S[LFTY])_MAP[BELTYPE]
                                                         equals any of PKI_BELTYPE_E::TCP,UDP,SCTP. PKI_WQE_S[LFPTR] points to the
                                                         TCP/UDP/SCTP header in this case. In an IP-in-IP case, [TAG_DST_LF] refers
                                                         to the inner IPv4/IPv6 TCP/UDP/SCTP, else the outer/only IPv4/IPv6 TCP/UDP/SCTP.
                                                         See PKI_WQE_S[TAG]. */
	uint64_t tag_dst_le                   : 1;  /**< When set, PKI hardware includes the inner IPv4/IPv6 destination address (in an
                                                         IP-in-IP case) in tuple tag generation when PKI_LTYPE(PKI_WQE_S[LETY])_MAP[BELTYPE]
                                                         equals either of PKI_BELTYPE_E::IP4,IP6. PKI_WQE_S[LEPTR] points to the inner
                                                         IPv4/IPv6 header in this case. See PKI_WQE_S[TAG]. */
	uint64_t tag_dst_ld                   : 1;  /**< When set, PKI hardware includes the outer IPv4/IPv6 UDP destination port (in an
                                                         IP-in-IP case) in tuple tag generation when PKI_LTYPE(PKI_WQE_S[LDTY])_MAP[BELTYPE]
                                                         equals PKI_BELTYPE_E::UDP. PKI_WQE_S[LDPTR] points to the UDP header in this case.
                                                         See PKI_WQE_S[TAG]. */
	uint64_t tag_dst_lc                   : 1;  /**< When set, PKI hardware includes the outermost IPv4/IPv6 destination address
                                                         in tuple tag generation when PKI_LTYPE(PKI_WQE_S[LCTY])_MAP[BELTYPE]
                                                         equals either of PKI_BELTYPE_E::IP4,IP6. PKI_WQE_S[LCPTR] points to the
                                                         IPv4/IPv6 header in this case. See PKI_WQE_S[TAG]. */
	uint64_t tag_dst_lb                   : 1;  /**< Reserved. Must be zero. */
	uint64_t len_lg                       : 1;  /**< Reserved. Must be zero. */
	uint64_t len_lf                       : 1;  /**< When set, PKI hardware performs an (inner or outer) IPv4/IPv6 TCP/UDP/SCTP
                                                         PKI_OPCODE_E::L4_LEN check when PKI_LTYPE(PKI_WQE_S[LFTY])_MAP[BELTYPE]
                                                         equals any of PKI_BELTYPE_E::TCP,UDP,SCTP. PKI_WQE_S[LFPTR] points to the
                                                         TCP/UDP/SCTP header in this case. In an IP-in-IP case, [LEN_LF] refers
                                                         to the inner IPv4/IPv6 TCP/UDP/SCTP, else the outer/only IPv4/IPv6 TCP/UDP/SCTP.
                                                         See PKI_WQE_S[ERRLEV,OPCODE] and PKI_ERRLEV_E::LF. */
	uint64_t len_le                       : 1;  /**< When set, PKI hardware performs an inner IPv4/IPv6 PKI_OPCODE_E::IP_MALD check (in an
                                                         IP-in-IP case) when PKI_LTYPE(PKI_WQE_S[LETY])_MAP[BELTYPE] equals either of
                                                         PKI_BELTYPE_E::IP4,IP6. PKI_WQE_S[LEPTR] points to the inner IPv4/IPv6 header in this
                                                         case. See PKI_WQE_S[ERRLEV,OPCODE] and PKI_ERRLEV_E::LE. */
	uint64_t len_ld                       : 1;  /**< When set, PKI hardware performs an outer IPv4/IPv6 UDP PKI_OPCODE_E::L4_LEN
                                                         check (in an IP-in-IP case) when PKI_LTYPE(PKI_WQE_S[LDTY])_MAP[BELTYPE]
                                                         equals PKI_BELTYPE_E::UDP. PKI_WQE_S[LDPTR] points to the UDP header in this case.
                                                         See PKI_WQE_S[ERRLEV,OPCODE] and PKI_ERRLEV_E::LD. */
	uint64_t len_lc                       : 1;  /**< When set, PKI hardware performs an outermost IPv4/IPv6 PKI_OPCODE_E::IP_MALD
                                                         check when PKI_LTYPE(PKI_WQE_S[LCTY])_MAP[BELTYPE] equals either of
                                                         PKI_BELTYPE_E::IP4,IP6. PKI_WQE_S[LCPTR] points to the IPv4/IPv6 header in this
                                                         case. See PKI_WQE_S[ERRLEV,OPCODE] and PKI_ERRLEV_E::LC. */
	uint64_t len_lb                       : 1;  /**< Reserved. Must be zero. */
	uint64_t csum_lg                      : 1;  /**< Reserved. Must be zero. */
	uint64_t csum_lf                      : 1;  /**< When set, PKI hardware performs an (inner or outer) IPv4/IPv6 TCP/UDP/SCTP
                                                         PKI_OPCODE_E::L4_CHK check when PKI_LTYPE(PKI_WQE_S[LFTY])_MAP[BELTYPE]
                                                         equals any of PKI_BELTYPE_E::TCP,UDP,SCTP. PKI_WQE_S[LFPTR] points to the
                                                         TCP/UDP/SCTP header in this case. In an IP-in-IP case, [CSUM_LF] refers
                                                         to the inner IPv4/IPv6 TCP/UDP/SCTP, else the outer/only IPv4/IPv6 TCP/UDP/SCTP.
                                                         See PKI_WQE_S[ERRLEV,OPCODE] and PKI_ERRLEV_E::LF. */
	uint64_t csum_le                      : 1;  /**< When set, PKI hardware performs an inner IPv4/IPv6 PKI_OPCODE_E::IP_CHK check (in an
                                                         IP-in-IP case) when PKI_LTYPE(PKI_WQE_S[LETY])_MAP[BELTYPE] equals either of
                                                         PKI_BELTYPE_E::IP4,IP6. PKI_WQE_S[LEPTR] points to the inner IPv4/IPv6 header in this
                                                         case. See PKI_WQE_S[ERRLEV,OPCODE] and PKI_ERRLEV_E::LE. */
	uint64_t csum_ld                      : 1;  /**< When set, PKI hardware performs an outer IPv4/IPv6 UDP PKI_OPCODE_E::L4_CHK
                                                         check (in an IP-in-IP case) when PKI_LTYPE(PKI_WQE_S[LDTY])_MAP[BELTYPE]
                                                         equals PKI_BELTYPE_E::UDP. PKI_WQE_S[LDPTR] points to the UDP header in this case.
                                                         See PKI_WQE_S[ERRLEV,OPCODE] and PKI_ERRLEV_E::LD. */
	uint64_t csum_lc                      : 1;  /**< When set, PKI hardware performs an outermost IPv4/IPv6 PKI_OPCODE_E::IP_CHK
                                                         check when PKI_LTYPE(PKI_WQE_S[LCTY])_MAP[BELTYPE] equals either of
                                                         PKI_BELTYPE_E::IP4,IP6. PKI_WQE_S[LCPTR] points to the IPv4/IPv6 header in this
                                                         case. See PKI_WQE_S[ERRLEV,OPCODE] and PKI_ERRLEV_E::LC. */
	uint64_t csum_lb                      : 1;  /**< Reserved. Must be zero. */
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
	struct cvmx_pki_clx_stylex_cfg2_s     cn73xx;
	struct cvmx_pki_clx_stylex_cfg2_s     cn78xx;
	struct cvmx_pki_clx_stylex_cfg2_s     cn78xxp1;
	struct cvmx_pki_clx_stylex_cfg2_s     cnf75xx;
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
	uint64_t clken                        : 1;  /**< Controls the conditional clocking within PKI.
                                                         0 = Allow hardware to control the clocks.
                                                         1 = Force the clocks to be always on. */
#else
	uint64_t clken                        : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_pki_clken_s               cn73xx;
	struct cvmx_pki_clken_s               cn78xx;
	struct cvmx_pki_clken_s               cn78xxp1;
	struct cvmx_pki_clken_s               cnf75xx;
};
typedef union cvmx_pki_clken cvmx_pki_clken_t;

/**
 * cvmx_pki_dstat#_stat0
 *
 * This register contains statistics indexed by PKI_QPG_TBLB()[DSTAT_ID].
 *
 */
union cvmx_pki_dstatx_stat0 {
	uint64_t u64;
	struct cvmx_pki_dstatx_stat0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t pkts                         : 32; /**< Number of non-dropped packets processed by PKI.
                                                         The corresponding wide statistic is PKI_STAT()_STAT0. */
#else
	uint64_t pkts                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pki_dstatx_stat0_s        cn73xx;
	struct cvmx_pki_dstatx_stat0_s        cn78xx;
	struct cvmx_pki_dstatx_stat0_s        cnf75xx;
};
typedef union cvmx_pki_dstatx_stat0 cvmx_pki_dstatx_stat0_t;

/**
 * cvmx_pki_dstat#_stat1
 *
 * This register contains statistics indexed by PKI_QPG_TBLB()[DSTAT_ID].
 *
 */
union cvmx_pki_dstatx_stat1 {
	uint64_t u64;
	struct cvmx_pki_dstatx_stat1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t octs                         : 40; /**< Number of non-dropped octets received by PKI (good and bad).
                                                         The corresponding wide statistic is PKI_STAT()_STAT1. */
#else
	uint64_t octs                         : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_pki_dstatx_stat1_s        cn73xx;
	struct cvmx_pki_dstatx_stat1_s        cn78xx;
	struct cvmx_pki_dstatx_stat1_s        cnf75xx;
};
typedef union cvmx_pki_dstatx_stat1 cvmx_pki_dstatx_stat1_t;

/**
 * cvmx_pki_dstat#_stat2
 *
 * This register contains statistics indexed by PKI_QPG_TBLB()[DSTAT_ID].
 *
 */
union cvmx_pki_dstatx_stat2 {
	uint64_t u64;
	struct cvmx_pki_dstatx_stat2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t err_pkts                     : 32; /**< Number of packets with errors, including length < minimum, length > maximum, FCS
                                                         errors, or PKI_WQE_S[ERRLEV] = RE or L2.
                                                         This corresponds to a sum across the wide statistics PKI_STAT()_STAT7, PKI_STAT()_STAT7,
                                                         PKI_STAT()_STAT8, PKI_STAT()_STAT9, PKI_STAT()_STAT10, PKI_STAT()_STAT11, and
                                                         PKI_STAT()_STAT12. */
#else
	uint64_t err_pkts                     : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pki_dstatx_stat2_s        cn73xx;
	struct cvmx_pki_dstatx_stat2_s        cn78xx;
	struct cvmx_pki_dstatx_stat2_s        cnf75xx;
};
typedef union cvmx_pki_dstatx_stat2 cvmx_pki_dstatx_stat2_t;

/**
 * cvmx_pki_dstat#_stat3
 *
 * This register contains statistics indexed by PKI_QPG_TBLB()[DSTAT_ID].
 *
 */
union cvmx_pki_dstatx_stat3 {
	uint64_t u64;
	struct cvmx_pki_dstatx_stat3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t drp_pkts                     : 32; /**< Inbound packets dropped by RED, buffer exhaustion, or PKI_CL()_STYLE()_CFG[DROP].
                                                         The corresponding wide statistic is PKI_STAT()_STAT3. */
#else
	uint64_t drp_pkts                     : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pki_dstatx_stat3_s        cn73xx;
	struct cvmx_pki_dstatx_stat3_s        cn78xx;
	struct cvmx_pki_dstatx_stat3_s        cnf75xx;
};
typedef union cvmx_pki_dstatx_stat3 cvmx_pki_dstatx_stat3_t;

/**
 * cvmx_pki_dstat#_stat4
 *
 * This register contains statistics indexed by PKI_QPG_TBLB()[DSTAT_ID].
 *
 */
union cvmx_pki_dstatx_stat4 {
	uint64_t u64;
	struct cvmx_pki_dstatx_stat4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t drp_octs                     : 40; /**< Inbound octets dropped by RED, buffer exhaustion, or PKI_CL()_STYLE()_CFG[DROP].
                                                         The corresponding wide statistic is PKI_STAT()_STAT4. */
#else
	uint64_t drp_octs                     : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_pki_dstatx_stat4_s        cn73xx;
	struct cvmx_pki_dstatx_stat4_s        cn78xx;
	struct cvmx_pki_dstatx_stat4_s        cnf75xx;
};
typedef union cvmx_pki_dstatx_stat4 cvmx_pki_dstatx_stat4_t;

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
	uint64_t ldfif_flip                   : 2;  /**< LDFIF RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the LDFIF ram
                                                         to test single-bit or double-bit error handling. */
	uint64_t ldfif_cdis                   : 1;  /**< LDFIF RAM ECC correction disable. */
	uint64_t pbe_flip                     : 2;  /**< PBE state RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the PBE
                                                         ram to test single-bit or double-bit error handling. */
	uint64_t pbe_cdis                     : 1;  /**< PBE state RAM ECC correction disable. */
	uint64_t wadr_flip                    : 2;  /**< WADR RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the WADR ram
                                                         to test single-bit or double-bit error handling. */
	uint64_t wadr_cdis                    : 1;  /**< WADR RAM ECC correction disable. */
	uint64_t nxtptag_flip                 : 2;  /**< NXTPTAG RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the NXTPTAG
                                                         ram to test single-bit or double-bit error handling. */
	uint64_t nxtptag_cdis                 : 1;  /**< NXTPTAG RAM ECC correction disable. */
	uint64_t curptag_flip                 : 2;  /**< CURPTAG RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the CURPTAG
                                                         ram to test single-bit or double-bit error handling. */
	uint64_t curptag_cdis                 : 1;  /**< CURPTAG RAM ECC correction disable. */
	uint64_t nxtblk_flip                  : 2;  /**< NXTBLK RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the NXTBLK
                                                         ram to test single-bit or double-bit error handling. */
	uint64_t nxtblk_cdis                  : 1;  /**< NXTBLK RAM ECC correction disable. */
	uint64_t kmem_flip                    : 2;  /**< KMEM RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the KMEM ram
                                                         to test single-bit or double-bit error handling. */
	uint64_t kmem_cdis                    : 1;  /**< KMEM RAM ECC correction disable. */
	uint64_t asm_flip                     : 2;  /**< ASM RAM flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the ASM ram to
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
	uint64_t pbe_cdis                     : 1;
	uint64_t pbe_flip                     : 2;
	uint64_t ldfif_cdis                   : 1;
	uint64_t ldfif_flip                   : 2;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_pki_ecc_ctl0_s            cn73xx;
	struct cvmx_pki_ecc_ctl0_s            cn78xx;
	struct cvmx_pki_ecc_ctl0_s            cn78xxp1;
	struct cvmx_pki_ecc_ctl0_s            cnf75xx;
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
	uint64_t reserved_51_63               : 13;
	uint64_t sws_flip                     : 2;  /**< SWS flip syndrome bits on write. */
	uint64_t sws_cdis                     : 1;  /**< SWS ECC correction disable. */
	uint64_t wqeout_flip                  : 2;  /**< WQEOUT flip syndrome bits on write. */
	uint64_t wqeout_cdis                  : 1;  /**< WQEOUT ECC correction disable. */
	uint64_t doa_flip                     : 2;  /**< DOA flip syndrome bits on write. */
	uint64_t doa_cdis                     : 1;  /**< DOA ECC correction disable. */
	uint64_t bpid_flip                    : 2;  /**< BPID flip syndrome bits on write. */
	uint64_t bpid_cdis                    : 1;  /**< BPID ECC correction disable. */
	uint64_t reserved_30_38               : 9;
	uint64_t plc_flip                     : 2;  /**< PLC flip syndrome bits on write. */
	uint64_t plc_cdis                     : 1;  /**< PLC ECC correction disable. */
	uint64_t pktwq_flip                   : 2;  /**< PKTWQ flip syndrome bits on write. */
	uint64_t pktwq_cdis                   : 1;  /**< PKTWQ ECC correction disable. */
	uint64_t reserved_21_23               : 3;
	uint64_t stylewq2_flip                : 2;  /**< STYLEWQ2 flip syndrome bits on write. */
	uint64_t stylewq2_cdis                : 1;  /**< STYLEWQ2 ECC correction disable. */
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
	uint64_t stylewq2_cdis                : 1;
	uint64_t stylewq2_flip                : 2;
	uint64_t reserved_21_23               : 3;
	uint64_t pktwq_cdis                   : 1;
	uint64_t pktwq_flip                   : 2;
	uint64_t plc_cdis                     : 1;
	uint64_t plc_flip                     : 2;
	uint64_t reserved_30_38               : 9;
	uint64_t bpid_cdis                    : 1;
	uint64_t bpid_flip                    : 2;
	uint64_t doa_cdis                     : 1;
	uint64_t doa_flip                     : 2;
	uint64_t wqeout_cdis                  : 1;
	uint64_t wqeout_flip                  : 2;
	uint64_t sws_cdis                     : 1;
	uint64_t sws_flip                     : 2;
	uint64_t reserved_51_63               : 13;
#endif
	} s;
	struct cvmx_pki_ecc_ctl1_s            cn73xx;
	struct cvmx_pki_ecc_ctl1_s            cn78xx;
	struct cvmx_pki_ecc_ctl1_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_51_63               : 13;
	uint64_t sws_flip                     : 2;  /**< SWS flip syndrome bits on write. */
	uint64_t sws_cdis                     : 1;  /**< SWS ECC correction disable. */
	uint64_t wqeout_flip                  : 2;  /**< WQEOUT flip syndrome bits on write. */
	uint64_t wqeout_cdis                  : 1;  /**< WQEOUT ECC correction disable. */
	uint64_t doa_flip                     : 2;  /**< DOA flip syndrome bits on write. */
	uint64_t doa_cdis                     : 1;  /**< DOA ECC correction disable. */
	uint64_t bpid_flip                    : 2;  /**< BPID flip syndrome bits on write. */
	uint64_t bpid_cdis                    : 1;  /**< BPID ECC correction disable. */
	uint64_t reserved_30_38               : 9;
	uint64_t plc_flip                     : 2;  /**< PLC flip syndrome bits on write. */
	uint64_t plc_cdis                     : 1;  /**< PLC ECC correction disable. */
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
	uint64_t plc_cdis                     : 1;
	uint64_t plc_flip                     : 2;
	uint64_t reserved_30_38               : 9;
	uint64_t bpid_cdis                    : 1;
	uint64_t bpid_flip                    : 2;
	uint64_t doa_cdis                     : 1;
	uint64_t doa_flip                     : 2;
	uint64_t wqeout_cdis                  : 1;
	uint64_t wqeout_flip                  : 2;
	uint64_t sws_cdis                     : 1;
	uint64_t sws_flip                     : 2;
	uint64_t reserved_51_63               : 13;
#endif
	} cn78xxp1;
	struct cvmx_pki_ecc_ctl1_s            cnf75xx;
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
	struct cvmx_pki_ecc_ctl2_s            cn73xx;
	struct cvmx_pki_ecc_ctl2_s            cn78xx;
	struct cvmx_pki_ecc_ctl2_s            cn78xxp1;
	struct cvmx_pki_ecc_ctl2_s            cnf75xx;
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
	uint64_t ldfif_dbe                    : 1;  /**< LDFIF ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC0_LDFIF_DBE. */
	uint64_t ldfif_sbe                    : 1;  /**< LDFIF ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC0_LDFIF_SBE. */
	uint64_t pbe_dbe                      : 1;  /**< PBE ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC0_PBE_DBE. */
	uint64_t pbe_sbe                      : 1;  /**< PBE ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC0_PBE_SBE. */
	uint64_t wadr_dbe                     : 1;  /**< WADR ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC0_WADR_DBE. */
	uint64_t wadr_sbe                     : 1;  /**< WADR ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC0_WADR_SBE. */
	uint64_t nxtptag_dbe                  : 1;  /**< NXTPTAG ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC0_NXTPTAG_DBE. */
	uint64_t nxtptag_sbe                  : 1;  /**< NXTPTAG ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC0_NXTPTAG_SBE. */
	uint64_t curptag_dbe                  : 1;  /**< CURPTAG ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC0_CURPTAG_DBE. */
	uint64_t curptag_sbe                  : 1;  /**< CURPTAG ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC0_CURPTAG_SBE. */
	uint64_t nxtblk_dbe                   : 1;  /**< NXTBLK ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC0_NXTBLK_DBE. */
	uint64_t nxtblk_sbe                   : 1;  /**< NXTBLK ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC0_NXTBLK_SBE. */
	uint64_t kmem_dbe                     : 1;  /**< KMEM ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC0_KMEM_DBE. If KMEM_DBE is
                                                         thrown, software may scrub the error by reloading PKI_CL()_PKIND()_KMEM(). */
	uint64_t kmem_sbe                     : 1;  /**< KMEM ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC0_KMEM_SBE. If KMEM_SBE is
                                                         thrown, software may scrub the error by reloading PKI_CL()_PKIND()_KMEM(). */
	uint64_t asm_dbe                      : 1;  /**< ASM ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC0_ASM_DBE. */
	uint64_t asm_sbe                      : 1;  /**< ASM ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC0_ASM_SBE. */
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
	uint64_t pbe_sbe                      : 1;
	uint64_t pbe_dbe                      : 1;
	uint64_t ldfif_sbe                    : 1;
	uint64_t ldfif_dbe                    : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_pki_ecc_int0_s            cn73xx;
	struct cvmx_pki_ecc_int0_s            cn78xx;
	struct cvmx_pki_ecc_int0_s            cn78xxp1;
	struct cvmx_pki_ecc_int0_s            cnf75xx;
};
typedef union cvmx_pki_ecc_int0 cvmx_pki_ecc_int0_t;

/**
 * cvmx_pki_ecc_int1
 */
union cvmx_pki_ecc_int1 {
	uint64_t u64;
	struct cvmx_pki_ecc_int1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_34_63               : 30;
	uint64_t sws_dbe                      : 1;  /**< PLC ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC1_SWS_DBE. */
	uint64_t sws_sbe                      : 1;  /**< PLC ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC1_SWS_SBE. */
	uint64_t wqeout_dbe                   : 1;  /**< PLC ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC1_WQEOUT_DBE. */
	uint64_t wqeout_sbe                   : 1;  /**< PLC ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC1_WQEOUT_SBE. */
	uint64_t doa_dbe                      : 1;  /**< PLC ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC1_DOA_DBE. */
	uint64_t doa_sbe                      : 1;  /**< PLC ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC1_DOA_SBE. */
	uint64_t bpid_dbe                     : 1;  /**< PLC ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC1_BPID_DBE. */
	uint64_t bpid_sbe                     : 1;  /**< PLC ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC1_BPID_SBE. */
	uint64_t reserved_20_25               : 6;
	uint64_t plc_dbe                      : 1;  /**< PLC ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC1_PLC_DBE. */
	uint64_t plc_sbe                      : 1;  /**< PLC ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC1_PLC_SBE. */
	uint64_t pktwq_dbe                    : 1;  /**< PKTWQ ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC1_PKTWQ_DBE. */
	uint64_t pktwq_sbe                    : 1;  /**< PKTWQ ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC1_PKTWQ_SBE. */
	uint64_t reserved_12_15               : 4;
	uint64_t tag_dbe                      : 1;  /**< TAG ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC1_TAG_DBE. */
	uint64_t tag_sbe                      : 1;  /**< TAG ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC1_TAG_SBE. */
	uint64_t aura_dbe                     : 1;  /**< AURA ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC1_AURA_DBE. */
	uint64_t aura_sbe                     : 1;  /**< AURA ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC1_AURA_SBE. */
	uint64_t chan_dbe                     : 1;  /**< CHAN ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC1_CHAN_DBE. */
	uint64_t chan_sbe                     : 1;  /**< CHAN ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC1_CHAN_SBE. */
	uint64_t pbtag_dbe                    : 1;  /**< PBTAG ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC1_PBTAG_DBE. */
	uint64_t pbtag_sbe                    : 1;  /**< PBTAG ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC1_PBTAG_SBE. */
	uint64_t stylewq_dbe                  : 1;  /**< STYLEWQ ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC1_STYLEWQ_DBE. */
	uint64_t stylewq_sbe                  : 1;  /**< STYLEWQ ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC1_STYLEWQ_SBE. */
	uint64_t qpg_dbe                      : 1;  /**< QPG ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC1_QPG_DBE. */
	uint64_t qpg_sbe                      : 1;  /**< QPG ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC1_QPG_SBE. */
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
	uint64_t plc_sbe                      : 1;
	uint64_t plc_dbe                      : 1;
	uint64_t reserved_20_25               : 6;
	uint64_t bpid_sbe                     : 1;
	uint64_t bpid_dbe                     : 1;
	uint64_t doa_sbe                      : 1;
	uint64_t doa_dbe                      : 1;
	uint64_t wqeout_sbe                   : 1;
	uint64_t wqeout_dbe                   : 1;
	uint64_t sws_sbe                      : 1;
	uint64_t sws_dbe                      : 1;
	uint64_t reserved_34_63               : 30;
#endif
	} s;
	struct cvmx_pki_ecc_int1_s            cn73xx;
	struct cvmx_pki_ecc_int1_s            cn78xx;
	struct cvmx_pki_ecc_int1_s            cn78xxp1;
	struct cvmx_pki_ecc_int1_s            cnf75xx;
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
	uint64_t imem_dbe                     : 1;  /**< IMEM ECC double-bit error. Throws PKI_INTSN_E::PKI_ECC2_IMEM_DBE. If IMEM_DBE is
                                                         thrown, software may scrub the error by reloading PKI_IMEM(). */
	uint64_t imem_sbe                     : 1;  /**< IMEM ECC single-bit error. Throws PKI_INTSN_E::PKI_ECC2_IMEM_SBE. If IMEM_SBE is
                                                         thrown, software may scrub the error by reloading PKI_IMEM(). */
#else
	uint64_t imem_sbe                     : 1;
	uint64_t imem_dbe                     : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_pki_ecc_int2_s            cn73xx;
	struct cvmx_pki_ecc_int2_s            cn78xx;
	struct cvmx_pki_ecc_int2_s            cn78xxp1;
	struct cvmx_pki_ecc_int2_s            cnf75xx;
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
	uint64_t maxlen                       : 16; /**< Byte count for max-sized frame check. See PKI_CL()_STYLE()_CFG[MAXERR_EN,MINMAX_SEL],
                                                         PKI_OPCODE_E::L2_OVERSIZE,L2_OVERRUN, PKI_ERRLEV_E::LA, and PKI_WQE_S[ERRLEV,OPCODE].
                                                         When BGX()_SMU()_RX_FRM_CTL[PTP_MODE] is set, BGX adds an extra 8B timestamp onto
                                                         the front of every packet sent to PKI. Thus, [MAXLEN] should be increased by 8
                                                         when BGX()_SMU()_RX_FRM_CTL[PTP_MODE] is set. */
	uint64_t minlen                       : 16; /**< Byte count for min-sized frame check. See PKI_CL()_STYLE()_CFG[MINERR_EN,MINMAX_SEL],
                                                         PKI_OPCODE_E::L2_UNDERSIZE,L2_FRAGMENT, PKI_ERRLEV_E::LA, and PKI_WQE_S[ERRLEV,OPCODE].
                                                         When BGX()_SMU()_RX_FRM_CTL[PTP_MODE] is set, BGX adds an extra 8B timestamp onto
                                                         the front of every packet sent to PKI. Thus, [MINLEN] should be increased by 8
                                                         when BGX()_SMU()_RX_FRM_CTL[PTP_MODE] is set. */
#else
	uint64_t minlen                       : 16;
	uint64_t maxlen                       : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pki_frm_len_chkx_s        cn73xx;
	struct cvmx_pki_frm_len_chkx_s        cn78xx;
	struct cvmx_pki_frm_len_chkx_s        cn78xxp1;
	struct cvmx_pki_frm_len_chkx_s        cnf75xx;
};
typedef union cvmx_pki_frm_len_chkx cvmx_pki_frm_len_chkx_t;

/**
 * cvmx_pki_gbl_pen
 *
 * This register contains global configuration information that applies to all
 * pkinds. The values are opaque to PKI HW.
 *
 * This is intended for communication between the higher-level software SDK, and the
 * SDK code that loads PKI_IMEM() with the parse engine code. This allows the loader to
 * appropriately select the parse engine code with only those features required, so that
 * performance will be optimized.
 */
union cvmx_pki_gbl_pen {
	uint64_t u64;
	struct cvmx_pki_gbl_pen_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t virt_pen                     : 1;  /**< Virtualization parsing enable.
                                                         0 = VXLAN/VXLANGPE/NVGRE/GENEVE is never used in any style. This enables internal power
                                                         and latency reductions.
                                                         1 = VXLAN/VXLANGPE/NVGRE/GENEVE parsing may be used.
                                                         See [L3_PEN] for supported [L3_PEN], [VIRT_PEN], [IL3_PEN], [L4_PEN,CLG_PEN] combinations. */
	uint64_t clg_pen                      : 1;  /**< Custom LG parsing enable.
                                                         0 = Custom LG is never used in any style; i.e. PKI_CL()_PKIND()_CFG[LG_CUSTOM_LAYER]
                                                         is zero for all indices. This enables internal power and latency reductions.
                                                         1 = Custom LG parsing may be used.
                                                         See [L3_PEN] for supported [L3_PEN], [VIRT_PEN], [IL3_PEN], [L4_PEN,CLG_PEN] combinations. */
	uint64_t cl2_pen                      : 1;  /**< Custom L2 parsing enable.
                                                         0 = Custom L2 is never used in any style; i.e. PKI_CL()_PKIND()_L2_CUSTOM[VALID]
                                                         is zero for all indices. This enables internal power and latency reductions.
                                                         1 = Custom L2 parsing may be used. */
	uint64_t l4_pen                       : 1;  /**< L4 parsing enable.
                                                         0 = L4 parsing is never used in any style. This enables internal power and latency
                                                         reductions.
                                                         1 = L4 parsing may be used.
                                                         See [L3_PEN] for supported [L3_PEN,VIRT_PEN,IL3_PEN,L4_PEN,CLG_PEN] combinations. */
	uint64_t il3_pen                      : 1;  /**< L3 inner parsing enable. Must be zero if L3_PEN is zero.
                                                         0 = L3 inner parsing is never used in any style. This enables internal power and latency
                                                         reductions.
                                                         1 = L3 inner (IP-in-IP) parsing may be used.
                                                         See [L3_PEN] for supported [L3_PEN,VIRT_PEN,IL3_PEN,L4_PEN,CLG_PEN] combinations. */
	uint64_t l3_pen                       : 1;  /**< L3 parsing enable.
                                                         0 = L3 parsing is never used in any style. This enables internal power and latency
                                                         reductions.
                                                         1 = L3 parsing may be used.
                                                         The supported [L3_PEN], [VIRT_PEN], [IL3_PEN], [L4_PEN,CLG_PEN] combinations are:
                                                         <pre>
                                                         L3_PEN VIRT_PEN IL3_PEN L4_PEN CLG_PEN
                                                         --------------------------------------
                                                           1       1        1      1       1
                                                           0       0        0      0       1
                                                           0       0        0      0       0
                                                         </pre> */
	uint64_t mpls_pen                     : 1;  /**< MPLS parsing enable.
                                                         0 = MPLS parsing is never used in any style; i.e. PKI_CL()_PKIND()_CFG[MPLS_EN]
                                                         is zero for all indices. This enables internal power and latency reductions.
                                                         1 = MPLS parsing may be used. */
	uint64_t fulc_pen                     : 1;  /**< Fulcrum parsing enable.
                                                         0 = Fulcrum parsing is never used in any style; i.e.
                                                         PKI_CL()_PKIND()_CFG[FULC_EN] is zero for all indices. This enables internal
                                                         power and latency reductions.
                                                         1 = Fulcrum parsing may be used. */
	uint64_t dsa_pen                      : 1;  /**< DSA parsing enable. Must be zero if HG_PEN is set.
                                                         0 = DSA parsing is never used in any style; i.e. PKI_CL()_PKIND()_CFG[DSA_EN] is
                                                         zero for all indices. This enables internal power and latency reductions.
                                                         1 = DSA parsing may be used. */
	uint64_t hg_pen                       : 1;  /**< HiGig parsing enable. Must be zero if DSA_PEN is set.
                                                         0 = HiGig parsing is never used in any style; i.e. PKI_CL()_PKIND()_CFG[HG2_EN,
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
	struct cvmx_pki_gbl_pen_s             cn73xx;
	struct cvmx_pki_gbl_pen_s             cn78xx;
	struct cvmx_pki_gbl_pen_s             cn78xxp1;
	struct cvmx_pki_gbl_pen_s             cnf75xx;
};
typedef union cvmx_pki_gbl_pen cvmx_pki_gbl_pen_t;

/**
 * cvmx_pki_gen_int
 */
union cvmx_pki_gen_int {
	uint64_t u64;
	struct cvmx_pki_gen_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t bufs_oflow                   : 1;  /**< Set when PKI receives a packet that exceeds 256 buffers.
                                                         Throws PKI_INTSN_E::PKI_GEN_BUFS_OFLOW. */
	uint64_t pkt_size_oflow               : 1;  /**< Set when PKI receives a packet that exceeds 64 KB.
                                                         Throws PKI_INTSN_E::PKI_GEN_PKT_SIZE_OFLOW. */
	uint64_t x2p_req_ofl                  : 1;  /**< Set when a device attempts to have more than the allocated requests outstanding to PKI.
                                                         Throws PKI_INTSN_E::PKI_GEN_X2P_REQ_OFL. */
	uint64_t drp_noavail                  : 1;  /**< Set when packet dropped due to no FPA pointers available for the aura the packet
                                                         requested. Throws PKI_INTSN_E::PKI_GEN_DRP_NOAVAIL. */
	uint64_t dat                          : 1;  /**< Set when data arrives before a SOP for the same reassembly ID for a packet. The first
                                                         detected
                                                         error associated with bits [DAT,SOP,EOP] of this register is only set here. A new bit can
                                                         be set when the previous reported bit is cleared. Throws PKI_INTSN_E::PKI_GEN_DAT. */
	uint64_t eop                          : 1;  /**< Set when an EOP is followed by an EOP for the same reassembly ID for a packet. The first
                                                         detected error associated with bits [DAT,EOP,SOP] of this register is only set here. A new
                                                         bit can be set when the previous reported bit is cleared. Also see PKI_PKT_ERR. Throws
                                                         PKI_INTSN_E::PKI_GEN_EOP. */
	uint64_t sop                          : 1;  /**< Set when a SOP is followed by an SOP for the same reassembly ID for a packet. The first
                                                         detected error associated with bits [DAT,EOP,SOP] of this register is only set here. A new
                                                         bit can be set when the previous reported bit is cleared. Also see PKI_PKT_ERR. Throws
                                                         PKI_INTSN_E::PKI_GEN_SOP. */
	uint64_t bckprs                       : 1;  /**< PKI asserted backpressure. Set when PKI was unable to accept the next valid data from
                                                         BGX, SRIO, DPI, LBK, etc. over X2P due to all internal resources being used up, and PKI will
                                                         backpressure X2P. Throws PKI_INTSN_E::PKI_GEN_BCKPRS. */
	uint64_t crcerr                       : 1;  /**< PKI calculated bad CRC in the L2 frame. Throws PKI_INTSN_E::PKI_GEN_CRCERR. */
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
	uint64_t pkt_size_oflow               : 1;
	uint64_t bufs_oflow                   : 1;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_pki_gen_int_s             cn73xx;
	struct cvmx_pki_gen_int_s             cn78xx;
	struct cvmx_pki_gen_int_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t x2p_req_ofl                  : 1;  /**< Set when a device attempts to have more than the allocated requests outstanding to PKI.
                                                         Throws PKI_INTSN_E::PKI_GEN_X2P_REQ_OFL. */
	uint64_t drp_noavail                  : 1;  /**< Set when packet dropped due to no FPA pointers available for the aura the packet
                                                         requested. Throws PKI_INTSN_E::PKI_GEN_DRP_NOAVAIL. */
	uint64_t dat                          : 1;  /**< Set when data arrives before a SOP for the same reassembly ID for a packet. The first
                                                         detected
                                                         error associated with bits [DAT,SOP,EOP] of this register is only set here. A new bit can
                                                         be set when the previous reported bit is cleared. Throws PKI_INTSN_E::PKI_GEN_DAT. */
	uint64_t eop                          : 1;  /**< Set when an EOP is followed by an EOP for the same reassembly ID for a packet. The first
                                                         detected error associated with bits [DAT,EOP,SOP] of this register is only set here. A new
                                                         bit can be set when the previous reported bit is cleared. Also see PKI_PKT_ERR. Throws
                                                         PKI_INTSN_E::PKI_GEN_EOP. */
	uint64_t sop                          : 1;  /**< Set when a SOP is followed by an SOP for the same reassembly ID for a packet. The first
                                                         detected error associated with bits [DAT,EOP,SOP] of this register is only set here. A new
                                                         bit can be set when the previous reported bit is cleared. Also see PKI_PKT_ERR. Throws
                                                         PKI_INTSN_E::PKI_GEN_SOP. */
	uint64_t bckprs                       : 1;  /**< PKI asserted backpressure. Set when PKI was unable to accept the next valid data from
                                                         BGX/ILK/DPI/LBK etc. over X2P due to all internal resources being used up, and PKI will
                                                         backpressure X2P. Throws PKI_INTSN_E::PKI_GEN_BCKPRS. */
	uint64_t crcerr                       : 1;  /**< PKI calculated bad CRC in the L2 frame. Throws PKI_INTSN_E::PKI_GEN_CRCERR. */
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
	} cn78xxp1;
	struct cvmx_pki_gen_int_s             cnf75xx;
};
typedef union cvmx_pki_gen_int cvmx_pki_gen_int_t;

/**
 * cvmx_pki_icg#_cfg
 *
 * This register configures the cluster group.
 *
 */
union cvmx_pki_icgx_cfg {
	uint64_t u64;
	struct cvmx_pki_icgx_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_53_63               : 11;
	uint64_t maxipe_use                   : 5;  /**< Reserved. Must be 0x14. */
	uint64_t reserved_36_47               : 12;
	uint64_t clusters                     : 4;  /**< Reserved. Must be 0x3. */
	uint64_t reserved_27_31               : 5;
	uint64_t release_rqd                  : 1;  /**< Reserved. Must be zero. */
	uint64_t mlo                          : 1;  /**< Reserved. Must be zero. */
	uint64_t pena                         : 1;  /**< Parse enable. Must be set after PKI has been initialized and PKI_IMEM() loaded.
                                                         0 = IPE transitions from start directly to done without executing a sequence, and the KMEM
                                                         bits effectively are copied through to the WQ.
                                                         1 = Normal parse engine operation. */
	uint64_t timer                        : 12; /**< Current hold-off timer. Enables even spreading of cluster utilization over time;
                                                         while [TIMER] is nonzero, a cluster in this group will not start parsing. When a
                                                         cluster in this group starts parsing, [TIMER] is set to [DELAY], and decrements
                                                         every coprocessor-clock. [TIMER] is zeroed if all clusters in this group are
                                                         idle. */
	uint64_t delay                        : 12; /**< Delay between cluster starts, as described under TIMER. If 0x0, a cluster can
                                                         start at any time relative to other clusters. DELAY should typically be
                                                         the approximate parsing delay (800) divided by the number of bits set in
                                                         [CLUSTERS]. The smallest useful nonzero value is 0xA0,
                                                         corresponding to the minimum number of cycles needed to fill one cluster with
                                                         packets. So set [DELAY] = maximum(0xA0,800/number_of_clusters). */
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
	struct cvmx_pki_icgx_cfg_s            cn73xx;
	struct cvmx_pki_icgx_cfg_s            cn78xx;
	struct cvmx_pki_icgx_cfg_s            cn78xxp1;
	struct cvmx_pki_icgx_cfg_s            cnf75xx;
};
typedef union cvmx_pki_icgx_cfg cvmx_pki_icgx_cfg_t;

/**
 * cvmx_pki_imem#
 */
union cvmx_pki_imemx {
	uint64_t u64;
	struct cvmx_pki_imemx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Parse engine instruction word. Software must reload PKI_IMEM() upon the detection
                                                         of PKI_ECC_INT2[IMEM_SBE] or PKI_ECC_INT2[IMEM_DBE] errors. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_pki_imemx_s               cn73xx;
	struct cvmx_pki_imemx_s               cn78xx;
	struct cvmx_pki_imemx_s               cn78xxp1;
	struct cvmx_pki_imemx_s               cnf75xx;
};
typedef union cvmx_pki_imemx cvmx_pki_imemx_t;

/**
 * cvmx_pki_ltype#_map
 *
 * This register is the layer type map, indexed by PKI_LTYPE_E.
 *
 */
union cvmx_pki_ltypex_map {
	uint64_t u64;
	struct cvmx_pki_ltypex_map_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t beltype                      : 3;  /**< For each given PKI_LTYPE_E, the protocol type backend hardware should assume
                                                         this layer type corresponds to. Enumerated by PKI_BELTYPE_E. The recommended
                                                         settings for each register index (PKI_LTYPE_E) are shown in the PKI_LTYPE_E
                                                         table. */
#else
	uint64_t beltype                      : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_pki_ltypex_map_s          cn73xx;
	struct cvmx_pki_ltypex_map_s          cn78xx;
	struct cvmx_pki_ltypex_map_s          cn78xxp1;
	struct cvmx_pki_ltypex_map_s          cnf75xx;
};
typedef union cvmx_pki_ltypex_map cvmx_pki_ltypex_map_t;

/**
 * cvmx_pki_pbe_eco
 */
union cvmx_pki_pbe_eco {
	uint64_t u64;
	struct cvmx_pki_pbe_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t eco_rw                       : 32; /**< Reserved for ECO usage. */
#else
	uint64_t eco_rw                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pki_pbe_eco_s             cn73xx;
	struct cvmx_pki_pbe_eco_s             cn78xx;
	struct cvmx_pki_pbe_eco_s             cnf75xx;
};
typedef union cvmx_pki_pbe_eco cvmx_pki_pbe_eco_t;

/**
 * cvmx_pki_pcam_lookup
 *
 * For diagnostic use only, this register performs a PCAM lookup against the provided
 * cluster and PCAM instance and loads results into PKI_PCAM_RESULT.
 */
union cvmx_pki_pcam_lookup {
	uint64_t u64;
	struct cvmx_pki_pcam_lookup_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t cl                           : 2;  /**< Cluster number within which to lookup. */
	uint64_t reserved_49_51               : 3;
	uint64_t pcam                         : 1;  /**< PCAM number within which to lookup. */
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
	struct cvmx_pki_pcam_lookup_s         cn73xx;
	struct cvmx_pki_pcam_lookup_s         cn78xx;
	struct cvmx_pki_pcam_lookup_s         cn78xxp1;
	struct cvmx_pki_pcam_lookup_s         cnf75xx;
};
typedef union cvmx_pki_pcam_lookup cvmx_pki_pcam_lookup_t;

/**
 * cvmx_pki_pcam_result
 *
 * For diagnostic use only, this register returns PCAM results for the most recent write to
 * PKI_PCAM_LOOKUP. The read will stall until the lookup is completed.
 * PKI_CL()_ECC_CTL[PCAM_EN] must be clear before accessing this register. Read stall
 * is implemented by delaying the PKI_PCAM_LOOKUP write acknowledge until the PCAM is
 * free and the lookup can be issued.
 */
union cvmx_pki_pcam_result {
	uint64_t u64;
	struct cvmx_pki_pcam_result_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t match                        : 1;  /**< Resulting match. */
	uint64_t entry                        : 8;  /**< Resulting matching entry number, unpredictable unless [MATCH] set and [CONFLICT] is clear. */
	uint64_t result                       : 32; /**< Resulting data from matching line's PKI_CL()_PCAM()_ACTION(), or zero if no
                                                         match. Unpredictable unless [CONFLICT] is clear. */
#else
	uint64_t result                       : 32;
	uint64_t entry                        : 8;
	uint64_t match                        : 1;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pki_pcam_result_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t conflict                     : 1;  /**< Conflict. The lookup resulted in multiple entries matching PKI_PCAM_LOOKUP[DATA], [TERM]
                                                         and [STYLE], or zero if no conflict. */
	uint64_t reserved_41_62               : 22;
	uint64_t match                        : 1;  /**< Resulting match. */
	uint64_t entry                        : 8;  /**< Resulting matching entry number, unpredictable unless [MATCH] set and [CONFLICT] is clear. */
	uint64_t result                       : 32; /**< Resulting data from matching line's PKI_CL()_PCAM()_ACTION(), or zero if no
                                                         match. Unpredictable unless [CONFLICT] is clear. */
#else
	uint64_t result                       : 32;
	uint64_t entry                        : 8;
	uint64_t match                        : 1;
	uint64_t reserved_41_62               : 22;
	uint64_t conflict                     : 1;
#endif
	} cn73xx;
	struct cvmx_pki_pcam_result_cn73xx    cn78xx;
	struct cvmx_pki_pcam_result_cn73xx    cn78xxp1;
	struct cvmx_pki_pcam_result_cn73xx    cnf75xx;
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
	struct cvmx_pki_pfe_diag_s            cn73xx;
	struct cvmx_pki_pfe_diag_s            cn78xx;
	struct cvmx_pki_pfe_diag_s            cn78xxp1;
	struct cvmx_pki_pfe_diag_s            cnf75xx;
};
typedef union cvmx_pki_pfe_diag cvmx_pki_pfe_diag_t;

/**
 * cvmx_pki_pfe_eco
 */
union cvmx_pki_pfe_eco {
	uint64_t u64;
	struct cvmx_pki_pfe_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t eco_rw                       : 32; /**< Reserved for ECO usage. */
#else
	uint64_t eco_rw                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pki_pfe_eco_s             cn73xx;
	struct cvmx_pki_pfe_eco_s             cn78xx;
	struct cvmx_pki_pfe_eco_s             cnf75xx;
};
typedef union cvmx_pki_pfe_eco cvmx_pki_pfe_eco_t;

/**
 * cvmx_pki_pix_clken
 */
union cvmx_pki_pix_clken {
	uint64_t u64;
	struct cvmx_pki_pix_clken_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t mech                         : 1;  /**< When set, force the conditional clocks on for mech. */
	uint64_t reserved_4_15                : 12;
	uint64_t cls                          : 4;  /**< When set, force the conditional clocks on for this cluster. */
#else
	uint64_t cls                          : 4;
	uint64_t reserved_4_15                : 12;
	uint64_t mech                         : 1;
	uint64_t reserved_17_63               : 47;
#endif
	} s;
	struct cvmx_pki_pix_clken_s           cn73xx;
	struct cvmx_pki_pix_clken_s           cn78xx;
	struct cvmx_pki_pix_clken_s           cn78xxp1;
	struct cvmx_pki_pix_clken_s           cnf75xx;
};
typedef union cvmx_pki_pix_clken cvmx_pki_pix_clken_t;

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
	struct cvmx_pki_pix_diag_s            cn73xx;
	struct cvmx_pki_pix_diag_s            cn78xx;
	struct cvmx_pki_pix_diag_s            cn78xxp1;
	struct cvmx_pki_pix_diag_s            cnf75xx;
};
typedef union cvmx_pki_pix_diag cvmx_pki_pix_diag_t;

/**
 * cvmx_pki_pix_eco
 */
union cvmx_pki_pix_eco {
	uint64_t u64;
	struct cvmx_pki_pix_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t eco_rw                       : 32; /**< Reserved for ECO usage. */
#else
	uint64_t eco_rw                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pki_pix_eco_s             cn73xx;
	struct cvmx_pki_pix_eco_s             cn78xx;
	struct cvmx_pki_pix_eco_s             cnf75xx;
};
typedef union cvmx_pki_pix_eco cvmx_pki_pix_eco_t;

/**
 * cvmx_pki_pkind#_icgsel
 */
union cvmx_pki_pkindx_icgsel {
	uint64_t u64;
	struct cvmx_pki_pkindx_icgsel_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t icg                          : 2;  /**< Reserved. Must be zero. */
#else
	uint64_t icg                          : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_pki_pkindx_icgsel_s       cn73xx;
	struct cvmx_pki_pkindx_icgsel_s       cn78xx;
	struct cvmx_pki_pkindx_icgsel_s       cn78xxp1;
	struct cvmx_pki_pkindx_icgsel_s       cnf75xx;
};
typedef union cvmx_pki_pkindx_icgsel cvmx_pki_pkindx_icgsel_t;

/**
 * cvmx_pki_pknd#_inb_stat0
 *
 * This register counts inbound statistics, indexed by pkind.
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
	struct cvmx_pki_pkndx_inb_stat0_s     cn73xx;
	struct cvmx_pki_pkndx_inb_stat0_s     cn78xx;
	struct cvmx_pki_pkndx_inb_stat0_s     cn78xxp1;
	struct cvmx_pki_pkndx_inb_stat0_s     cnf75xx;
};
typedef union cvmx_pki_pkndx_inb_stat0 cvmx_pki_pkndx_inb_stat0_t;

/**
 * cvmx_pki_pknd#_inb_stat1
 *
 * This register counts inbound statistics, indexed by pkind.
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
	struct cvmx_pki_pkndx_inb_stat1_s     cn73xx;
	struct cvmx_pki_pkndx_inb_stat1_s     cn78xx;
	struct cvmx_pki_pkndx_inb_stat1_s     cn78xxp1;
	struct cvmx_pki_pkndx_inb_stat1_s     cnf75xx;
};
typedef union cvmx_pki_pkndx_inb_stat1 cvmx_pki_pkndx_inb_stat1_t;

/**
 * cvmx_pki_pknd#_inb_stat2
 *
 * This register counts inbound statistics, indexed by pkind.
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
	struct cvmx_pki_pkndx_inb_stat2_s     cn73xx;
	struct cvmx_pki_pkndx_inb_stat2_s     cn78xx;
	struct cvmx_pki_pkndx_inb_stat2_s     cn78xxp1;
	struct cvmx_pki_pkndx_inb_stat2_s     cnf75xx;
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
	struct cvmx_pki_pkt_err_s             cn73xx;
	struct cvmx_pki_pkt_err_s             cn78xx;
	struct cvmx_pki_pkt_err_s             cn78xxp1;
	struct cvmx_pki_pkt_err_s             cnf75xx;
};
typedef union cvmx_pki_pkt_err cvmx_pki_pkt_err_t;

/**
 * cvmx_pki_ptag_avail
 *
 * For diagnostic use only.
 *
 */
union cvmx_pki_ptag_avail {
	uint64_t u64;
	struct cvmx_pki_ptag_avail_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t avail                        : 8;  /**< Number of ptags available for use. Decreasing the number of ptags will reduce
                                                         the number of packets waiting for parsing, which will lead to sooner
                                                         backpressure/packet drop, but will decrease the small-packet latency of PKI by
                                                         reducing buffer-bloat. AVAIL must be at least as great as the number of
                                                         reassembly IDs used by the system intends to use or the PTAGS can be exhausted
                                                         and PKI will hang. */
#else
	uint64_t avail                        : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_pki_ptag_avail_s          cn73xx;
	struct cvmx_pki_ptag_avail_s          cn78xx;
	struct cvmx_pki_ptag_avail_s          cnf75xx;
};
typedef union cvmx_pki_ptag_avail cvmx_pki_ptag_avail_t;

/**
 * cvmx_pki_qpg_tbl#
 *
 * These registers are used by PKI BE to indirectly calculate the Portadd/Aura/Group
 * from the Diffsrv, HiGig or VLAN information as described in QPG. See also
 * PKI_QPG_TBLB().
 */
union cvmx_pki_qpg_tblx {
	uint64_t u64;
	struct cvmx_pki_qpg_tblx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t padd                         : 12; /**< Port to channel adder for calculating PKI_WQE_S[CHAN]. */
	uint64_t grptag_ok                    : 3;  /**< Number of PKI_WQE_S[TAG] bits to add into PKI_WQE_S[GRP] if no error is detected. */
	uint64_t reserved_42_44               : 3;
	uint64_t grp_ok                       : 10; /**< SSO group to schedule packet to and to load PKI_WQE_S[GRP] with if no error is detected. */
	uint64_t grptag_bad                   : 3;  /**< Number of PKI_WQE_S[TAG] bits to add into PKI_WQE_S[GRP] if an error is detected. */
	uint64_t reserved_26_28               : 3;
	uint64_t grp_bad                      : 10; /**< SSO group to schedule packet to and to load PKI_WQE_S[GRP] with if an error is detected. */
	uint64_t reserved_12_15               : 4;
	uint64_t aura_node                    : 2;  /**< Reserved. */
	uint64_t laura                        : 10; /**< Aura on local node for QOS calculations and loading into PKI_WQE_S[AURA]. */
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
	struct cvmx_pki_qpg_tblx_s            cn73xx;
	struct cvmx_pki_qpg_tblx_s            cn78xx;
	struct cvmx_pki_qpg_tblx_s            cn78xxp1;
	struct cvmx_pki_qpg_tblx_s            cnf75xx;
};
typedef union cvmx_pki_qpg_tblx cvmx_pki_qpg_tblx_t;

/**
 * cvmx_pki_qpg_tblb#
 *
 * This register configures the QPG table. See also PKI_QPG_TBL().
 *
 */
union cvmx_pki_qpg_tblbx {
	uint64_t u64;
	struct cvmx_pki_qpg_tblbx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t dstat_id                     : 10; /**< Deep statistic bucket to use for traffic to this QPG. This determines which
                                                         index of PKI_DSTAT()_STAT0..PKI_DSTAT()_STAT4 will increment. Additionally, if
                                                         PKI_STAT_CTL[MODE] = 0x2, then DSTAT_ID values 0-63 will increment
                                                         PKI_STAT()_STAT0..PKI_STAT()_STAT18 and PKI_STAT()_HIST0..PKI_STAT()_HIST6. */
#else
	uint64_t dstat_id                     : 10;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_pki_qpg_tblbx_s           cn73xx;
	struct cvmx_pki_qpg_tblbx_s           cn78xx;
	struct cvmx_pki_qpg_tblbx_s           cnf75xx;
};
typedef union cvmx_pki_qpg_tblbx cvmx_pki_qpg_tblbx_t;

/**
 * cvmx_pki_reasm_sop#
 */
union cvmx_pki_reasm_sopx {
	uint64_t u64;
	struct cvmx_pki_reasm_sopx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sop                          : 64; /**< When set, a SOP was detected on a reassembly ID. When clear, a SOP has not yet been
                                                         received, or an EOP was received on the reassembly ID. The total number of available
                                                         reassembly IDs is described with the PKI_REASM_E::NUM_REASM enumeration. Not all
                                                         bits are implemented. Only PKI_REASM_SOP(0)[SOP]<63:0>,
                                                         PKI_REASM_SOP(1)[SOP]<31:0> are present in this implementation. */
#else
	uint64_t sop                          : 64;
#endif
	} s;
	struct cvmx_pki_reasm_sopx_s          cn73xx;
	struct cvmx_pki_reasm_sopx_s          cn78xx;
	struct cvmx_pki_reasm_sopx_s          cn78xxp1;
	struct cvmx_pki_reasm_sopx_s          cnf75xx;
};
typedef union cvmx_pki_reasm_sopx cvmx_pki_reasm_sopx_t;

/**
 * cvmx_pki_req_wgt
 *
 * This register controls the round-robin weights between each PKI requestor. For diagnostic
 * tuning only.
 */
union cvmx_pki_req_wgt {
	uint64_t u64;
	struct cvmx_pki_req_wgt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t wgt8                         : 4;  /**< Reserved. */
	uint64_t wgt7                         : 4;  /**< Reserved. */
	uint64_t wgt6                         : 4;  /**< Reserved. */
	uint64_t wgt5                         : 4;  /**< Reserved. */
	uint64_t wgt4                         : 4;  /**< Weight for LBK */
	uint64_t wgt3                         : 4;  /**< Weight for DPI. */
	uint64_t wgt2                         : 4;  /**< Weight for SRIO1. */
	uint64_t wgt1                         : 4;  /**< Weight for SRIO0. */
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
	struct cvmx_pki_req_wgt_s             cn73xx;
	struct cvmx_pki_req_wgt_s             cn78xx;
	struct cvmx_pki_req_wgt_s             cn78xxp1;
	struct cvmx_pki_req_wgt_s             cnf75xx;
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
                                                         occur to the PKI until this is clear. */
	uint64_t reserved_33_62               : 30;
	uint64_t active                       : 1;  /**< When set, PKI is actively processing packet traffic. It is recommenced that software wait
                                                         until ACTIVE is clear before setting RST. */
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
	struct cvmx_pki_sft_rst_s             cn73xx;
	struct cvmx_pki_sft_rst_s             cn78xx;
	struct cvmx_pki_sft_rst_s             cn78xxp1;
	struct cvmx_pki_sft_rst_s             cnf75xx;
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
	struct cvmx_pki_statx_hist0_s         cn73xx;
	struct cvmx_pki_statx_hist0_s         cn78xx;
	struct cvmx_pki_statx_hist0_s         cn78xxp1;
	struct cvmx_pki_statx_hist0_s         cnf75xx;
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
	struct cvmx_pki_statx_hist1_s         cn73xx;
	struct cvmx_pki_statx_hist1_s         cn78xx;
	struct cvmx_pki_statx_hist1_s         cn78xxp1;
	struct cvmx_pki_statx_hist1_s         cnf75xx;
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
	struct cvmx_pki_statx_hist2_s         cn73xx;
	struct cvmx_pki_statx_hist2_s         cn78xx;
	struct cvmx_pki_statx_hist2_s         cn78xxp1;
	struct cvmx_pki_statx_hist2_s         cnf75xx;
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
	struct cvmx_pki_statx_hist3_s         cn73xx;
	struct cvmx_pki_statx_hist3_s         cn78xx;
	struct cvmx_pki_statx_hist3_s         cn78xxp1;
	struct cvmx_pki_statx_hist3_s         cnf75xx;
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
	struct cvmx_pki_statx_hist4_s         cn73xx;
	struct cvmx_pki_statx_hist4_s         cn78xx;
	struct cvmx_pki_statx_hist4_s         cn78xxp1;
	struct cvmx_pki_statx_hist4_s         cnf75xx;
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
	struct cvmx_pki_statx_hist5_s         cn73xx;
	struct cvmx_pki_statx_hist5_s         cn78xx;
	struct cvmx_pki_statx_hist5_s         cn78xxp1;
	struct cvmx_pki_statx_hist5_s         cnf75xx;
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
	struct cvmx_pki_statx_hist6_s         cn73xx;
	struct cvmx_pki_statx_hist6_s         cn78xx;
	struct cvmx_pki_statx_hist6_s         cn78xxp1;
	struct cvmx_pki_statx_hist6_s         cnf75xx;
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
	struct cvmx_pki_statx_stat0_s         cn73xx;
	struct cvmx_pki_statx_stat0_s         cn78xx;
	struct cvmx_pki_statx_stat0_s         cn78xxp1;
	struct cvmx_pki_statx_stat0_s         cnf75xx;
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
	struct cvmx_pki_statx_stat1_s         cn73xx;
	struct cvmx_pki_statx_stat1_s         cn78xx;
	struct cvmx_pki_statx_stat1_s         cn78xxp1;
	struct cvmx_pki_statx_stat1_s         cnf75xx;
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
	struct cvmx_pki_statx_stat10_s        cn73xx;
	struct cvmx_pki_statx_stat10_s        cn78xx;
	struct cvmx_pki_statx_stat10_s        cn78xxp1;
	struct cvmx_pki_statx_stat10_s        cnf75xx;
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
	struct cvmx_pki_statx_stat11_s        cn73xx;
	struct cvmx_pki_statx_stat11_s        cn78xx;
	struct cvmx_pki_statx_stat11_s        cn78xxp1;
	struct cvmx_pki_statx_stat11_s        cnf75xx;
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
	uint64_t l2err                        : 48; /**< Number of non-dropped packets with receive errors (PKI_WQE_S[ERRLEV]==RE or L2) not
                                                         covered by
                                                         more specific length or FCS statistic error registers. */
#else
	uint64_t l2err                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat12_s        cn73xx;
	struct cvmx_pki_statx_stat12_s        cn78xx;
	struct cvmx_pki_statx_stat12_s        cn78xxp1;
	struct cvmx_pki_statx_stat12_s        cnf75xx;
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
	uint64_t spec                         : 48; /**< Number of non-dropped packets with special handling (WEQ[SH] set). For profiling
                                                         and diagnostic use only. */
#else
	uint64_t spec                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat13_s        cn73xx;
	struct cvmx_pki_statx_stat13_s        cn78xx;
	struct cvmx_pki_statx_stat13_s        cn78xxp1;
	struct cvmx_pki_statx_stat13_s        cnf75xx;
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
	uint64_t drp_bcast                    : 48; /**< Number of packets with L2 broadcast DMAC that were dropped by RED, buffer exhaustion, or
                                                         PKI_CL()_STYLE()_CFG[DROP]. See PKI_WQE_S[L2B] for the definition of L2 broadcast. */
#else
	uint64_t drp_bcast                    : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat14_s        cn73xx;
	struct cvmx_pki_statx_stat14_s        cn78xx;
	struct cvmx_pki_statx_stat14_s        cn78xxp1;
	struct cvmx_pki_statx_stat14_s        cnf75xx;
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
	uint64_t drp_mcast                    : 48; /**< Number of packets with L2 multicast DMAC that were dropped by RED, buffer exhaustion, or
                                                         PKI_CL()_STYLE()_CFG[DROP]. See PKI_WQE_S[L2M] for the definition of L2 multicast. */
#else
	uint64_t drp_mcast                    : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat15_s        cn73xx;
	struct cvmx_pki_statx_stat15_s        cn78xx;
	struct cvmx_pki_statx_stat15_s        cn78xxp1;
	struct cvmx_pki_statx_stat15_s        cnf75xx;
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
                                                         or buffer exhaustion. See PKI_WQE_S[L3B] for the definition of L2 multicast. */
#else
	uint64_t drp_bcast                    : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat16_s        cn73xx;
	struct cvmx_pki_statx_stat16_s        cn78xx;
	struct cvmx_pki_statx_stat16_s        cn78xxp1;
	struct cvmx_pki_statx_stat16_s        cnf75xx;
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
                                                         to RED or buffer exhaustion. See PKI_WQE_S[L3M] for the definition of L3 multicast. */
#else
	uint64_t drp_mcast                    : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat17_s        cn73xx;
	struct cvmx_pki_statx_stat17_s        cn78xx;
	struct cvmx_pki_statx_stat17_s        cn78xxp1;
	struct cvmx_pki_statx_stat17_s        cnf75xx;
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
	uint64_t drp_spec                     : 48; /**< Number of packets dropped with special handling (PKI_WQE_S[SH] set). For
                                                         profiling and diagnostic use only. */
#else
	uint64_t drp_spec                     : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat18_s        cn73xx;
	struct cvmx_pki_statx_stat18_s        cn78xx;
	struct cvmx_pki_statx_stat18_s        cn78xxp1;
	struct cvmx_pki_statx_stat18_s        cnf75xx;
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
	uint64_t raw                          : 48; /**< Number of non-dropped packets with PKI_WQE_S[RAW] set. */
#else
	uint64_t raw                          : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat2_s         cn73xx;
	struct cvmx_pki_statx_stat2_s         cn78xx;
	struct cvmx_pki_statx_stat2_s         cn78xxp1;
	struct cvmx_pki_statx_stat2_s         cnf75xx;
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
	uint64_t drp_pkts                     : 48; /**< Inbound packets dropped by RED, buffer exhaustion, or PKI_CL()_STYLE()_CFG[DROP]. */
#else
	uint64_t drp_pkts                     : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat3_s         cn73xx;
	struct cvmx_pki_statx_stat3_s         cn78xx;
	struct cvmx_pki_statx_stat3_s         cn78xxp1;
	struct cvmx_pki_statx_stat3_s         cnf75xx;
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
	uint64_t drp_octs                     : 48; /**< Inbound octets dropped by RED, buffer exhaustion, or PKI_CL()_STYLE()_CFG[DROP]. */
#else
	uint64_t drp_octs                     : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat4_s         cn73xx;
	struct cvmx_pki_statx_stat4_s         cn78xx;
	struct cvmx_pki_statx_stat4_s         cn78xxp1;
	struct cvmx_pki_statx_stat4_s         cnf75xx;
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
                                                         PKI_WQE_S[L2B] for the definition of L2 broadcast. */
#else
	uint64_t bcast                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat5_s         cn73xx;
	struct cvmx_pki_statx_stat5_s         cn78xx;
	struct cvmx_pki_statx_stat5_s         cn78xxp1;
	struct cvmx_pki_statx_stat5_s         cnf75xx;
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
                                                         PKI_WQE_S[L2M] for the definition of L2 multicast. */
#else
	uint64_t mcast                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pki_statx_stat6_s         cn73xx;
	struct cvmx_pki_statx_stat6_s         cn78xx;
	struct cvmx_pki_statx_stat6_s         cn78xxp1;
	struct cvmx_pki_statx_stat6_s         cnf75xx;
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
	struct cvmx_pki_statx_stat7_s         cn73xx;
	struct cvmx_pki_statx_stat7_s         cn78xx;
	struct cvmx_pki_statx_stat7_s         cn78xxp1;
	struct cvmx_pki_statx_stat7_s         cnf75xx;
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
	struct cvmx_pki_statx_stat8_s         cn73xx;
	struct cvmx_pki_statx_stat8_s         cn78xx;
	struct cvmx_pki_statx_stat8_s         cn78xxp1;
	struct cvmx_pki_statx_stat8_s         cnf75xx;
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
	struct cvmx_pki_statx_stat9_s         cn73xx;
	struct cvmx_pki_statx_stat9_s         cn78xx;
	struct cvmx_pki_statx_stat9_s         cn78xxp1;
	struct cvmx_pki_statx_stat9_s         cnf75xx;
};
typedef union cvmx_pki_statx_stat9 cvmx_pki_statx_stat9_t;

/**
 * cvmx_pki_stat_ctl
 *
 * This register controls how the PKI statistics counters are handled.
 *
 */
union cvmx_pki_stat_ctl {
	uint64_t u64;
	struct cvmx_pki_stat_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t mode                         : 2;  /**< The PKI_STAT*_X registers can be indexed either by port kind (pkind), or final style.
                                                         (Does not apply to the PKI_STAT_INB* nor PKI_DSTAT_* registers.)
                                                         _ 0x0 = X represents the packet's pkind.
                                                         _ 0x1 = X represents the low 6-bits of packet's final style.
                                                         _ 0x2 = X represents the packet's PKI_QPG_TBLB()[DSTAT_ID].
                                                           PKI_STAT()_STAT0..PKI_STAT()_STAT18 and PKI_STAT()_HIST0..PKI_STAT()_HIST6 will only
                                                           be incremented if the DSTAT_ID is less than 64; i.e. fits in the index of the
                                                           PKI_STAT()_STAT0 etc.
                                                         _ 0x3 = Reserved. */
#else
	uint64_t mode                         : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_pki_stat_ctl_s            cn73xx;
	struct cvmx_pki_stat_ctl_s            cn78xx;
	struct cvmx_pki_stat_ctl_s            cn78xxp1;
	struct cvmx_pki_stat_ctl_s            cnf75xx;
};
typedef union cvmx_pki_stat_ctl cvmx_pki_stat_ctl_t;

/**
 * cvmx_pki_style#_buf
 *
 * This register configures the PKI BE skip amounts and other information.
 * It is indexed by final style, PKI_WQE_S[STYLE]<5:0>.
 */
union cvmx_pki_stylex_buf {
	uint64_t u64;
	struct cvmx_pki_stylex_buf_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t pkt_lend                     : 1;  /**< Packet little-endian. Changes write operations of packet data to L2C to be in little-
                                                         endian. Does not change the WQE header format, which is properly endian neutral. */
	uint64_t wqe_hsz                      : 2;  /**< Work queue header size:
                                                         0x0 = WORD0..4, standard PKI_WQE_S. Note FIRST_SKIP may be set to not include WORD4 in
                                                         memory.
                                                         0x1 = WORD0..5.
                                                         0x2 = Reserved.
                                                         0x3 = Reserved.
                                                         Selects which parse engine words are transferred to the PKI BE. If a word is not
                                                         transferred and the word will reach memory (FIRST_SKIP is greater than that word
                                                         number), then the final WQE memory word will be zero, not the parse engine register
                                                         contents. */
	uint64_t wqe_skip                     : 2;  /**< WQE start offset. The number of 128-byte cache lines to skip between the buffer pointer
                                                         and WORD0 of the work-queue entry.
                                                         If [DIS_WQ_DAT]=1, legal values must satisfy:
                                                           * [MB_SIZE] >= (PKI_STYLE()_BUF[WQE_SKIP] * (128/8)) + 18
                                                         If [DIS_WQ_DAT]=0, legal values must satisfy:
                                                           * ([WQE_SKIP] * (128/8)) + 4 <= [FIRST_SKIP], to ensure the minimum of four
                                                             work-queue entry words will fit within [FIRST_SKIP]. */
	uint64_t first_skip                   : 6;  /**< The number of eight-byte words from the top of the first MBUF that the PKI stores the next
                                                         pointer.
                                                         If [DIS_WQ_DAT]=1, legal values must satisfy:
                                                           * [FIRST_SKIP] <= PKI_STYLE()_BUF[MB_SIZE] - 18.
                                                         If [DIS_WQ_DAT]=0, legal values must satisfy:
                                                           * [FIRST_SKIP] <= PKI_STYLE()_BUF[MB_SIZE] - 18.
                                                           * ([WQE_SKIP] * (128/8)) + X <= [FIRST_SKIP].
                                                           _ X must be at least 0x4 to ensure the minimum of four work-queue entry,
                                                             but 0x5 is recommended minimum. X = 0x4 will drop WQE WORD4, for use in
                                                             backward compatible applications. */
	uint64_t later_skip                   : 6;  /**< The number of eight-byte words from the top of any MBUF that is not the first MBUF that
                                                         PKI writes the next-pointer to. Legal values are 0 to PKI_STYLE()_BUF[MB_SIZE] - 18. */
	uint64_t opc_mode                     : 2;  /**< Select the style of write to the L2C.
                                                         0x0 = all packet data and next-buffer pointers are written through to memory.
                                                         0x1 = all packet data and next-buffer pointers are written into the cache.
                                                         0x2 = the first aligned cache block holding the WQE and/or front packet data are written
                                                         to
                                                         the L2 cache. All remaining cache blocks are not written to the L2 cache.
                                                         0x3 = the first two aligned cache blocks holding the WQE and front packet data are written
                                                         to the L2 cache. All remaining cache blocks are not written to the L2 cache. */
	uint64_t dis_wq_dat                   : 1;  /**< Separate first data buffer from the work queue entry.
                                                         0 = The packet link pointer will be at word [FIRST_SKIP] immediately followed by packet
                                                         data, in the same buffer as the work queue entry.
                                                         1 = The packet link pointer will be at word [FIRST_SKIP] in a new buffer separate from the
                                                         work queue entry. Words following the WQE in the same cache line will be zeroed, other
                                                         lines in the buffer will not be modified and will retain stale data (from the buffer's
                                                         previous use). This setting may decrease the peak PKI performance by up to half on small
                                                         packets. */
	uint64_t mb_size                      : 13; /**< The number of eight-byte words between the start of a buffer and the last word
                                                         that PKI may write into that buffer. The total amount of data stored by PKI into
                                                         the buffer will be MB_SIZE minus FIRST_SKIP or LATER_SKIP.
                                                         This must be even, in the range of 32 to 4096. This must be less than or equal
                                                         to the maximum size of every free page in every FPA pool this style may use.
                                                         If [DIS_WQ_DAT]=1, legal values must satisfy:
                                                           * MB_SIZE >= (PKI_STYLE()_BUF[WQE_SKIP] * (128/8)) + 18
                                                           * MB_SIZE >= PKI_STYLE()_BUF[FIRST_SKIP] + 18.
                                                           * MB_SIZE >= PKI_STYLE()_BUF[LATER_SKIP] + 18.
                                                         If [DIS_WQ_DAT]=0, legal values must satisfy:
                                                           * MB_SIZE >= PKI_STYLE()_BUF[FIRST_SKIP] + 18.
                                                           * MB_SIZE >= PKI_STYLE()_BUF[LATER_SKIP] + 18. */
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
	struct cvmx_pki_stylex_buf_s          cn73xx;
	struct cvmx_pki_stylex_buf_s          cn78xx;
	struct cvmx_pki_stylex_buf_s          cn78xxp1;
	struct cvmx_pki_stylex_buf_s          cnf75xx;
};
typedef union cvmx_pki_stylex_buf cvmx_pki_stylex_buf_t;

/**
 * cvmx_pki_style#_tag_mask
 *
 * This register configures the PKI BE tag algorithm.
 * It is indexed by final style, PKI_WQE_S[STYLE]<5:0>.
 */
union cvmx_pki_stylex_tag_mask {
	uint64_t u64;
	struct cvmx_pki_stylex_tag_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t mask                         : 16; /**< When set, each bit excludes corresponding bit of the tuple computed tag from being
                                                         included in the final tag. PKI_CL()_STYLE()_CFG2 [TAG_MASKEN] must be set. Does
                                                         not affect tags from packets with a PKI_INST_HDR_S when PKI_INST_HDR_S[UTAG] is set. */
#else
	uint64_t mask                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_pki_stylex_tag_mask_s     cn73xx;
	struct cvmx_pki_stylex_tag_mask_s     cn78xx;
	struct cvmx_pki_stylex_tag_mask_s     cn78xxp1;
	struct cvmx_pki_stylex_tag_mask_s     cnf75xx;
};
typedef union cvmx_pki_stylex_tag_mask cvmx_pki_stylex_tag_mask_t;

/**
 * cvmx_pki_style#_tag_sel
 *
 * This register configures the PKI BE tag algorithm.
 * It is indexed by final style, PKI_WQE_S[STYLE]<5:0>.
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
	uint64_t tag_idx0                     : 3;  /**< Index for TAG_INC<0>. This value is multiplied by 4 to index into PKI_TAG_INC()_MASK.
                                                         See PKI_WQE_S[TAG]. */
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
	struct cvmx_pki_stylex_tag_sel_s      cn73xx;
	struct cvmx_pki_stylex_tag_sel_s      cn78xx;
	struct cvmx_pki_stylex_tag_sel_s      cn78xxp1;
	struct cvmx_pki_stylex_tag_sel_s      cnf75xx;
};
typedef union cvmx_pki_stylex_tag_sel cvmx_pki_stylex_tag_sel_t;

/**
 * cvmx_pki_style#_wq2
 *
 * This register configures the PKI BE WQE generation.
 * It is indexed by final style, PKI_WQE_S[STYLE]<5:0>.
 */
union cvmx_pki_stylex_wq2 {
	uint64_t u64;
	struct cvmx_pki_stylex_wq2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Data for WQ2<63:0>. This is ORed over any parser calculated WQ2<63:0> fields,
                                                         and is used to emulate as if the parser set a WQ field such as
                                                         PKI_WQE_S[PF1]. PKI_INST_HDR_S packets may also want to use this mode to set
                                                         PKI_WQE_S[LCTY] to IP when PKI parsing IP is disabled. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_pki_stylex_wq2_s          cn73xx;
	struct cvmx_pki_stylex_wq2_s          cn78xx;
	struct cvmx_pki_stylex_wq2_s          cn78xxp1;
	struct cvmx_pki_stylex_wq2_s          cnf75xx;
};
typedef union cvmx_pki_stylex_wq2 cvmx_pki_stylex_wq2_t;

/**
 * cvmx_pki_style#_wq4
 *
 * This register configures the PKI BE WQE generation.
 * It is indexed by final style, PKI_WQE_S[STYLE]<5:0>.
 */
union cvmx_pki_stylex_wq4 {
	uint64_t u64;
	struct cvmx_pki_stylex_wq4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Data for WQ4<63:0>. This is ORed over any parser calculated WQ4<63:0> fields,
                                                         and is used to emulate as if the parser set a WQ pointer field. PKI_INST_HDR_S
                                                         packets may also want to use this mode to set PKI_WQE_S[LCPTR] to the start of
                                                         IP when PKI parsing IP is disabled. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_pki_stylex_wq4_s          cn73xx;
	struct cvmx_pki_stylex_wq4_s          cn78xx;
	struct cvmx_pki_stylex_wq4_s          cn78xxp1;
	struct cvmx_pki_stylex_wq4_s          cnf75xx;
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
	uint64_t ptr_sel                      : 4;  /**< Which pointer to use for the bitmask in PKI_TAG_INC()_MASK.
                                                         0x0 = Absolute from start of packet.
                                                         0x1-0x7 = Reserved.
                                                         0x8 = Relative to start of PKI_WQE_S[LAPTR]. LAPTR must be valid (see
                                                         PKI_WQE_S[LAPTR]) or mask is ignored.
                                                         0x9 = Relative to start of PKI_WQE_S[LBPTR]. LBPTR must be valid (see
                                                         PKI_WQE_S[LBPTR]) or mask is ignored.
                                                         0xA = Relative to start of PKI_WQE_S[LCPTR]. LCPTR must be valid (see
                                                         PKI_WQE_S[LCPTR]) or mask is ignored.
                                                         0xB = Relative to start of PKI_WQE_S[LDPTR]. LDPTR must be valid (see
                                                         PKI_WQE_S[LDPTR]) or mask is ignored.
                                                         0xC = Relative to start of PKI_WQE_S[LEPTR]. LEPTR must be valid (see
                                                         PKI_WQE_S[LEPTR]) or mask is ignored.
                                                         0xD = Relative to start of PKI_WQE_S[LFPTR]. LFPTR must be valid (see
                                                         PKI_WQE_S[LFPTR]) or mask is ignored.
                                                         0xE = Relative to start of PKI_WQE_S[LGPTR]. LGPTR must be valid (see
                                                         PKI_WQE_S[LGPTR]) or mask is ignored.
                                                         0xF = Relative to start of PKI_WQE_S[VLPTR]. VLPTR must be valid (see
                                                         PKI_WQE_S[VLPTR]) or mask is ignored. */
	uint64_t offset                       : 8;  /**< Offset for PKI_TAG_INC()_MASK. Number of bytes to add to the selected pointer before
                                                         applying the mask. */
#else
	uint64_t offset                       : 8;
	uint64_t ptr_sel                      : 4;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_pki_tag_incx_ctl_s        cn73xx;
	struct cvmx_pki_tag_incx_ctl_s        cn78xx;
	struct cvmx_pki_tag_incx_ctl_s        cn78xxp1;
	struct cvmx_pki_tag_incx_ctl_s        cnf75xx;
};
typedef union cvmx_pki_tag_incx_ctl cvmx_pki_tag_incx_ctl_t;

/**
 * cvmx_pki_tag_inc#_mask
 */
union cvmx_pki_tag_incx_mask {
	uint64_t u64;
	struct cvmx_pki_tag_incx_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t en                           : 64; /**< Include byte in mask-tag algorithm. Each [EN] bit corresponds to enabling a byte
                                                         in the data stream, for 64 consecutive bytes in total. Which array index is used
                                                         is controlled by PKI_TAG_INC()_CTL as described in PKI_WQE_S[TAG]. */
#else
	uint64_t en                           : 64;
#endif
	} s;
	struct cvmx_pki_tag_incx_mask_s       cn73xx;
	struct cvmx_pki_tag_incx_mask_s       cn78xx;
	struct cvmx_pki_tag_incx_mask_s       cn78xxp1;
	struct cvmx_pki_tag_incx_mask_s       cnf75xx;
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
                                                         ensure tagging is symmetric between source/destination flows. Typically different from DST
                                                         for maximum security. */
	uint64_t src6                         : 16; /**< Secret for the source tuple IPv6 tag CRC calculation. Typically different from SRC for
                                                         maximum security. */
	uint64_t dst                          : 16; /**< Secret for the destination tuple tag CRC calculation. Typically identical to SRC to
                                                         ensure tagging is symmetric between source/destination flows. */
	uint64_t src                          : 16; /**< Secret for the source tuple tag CRC calculation. */
#else
	uint64_t src                          : 16;
	uint64_t dst                          : 16;
	uint64_t src6                         : 16;
	uint64_t dst6                         : 16;
#endif
	} s;
	struct cvmx_pki_tag_secret_s          cn73xx;
	struct cvmx_pki_tag_secret_s          cn78xx;
	struct cvmx_pki_tag_secret_s          cn78xxp1;
	struct cvmx_pki_tag_secret_s          cnf75xx;
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
	struct cvmx_pki_x2p_req_ofl_s         cn73xx;
	struct cvmx_pki_x2p_req_ofl_s         cn78xx;
	struct cvmx_pki_x2p_req_ofl_s         cn78xxp1;
	struct cvmx_pki_x2p_req_ofl_s         cnf75xx;
};
typedef union cvmx_pki_x2p_req_ofl cvmx_pki_x2p_req_ofl_t;

#endif
