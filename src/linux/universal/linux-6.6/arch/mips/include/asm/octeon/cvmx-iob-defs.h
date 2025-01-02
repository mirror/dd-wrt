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
 * cvmx-iob-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon iob.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_IOB_DEFS_H__
#define __CVMX_IOB_DEFS_H__

#define CVMX_IOB_BIST_STATUS (CVMX_ADD_IO_SEG(0x00011800F00007F8ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_CHIP_CUR_PWR CVMX_IOB_CHIP_CUR_PWR_FUNC()
static inline uint64_t CVMX_IOB_CHIP_CUR_PWR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_IOB_CHIP_CUR_PWR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000828ull);
}
#else
#define CVMX_IOB_CHIP_CUR_PWR (CVMX_ADD_IO_SEG(0x00011800F0000828ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_CHIP_GLB_PWR_THROTTLE CVMX_IOB_CHIP_GLB_PWR_THROTTLE_FUNC()
static inline uint64_t CVMX_IOB_CHIP_GLB_PWR_THROTTLE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_IOB_CHIP_GLB_PWR_THROTTLE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000808ull);
}
#else
#define CVMX_IOB_CHIP_GLB_PWR_THROTTLE (CVMX_ADD_IO_SEG(0x00011800F0000808ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_CHIP_PWR_OUT CVMX_IOB_CHIP_PWR_OUT_FUNC()
static inline uint64_t CVMX_IOB_CHIP_PWR_OUT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_IOB_CHIP_PWR_OUT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000818ull);
}
#else
#define CVMX_IOB_CHIP_PWR_OUT (CVMX_ADD_IO_SEG(0x00011800F0000818ull))
#endif
#define CVMX_IOB_CTL_STATUS (CVMX_ADD_IO_SEG(0x00011800F0000050ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_DWB_PRI_CNT CVMX_IOB_DWB_PRI_CNT_FUNC()
static inline uint64_t CVMX_IOB_DWB_PRI_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN58XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_IOB_DWB_PRI_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000028ull);
}
#else
#define CVMX_IOB_DWB_PRI_CNT (CVMX_ADD_IO_SEG(0x00011800F0000028ull))
#endif
#define CVMX_IOB_FAU_TIMEOUT (CVMX_ADD_IO_SEG(0x00011800F0000000ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_I2C_PRI_CNT CVMX_IOB_I2C_PRI_CNT_FUNC()
static inline uint64_t CVMX_IOB_I2C_PRI_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN58XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_IOB_I2C_PRI_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000010ull);
}
#else
#define CVMX_IOB_I2C_PRI_CNT (CVMX_ADD_IO_SEG(0x00011800F0000010ull))
#endif
#define CVMX_IOB_INB_CONTROL_MATCH (CVMX_ADD_IO_SEG(0x00011800F0000078ull))
#define CVMX_IOB_INB_CONTROL_MATCH_ENB (CVMX_ADD_IO_SEG(0x00011800F0000088ull))
#define CVMX_IOB_INB_DATA_MATCH (CVMX_ADD_IO_SEG(0x00011800F0000070ull))
#define CVMX_IOB_INB_DATA_MATCH_ENB (CVMX_ADD_IO_SEG(0x00011800F0000080ull))
#define CVMX_IOB_INT_ENB (CVMX_ADD_IO_SEG(0x00011800F0000060ull))
#define CVMX_IOB_INT_SUM (CVMX_ADD_IO_SEG(0x00011800F0000058ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_N2C_L2C_PRI_CNT CVMX_IOB_N2C_L2C_PRI_CNT_FUNC()
static inline uint64_t CVMX_IOB_N2C_L2C_PRI_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN58XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_IOB_N2C_L2C_PRI_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000020ull);
}
#else
#define CVMX_IOB_N2C_L2C_PRI_CNT (CVMX_ADD_IO_SEG(0x00011800F0000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_N2C_RSP_PRI_CNT CVMX_IOB_N2C_RSP_PRI_CNT_FUNC()
static inline uint64_t CVMX_IOB_N2C_RSP_PRI_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN58XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_IOB_N2C_RSP_PRI_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000008ull);
}
#else
#define CVMX_IOB_N2C_RSP_PRI_CNT (CVMX_ADD_IO_SEG(0x00011800F0000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_OUTB_COM_PRI_CNT CVMX_IOB_OUTB_COM_PRI_CNT_FUNC()
static inline uint64_t CVMX_IOB_OUTB_COM_PRI_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN58XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_IOB_OUTB_COM_PRI_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000040ull);
}
#else
#define CVMX_IOB_OUTB_COM_PRI_CNT (CVMX_ADD_IO_SEG(0x00011800F0000040ull))
#endif
#define CVMX_IOB_OUTB_CONTROL_MATCH (CVMX_ADD_IO_SEG(0x00011800F0000098ull))
#define CVMX_IOB_OUTB_CONTROL_MATCH_ENB (CVMX_ADD_IO_SEG(0x00011800F00000A8ull))
#define CVMX_IOB_OUTB_DATA_MATCH (CVMX_ADD_IO_SEG(0x00011800F0000090ull))
#define CVMX_IOB_OUTB_DATA_MATCH_ENB (CVMX_ADD_IO_SEG(0x00011800F00000A0ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_OUTB_FPA_PRI_CNT CVMX_IOB_OUTB_FPA_PRI_CNT_FUNC()
static inline uint64_t CVMX_IOB_OUTB_FPA_PRI_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN58XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_IOB_OUTB_FPA_PRI_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000048ull);
}
#else
#define CVMX_IOB_OUTB_FPA_PRI_CNT (CVMX_ADD_IO_SEG(0x00011800F0000048ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_OUTB_REQ_PRI_CNT CVMX_IOB_OUTB_REQ_PRI_CNT_FUNC()
static inline uint64_t CVMX_IOB_OUTB_REQ_PRI_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN58XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_IOB_OUTB_REQ_PRI_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000038ull);
}
#else
#define CVMX_IOB_OUTB_REQ_PRI_CNT (CVMX_ADD_IO_SEG(0x00011800F0000038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_P2C_REQ_PRI_CNT CVMX_IOB_P2C_REQ_PRI_CNT_FUNC()
static inline uint64_t CVMX_IOB_P2C_REQ_PRI_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN58XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_IOB_P2C_REQ_PRI_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000018ull);
}
#else
#define CVMX_IOB_P2C_REQ_PRI_CNT (CVMX_ADD_IO_SEG(0x00011800F0000018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_PKT_ERR CVMX_IOB_PKT_ERR_FUNC()
static inline uint64_t CVMX_IOB_PKT_ERR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_IOB_PKT_ERR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000068ull);
}
#else
#define CVMX_IOB_PKT_ERR (CVMX_ADD_IO_SEG(0x00011800F0000068ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_PP_BIST_STATUS CVMX_IOB_PP_BIST_STATUS_FUNC()
static inline uint64_t CVMX_IOB_PP_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_IOB_PP_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000700ull);
}
#else
#define CVMX_IOB_PP_BIST_STATUS (CVMX_ADD_IO_SEG(0x00011800F0000700ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_TO_CMB_CREDITS CVMX_IOB_TO_CMB_CREDITS_FUNC()
static inline uint64_t CVMX_IOB_TO_CMB_CREDITS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_IOB_TO_CMB_CREDITS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F00000B0ull);
}
#else
#define CVMX_IOB_TO_CMB_CREDITS (CVMX_ADD_IO_SEG(0x00011800F00000B0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_TO_NCB_DID_00_CREDITS CVMX_IOB_TO_NCB_DID_00_CREDITS_FUNC()
static inline uint64_t CVMX_IOB_TO_NCB_DID_00_CREDITS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_IOB_TO_NCB_DID_00_CREDITS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000800ull);
}
#else
#define CVMX_IOB_TO_NCB_DID_00_CREDITS (CVMX_ADD_IO_SEG(0x00011800F0000800ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_TO_NCB_DID_111_CREDITS CVMX_IOB_TO_NCB_DID_111_CREDITS_FUNC()
static inline uint64_t CVMX_IOB_TO_NCB_DID_111_CREDITS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_IOB_TO_NCB_DID_111_CREDITS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000B78ull);
}
#else
#define CVMX_IOB_TO_NCB_DID_111_CREDITS (CVMX_ADD_IO_SEG(0x00011800F0000B78ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_TO_NCB_DID_223_CREDITS CVMX_IOB_TO_NCB_DID_223_CREDITS_FUNC()
static inline uint64_t CVMX_IOB_TO_NCB_DID_223_CREDITS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_IOB_TO_NCB_DID_223_CREDITS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000EF8ull);
}
#else
#define CVMX_IOB_TO_NCB_DID_223_CREDITS (CVMX_ADD_IO_SEG(0x00011800F0000EF8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_TO_NCB_DID_24_CREDITS CVMX_IOB_TO_NCB_DID_24_CREDITS_FUNC()
static inline uint64_t CVMX_IOB_TO_NCB_DID_24_CREDITS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_IOB_TO_NCB_DID_24_CREDITS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F00008C0ull);
}
#else
#define CVMX_IOB_TO_NCB_DID_24_CREDITS (CVMX_ADD_IO_SEG(0x00011800F00008C0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_TO_NCB_DID_32_CREDITS CVMX_IOB_TO_NCB_DID_32_CREDITS_FUNC()
static inline uint64_t CVMX_IOB_TO_NCB_DID_32_CREDITS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_IOB_TO_NCB_DID_32_CREDITS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000900ull);
}
#else
#define CVMX_IOB_TO_NCB_DID_32_CREDITS (CVMX_ADD_IO_SEG(0x00011800F0000900ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_TO_NCB_DID_40_CREDITS CVMX_IOB_TO_NCB_DID_40_CREDITS_FUNC()
static inline uint64_t CVMX_IOB_TO_NCB_DID_40_CREDITS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_IOB_TO_NCB_DID_40_CREDITS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000940ull);
}
#else
#define CVMX_IOB_TO_NCB_DID_40_CREDITS (CVMX_ADD_IO_SEG(0x00011800F0000940ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_TO_NCB_DID_55_CREDITS CVMX_IOB_TO_NCB_DID_55_CREDITS_FUNC()
static inline uint64_t CVMX_IOB_TO_NCB_DID_55_CREDITS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_IOB_TO_NCB_DID_55_CREDITS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F00009B8ull);
}
#else
#define CVMX_IOB_TO_NCB_DID_55_CREDITS (CVMX_ADD_IO_SEG(0x00011800F00009B8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_TO_NCB_DID_64_CREDITS CVMX_IOB_TO_NCB_DID_64_CREDITS_FUNC()
static inline uint64_t CVMX_IOB_TO_NCB_DID_64_CREDITS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_IOB_TO_NCB_DID_64_CREDITS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000A00ull);
}
#else
#define CVMX_IOB_TO_NCB_DID_64_CREDITS (CVMX_ADD_IO_SEG(0x00011800F0000A00ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_TO_NCB_DID_79_CREDITS CVMX_IOB_TO_NCB_DID_79_CREDITS_FUNC()
static inline uint64_t CVMX_IOB_TO_NCB_DID_79_CREDITS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_IOB_TO_NCB_DID_79_CREDITS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000A78ull);
}
#else
#define CVMX_IOB_TO_NCB_DID_79_CREDITS (CVMX_ADD_IO_SEG(0x00011800F0000A78ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_TO_NCB_DID_96_CREDITS CVMX_IOB_TO_NCB_DID_96_CREDITS_FUNC()
static inline uint64_t CVMX_IOB_TO_NCB_DID_96_CREDITS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_IOB_TO_NCB_DID_96_CREDITS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000B00ull);
}
#else
#define CVMX_IOB_TO_NCB_DID_96_CREDITS (CVMX_ADD_IO_SEG(0x00011800F0000B00ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOB_TO_NCB_DID_98_CREDITS CVMX_IOB_TO_NCB_DID_98_CREDITS_FUNC()
static inline uint64_t CVMX_IOB_TO_NCB_DID_98_CREDITS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_IOB_TO_NCB_DID_98_CREDITS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000B10ull);
}
#else
#define CVMX_IOB_TO_NCB_DID_98_CREDITS (CVMX_ADD_IO_SEG(0x00011800F0000B10ull))
#endif

/**
 * cvmx_iob_bist_status
 *
 * The result of the BIST run on the IOB memories.
 *
 */
union cvmx_iob_bist_status {
	uint64_t u64;
	struct cvmx_iob_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t ibd                          : 1;  /**< ibd_bist_mem0_status */
	uint64_t icd                          : 1;  /**< icd_ncb_fifo_bist_status */
#else
	uint64_t icd                          : 1;
	uint64_t ibd                          : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_iob_bist_status_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t icnrcb                       : 1;  /**< Reserved */
	uint64_t icr0                         : 1;  /**< Reserved */
	uint64_t icr1                         : 1;  /**< Reserved */
	uint64_t icnr1                        : 1;  /**< Reserved */
	uint64_t icnr0                        : 1;  /**< icnr_reg_mem0_bist_status */
	uint64_t ibdr0                        : 1;  /**< ibdr_bist_req_fifo0_status */
	uint64_t ibdr1                        : 1;  /**< ibdr_bist_req_fifo1_status */
	uint64_t ibr0                         : 1;  /**< ibr_bist_rsp_fifo0_status */
	uint64_t ibr1                         : 1;  /**< ibr_bist_rsp_fifo1_status */
	uint64_t icnrt                        : 1;  /**< Reserved */
	uint64_t ibrq0                        : 1;  /**< ibrq_bist_req_fifo0_status */
	uint64_t ibrq1                        : 1;  /**< ibrq_bist_req_fifo1_status */
	uint64_t icrn0                        : 1;  /**< icr_ncb_bist_mem0_status */
	uint64_t icrn1                        : 1;  /**< icr_ncb_bist_mem1_status */
	uint64_t icrp0                        : 1;  /**< icr_pko_bist_mem0_status */
	uint64_t icrp1                        : 1;  /**< icr_pko_bist_mem1_status */
	uint64_t ibd                          : 1;  /**< ibd_bist_mem0_status */
	uint64_t icd                          : 1;  /**< icd_ncb_fifo_bist_status */
#else
	uint64_t icd                          : 1;
	uint64_t ibd                          : 1;
	uint64_t icrp1                        : 1;
	uint64_t icrp0                        : 1;
	uint64_t icrn1                        : 1;
	uint64_t icrn0                        : 1;
	uint64_t ibrq1                        : 1;
	uint64_t ibrq0                        : 1;
	uint64_t icnrt                        : 1;
	uint64_t ibr1                         : 1;
	uint64_t ibr0                         : 1;
	uint64_t ibdr1                        : 1;
	uint64_t ibdr0                        : 1;
	uint64_t icnr0                        : 1;
	uint64_t icnr1                        : 1;
	uint64_t icr1                         : 1;
	uint64_t icr0                         : 1;
	uint64_t icnrcb                       : 1;
	uint64_t reserved_18_63               : 46;
#endif
	} cn30xx;
	struct cvmx_iob_bist_status_cn30xx    cn31xx;
	struct cvmx_iob_bist_status_cn30xx    cn38xx;
	struct cvmx_iob_bist_status_cn30xx    cn38xxp2;
	struct cvmx_iob_bist_status_cn30xx    cn50xx;
	struct cvmx_iob_bist_status_cn30xx    cn52xx;
	struct cvmx_iob_bist_status_cn30xx    cn52xxp1;
	struct cvmx_iob_bist_status_cn30xx    cn56xx;
	struct cvmx_iob_bist_status_cn30xx    cn56xxp1;
	struct cvmx_iob_bist_status_cn30xx    cn58xx;
	struct cvmx_iob_bist_status_cn30xx    cn58xxp1;
	struct cvmx_iob_bist_status_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t xmdfif                       : 1;  /**< xmdfif_bist_status */
	uint64_t xmcfif                       : 1;  /**< xmcfif_bist_status */
	uint64_t iorfif                       : 1;  /**< iorfif_bist_status */
	uint64_t rsdfif                       : 1;  /**< rsdfif_bist_status */
	uint64_t iocfif                       : 1;  /**< iocfif_bist_status */
	uint64_t icnrcb                       : 1;  /**< icnr_cb_reg_fifo_bist_status */
	uint64_t icr0                         : 1;  /**< icr_bist_req_fifo0_status */
	uint64_t icr1                         : 1;  /**< icr_bist_req_fifo1_status */
	uint64_t icnr1                        : 1;  /**< Reserved */
	uint64_t icnr0                        : 1;  /**< icnr_reg_mem0_bist_status */
	uint64_t ibdr0                        : 1;  /**< ibdr_bist_req_fifo0_status */
	uint64_t ibdr1                        : 1;  /**< ibdr_bist_req_fifo1_status */
	uint64_t ibr0                         : 1;  /**< ibr_bist_rsp_fifo0_status */
	uint64_t ibr1                         : 1;  /**< ibr_bist_rsp_fifo1_status */
	uint64_t icnrt                        : 1;  /**< icnr_tag_cb_reg_fifo_bist_status */
	uint64_t ibrq0                        : 1;  /**< ibrq_bist_req_fifo0_status */
	uint64_t ibrq1                        : 1;  /**< ibrq_bist_req_fifo1_status */
	uint64_t icrn0                        : 1;  /**< icr_ncb_bist_mem0_status */
	uint64_t icrn1                        : 1;  /**< icr_ncb_bist_mem1_status */
	uint64_t icrp0                        : 1;  /**< icr_pko_bist_mem0_status */
	uint64_t icrp1                        : 1;  /**< icr_pko_bist_mem1_status */
	uint64_t ibd                          : 1;  /**< ibd_bist_mem0_status */
	uint64_t icd                          : 1;  /**< icd_ncb_fifo_bist_status */
#else
	uint64_t icd                          : 1;
	uint64_t ibd                          : 1;
	uint64_t icrp1                        : 1;
	uint64_t icrp0                        : 1;
	uint64_t icrn1                        : 1;
	uint64_t icrn0                        : 1;
	uint64_t ibrq1                        : 1;
	uint64_t ibrq0                        : 1;
	uint64_t icnrt                        : 1;
	uint64_t ibr1                         : 1;
	uint64_t ibr0                         : 1;
	uint64_t ibdr1                        : 1;
	uint64_t ibdr0                        : 1;
	uint64_t icnr0                        : 1;
	uint64_t icnr1                        : 1;
	uint64_t icr1                         : 1;
	uint64_t icr0                         : 1;
	uint64_t icnrcb                       : 1;
	uint64_t iocfif                       : 1;
	uint64_t rsdfif                       : 1;
	uint64_t iorfif                       : 1;
	uint64_t xmcfif                       : 1;
	uint64_t xmdfif                       : 1;
	uint64_t reserved_23_63               : 41;
#endif
	} cn61xx;
	struct cvmx_iob_bist_status_cn61xx    cn63xx;
	struct cvmx_iob_bist_status_cn61xx    cn63xxp1;
	struct cvmx_iob_bist_status_cn61xx    cn66xx;
	struct cvmx_iob_bist_status_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t xmdfif                       : 1;  /**< xmdfif_bist_status */
	uint64_t xmcfif                       : 1;  /**< xmcfif_bist_status */
	uint64_t iorfif                       : 1;  /**< iorfif_bist_status */
	uint64_t rsdfif                       : 1;  /**< rsdfif_bist_status */
	uint64_t iocfif                       : 1;  /**< iocfif_bist_status */
	uint64_t icnrcb                       : 1;  /**< icnr_cb_reg_fifo_bist_status */
	uint64_t icr0                         : 1;  /**< icr_bist_req_fifo0_status */
	uint64_t icr1                         : 1;  /**< icr_bist_req_fifo1_status */
	uint64_t icnr0                        : 1;  /**< icnr_reg_mem0_bist_status */
	uint64_t ibr0                         : 1;  /**< ibr_bist_rsp_fifo0_status */
	uint64_t ibr1                         : 1;  /**< ibr_bist_rsp_fifo1_status */
	uint64_t icnrt                        : 1;  /**< icnr_tag_cb_reg_fifo_bist_status */
	uint64_t ibrq0                        : 1;  /**< ibrq_bist_req_fifo0_status */
	uint64_t ibrq1                        : 1;  /**< ibrq_bist_req_fifo1_status */
	uint64_t icrn0                        : 1;  /**< icr_ncb_bist_mem0_status */
	uint64_t icrn1                        : 1;  /**< icr_ncb_bist_mem1_status */
	uint64_t ibd                          : 1;  /**< ibd_bist_mem0_status */
	uint64_t icd                          : 1;  /**< icd_ncb_fifo_bist_status */
#else
	uint64_t icd                          : 1;
	uint64_t ibd                          : 1;
	uint64_t icrn1                        : 1;
	uint64_t icrn0                        : 1;
	uint64_t ibrq1                        : 1;
	uint64_t ibrq0                        : 1;
	uint64_t icnrt                        : 1;
	uint64_t ibr1                         : 1;
	uint64_t ibr0                         : 1;
	uint64_t icnr0                        : 1;
	uint64_t icr1                         : 1;
	uint64_t icr0                         : 1;
	uint64_t icnrcb                       : 1;
	uint64_t iocfif                       : 1;
	uint64_t rsdfif                       : 1;
	uint64_t iorfif                       : 1;
	uint64_t xmcfif                       : 1;
	uint64_t xmdfif                       : 1;
	uint64_t reserved_18_63               : 46;
#endif
	} cn68xx;
	struct cvmx_iob_bist_status_cn68xx    cn68xxp1;
	struct cvmx_iob_bist_status_cn61xx    cn70xx;
	struct cvmx_iob_bist_status_cn61xx    cn70xxp1;
	struct cvmx_iob_bist_status_cn61xx    cnf71xx;
};
typedef union cvmx_iob_bist_status cvmx_iob_bist_status_t;

/**
 * cvmx_iob_chip_cur_pwr
 */
union cvmx_iob_chip_cur_pwr {
	uint64_t u64;
	struct cvmx_iob_chip_cur_pwr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t current_power_setting        : 8;  /**< Global throttling value currently being used.
                                                         Throttling can force units (CPU cores, in particular) idle for a
                                                         portion of time, which will reduce power consumption.  When
                                                         CURRENT_POWER_SETTING is equal to zero, the unit is idle most
                                                         of the time and consumes minimum power. When CURRENT_POWER_SETTING
                                                         is equal to 0xFF, units are never idled to reduce power.
                                                         The hardware generally uses a CURRENT_POWER_SETTING value that
                                                         is as large as possible (in order to maximize performance) subject
                                                         to the following constraints (in priority order):
                                                           - PWR_MIN <= CURRENT_POWER_SETTING <= PWR_MAX
                                                           - Power limits from the PWR_SETTING feedback control system
                                                         In the case of the CPU cores, CURRENT_POWER_SETTING effectively
                                                         limits the CP0 PowThrottle[POWLIM] value:
                                                           effective POWLIM = MINIMUM(CURRENT_POWER_SETTING,PowThrottle[POWLIM]) */
#else
	uint64_t current_power_setting        : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_iob_chip_cur_pwr_s        cn70xx;
	struct cvmx_iob_chip_cur_pwr_s        cn70xxp1;
};
typedef union cvmx_iob_chip_cur_pwr cvmx_iob_chip_cur_pwr_t;

/**
 * cvmx_iob_chip_glb_pwr_throttle
 *
 * Controls the min/max power settings.
 *
 */
union cvmx_iob_chip_glb_pwr_throttle {
	uint64_t u64;
	struct cvmx_iob_chip_glb_pwr_throttle_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_34_63               : 30;
	uint64_t pwr_bw                       : 2;  /**< Configures the reaction time of the closed-loop feedback
                                                         control system for the AVG_CHIP_POWER power approximation.
                                                         Higher numbers decrease bandwidth, reducing response time,
                                                         which could lead to greater tracking error, but reduce
                                                         ringing. */
	uint64_t pwr_max                      : 8;  /**< Maximum allowed CURRENT_POWER_SETTING value. PWR_MAX must
                                                         be >= PWR_MIN. */
	uint64_t pwr_min                      : 8;  /**< Minimum allowed CURRENT_POWER_SETTING value. PWR_MIN must
                                                         be <= PWR_MAX.
                                                         We recommend a PWR_MIN value larger than zero to set a
                                                         minimum performance level in case PWR_SETTING is set to
                                                         an unreachable goal. See the CPU CP0 PowThrottle description.
                                                         PWR_MIN = 50% of PowThrottle[MAXPOW] could be a good
                                                         choice, for example. */
	uint64_t pwr_setting                  : 16; /**< A power limiter for the chip.
                                                         A limiter of the power consumption of the chip. This power
                                                         limiting is implemented by a closed-loop feedback control
                                                         system for the AVG_CHIP_POWER power approximation. The
                                                         direct output of the PWR_SETTING feedback control system
                                                         is the CURRENT_POWER_SETTING value. The power consumed
                                                         by the chip (estimated currently by the AVG_CHIP_POWER
                                                         value) is an indirect output of the PWR_SETTING feedback
                                                         control system.
                                                         PWR_SETTING is not used by the hardware when PWR_MIN equals
                                                         PWR_MAX. PWR_MIN and PWR_MAX threshold requirements always
                                                         supercede PWR_SETTING limits. (For maximum PWR_SETTING
                                                         feedback control freedom, set PWR_MIN=0 and PWR_MAX=0xff.)
                                                         PWR_SETTING equal to 0 forces the chip to consume near
                                                         minimum power. Increasing PWR_SETTING value from 0 to
                                                         0xffff increases the power that the chip is alloed to
                                                         consume linearly (roughly) from minimum to maximum. */
#else
	uint64_t pwr_setting                  : 16;
	uint64_t pwr_min                      : 8;
	uint64_t pwr_max                      : 8;
	uint64_t pwr_bw                       : 2;
	uint64_t reserved_34_63               : 30;
#endif
	} s;
	struct cvmx_iob_chip_glb_pwr_throttle_s cn70xx;
	struct cvmx_iob_chip_glb_pwr_throttle_s cn70xxp1;
};
typedef union cvmx_iob_chip_glb_pwr_throttle cvmx_iob_chip_glb_pwr_throttle_t;

/**
 * cvmx_iob_chip_pwr_out
 *
 * Power numbers from the various partitions on the chip.
 *
 */
union cvmx_iob_chip_pwr_out {
	uint64_t u64;
	struct cvmx_iob_chip_pwr_out_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cpu_pwr                      : 16; /**< An estimate of the current CPU core complex power consumption.
                                                         The CPU core complex includes the caches and DRAM controller(s),
                                                         as well as all CPU cores. Linearly larger values indicate linearly
                                                         higher power consumption. This power consumption estimate is
                                                         energy per core clock. */
	uint64_t chip_power                   : 16; /**< An estimate of the current total power consumption by the chip.
                                                         Linearly larger values indicate linearly higher power consumption.
                                                         CHIP_POWER is the sum of CPU_POWER and COPROC_POWER. */
	uint64_t coproc_power                 : 16; /**< An estimate of the current coprocessor power consumption.
                                                         Linearly larger values indicate linearly higher power consumption.
                                                         This estimate is energy per core clock, and will
                                                         generally decrease as the ratio of core to coprocessor clock
                                                         speed increases. */
	uint64_t avg_chip_power               : 16; /**< An average of CHIP_POWER, calculated using an IIR filter with
                                                         an average weight of 16K core clocks. */
#else
	uint64_t avg_chip_power               : 16;
	uint64_t coproc_power                 : 16;
	uint64_t chip_power                   : 16;
	uint64_t cpu_pwr                      : 16;
#endif
	} s;
	struct cvmx_iob_chip_pwr_out_s        cn70xx;
	struct cvmx_iob_chip_pwr_out_s        cn70xxp1;
};
typedef union cvmx_iob_chip_pwr_out cvmx_iob_chip_pwr_out_t;

/**
 * cvmx_iob_ctl_status
 *
 * IOB Control Status = IOB Control and Status Register
 * Provides control for IOB functions.
 */
union cvmx_iob_ctl_status {
	uint64_t u64;
	struct cvmx_iob_ctl_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t fif_dly                      : 1;  /**< Delay async FIFO counts to be used when clock ratio
                                                         is greater then 3:1. Writes should be followed by an
                                                         immediate read. */
	uint64_t xmc_per                      : 4;  /**< IBC XMC PUSH EARLY */
	uint64_t reserved_3_5                 : 3;
	uint64_t pko_enb                      : 1;  /**< Toggles the endian style of the FAU for the PKO.
                                                         '0' is for big-endian and '1' is for little-endian. */
	uint64_t dwb_enb                      : 1;  /**< Enables the DWB function of the IOB. */
	uint64_t fau_end                      : 1;  /**< Toggles the endian style of the FAU. '0' is for
                                                         big-endian and '1' is for little-endian. */
#else
	uint64_t fau_end                      : 1;
	uint64_t dwb_enb                      : 1;
	uint64_t pko_enb                      : 1;
	uint64_t reserved_3_5                 : 3;
	uint64_t xmc_per                      : 4;
	uint64_t fif_dly                      : 1;
	uint64_t reserved_11_63               : 53;
#endif
	} s;
	struct cvmx_iob_ctl_status_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t outb_mat                     : 1;  /**< Was a match on the outbound bus to the inb pattern
                                                         matchers. */
	uint64_t inb_mat                      : 1;  /**< Was a match on the inbound bus to the inb pattern
                                                         matchers. */
	uint64_t pko_enb                      : 1;  /**< Toggles the endian style of the FAU for the PKO.
                                                         '0' is for big-endian and '1' is for little-endian. */
	uint64_t dwb_enb                      : 1;  /**< Enables the DWB function of the IOB. */
	uint64_t fau_end                      : 1;  /**< Toggles the endian style of the FAU. '0' is for
                                                         big-endian and '1' is for little-endian. */
#else
	uint64_t fau_end                      : 1;
	uint64_t dwb_enb                      : 1;
	uint64_t pko_enb                      : 1;
	uint64_t inb_mat                      : 1;
	uint64_t outb_mat                     : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} cn30xx;
	struct cvmx_iob_ctl_status_cn30xx     cn31xx;
	struct cvmx_iob_ctl_status_cn30xx     cn38xx;
	struct cvmx_iob_ctl_status_cn30xx     cn38xxp2;
	struct cvmx_iob_ctl_status_cn30xx     cn50xx;
	struct cvmx_iob_ctl_status_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t rr_mode                      : 1;  /**< When set to '1' will enable Round-Robin mode of next
                                                         transaction that could arbitrate for the XMB. */
	uint64_t outb_mat                     : 1;  /**< Was a match on the outbound bus to the inb pattern
                                                         matchers. PASS2 FIELD. */
	uint64_t inb_mat                      : 1;  /**< Was a match on the inbound bus to the inb pattern
                                                         matchers. PASS2 FIELD. */
	uint64_t pko_enb                      : 1;  /**< Toggles the endian style of the FAU for the PKO.
                                                         '0' is for big-endian and '1' is for little-endian. */
	uint64_t dwb_enb                      : 1;  /**< Enables the DWB function of the IOB. */
	uint64_t fau_end                      : 1;  /**< Toggles the endian style of the FAU. '0' is for
                                                         big-endian and '1' is for little-endian. */
#else
	uint64_t fau_end                      : 1;
	uint64_t dwb_enb                      : 1;
	uint64_t pko_enb                      : 1;
	uint64_t inb_mat                      : 1;
	uint64_t outb_mat                     : 1;
	uint64_t rr_mode                      : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} cn52xx;
	struct cvmx_iob_ctl_status_cn30xx     cn52xxp1;
	struct cvmx_iob_ctl_status_cn30xx     cn56xx;
	struct cvmx_iob_ctl_status_cn30xx     cn56xxp1;
	struct cvmx_iob_ctl_status_cn30xx     cn58xx;
	struct cvmx_iob_ctl_status_cn30xx     cn58xxp1;
	struct cvmx_iob_ctl_status_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t fif_dly                      : 1;  /**< Delay async FIFO counts to be used when clock ratio
                                                         is greater then 3:1. Writes should be followed by an
                                                         immediate read. */
	uint64_t xmc_per                      : 4;  /**< IBC XMC PUSH EARLY */
	uint64_t rr_mode                      : 1;  /**< When set to '1' will enable Round-Robin mode of next
                                                         transaction that could arbitrate for the XMB. */
	uint64_t outb_mat                     : 1;  /**< Was a match on the outbound bus to the inb pattern
                                                         matchers. PASS2 FIELD. */
	uint64_t inb_mat                      : 1;  /**< Was a match on the inbound bus to the inb pattern
                                                         matchers. PASS2 FIELD. */
	uint64_t pko_enb                      : 1;  /**< Toggles the endian style of the FAU for the PKO.
                                                         '0' is for big-endian and '1' is for little-endian. */
	uint64_t dwb_enb                      : 1;  /**< Enables the DWB function of the IOB. */
	uint64_t fau_end                      : 1;  /**< Toggles the endian style of the FAU. '0' is for
                                                         big-endian and '1' is for little-endian. */
#else
	uint64_t fau_end                      : 1;
	uint64_t dwb_enb                      : 1;
	uint64_t pko_enb                      : 1;
	uint64_t inb_mat                      : 1;
	uint64_t outb_mat                     : 1;
	uint64_t rr_mode                      : 1;
	uint64_t xmc_per                      : 4;
	uint64_t fif_dly                      : 1;
	uint64_t reserved_11_63               : 53;
#endif
	} cn61xx;
	struct cvmx_iob_ctl_status_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t xmc_per                      : 4;  /**< IBC XMC PUSH EARLY */
	uint64_t rr_mode                      : 1;  /**< When set to '1' will enable Round-Robin mode of next
                                                         transaction that could arbitrate for the XMB. */
	uint64_t outb_mat                     : 1;  /**< Was a match on the outbound bus to the inb pattern
                                                         matchers. PASS2 FIELD. */
	uint64_t inb_mat                      : 1;  /**< Was a match on the inbound bus to the inb pattern
                                                         matchers. PASS2 FIELD. */
	uint64_t pko_enb                      : 1;  /**< Toggles the endian style of the FAU for the PKO.
                                                         '0' is for big-endian and '1' is for little-endian. */
	uint64_t dwb_enb                      : 1;  /**< Enables the DWB function of the IOB. */
	uint64_t fau_end                      : 1;  /**< Toggles the endian style of the FAU. '0' is for
                                                         big-endian and '1' is for little-endian. */
#else
	uint64_t fau_end                      : 1;
	uint64_t dwb_enb                      : 1;
	uint64_t pko_enb                      : 1;
	uint64_t inb_mat                      : 1;
	uint64_t outb_mat                     : 1;
	uint64_t rr_mode                      : 1;
	uint64_t xmc_per                      : 4;
	uint64_t reserved_10_63               : 54;
#endif
	} cn63xx;
	struct cvmx_iob_ctl_status_cn63xx     cn63xxp1;
	struct cvmx_iob_ctl_status_cn61xx     cn66xx;
	struct cvmx_iob_ctl_status_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t fif_dly                      : 1;  /**< Delay async FIFO counts to be used when clock ratio
                                                         is greater then 3:1. Writes should be followed by an
                                                         immediate read. */
	uint64_t xmc_per                      : 4;  /**< IBC XMC PUSH EARLY */
	uint64_t rsvr5                        : 1;  /**< Reserved */
	uint64_t outb_mat                     : 1;  /**< Was a match on the outbound bus to the inb pattern
                                                         matchers. */
	uint64_t inb_mat                      : 1;  /**< Was a match on the inbound bus to the inb pattern
                                                         matchers. */
	uint64_t pko_enb                      : 1;  /**< Toggles the endian style of the FAU for the PKO.
                                                         '0' is for big-endian and '1' is for little-endian. */
	uint64_t dwb_enb                      : 1;  /**< Enables the DWB function of the IOB. */
	uint64_t fau_end                      : 1;  /**< Toggles the endian style of the FAU. '0' is for
                                                         big-endian and '1' is for little-endian. */
#else
	uint64_t fau_end                      : 1;
	uint64_t dwb_enb                      : 1;
	uint64_t pko_enb                      : 1;
	uint64_t inb_mat                      : 1;
	uint64_t outb_mat                     : 1;
	uint64_t rsvr5                        : 1;
	uint64_t xmc_per                      : 4;
	uint64_t fif_dly                      : 1;
	uint64_t reserved_11_63               : 53;
#endif
	} cn68xx;
	struct cvmx_iob_ctl_status_cn68xx     cn68xxp1;
	struct cvmx_iob_ctl_status_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t xmc_per                      : 4;  /**< IBC XMC PUSH EARLY */
	uint64_t rr_mode                      : 1;  /**< When set to '1' will enable Round-Robin mode of next
                                                         transaction that could arbitrate for the XMB. */
	uint64_t rsv4                         : 1;  /**< Reserved. */
	uint64_t rsv3                         : 1;  /**< Reserved. */
	uint64_t pko_enb                      : 1;  /**< Toggles the endian style of the FAU for the PKO.
                                                         '0' is for big-endian and '1' is for little-endian. */
	uint64_t dwb_enb                      : 1;  /**< Enables the DWB function of the IOB. */
	uint64_t fau_end                      : 1;  /**< Toggles the endian style of the FAU. '0' is for
                                                         big-endian and '1' is for little-endian. */
#else
	uint64_t fau_end                      : 1;
	uint64_t dwb_enb                      : 1;
	uint64_t pko_enb                      : 1;
	uint64_t rsv3                         : 1;
	uint64_t rsv4                         : 1;
	uint64_t rr_mode                      : 1;
	uint64_t xmc_per                      : 4;
	uint64_t reserved_10_63               : 54;
#endif
	} cn70xx;
	struct cvmx_iob_ctl_status_cn70xx     cn70xxp1;
	struct cvmx_iob_ctl_status_cn61xx     cnf71xx;
};
typedef union cvmx_iob_ctl_status cvmx_iob_ctl_status_t;

/**
 * cvmx_iob_dwb_pri_cnt
 *
 * DWB To CMB Priority Counter = Don't Write Back to CMB Priority Counter Enable and Timer Value
 * Enables and supplies the timeout count for raising the priority of Don't Write Back request to
 * the L2C.
 */
union cvmx_iob_dwb_pri_cnt {
	uint64_t u64;
	struct cvmx_iob_dwb_pri_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t cnt_enb                      : 1;  /**< Enables the raising of CMB access priority
                                                         when CNT_VAL is reached. */
	uint64_t cnt_val                      : 15; /**< Number of core clocks to wait before raising
                                                         the priority for access to CMB. */
#else
	uint64_t cnt_val                      : 15;
	uint64_t cnt_enb                      : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_iob_dwb_pri_cnt_s         cn38xx;
	struct cvmx_iob_dwb_pri_cnt_s         cn38xxp2;
	struct cvmx_iob_dwb_pri_cnt_s         cn52xx;
	struct cvmx_iob_dwb_pri_cnt_s         cn52xxp1;
	struct cvmx_iob_dwb_pri_cnt_s         cn56xx;
	struct cvmx_iob_dwb_pri_cnt_s         cn56xxp1;
	struct cvmx_iob_dwb_pri_cnt_s         cn58xx;
	struct cvmx_iob_dwb_pri_cnt_s         cn58xxp1;
	struct cvmx_iob_dwb_pri_cnt_s         cn61xx;
	struct cvmx_iob_dwb_pri_cnt_s         cn63xx;
	struct cvmx_iob_dwb_pri_cnt_s         cn63xxp1;
	struct cvmx_iob_dwb_pri_cnt_s         cn66xx;
	struct cvmx_iob_dwb_pri_cnt_s         cn70xx;
	struct cvmx_iob_dwb_pri_cnt_s         cn70xxp1;
	struct cvmx_iob_dwb_pri_cnt_s         cnf71xx;
};
typedef union cvmx_iob_dwb_pri_cnt cvmx_iob_dwb_pri_cnt_t;

/**
 * cvmx_iob_fau_timeout
 *
 * FAU Timeout = Fetch and Add Unit Tag-Switch Timeout
 * How many clokc ticks the FAU unit will wait for a tag-switch before timeing out.
 * for Queue 0.
 */
union cvmx_iob_fau_timeout {
	uint64_t u64;
	struct cvmx_iob_fau_timeout_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t tout_enb                     : 1;  /**< The enable for the FAU timeout feature.
                                                         '1' will enable the timeout, '0' will disable. */
	uint64_t tout_val                     : 12; /**< When a tag request arrives from the PP a timer is
                                                         started associate with that PP. The timer which
                                                         increments every 256 eclks is compared to TOUT_VAL.
                                                         When the two are equal the IOB will flag the tag
                                                         request to complete as a time-out tag operation.
                                                         The 256 count timer used to increment the PP
                                                         associated timer is always running so the first
                                                         increment of the PP associated timer may occur any
                                                         where within the first 256 eclks.  Note that '0'
                                                         is an illegal value. */
#else
	uint64_t tout_val                     : 12;
	uint64_t tout_enb                     : 1;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_iob_fau_timeout_s         cn30xx;
	struct cvmx_iob_fau_timeout_s         cn31xx;
	struct cvmx_iob_fau_timeout_s         cn38xx;
	struct cvmx_iob_fau_timeout_s         cn38xxp2;
	struct cvmx_iob_fau_timeout_s         cn50xx;
	struct cvmx_iob_fau_timeout_s         cn52xx;
	struct cvmx_iob_fau_timeout_s         cn52xxp1;
	struct cvmx_iob_fau_timeout_s         cn56xx;
	struct cvmx_iob_fau_timeout_s         cn56xxp1;
	struct cvmx_iob_fau_timeout_s         cn58xx;
	struct cvmx_iob_fau_timeout_s         cn58xxp1;
	struct cvmx_iob_fau_timeout_s         cn61xx;
	struct cvmx_iob_fau_timeout_s         cn63xx;
	struct cvmx_iob_fau_timeout_s         cn63xxp1;
	struct cvmx_iob_fau_timeout_s         cn66xx;
	struct cvmx_iob_fau_timeout_s         cn68xx;
	struct cvmx_iob_fau_timeout_s         cn68xxp1;
	struct cvmx_iob_fau_timeout_s         cn70xx;
	struct cvmx_iob_fau_timeout_s         cn70xxp1;
	struct cvmx_iob_fau_timeout_s         cnf71xx;
};
typedef union cvmx_iob_fau_timeout cvmx_iob_fau_timeout_t;

/**
 * cvmx_iob_i2c_pri_cnt
 *
 * IPD To CMB Store Priority Counter = IPD to CMB Store Priority Counter Enable and Timer Value
 * Enables and supplies the timeout count for raising the priority of IPD Store access to the
 * CMB.
 */
union cvmx_iob_i2c_pri_cnt {
	uint64_t u64;
	struct cvmx_iob_i2c_pri_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t cnt_enb                      : 1;  /**< Enables the raising of CMB access priority
                                                         when CNT_VAL is reached. */
	uint64_t cnt_val                      : 15; /**< Number of core clocks to wait before raising
                                                         the priority for access to CMB. */
#else
	uint64_t cnt_val                      : 15;
	uint64_t cnt_enb                      : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_iob_i2c_pri_cnt_s         cn38xx;
	struct cvmx_iob_i2c_pri_cnt_s         cn38xxp2;
	struct cvmx_iob_i2c_pri_cnt_s         cn52xx;
	struct cvmx_iob_i2c_pri_cnt_s         cn52xxp1;
	struct cvmx_iob_i2c_pri_cnt_s         cn56xx;
	struct cvmx_iob_i2c_pri_cnt_s         cn56xxp1;
	struct cvmx_iob_i2c_pri_cnt_s         cn58xx;
	struct cvmx_iob_i2c_pri_cnt_s         cn58xxp1;
	struct cvmx_iob_i2c_pri_cnt_s         cn61xx;
	struct cvmx_iob_i2c_pri_cnt_s         cn63xx;
	struct cvmx_iob_i2c_pri_cnt_s         cn63xxp1;
	struct cvmx_iob_i2c_pri_cnt_s         cn66xx;
	struct cvmx_iob_i2c_pri_cnt_s         cn70xx;
	struct cvmx_iob_i2c_pri_cnt_s         cn70xxp1;
	struct cvmx_iob_i2c_pri_cnt_s         cnf71xx;
};
typedef union cvmx_iob_i2c_pri_cnt cvmx_iob_i2c_pri_cnt_t;

/**
 * cvmx_iob_inb_control_match
 *
 * Match pattern for the inbound control to set the INB_MATCH_BIT.
 *
 */
union cvmx_iob_inb_control_match {
	uint64_t u64;
	struct cvmx_iob_inb_control_match_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t mask                         : 8;  /**< Pattern to match on the inbound NCB. */
	uint64_t opc                          : 4;  /**< Pattern to match on the inbound NCB. */
	uint64_t dst                          : 9;  /**< Pattern to match on the inbound NCB. */
	uint64_t src                          : 8;  /**< Pattern to match on the inbound NCB. */
#else
	uint64_t src                          : 8;
	uint64_t dst                          : 9;
	uint64_t opc                          : 4;
	uint64_t mask                         : 8;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_iob_inb_control_match_s   cn30xx;
	struct cvmx_iob_inb_control_match_s   cn31xx;
	struct cvmx_iob_inb_control_match_s   cn38xx;
	struct cvmx_iob_inb_control_match_s   cn38xxp2;
	struct cvmx_iob_inb_control_match_s   cn50xx;
	struct cvmx_iob_inb_control_match_s   cn52xx;
	struct cvmx_iob_inb_control_match_s   cn52xxp1;
	struct cvmx_iob_inb_control_match_s   cn56xx;
	struct cvmx_iob_inb_control_match_s   cn56xxp1;
	struct cvmx_iob_inb_control_match_s   cn58xx;
	struct cvmx_iob_inb_control_match_s   cn58xxp1;
	struct cvmx_iob_inb_control_match_s   cn61xx;
	struct cvmx_iob_inb_control_match_s   cn63xx;
	struct cvmx_iob_inb_control_match_s   cn63xxp1;
	struct cvmx_iob_inb_control_match_s   cn66xx;
	struct cvmx_iob_inb_control_match_s   cn68xx;
	struct cvmx_iob_inb_control_match_s   cn68xxp1;
	struct cvmx_iob_inb_control_match_s   cn70xx;
	struct cvmx_iob_inb_control_match_s   cn70xxp1;
	struct cvmx_iob_inb_control_match_s   cnf71xx;
};
typedef union cvmx_iob_inb_control_match cvmx_iob_inb_control_match_t;

/**
 * cvmx_iob_inb_control_match_enb
 *
 * Enables the match of the corresponding bit in the IOB_INB_CONTROL_MATCH reister.
 *
 */
union cvmx_iob_inb_control_match_enb {
	uint64_t u64;
	struct cvmx_iob_inb_control_match_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t mask                         : 8;  /**< Pattern to match on the inbound NCB. */
	uint64_t opc                          : 4;  /**< Pattern to match on the inbound NCB. */
	uint64_t dst                          : 9;  /**< Pattern to match on the inbound NCB. */
	uint64_t src                          : 8;  /**< Pattern to match on the inbound NCB. */
#else
	uint64_t src                          : 8;
	uint64_t dst                          : 9;
	uint64_t opc                          : 4;
	uint64_t mask                         : 8;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_iob_inb_control_match_enb_s cn30xx;
	struct cvmx_iob_inb_control_match_enb_s cn31xx;
	struct cvmx_iob_inb_control_match_enb_s cn38xx;
	struct cvmx_iob_inb_control_match_enb_s cn38xxp2;
	struct cvmx_iob_inb_control_match_enb_s cn50xx;
	struct cvmx_iob_inb_control_match_enb_s cn52xx;
	struct cvmx_iob_inb_control_match_enb_s cn52xxp1;
	struct cvmx_iob_inb_control_match_enb_s cn56xx;
	struct cvmx_iob_inb_control_match_enb_s cn56xxp1;
	struct cvmx_iob_inb_control_match_enb_s cn58xx;
	struct cvmx_iob_inb_control_match_enb_s cn58xxp1;
	struct cvmx_iob_inb_control_match_enb_s cn61xx;
	struct cvmx_iob_inb_control_match_enb_s cn63xx;
	struct cvmx_iob_inb_control_match_enb_s cn63xxp1;
	struct cvmx_iob_inb_control_match_enb_s cn66xx;
	struct cvmx_iob_inb_control_match_enb_s cn68xx;
	struct cvmx_iob_inb_control_match_enb_s cn68xxp1;
	struct cvmx_iob_inb_control_match_enb_s cn70xx;
	struct cvmx_iob_inb_control_match_enb_s cn70xxp1;
	struct cvmx_iob_inb_control_match_enb_s cnf71xx;
};
typedef union cvmx_iob_inb_control_match_enb cvmx_iob_inb_control_match_enb_t;

/**
 * cvmx_iob_inb_data_match
 *
 * Match pattern for the inbound data to set the INB_MATCH_BIT.
 *
 */
union cvmx_iob_inb_data_match {
	uint64_t u64;
	struct cvmx_iob_inb_data_match_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Pattern to match on the inbound NCB. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_iob_inb_data_match_s      cn30xx;
	struct cvmx_iob_inb_data_match_s      cn31xx;
	struct cvmx_iob_inb_data_match_s      cn38xx;
	struct cvmx_iob_inb_data_match_s      cn38xxp2;
	struct cvmx_iob_inb_data_match_s      cn50xx;
	struct cvmx_iob_inb_data_match_s      cn52xx;
	struct cvmx_iob_inb_data_match_s      cn52xxp1;
	struct cvmx_iob_inb_data_match_s      cn56xx;
	struct cvmx_iob_inb_data_match_s      cn56xxp1;
	struct cvmx_iob_inb_data_match_s      cn58xx;
	struct cvmx_iob_inb_data_match_s      cn58xxp1;
	struct cvmx_iob_inb_data_match_s      cn61xx;
	struct cvmx_iob_inb_data_match_s      cn63xx;
	struct cvmx_iob_inb_data_match_s      cn63xxp1;
	struct cvmx_iob_inb_data_match_s      cn66xx;
	struct cvmx_iob_inb_data_match_s      cn68xx;
	struct cvmx_iob_inb_data_match_s      cn68xxp1;
	struct cvmx_iob_inb_data_match_s      cn70xx;
	struct cvmx_iob_inb_data_match_s      cn70xxp1;
	struct cvmx_iob_inb_data_match_s      cnf71xx;
};
typedef union cvmx_iob_inb_data_match cvmx_iob_inb_data_match_t;

/**
 * cvmx_iob_inb_data_match_enb
 *
 * Enables the match of the corresponding bit in the IOB_INB_DATA_MATCH reister.
 *
 */
union cvmx_iob_inb_data_match_enb {
	uint64_t u64;
	struct cvmx_iob_inb_data_match_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Bit to enable match of. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_iob_inb_data_match_enb_s  cn30xx;
	struct cvmx_iob_inb_data_match_enb_s  cn31xx;
	struct cvmx_iob_inb_data_match_enb_s  cn38xx;
	struct cvmx_iob_inb_data_match_enb_s  cn38xxp2;
	struct cvmx_iob_inb_data_match_enb_s  cn50xx;
	struct cvmx_iob_inb_data_match_enb_s  cn52xx;
	struct cvmx_iob_inb_data_match_enb_s  cn52xxp1;
	struct cvmx_iob_inb_data_match_enb_s  cn56xx;
	struct cvmx_iob_inb_data_match_enb_s  cn56xxp1;
	struct cvmx_iob_inb_data_match_enb_s  cn58xx;
	struct cvmx_iob_inb_data_match_enb_s  cn58xxp1;
	struct cvmx_iob_inb_data_match_enb_s  cn61xx;
	struct cvmx_iob_inb_data_match_enb_s  cn63xx;
	struct cvmx_iob_inb_data_match_enb_s  cn63xxp1;
	struct cvmx_iob_inb_data_match_enb_s  cn66xx;
	struct cvmx_iob_inb_data_match_enb_s  cn68xx;
	struct cvmx_iob_inb_data_match_enb_s  cn68xxp1;
	struct cvmx_iob_inb_data_match_enb_s  cn70xx;
	struct cvmx_iob_inb_data_match_enb_s  cn70xxp1;
	struct cvmx_iob_inb_data_match_enb_s  cnf71xx;
};
typedef union cvmx_iob_inb_data_match_enb cvmx_iob_inb_data_match_enb_t;

/**
 * cvmx_iob_int_enb
 *
 * The IOB's interrupt enable register.
 *
 */
union cvmx_iob_int_enb {
	uint64_t u64;
	struct cvmx_iob_int_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t outb_mat                     : 1;  /**< When set (1) and bit 7 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
	uint64_t inb_mat                      : 1;  /**< When set (1) and bit 6 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
	uint64_t p_dat                        : 1;  /**< When set (1) and bit 5 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
	uint64_t np_dat                       : 1;  /**< When set (1) and bit 4 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
	uint64_t p_eop                        : 1;  /**< When set (1) and bit 3 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
	uint64_t p_sop                        : 1;  /**< When set (1) and bit 2 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
	uint64_t np_eop                       : 1;  /**< When set (1) and bit 1 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
	uint64_t np_sop                       : 1;  /**< When set (1) and bit 0 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
#else
	uint64_t np_sop                       : 1;
	uint64_t np_eop                       : 1;
	uint64_t p_sop                        : 1;
	uint64_t p_eop                        : 1;
	uint64_t np_dat                       : 1;
	uint64_t p_dat                        : 1;
	uint64_t inb_mat                      : 1;
	uint64_t outb_mat                     : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_iob_int_enb_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t p_eop                        : 1;  /**< When set (1) and bit 3 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
	uint64_t p_sop                        : 1;  /**< When set (1) and bit 2 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
	uint64_t np_eop                       : 1;  /**< When set (1) and bit 1 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
	uint64_t np_sop                       : 1;  /**< When set (1) and bit 0 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
#else
	uint64_t np_sop                       : 1;
	uint64_t np_eop                       : 1;
	uint64_t p_sop                        : 1;
	uint64_t p_eop                        : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} cn30xx;
	struct cvmx_iob_int_enb_cn30xx        cn31xx;
	struct cvmx_iob_int_enb_cn30xx        cn38xx;
	struct cvmx_iob_int_enb_cn30xx        cn38xxp2;
	struct cvmx_iob_int_enb_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t p_dat                        : 1;  /**< When set (1) and bit 5 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
	uint64_t np_dat                       : 1;  /**< When set (1) and bit 4 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
	uint64_t p_eop                        : 1;  /**< When set (1) and bit 3 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
	uint64_t p_sop                        : 1;  /**< When set (1) and bit 2 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
	uint64_t np_eop                       : 1;  /**< When set (1) and bit 1 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
	uint64_t np_sop                       : 1;  /**< When set (1) and bit 0 of the IOB_INT_SUM
                                                         register is asserted the IOB will assert an
                                                         interrupt. */
#else
	uint64_t np_sop                       : 1;
	uint64_t np_eop                       : 1;
	uint64_t p_sop                        : 1;
	uint64_t p_eop                        : 1;
	uint64_t np_dat                       : 1;
	uint64_t p_dat                        : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} cn50xx;
	struct cvmx_iob_int_enb_cn50xx        cn52xx;
	struct cvmx_iob_int_enb_cn50xx        cn52xxp1;
	struct cvmx_iob_int_enb_cn50xx        cn56xx;
	struct cvmx_iob_int_enb_cn50xx        cn56xxp1;
	struct cvmx_iob_int_enb_cn50xx        cn58xx;
	struct cvmx_iob_int_enb_cn50xx        cn58xxp1;
	struct cvmx_iob_int_enb_cn50xx        cn61xx;
	struct cvmx_iob_int_enb_cn50xx        cn63xx;
	struct cvmx_iob_int_enb_cn50xx        cn63xxp1;
	struct cvmx_iob_int_enb_cn50xx        cn66xx;
	struct cvmx_iob_int_enb_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} cn68xx;
	struct cvmx_iob_int_enb_cn68xx        cn68xxp1;
	struct cvmx_iob_int_enb_s             cn70xx;
	struct cvmx_iob_int_enb_s             cn70xxp1;
	struct cvmx_iob_int_enb_cn50xx        cnf71xx;
};
typedef union cvmx_iob_int_enb cvmx_iob_int_enb_t;

/**
 * cvmx_iob_int_sum
 *
 * Contains the diffrent interrupt summary bits of the IOB.
 *
 */
union cvmx_iob_int_sum {
	uint64_t u64;
	struct cvmx_iob_int_sum_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t outb_mat                     : 1;  /**< Was a match on the outbound bus to the inb pattern
                                                         matchers. */
	uint64_t inb_mat                      : 1;  /**< Was a match on the inbound bus to the inb pattern
                                                         matchers. */
	uint64_t p_dat                        : 1;  /**< Set when a data arrives before a SOP for the same
                                                         port for a passthrough packet.
                                                         The first detected error associated with bits [5:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
	uint64_t np_dat                       : 1;  /**< Set when a data arrives before a SOP for the same
                                                         port for a non-passthrough packet.
                                                         The first detected error associated with bits [5:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
	uint64_t p_eop                        : 1;  /**< Set when a EOP is followed by an EOP for the same
                                                         port for a passthrough packet.
                                                         The first detected error associated with bits [5:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
	uint64_t p_sop                        : 1;  /**< Set when a SOP is followed by an SOP for the same
                                                         port for a passthrough packet.
                                                         The first detected error associated with bits [5:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
	uint64_t np_eop                       : 1;  /**< Set when a EOP is followed by an EOP for the same
                                                         port for a non-passthrough packet.
                                                         The first detected error associated with bits [5:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
	uint64_t np_sop                       : 1;  /**< Set when a SOP is followed by an SOP for the same
                                                         port for a non-passthrough packet.
                                                         The first detected error associated with bits [5:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
#else
	uint64_t np_sop                       : 1;
	uint64_t np_eop                       : 1;
	uint64_t p_sop                        : 1;
	uint64_t p_eop                        : 1;
	uint64_t np_dat                       : 1;
	uint64_t p_dat                        : 1;
	uint64_t inb_mat                      : 1;
	uint64_t outb_mat                     : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_iob_int_sum_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t p_eop                        : 1;  /**< Set when a EOP is followed by an EOP for the same
                                                         port for a passthrough packet.
                                                         The first detected error associated with bits [3:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
	uint64_t p_sop                        : 1;  /**< Set when a SOP is followed by an SOP for the same
                                                         port for a passthrough packet.
                                                         The first detected error associated with bits [3:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
	uint64_t np_eop                       : 1;  /**< Set when a EOP is followed by an EOP for the same
                                                         port for a non-passthrough packet.
                                                         The first detected error associated with bits [3:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
	uint64_t np_sop                       : 1;  /**< Set when a SOP is followed by an SOP for the same
                                                         port for a non-passthrough packet.
                                                         The first detected error associated with bits [3:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
#else
	uint64_t np_sop                       : 1;
	uint64_t np_eop                       : 1;
	uint64_t p_sop                        : 1;
	uint64_t p_eop                        : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} cn30xx;
	struct cvmx_iob_int_sum_cn30xx        cn31xx;
	struct cvmx_iob_int_sum_cn30xx        cn38xx;
	struct cvmx_iob_int_sum_cn30xx        cn38xxp2;
	struct cvmx_iob_int_sum_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t p_dat                        : 1;  /**< Set when a data arrives before a SOP for the same
                                                         port for a passthrough packet.
                                                         The first detected error associated with bits [5:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
	uint64_t np_dat                       : 1;  /**< Set when a data arrives before a SOP for the same
                                                         port for a non-passthrough packet.
                                                         The first detected error associated with bits [5:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
	uint64_t p_eop                        : 1;  /**< Set when a EOP is followed by an EOP for the same
                                                         port for a passthrough packet.
                                                         The first detected error associated with bits [3:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
	uint64_t p_sop                        : 1;  /**< Set when a SOP is followed by an SOP for the same
                                                         port for a passthrough packet.
                                                         The first detected error associated with bits [3:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
	uint64_t np_eop                       : 1;  /**< Set when a EOP is followed by an EOP for the same
                                                         port for a non-passthrough packet.
                                                         The first detected error associated with bits [3:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
	uint64_t np_sop                       : 1;  /**< Set when a SOP is followed by an SOP for the same
                                                         port for a non-passthrough packet.
                                                         The first detected error associated with bits [3:0]
                                                         of this register will only be set here. A new bit
                                                         can be set when the previous reported bit is cleared. */
#else
	uint64_t np_sop                       : 1;
	uint64_t np_eop                       : 1;
	uint64_t p_sop                        : 1;
	uint64_t p_eop                        : 1;
	uint64_t np_dat                       : 1;
	uint64_t p_dat                        : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} cn50xx;
	struct cvmx_iob_int_sum_cn50xx        cn52xx;
	struct cvmx_iob_int_sum_cn50xx        cn52xxp1;
	struct cvmx_iob_int_sum_cn50xx        cn56xx;
	struct cvmx_iob_int_sum_cn50xx        cn56xxp1;
	struct cvmx_iob_int_sum_cn50xx        cn58xx;
	struct cvmx_iob_int_sum_cn50xx        cn58xxp1;
	struct cvmx_iob_int_sum_cn50xx        cn61xx;
	struct cvmx_iob_int_sum_cn50xx        cn63xx;
	struct cvmx_iob_int_sum_cn50xx        cn63xxp1;
	struct cvmx_iob_int_sum_cn50xx        cn66xx;
	struct cvmx_iob_int_sum_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} cn68xx;
	struct cvmx_iob_int_sum_cn68xx        cn68xxp1;
	struct cvmx_iob_int_sum_s             cn70xx;
	struct cvmx_iob_int_sum_s             cn70xxp1;
	struct cvmx_iob_int_sum_cn50xx        cnf71xx;
};
typedef union cvmx_iob_int_sum cvmx_iob_int_sum_t;

/**
 * cvmx_iob_n2c_l2c_pri_cnt
 *
 * NCB To CMB L2C Priority Counter = NCB to CMB L2C Priority Counter Enable and Timer Value
 * Enables and supplies the timeout count for raising the priority of NCB Store/Load access to
 * the CMB.
 */
union cvmx_iob_n2c_l2c_pri_cnt {
	uint64_t u64;
	struct cvmx_iob_n2c_l2c_pri_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t cnt_enb                      : 1;  /**< Enables the raising of CMB access priority
                                                         when CNT_VAL is reached. */
	uint64_t cnt_val                      : 15; /**< Number of core clocks to wait before raising
                                                         the priority for access to CMB. */
#else
	uint64_t cnt_val                      : 15;
	uint64_t cnt_enb                      : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_iob_n2c_l2c_pri_cnt_s     cn38xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s     cn38xxp2;
	struct cvmx_iob_n2c_l2c_pri_cnt_s     cn52xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s     cn52xxp1;
	struct cvmx_iob_n2c_l2c_pri_cnt_s     cn56xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s     cn56xxp1;
	struct cvmx_iob_n2c_l2c_pri_cnt_s     cn58xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s     cn58xxp1;
	struct cvmx_iob_n2c_l2c_pri_cnt_s     cn61xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s     cn63xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s     cn63xxp1;
	struct cvmx_iob_n2c_l2c_pri_cnt_s     cn66xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s     cn70xx;
	struct cvmx_iob_n2c_l2c_pri_cnt_s     cn70xxp1;
	struct cvmx_iob_n2c_l2c_pri_cnt_s     cnf71xx;
};
typedef union cvmx_iob_n2c_l2c_pri_cnt cvmx_iob_n2c_l2c_pri_cnt_t;

/**
 * cvmx_iob_n2c_rsp_pri_cnt
 *
 * NCB To CMB Response Priority Counter = NCB to CMB Response Priority Counter Enable and Timer
 * Value
 * Enables and supplies the timeout count for raising the priority of NCB Responses access to the
 * CMB.
 */
union cvmx_iob_n2c_rsp_pri_cnt {
	uint64_t u64;
	struct cvmx_iob_n2c_rsp_pri_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t cnt_enb                      : 1;  /**< Enables the raising of CMB access priority
                                                         when CNT_VAL is reached. */
	uint64_t cnt_val                      : 15; /**< Number of core clocks to wait before raising
                                                         the priority for access to CMB. */
#else
	uint64_t cnt_val                      : 15;
	uint64_t cnt_enb                      : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_iob_n2c_rsp_pri_cnt_s     cn38xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s     cn38xxp2;
	struct cvmx_iob_n2c_rsp_pri_cnt_s     cn52xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s     cn52xxp1;
	struct cvmx_iob_n2c_rsp_pri_cnt_s     cn56xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s     cn56xxp1;
	struct cvmx_iob_n2c_rsp_pri_cnt_s     cn58xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s     cn58xxp1;
	struct cvmx_iob_n2c_rsp_pri_cnt_s     cn61xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s     cn63xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s     cn63xxp1;
	struct cvmx_iob_n2c_rsp_pri_cnt_s     cn66xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s     cn70xx;
	struct cvmx_iob_n2c_rsp_pri_cnt_s     cn70xxp1;
	struct cvmx_iob_n2c_rsp_pri_cnt_s     cnf71xx;
};
typedef union cvmx_iob_n2c_rsp_pri_cnt cvmx_iob_n2c_rsp_pri_cnt_t;

/**
 * cvmx_iob_outb_com_pri_cnt
 *
 * Commit To NCB Priority Counter = Commit to NCB Priority Counter Enable and Timer Value
 * Enables and supplies the timeout count for raising the priority of Commit request to the
 * Outbound NCB.
 */
union cvmx_iob_outb_com_pri_cnt {
	uint64_t u64;
	struct cvmx_iob_outb_com_pri_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t cnt_enb                      : 1;  /**< Enables the raising of NCB access priority
                                                         when CNT_VAL is reached. */
	uint64_t cnt_val                      : 15; /**< Number of core clocks to wait before raising
                                                         the priority for access to NCB. */
#else
	uint64_t cnt_val                      : 15;
	uint64_t cnt_enb                      : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_iob_outb_com_pri_cnt_s    cn38xx;
	struct cvmx_iob_outb_com_pri_cnt_s    cn38xxp2;
	struct cvmx_iob_outb_com_pri_cnt_s    cn52xx;
	struct cvmx_iob_outb_com_pri_cnt_s    cn52xxp1;
	struct cvmx_iob_outb_com_pri_cnt_s    cn56xx;
	struct cvmx_iob_outb_com_pri_cnt_s    cn56xxp1;
	struct cvmx_iob_outb_com_pri_cnt_s    cn58xx;
	struct cvmx_iob_outb_com_pri_cnt_s    cn58xxp1;
	struct cvmx_iob_outb_com_pri_cnt_s    cn61xx;
	struct cvmx_iob_outb_com_pri_cnt_s    cn63xx;
	struct cvmx_iob_outb_com_pri_cnt_s    cn63xxp1;
	struct cvmx_iob_outb_com_pri_cnt_s    cn66xx;
	struct cvmx_iob_outb_com_pri_cnt_s    cn68xx;
	struct cvmx_iob_outb_com_pri_cnt_s    cn68xxp1;
	struct cvmx_iob_outb_com_pri_cnt_s    cn70xx;
	struct cvmx_iob_outb_com_pri_cnt_s    cn70xxp1;
	struct cvmx_iob_outb_com_pri_cnt_s    cnf71xx;
};
typedef union cvmx_iob_outb_com_pri_cnt cvmx_iob_outb_com_pri_cnt_t;

/**
 * cvmx_iob_outb_control_match
 *
 * Match pattern for the outbound control to set the OUTB_MATCH_BIT.
 *
 */
union cvmx_iob_outb_control_match {
	uint64_t u64;
	struct cvmx_iob_outb_control_match_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t mask                         : 8;  /**< Pattern to match on the outbound NCB. */
	uint64_t eot                          : 1;  /**< Pattern to match on the outbound NCB. */
	uint64_t dst                          : 8;  /**< Pattern to match on the outbound NCB. */
	uint64_t src                          : 9;  /**< Pattern to match on the outbound NCB. */
#else
	uint64_t src                          : 9;
	uint64_t dst                          : 8;
	uint64_t eot                          : 1;
	uint64_t mask                         : 8;
	uint64_t reserved_26_63               : 38;
#endif
	} s;
	struct cvmx_iob_outb_control_match_s  cn30xx;
	struct cvmx_iob_outb_control_match_s  cn31xx;
	struct cvmx_iob_outb_control_match_s  cn38xx;
	struct cvmx_iob_outb_control_match_s  cn38xxp2;
	struct cvmx_iob_outb_control_match_s  cn50xx;
	struct cvmx_iob_outb_control_match_s  cn52xx;
	struct cvmx_iob_outb_control_match_s  cn52xxp1;
	struct cvmx_iob_outb_control_match_s  cn56xx;
	struct cvmx_iob_outb_control_match_s  cn56xxp1;
	struct cvmx_iob_outb_control_match_s  cn58xx;
	struct cvmx_iob_outb_control_match_s  cn58xxp1;
	struct cvmx_iob_outb_control_match_s  cn61xx;
	struct cvmx_iob_outb_control_match_s  cn63xx;
	struct cvmx_iob_outb_control_match_s  cn63xxp1;
	struct cvmx_iob_outb_control_match_s  cn66xx;
	struct cvmx_iob_outb_control_match_s  cn68xx;
	struct cvmx_iob_outb_control_match_s  cn68xxp1;
	struct cvmx_iob_outb_control_match_s  cn70xx;
	struct cvmx_iob_outb_control_match_s  cn70xxp1;
	struct cvmx_iob_outb_control_match_s  cnf71xx;
};
typedef union cvmx_iob_outb_control_match cvmx_iob_outb_control_match_t;

/**
 * cvmx_iob_outb_control_match_enb
 *
 * Enables the match of the corresponding bit in the IOB_OUTB_CONTROL_MATCH reister.
 *
 */
union cvmx_iob_outb_control_match_enb {
	uint64_t u64;
	struct cvmx_iob_outb_control_match_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t mask                         : 8;  /**< Pattern to match on the outbound NCB. */
	uint64_t eot                          : 1;  /**< Pattern to match on the outbound NCB. */
	uint64_t dst                          : 8;  /**< Pattern to match on the outbound NCB. */
	uint64_t src                          : 9;  /**< Pattern to match on the outbound NCB. */
#else
	uint64_t src                          : 9;
	uint64_t dst                          : 8;
	uint64_t eot                          : 1;
	uint64_t mask                         : 8;
	uint64_t reserved_26_63               : 38;
#endif
	} s;
	struct cvmx_iob_outb_control_match_enb_s cn30xx;
	struct cvmx_iob_outb_control_match_enb_s cn31xx;
	struct cvmx_iob_outb_control_match_enb_s cn38xx;
	struct cvmx_iob_outb_control_match_enb_s cn38xxp2;
	struct cvmx_iob_outb_control_match_enb_s cn50xx;
	struct cvmx_iob_outb_control_match_enb_s cn52xx;
	struct cvmx_iob_outb_control_match_enb_s cn52xxp1;
	struct cvmx_iob_outb_control_match_enb_s cn56xx;
	struct cvmx_iob_outb_control_match_enb_s cn56xxp1;
	struct cvmx_iob_outb_control_match_enb_s cn58xx;
	struct cvmx_iob_outb_control_match_enb_s cn58xxp1;
	struct cvmx_iob_outb_control_match_enb_s cn61xx;
	struct cvmx_iob_outb_control_match_enb_s cn63xx;
	struct cvmx_iob_outb_control_match_enb_s cn63xxp1;
	struct cvmx_iob_outb_control_match_enb_s cn66xx;
	struct cvmx_iob_outb_control_match_enb_s cn68xx;
	struct cvmx_iob_outb_control_match_enb_s cn68xxp1;
	struct cvmx_iob_outb_control_match_enb_s cn70xx;
	struct cvmx_iob_outb_control_match_enb_s cn70xxp1;
	struct cvmx_iob_outb_control_match_enb_s cnf71xx;
};
typedef union cvmx_iob_outb_control_match_enb cvmx_iob_outb_control_match_enb_t;

/**
 * cvmx_iob_outb_data_match
 *
 * Match pattern for the outbound data to set the OUTB_MATCH_BIT.
 *
 */
union cvmx_iob_outb_data_match {
	uint64_t u64;
	struct cvmx_iob_outb_data_match_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Pattern to match on the outbound NCB. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_iob_outb_data_match_s     cn30xx;
	struct cvmx_iob_outb_data_match_s     cn31xx;
	struct cvmx_iob_outb_data_match_s     cn38xx;
	struct cvmx_iob_outb_data_match_s     cn38xxp2;
	struct cvmx_iob_outb_data_match_s     cn50xx;
	struct cvmx_iob_outb_data_match_s     cn52xx;
	struct cvmx_iob_outb_data_match_s     cn52xxp1;
	struct cvmx_iob_outb_data_match_s     cn56xx;
	struct cvmx_iob_outb_data_match_s     cn56xxp1;
	struct cvmx_iob_outb_data_match_s     cn58xx;
	struct cvmx_iob_outb_data_match_s     cn58xxp1;
	struct cvmx_iob_outb_data_match_s     cn61xx;
	struct cvmx_iob_outb_data_match_s     cn63xx;
	struct cvmx_iob_outb_data_match_s     cn63xxp1;
	struct cvmx_iob_outb_data_match_s     cn66xx;
	struct cvmx_iob_outb_data_match_s     cn68xx;
	struct cvmx_iob_outb_data_match_s     cn68xxp1;
	struct cvmx_iob_outb_data_match_s     cn70xx;
	struct cvmx_iob_outb_data_match_s     cn70xxp1;
	struct cvmx_iob_outb_data_match_s     cnf71xx;
};
typedef union cvmx_iob_outb_data_match cvmx_iob_outb_data_match_t;

/**
 * cvmx_iob_outb_data_match_enb
 *
 * Enables the match of the corresponding bit in the IOB_OUTB_DATA_MATCH reister.
 *
 */
union cvmx_iob_outb_data_match_enb {
	uint64_t u64;
	struct cvmx_iob_outb_data_match_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Bit to enable match of. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_iob_outb_data_match_enb_s cn30xx;
	struct cvmx_iob_outb_data_match_enb_s cn31xx;
	struct cvmx_iob_outb_data_match_enb_s cn38xx;
	struct cvmx_iob_outb_data_match_enb_s cn38xxp2;
	struct cvmx_iob_outb_data_match_enb_s cn50xx;
	struct cvmx_iob_outb_data_match_enb_s cn52xx;
	struct cvmx_iob_outb_data_match_enb_s cn52xxp1;
	struct cvmx_iob_outb_data_match_enb_s cn56xx;
	struct cvmx_iob_outb_data_match_enb_s cn56xxp1;
	struct cvmx_iob_outb_data_match_enb_s cn58xx;
	struct cvmx_iob_outb_data_match_enb_s cn58xxp1;
	struct cvmx_iob_outb_data_match_enb_s cn61xx;
	struct cvmx_iob_outb_data_match_enb_s cn63xx;
	struct cvmx_iob_outb_data_match_enb_s cn63xxp1;
	struct cvmx_iob_outb_data_match_enb_s cn66xx;
	struct cvmx_iob_outb_data_match_enb_s cn68xx;
	struct cvmx_iob_outb_data_match_enb_s cn68xxp1;
	struct cvmx_iob_outb_data_match_enb_s cn70xx;
	struct cvmx_iob_outb_data_match_enb_s cn70xxp1;
	struct cvmx_iob_outb_data_match_enb_s cnf71xx;
};
typedef union cvmx_iob_outb_data_match_enb cvmx_iob_outb_data_match_enb_t;

/**
 * cvmx_iob_outb_fpa_pri_cnt
 *
 * FPA To NCB Priority Counter = FPA Returns to NCB Priority Counter Enable and Timer Value
 * Enables and supplies the timeout count for raising the priority of FPA Rreturn Page request to
 * the Outbound NCB.
 */
union cvmx_iob_outb_fpa_pri_cnt {
	uint64_t u64;
	struct cvmx_iob_outb_fpa_pri_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t cnt_enb                      : 1;  /**< Enables the raising of NCB access priority
                                                         when CNT_VAL is reached. */
	uint64_t cnt_val                      : 15; /**< Number of core clocks to wait before raising
                                                         the priority for access to NCB. */
#else
	uint64_t cnt_val                      : 15;
	uint64_t cnt_enb                      : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn38xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn38xxp2;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn52xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn52xxp1;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn56xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn56xxp1;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn58xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn58xxp1;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn61xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn63xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn63xxp1;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn66xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn68xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn68xxp1;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn70xx;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cn70xxp1;
	struct cvmx_iob_outb_fpa_pri_cnt_s    cnf71xx;
};
typedef union cvmx_iob_outb_fpa_pri_cnt cvmx_iob_outb_fpa_pri_cnt_t;

/**
 * cvmx_iob_outb_req_pri_cnt
 *
 * Request To NCB Priority Counter = Request to NCB Priority Counter Enable and Timer Value
 * Enables and supplies the timeout count for raising the priority of Request transfers to the
 * Outbound NCB.
 */
union cvmx_iob_outb_req_pri_cnt {
	uint64_t u64;
	struct cvmx_iob_outb_req_pri_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t cnt_enb                      : 1;  /**< Enables the raising of NCB access priority
                                                         when CNT_VAL is reached. */
	uint64_t cnt_val                      : 15; /**< Number of core clocks to wait before raising
                                                         the priority for access to NCB. */
#else
	uint64_t cnt_val                      : 15;
	uint64_t cnt_enb                      : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_iob_outb_req_pri_cnt_s    cn38xx;
	struct cvmx_iob_outb_req_pri_cnt_s    cn38xxp2;
	struct cvmx_iob_outb_req_pri_cnt_s    cn52xx;
	struct cvmx_iob_outb_req_pri_cnt_s    cn52xxp1;
	struct cvmx_iob_outb_req_pri_cnt_s    cn56xx;
	struct cvmx_iob_outb_req_pri_cnt_s    cn56xxp1;
	struct cvmx_iob_outb_req_pri_cnt_s    cn58xx;
	struct cvmx_iob_outb_req_pri_cnt_s    cn58xxp1;
	struct cvmx_iob_outb_req_pri_cnt_s    cn61xx;
	struct cvmx_iob_outb_req_pri_cnt_s    cn63xx;
	struct cvmx_iob_outb_req_pri_cnt_s    cn63xxp1;
	struct cvmx_iob_outb_req_pri_cnt_s    cn66xx;
	struct cvmx_iob_outb_req_pri_cnt_s    cn68xx;
	struct cvmx_iob_outb_req_pri_cnt_s    cn68xxp1;
	struct cvmx_iob_outb_req_pri_cnt_s    cn70xx;
	struct cvmx_iob_outb_req_pri_cnt_s    cn70xxp1;
	struct cvmx_iob_outb_req_pri_cnt_s    cnf71xx;
};
typedef union cvmx_iob_outb_req_pri_cnt cvmx_iob_outb_req_pri_cnt_t;

/**
 * cvmx_iob_p2c_req_pri_cnt
 *
 * PKO To CMB Response Priority Counter = PKO to CMB Response Priority Counter Enable and Timer
 * Value
 * Enables and supplies the timeout count for raising the priority of PKO Load access to the CMB.
 */
union cvmx_iob_p2c_req_pri_cnt {
	uint64_t u64;
	struct cvmx_iob_p2c_req_pri_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t cnt_enb                      : 1;  /**< Enables the raising of CMB access priority
                                                         when CNT_VAL is reached. */
	uint64_t cnt_val                      : 15; /**< Number of core clocks to wait before raising
                                                         the priority for access to CMB. */
#else
	uint64_t cnt_val                      : 15;
	uint64_t cnt_enb                      : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_iob_p2c_req_pri_cnt_s     cn38xx;
	struct cvmx_iob_p2c_req_pri_cnt_s     cn38xxp2;
	struct cvmx_iob_p2c_req_pri_cnt_s     cn52xx;
	struct cvmx_iob_p2c_req_pri_cnt_s     cn52xxp1;
	struct cvmx_iob_p2c_req_pri_cnt_s     cn56xx;
	struct cvmx_iob_p2c_req_pri_cnt_s     cn56xxp1;
	struct cvmx_iob_p2c_req_pri_cnt_s     cn58xx;
	struct cvmx_iob_p2c_req_pri_cnt_s     cn58xxp1;
	struct cvmx_iob_p2c_req_pri_cnt_s     cn61xx;
	struct cvmx_iob_p2c_req_pri_cnt_s     cn63xx;
	struct cvmx_iob_p2c_req_pri_cnt_s     cn63xxp1;
	struct cvmx_iob_p2c_req_pri_cnt_s     cn66xx;
	struct cvmx_iob_p2c_req_pri_cnt_s     cn70xx;
	struct cvmx_iob_p2c_req_pri_cnt_s     cn70xxp1;
	struct cvmx_iob_p2c_req_pri_cnt_s     cnf71xx;
};
typedef union cvmx_iob_p2c_req_pri_cnt cvmx_iob_p2c_req_pri_cnt_t;

/**
 * cvmx_iob_pkt_err
 *
 * Provides status about the failing packet recevie error.
 *
 */
union cvmx_iob_pkt_err {
	uint64_t u64;
	struct cvmx_iob_pkt_err_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t vport                        : 6;  /**< When IOB_INT_SUM[3:0] bit is set, this field
                                                         latches the failing vport associate with the
                                                         IOB_INT_SUM[3:0] bit set. */
	uint64_t port                         : 6;  /**< When IOB_INT_SUM[3:0] bit is set, this field
                                                         latches the failing port associate with the
                                                         IOB_INT_SUM[3:0] bit set. */
#else
	uint64_t port                         : 6;
	uint64_t vport                        : 6;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_iob_pkt_err_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t port                         : 6;  /**< When IOB_INT_SUM[3:0] bit is set, this field
                                                         latches the failing port associate with the
                                                         IOB_INT_SUM[3:0] bit set. */
#else
	uint64_t port                         : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} cn30xx;
	struct cvmx_iob_pkt_err_cn30xx        cn31xx;
	struct cvmx_iob_pkt_err_cn30xx        cn38xx;
	struct cvmx_iob_pkt_err_cn30xx        cn38xxp2;
	struct cvmx_iob_pkt_err_cn30xx        cn50xx;
	struct cvmx_iob_pkt_err_cn30xx        cn52xx;
	struct cvmx_iob_pkt_err_cn30xx        cn52xxp1;
	struct cvmx_iob_pkt_err_cn30xx        cn56xx;
	struct cvmx_iob_pkt_err_cn30xx        cn56xxp1;
	struct cvmx_iob_pkt_err_cn30xx        cn58xx;
	struct cvmx_iob_pkt_err_cn30xx        cn58xxp1;
	struct cvmx_iob_pkt_err_s             cn61xx;
	struct cvmx_iob_pkt_err_s             cn63xx;
	struct cvmx_iob_pkt_err_s             cn63xxp1;
	struct cvmx_iob_pkt_err_s             cn66xx;
	struct cvmx_iob_pkt_err_s             cn70xx;
	struct cvmx_iob_pkt_err_s             cn70xxp1;
	struct cvmx_iob_pkt_err_s             cnf71xx;
};
typedef union cvmx_iob_pkt_err cvmx_iob_pkt_err_t;

/**
 * cvmx_iob_pp_bist_status
 *
 * The result of the BIST run on the PPs.
 *
 */
union cvmx_iob_pp_bist_status {
	uint64_t u64;
	struct cvmx_iob_pp_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t pp_bstat                     : 4;  /**< BIST Status of PPs. Bit vector position is the number of the PP (i.e. PP0 == PP_BSTAT[0]). */
#else
	uint64_t pp_bstat                     : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_iob_pp_bist_status_s      cn70xx;
	struct cvmx_iob_pp_bist_status_s      cn70xxp1;
};
typedef union cvmx_iob_pp_bist_status cvmx_iob_pp_bist_status_t;

/**
 * cvmx_iob_to_cmb_credits
 *
 * Controls the number of reads and writes that may be outstanding to the L2C (via the CMB).
 *
 */
union cvmx_iob_to_cmb_credits {
	uint64_t u64;
	struct cvmx_iob_to_cmb_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t ncb_rd                       : 3;  /**< Number of NCB reads that can be out to L2C where
                                                         0 == 8-credits. */
	uint64_t ncb_wr                       : 3;  /**< Number of NCB/PKI writes that can be out to L2C
                                                         where 0 == 8-credits. */
#else
	uint64_t ncb_wr                       : 3;
	uint64_t ncb_rd                       : 3;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_iob_to_cmb_credits_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t pko_rd                       : 3;  /**< Number of PKO reads that can be out to L2C where
                                                         0 == 8-credits. */
	uint64_t ncb_rd                       : 3;  /**< Number of NCB reads that can be out to L2C where
                                                         0 == 8-credits. */
	uint64_t ncb_wr                       : 3;  /**< Number of NCB/PKI writes that can be out to L2C
                                                         where 0 == 8-credits. */
#else
	uint64_t ncb_wr                       : 3;
	uint64_t ncb_rd                       : 3;
	uint64_t pko_rd                       : 3;
	uint64_t reserved_9_63                : 55;
#endif
	} cn52xx;
	struct cvmx_iob_to_cmb_credits_cn52xx cn61xx;
	struct cvmx_iob_to_cmb_credits_cn52xx cn63xx;
	struct cvmx_iob_to_cmb_credits_cn52xx cn63xxp1;
	struct cvmx_iob_to_cmb_credits_cn52xx cn66xx;
	struct cvmx_iob_to_cmb_credits_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t dwb                          : 3;  /**< Number of DWBs  that can be out to L2C where
                                                         0 == 8-credits. */
	uint64_t ncb_rd                       : 3;  /**< Number of NCB reads that can be out to L2C where
                                                         0 == 8-credits. */
	uint64_t ncb_wr                       : 3;  /**< Number of NCB/PKI writes that can be out to L2C
                                                         where 0 == 8-credits. */
#else
	uint64_t ncb_wr                       : 3;
	uint64_t ncb_rd                       : 3;
	uint64_t dwb                          : 3;
	uint64_t reserved_9_63                : 55;
#endif
	} cn68xx;
	struct cvmx_iob_to_cmb_credits_cn68xx cn68xxp1;
	struct cvmx_iob_to_cmb_credits_cn52xx cn70xx;
	struct cvmx_iob_to_cmb_credits_cn52xx cn70xxp1;
	struct cvmx_iob_to_cmb_credits_cn52xx cnf71xx;
};
typedef union cvmx_iob_to_cmb_credits cvmx_iob_to_cmb_credits_t;

/**
 * cvmx_iob_to_ncb_did_00_credits
 *
 * IOB_TO_NCB_DID_00_CREDITS = IOB NCB DID 00 Credits
 *
 * Number of credits for NCB DID 00.
 */
union cvmx_iob_to_ncb_did_00_credits {
	uint64_t u64;
	struct cvmx_iob_to_ncb_did_00_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t crd                          : 7;  /**< Number of credits for DID. Writing this field will
                                                         casuse the credits to be set to the value written.
                                                         Reading this field will give the number of credits
                                                         PRESENTLY available. */
#else
	uint64_t crd                          : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_iob_to_ncb_did_00_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_00_credits_s cn68xxp1;
};
typedef union cvmx_iob_to_ncb_did_00_credits cvmx_iob_to_ncb_did_00_credits_t;

/**
 * cvmx_iob_to_ncb_did_111_credits
 *
 * IOB_TO_NCB_DID_111_CREDITS = IOB NCB DID 111 Credits
 *
 * Number of credits for NCB DID 111.
 */
union cvmx_iob_to_ncb_did_111_credits {
	uint64_t u64;
	struct cvmx_iob_to_ncb_did_111_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t crd                          : 7;  /**< Number of credits for DID. Writing this field will
                                                         casuse the credits to be set to the value written.
                                                         Reading this field will give the number of credits
                                                         PRESENTLY available. */
#else
	uint64_t crd                          : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_iob_to_ncb_did_111_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_111_credits_s cn68xxp1;
};
typedef union cvmx_iob_to_ncb_did_111_credits cvmx_iob_to_ncb_did_111_credits_t;

/**
 * cvmx_iob_to_ncb_did_223_credits
 *
 * IOB_TO_NCB_DID_223_CREDITS = IOB NCB DID 223 Credits
 *
 * Number of credits for NCB DID 223.
 */
union cvmx_iob_to_ncb_did_223_credits {
	uint64_t u64;
	struct cvmx_iob_to_ncb_did_223_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t crd                          : 7;  /**< Number of credits for DID. Writing this field will
                                                         casuse the credits to be set to the value written.
                                                         Reading this field will give the number of credits
                                                         PRESENTLY available. */
#else
	uint64_t crd                          : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_iob_to_ncb_did_223_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_223_credits_s cn68xxp1;
};
typedef union cvmx_iob_to_ncb_did_223_credits cvmx_iob_to_ncb_did_223_credits_t;

/**
 * cvmx_iob_to_ncb_did_24_credits
 *
 * IOB_TO_NCB_DID_24_CREDITS = IOB NCB DID 24 Credits
 *
 * Number of credits for NCB DID 24.
 */
union cvmx_iob_to_ncb_did_24_credits {
	uint64_t u64;
	struct cvmx_iob_to_ncb_did_24_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t crd                          : 7;  /**< Number of credits for DID. Writing this field will
                                                         casuse the credits to be set to the value written.
                                                         Reading this field will give the number of credits
                                                         PRESENTLY available. */
#else
	uint64_t crd                          : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_iob_to_ncb_did_24_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_24_credits_s cn68xxp1;
};
typedef union cvmx_iob_to_ncb_did_24_credits cvmx_iob_to_ncb_did_24_credits_t;

/**
 * cvmx_iob_to_ncb_did_32_credits
 *
 * IOB_TO_NCB_DID_32_CREDITS = IOB NCB DID 32 Credits
 *
 * Number of credits for NCB DID 32.
 */
union cvmx_iob_to_ncb_did_32_credits {
	uint64_t u64;
	struct cvmx_iob_to_ncb_did_32_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t crd                          : 7;  /**< Number of credits for DID. Writing this field will
                                                         casuse the credits to be set to the value written.
                                                         Reading this field will give the number of credits
                                                         PRESENTLY available. */
#else
	uint64_t crd                          : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_iob_to_ncb_did_32_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_32_credits_s cn68xxp1;
};
typedef union cvmx_iob_to_ncb_did_32_credits cvmx_iob_to_ncb_did_32_credits_t;

/**
 * cvmx_iob_to_ncb_did_40_credits
 *
 * IOB_TO_NCB_DID_40_CREDITS = IOB NCB DID 40 Credits
 *
 * Number of credits for NCB DID 40.
 */
union cvmx_iob_to_ncb_did_40_credits {
	uint64_t u64;
	struct cvmx_iob_to_ncb_did_40_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t crd                          : 7;  /**< Number of credits for DID. Writing this field will
                                                         casuse the credits to be set to the value written.
                                                         Reading this field will give the number of credits
                                                         PRESENTLY available. */
#else
	uint64_t crd                          : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_iob_to_ncb_did_40_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_40_credits_s cn68xxp1;
};
typedef union cvmx_iob_to_ncb_did_40_credits cvmx_iob_to_ncb_did_40_credits_t;

/**
 * cvmx_iob_to_ncb_did_55_credits
 *
 * IOB_TO_NCB_DID_55_CREDITS = IOB NCB DID 55 Credits
 *
 * Number of credits for NCB DID 55.
 */
union cvmx_iob_to_ncb_did_55_credits {
	uint64_t u64;
	struct cvmx_iob_to_ncb_did_55_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t crd                          : 7;  /**< Number of credits for DID. Writing this field will
                                                         casuse the credits to be set to the value written.
                                                         Reading this field will give the number of credits
                                                         PRESENTLY available. */
#else
	uint64_t crd                          : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_iob_to_ncb_did_55_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_55_credits_s cn68xxp1;
};
typedef union cvmx_iob_to_ncb_did_55_credits cvmx_iob_to_ncb_did_55_credits_t;

/**
 * cvmx_iob_to_ncb_did_64_credits
 *
 * IOB_TO_NCB_DID_64_CREDITS = IOB NCB DID 64 Credits
 *
 * Number of credits for NCB DID 64.
 */
union cvmx_iob_to_ncb_did_64_credits {
	uint64_t u64;
	struct cvmx_iob_to_ncb_did_64_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t crd                          : 7;  /**< Number of credits for DID. Writing this field will
                                                         casuse the credits to be set to the value written.
                                                         Reading this field will give the number of credits
                                                         PRESENTLY available. */
#else
	uint64_t crd                          : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_iob_to_ncb_did_64_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_64_credits_s cn68xxp1;
};
typedef union cvmx_iob_to_ncb_did_64_credits cvmx_iob_to_ncb_did_64_credits_t;

/**
 * cvmx_iob_to_ncb_did_79_credits
 *
 * IOB_TO_NCB_DID_79_CREDITS = IOB NCB DID 79 Credits
 *
 * Number of credits for NCB DID 79.
 */
union cvmx_iob_to_ncb_did_79_credits {
	uint64_t u64;
	struct cvmx_iob_to_ncb_did_79_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t crd                          : 7;  /**< Number of credits for DID. Writing this field will
                                                         casuse the credits to be set to the value written.
                                                         Reading this field will give the number of credits
                                                         PRESENTLY available. */
#else
	uint64_t crd                          : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_iob_to_ncb_did_79_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_79_credits_s cn68xxp1;
};
typedef union cvmx_iob_to_ncb_did_79_credits cvmx_iob_to_ncb_did_79_credits_t;

/**
 * cvmx_iob_to_ncb_did_96_credits
 *
 * IOB_TO_NCB_DID_96_CREDITS = IOB NCB DID 96 Credits
 *
 * Number of credits for NCB DID 96.
 */
union cvmx_iob_to_ncb_did_96_credits {
	uint64_t u64;
	struct cvmx_iob_to_ncb_did_96_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t crd                          : 7;  /**< Number of credits for DID. Writing this field will
                                                         casuse the credits to be set to the value written.
                                                         Reading this field will give the number of credits
                                                         PRESENTLY available. */
#else
	uint64_t crd                          : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_iob_to_ncb_did_96_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_96_credits_s cn68xxp1;
};
typedef union cvmx_iob_to_ncb_did_96_credits cvmx_iob_to_ncb_did_96_credits_t;

/**
 * cvmx_iob_to_ncb_did_98_credits
 *
 * IOB_TO_NCB_DID_98_CREDITS = IOB NCB DID 96 Credits
 *
 * Number of credits for NCB DID 98.
 */
union cvmx_iob_to_ncb_did_98_credits {
	uint64_t u64;
	struct cvmx_iob_to_ncb_did_98_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t crd                          : 7;  /**< Number of credits for DID. Writing this field will
                                                         casuse the credits to be set to the value written.
                                                         Reading this field will give the number of credits
                                                         PRESENTLY available. */
#else
	uint64_t crd                          : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_iob_to_ncb_did_98_credits_s cn68xx;
	struct cvmx_iob_to_ncb_did_98_credits_s cn68xxp1;
};
typedef union cvmx_iob_to_ncb_did_98_credits cvmx_iob_to_ncb_did_98_credits_t;

#endif
