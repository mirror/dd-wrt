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
 * cvmx-ase-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon ase.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_ASE_DEFS_H__
#define __CVMX_ASE_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_BACKDOOR_REQ_CTL CVMX_ASE_BACKDOOR_REQ_CTL_FUNC()
static inline uint64_t CVMX_ASE_BACKDOOR_REQ_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_BACKDOOR_REQ_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DD000800ull);
}
#else
#define CVMX_ASE_BACKDOOR_REQ_CTL (CVMX_ADD_IO_SEG(0x00011800DD000800ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ASE_BACKDOOR_REQ_DATAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ASE_BACKDOOR_REQ_DATAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800DD000880ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_ASE_BACKDOOR_REQ_DATAX(offset) (CVMX_ADD_IO_SEG(0x00011800DD000880ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_BACKDOOR_RSP_CTL CVMX_ASE_BACKDOOR_RSP_CTL_FUNC()
static inline uint64_t CVMX_ASE_BACKDOOR_RSP_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_BACKDOOR_RSP_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DD000900ull);
}
#else
#define CVMX_ASE_BACKDOOR_RSP_CTL (CVMX_ADD_IO_SEG(0x00011800DD000900ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ASE_BACKDOOR_RSP_DATAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ASE_BACKDOOR_RSP_DATAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800DD000980ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_ASE_BACKDOOR_RSP_DATAX(offset) (CVMX_ADD_IO_SEG(0x00011800DD000980ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_BIST_STATUS0 CVMX_ASE_BIST_STATUS0_FUNC()
static inline uint64_t CVMX_ASE_BIST_STATUS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_BIST_STATUS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DD000008ull);
}
#else
#define CVMX_ASE_BIST_STATUS0 (CVMX_ADD_IO_SEG(0x00011800DD000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_BIST_STATUS1 CVMX_ASE_BIST_STATUS1_FUNC()
static inline uint64_t CVMX_ASE_BIST_STATUS1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_BIST_STATUS1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DD000010ull);
}
#else
#define CVMX_ASE_BIST_STATUS1 (CVMX_ADD_IO_SEG(0x00011800DD000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_CONFIG CVMX_ASE_CONFIG_FUNC()
static inline uint64_t CVMX_ASE_CONFIG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_CONFIG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DD000100ull);
}
#else
#define CVMX_ASE_CONFIG (CVMX_ADD_IO_SEG(0x00011800DD000100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_ECC_CTL CVMX_ASE_ECC_CTL_FUNC()
static inline uint64_t CVMX_ASE_ECC_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_ECC_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DD000018ull);
}
#else
#define CVMX_ASE_ECC_CTL (CVMX_ADD_IO_SEG(0x00011800DD000018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_ECC_INT CVMX_ASE_ECC_INT_FUNC()
static inline uint64_t CVMX_ASE_ECC_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_ECC_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DD000028ull);
}
#else
#define CVMX_ASE_ECC_INT (CVMX_ADD_IO_SEG(0x00011800DD000028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_GEN_INT CVMX_ASE_GEN_INT_FUNC()
static inline uint64_t CVMX_ASE_GEN_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_GEN_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DD000020ull);
}
#else
#define CVMX_ASE_GEN_INT (CVMX_ADD_IO_SEG(0x00011800DD000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_LIP_CONFIG CVMX_ASE_LIP_CONFIG_FUNC()
static inline uint64_t CVMX_ASE_LIP_CONFIG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_LIP_CONFIG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DD400000ull);
}
#else
#define CVMX_ASE_LIP_CONFIG (CVMX_ADD_IO_SEG(0x00011800DD400000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_LIP_SPARE CVMX_ASE_LIP_SPARE_FUNC()
static inline uint64_t CVMX_ASE_LIP_SPARE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_LIP_SPARE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DD7FFFF8ull);
}
#else
#define CVMX_ASE_LIP_SPARE (CVMX_ADD_IO_SEG(0x00011800DD7FFFF8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_LOP_CONFIG CVMX_ASE_LOP_CONFIG_FUNC()
static inline uint64_t CVMX_ASE_LOP_CONFIG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_LOP_CONFIG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DD800000ull);
}
#else
#define CVMX_ASE_LOP_CONFIG (CVMX_ADD_IO_SEG(0x00011800DD800000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_LOP_SPARE CVMX_ASE_LOP_SPARE_FUNC()
static inline uint64_t CVMX_ASE_LOP_SPARE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_LOP_SPARE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DDBFFFF8ull);
}
#else
#define CVMX_ASE_LOP_SPARE (CVMX_ADD_IO_SEG(0x00011800DDBFFFF8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_LUE_CONFIG CVMX_ASE_LUE_CONFIG_FUNC()
static inline uint64_t CVMX_ASE_LUE_CONFIG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_LUE_CONFIG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DDC00000ull);
}
#else
#define CVMX_ASE_LUE_CONFIG (CVMX_ADD_IO_SEG(0x00011800DDC00000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_LUE_DBG_CTL0 CVMX_ASE_LUE_DBG_CTL0_FUNC()
static inline uint64_t CVMX_ASE_LUE_DBG_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_LUE_DBG_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DDC00030ull);
}
#else
#define CVMX_ASE_LUE_DBG_CTL0 (CVMX_ADD_IO_SEG(0x00011800DDC00030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_LUE_DBG_CTL1 CVMX_ASE_LUE_DBG_CTL1_FUNC()
static inline uint64_t CVMX_ASE_LUE_DBG_CTL1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_LUE_DBG_CTL1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DDC00038ull);
}
#else
#define CVMX_ASE_LUE_DBG_CTL1 (CVMX_ADD_IO_SEG(0x00011800DDC00038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_LUE_ERROR_LOG CVMX_ASE_LUE_ERROR_LOG_FUNC()
static inline uint64_t CVMX_ASE_LUE_ERROR_LOG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_LUE_ERROR_LOG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DDC00018ull);
}
#else
#define CVMX_ASE_LUE_ERROR_LOG (CVMX_ADD_IO_SEG(0x00011800DDC00018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_LUE_ERROR_LOG_ENABLE CVMX_ASE_LUE_ERROR_LOG_ENABLE_FUNC()
static inline uint64_t CVMX_ASE_LUE_ERROR_LOG_ENABLE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_LUE_ERROR_LOG_ENABLE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DDC00010ull);
}
#else
#define CVMX_ASE_LUE_ERROR_LOG_ENABLE (CVMX_ADD_IO_SEG(0x00011800DDC00010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_LUE_PERFORMANCE_CONTROL0 CVMX_ASE_LUE_PERFORMANCE_CONTROL0_FUNC()
static inline uint64_t CVMX_ASE_LUE_PERFORMANCE_CONTROL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_LUE_PERFORMANCE_CONTROL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DDC00060ull);
}
#else
#define CVMX_ASE_LUE_PERFORMANCE_CONTROL0 (CVMX_ADD_IO_SEG(0x00011800DDC00060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_LUE_PERFORMANCE_CONTROL1 CVMX_ASE_LUE_PERFORMANCE_CONTROL1_FUNC()
static inline uint64_t CVMX_ASE_LUE_PERFORMANCE_CONTROL1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_LUE_PERFORMANCE_CONTROL1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DDC00068ull);
}
#else
#define CVMX_ASE_LUE_PERFORMANCE_CONTROL1 (CVMX_ADD_IO_SEG(0x00011800DDC00068ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ASE_LUE_PERFORMANCE_CONTROLX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset >= 2) && (offset <= 3))))))
		cvmx_warn("CVMX_ASE_LUE_PERFORMANCE_CONTROLX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800DDC00070ull) + ((offset) & 3) * 8 - 8*2;
}
#else
#define CVMX_ASE_LUE_PERFORMANCE_CONTROLX(offset) (CVMX_ADD_IO_SEG(0x00011800DDC00070ull) + ((offset) & 3) * 8 - 8*2)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ASE_LUE_PERFORMANCE_COUNTERX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3)))))
		cvmx_warn("CVMX_ASE_LUE_PERFORMANCE_COUNTERX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800DDC00080ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_ASE_LUE_PERFORMANCE_COUNTERX(offset) (CVMX_ADD_IO_SEG(0x00011800DDC00080ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_LUE_PERF_FILT CVMX_ASE_LUE_PERF_FILT_FUNC()
static inline uint64_t CVMX_ASE_LUE_PERF_FILT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_LUE_PERF_FILT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DDC00040ull);
}
#else
#define CVMX_ASE_LUE_PERF_FILT (CVMX_ADD_IO_SEG(0x00011800DDC00040ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_LUE_SPARE CVMX_ASE_LUE_SPARE_FUNC()
static inline uint64_t CVMX_ASE_LUE_SPARE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_LUE_SPARE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DDFFFFF8ull);
}
#else
#define CVMX_ASE_LUE_SPARE (CVMX_ADD_IO_SEG(0x00011800DDFFFFF8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_LUE_TWE_BWE_ENABLE CVMX_ASE_LUE_TWE_BWE_ENABLE_FUNC()
static inline uint64_t CVMX_ASE_LUE_TWE_BWE_ENABLE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_LUE_TWE_BWE_ENABLE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DDC00008ull);
}
#else
#define CVMX_ASE_LUE_TWE_BWE_ENABLE (CVMX_ADD_IO_SEG(0x00011800DDC00008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_LUF_ERROR_LOG CVMX_ASE_LUF_ERROR_LOG_FUNC()
static inline uint64_t CVMX_ASE_LUF_ERROR_LOG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_LUF_ERROR_LOG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DD000110ull);
}
#else
#define CVMX_ASE_LUF_ERROR_LOG (CVMX_ADD_IO_SEG(0x00011800DD000110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_SFT_RST CVMX_ASE_SFT_RST_FUNC()
static inline uint64_t CVMX_ASE_SFT_RST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_SFT_RST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DD000000ull);
}
#else
#define CVMX_ASE_SFT_RST (CVMX_ADD_IO_SEG(0x00011800DD000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ASE_SPARE CVMX_ASE_SPARE_FUNC()
static inline uint64_t CVMX_ASE_SPARE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ASE_SPARE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DD3FFFF8ull);
}
#else
#define CVMX_ASE_SPARE (CVMX_ADD_IO_SEG(0x00011800DD3FFFF8ull))
#endif

/**
 * cvmx_ase_backdoor_req_ctl
 *
 * This register is used to configure and trigger backdoor requests. Backdoor requests
 * can be inserted at any time. They are inserted into the request stream from LAP. The
 * request packet needs to be written to ASE_BACKDOOR_REQ_DATA(), and must be written
 * before [VALID] is triggered. Both [CNT] and [VALID] can be written in the same
 * cycle. The hardware clears the [VALID] bit when the request is sent. If another
 * [VALID]=1 is written before the bit is cleared, it will not trigger another
 * SOP. Software should take care to wait for the response before issuing another
 * request. Hardware deasserts ASE_BACKDOOR_RSP_CTL[VALID] when [VALID] is triggered.
 */
union cvmx_ase_backdoor_req_ctl {
	uint64_t u64;
	struct cvmx_ase_backdoor_req_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t valid                        : 1;  /**< Valid. Writing 1 triggers [CNT] beats to be sent as a packet into ASE. */
	uint64_t reserved_4_62                : 59;
	uint64_t cnt                          : 4;  /**< Number of DATA beats to send. Valid values are 0x2 - 0xB. */
#else
	uint64_t cnt                          : 4;
	uint64_t reserved_4_62                : 59;
	uint64_t valid                        : 1;
#endif
	} s;
	struct cvmx_ase_backdoor_req_ctl_s    cn78xx;
	struct cvmx_ase_backdoor_req_ctl_s    cn78xxp1;
};
typedef union cvmx_ase_backdoor_req_ctl cvmx_ase_backdoor_req_ctl_t;

/**
 * cvmx_ase_backdoor_req_data#
 *
 * The lowest address is first beat (aka control word) and has the SOP. The next address is next
 * beat, etc. The register offset indicated by ASE_BACKDOOR_REQ_CTL[CNT] has the EOP. See further
 * information in ASE_BACKDOOR_REQ_CTL.
 */
union cvmx_ase_backdoor_req_datax {
	uint64_t u64;
	struct cvmx_ase_backdoor_req_datax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Beat N of the backdoor request packet. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_ase_backdoor_req_datax_s  cn78xx;
	struct cvmx_ase_backdoor_req_datax_s  cn78xxp1;
};
typedef union cvmx_ase_backdoor_req_datax cvmx_ase_backdoor_req_datax_t;

/**
 * cvmx_ase_backdoor_rsp_ctl
 *
 * This register is used to indicate that the backdoor response is complete. See
 * description in ASE_BACKDOOR_REQ_CTL. Hardware asserts [VALID] when the full response
 * packet has been received and has been posted to [CNT] and
 * ASE_BACKDOOR_RSP_DATA(). Hardware does not change [CNT] and ASE_BACKDOOR_RSP_DATA()
 * while [VALID] is asserted. Hardware deasserts [VALID] when
 * ASE_BACKDOOR_REQ_CTL[VALID] is triggered.
 */
union cvmx_ase_backdoor_rsp_ctl {
	uint64_t u64;
	struct cvmx_ase_backdoor_rsp_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t valid                        : 1;  /**< Valid. Asserted means there is valid response packet data. */
	uint64_t reserved_3_62                : 60;
	uint64_t cnt                          : 3;  /**< Number of DATA beats received. Valid values are 2 to 5. */
#else
	uint64_t cnt                          : 3;
	uint64_t reserved_3_62                : 60;
	uint64_t valid                        : 1;
#endif
	} s;
	struct cvmx_ase_backdoor_rsp_ctl_s    cn78xx;
	struct cvmx_ase_backdoor_rsp_ctl_s    cn78xxp1;
};
typedef union cvmx_ase_backdoor_rsp_ctl cvmx_ase_backdoor_rsp_ctl_t;

/**
 * cvmx_ase_backdoor_rsp_data#
 *
 * The lowest address is first beat (aka control word) and has the SOP. The next address is next
 * beat, etc. The register offset indicated by ASE_BACKDOOR_RSP_CTL[CNT] has the EOP. See further
 * information in ASE_BACKDOOR_RSP_CTL.
 */
union cvmx_ase_backdoor_rsp_datax {
	uint64_t u64;
	struct cvmx_ase_backdoor_rsp_datax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Beat N of the backdoor response packet. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_ase_backdoor_rsp_datax_s  cn78xx;
	struct cvmx_ase_backdoor_rsp_datax_s  cn78xxp1;
};
typedef union cvmx_ase_backdoor_rsp_datax cvmx_ase_backdoor_rsp_datax_t;

/**
 * cvmx_ase_bist_status0
 *
 * This is BIST status register 0.
 *
 */
union cvmx_ase_bist_status0 {
	uint64_t u64;
	struct cvmx_ase_bist_status0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t lue_rmc_ndone                : 4;  /**< Combined `BIST is not complete' for the LUE RMC[3..0] RAMs. */
	uint64_t reserved_51_55               : 5;
	uint64_t lue_rft_ndone                : 1;  /**< BIST is not complete for the LUE HST RFT RAM. */
	uint64_t lue_tat_ndone                : 1;  /**< BIST is not complete for the LUE HST TAT RAM. */
	uint64_t lue_kdb_ndone                : 1;  /**< BIST is not complete for the LUE KRQ KDB RAM. */
	uint64_t reserved_42_47               : 6;
	uint64_t lop_txb_ndone                : 2;  /**< BIST is not complete on the LOP TXBUFF RAM banks. */
	uint64_t reserved_36_39               : 4;
	uint64_t lip_newq_ndone               : 1;  /**< BIST is not complete on the LIP NEWQ RAM. */
	uint64_t lip_pht_ndone                : 1;  /**< BIST is not complete on the LIP PHT RAM. */
	uint64_t lip_gdt_ndone                : 1;  /**< BIST is not complete on the LIP GDT RAM. */
	uint64_t lip_isf_ndone                : 1;  /**< BIST is not complete on the LIP ISF RAM. */
	uint64_t reserved_19_31               : 13;
	uint64_t lue_rft_status               : 1;  /**< BIST status for the LUE HST RFT RAM. Only valid if [LUE_RFT_NDONE] == 0. */
	uint64_t lue_tat_status               : 1;  /**< BIST status for the LUE HST TAT RAM. Only valid if [LUE_TAT_NDONE] == 0. */
	uint64_t lue_kdb_status               : 1;  /**< BIST status for the LUE KRQ KDB RAM. Only valid if [LUE_KDB_NDONE] == 0. */
	uint64_t reserved_10_15               : 6;
	uint64_t lop_txb_status               : 2;  /**< BIST status for the LOP TXBUFF RAM banks. Only valid if [LOP_TXB_NDONE]<> == 0. */
	uint64_t reserved_4_7                 : 4;
	uint64_t lip_newq_status              : 1;  /**< BIST status for the LIP NEWQ RAM. Only valid if [LIP_NEWQ_NDONE] == 0. */
	uint64_t lip_pht_status               : 1;  /**< BIST status for the LIP PHT RAM. Only valid if [LIP_PHT_NDONE] == 0. */
	uint64_t lip_gdt_status               : 1;  /**< BIST status for the LIP GDT RAM. Only valid if [LIP_GDT_NDONE] == 0. */
	uint64_t lip_isf_status               : 1;  /**< BIST status for the LIP ISF RAM. Only valid if [LIP_ISF_NDONE] == 0. */
#else
	uint64_t lip_isf_status               : 1;
	uint64_t lip_gdt_status               : 1;
	uint64_t lip_pht_status               : 1;
	uint64_t lip_newq_status              : 1;
	uint64_t reserved_4_7                 : 4;
	uint64_t lop_txb_status               : 2;
	uint64_t reserved_10_15               : 6;
	uint64_t lue_kdb_status               : 1;
	uint64_t lue_tat_status               : 1;
	uint64_t lue_rft_status               : 1;
	uint64_t reserved_19_31               : 13;
	uint64_t lip_isf_ndone                : 1;
	uint64_t lip_gdt_ndone                : 1;
	uint64_t lip_pht_ndone                : 1;
	uint64_t lip_newq_ndone               : 1;
	uint64_t reserved_36_39               : 4;
	uint64_t lop_txb_ndone                : 2;
	uint64_t reserved_42_47               : 6;
	uint64_t lue_kdb_ndone                : 1;
	uint64_t lue_tat_ndone                : 1;
	uint64_t lue_rft_ndone                : 1;
	uint64_t reserved_51_55               : 5;
	uint64_t lue_rmc_ndone                : 4;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_ase_bist_status0_s        cn78xx;
	struct cvmx_ase_bist_status0_s        cn78xxp1;
};
typedef union cvmx_ase_bist_status0 cvmx_ase_bist_status0_t;

/**
 * cvmx_ase_bist_status1
 *
 * This is the per-LUE RMC engine BIST status register 1.
 *
 */
union cvmx_ase_bist_status1 {
	uint64_t u64;
	struct cvmx_ase_bist_status1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t lue_rmc3_kdt_status          : 4;  /**< BIST status for the LUE per-RMC KDT RAMs. Only valid if ASE_BIST_STATUS0[LUE_RMC_NDONE<3>] == 0. */
	uint64_t lue_rmc3_rul_status          : 1;  /**< BIST status for the LUE per-RMC RUL RAM. Only valid if ASE_BIST_STATUS0[LUE_RMC_NDONE<3>] == 0. */
	uint64_t lue_rmc3_rft_status          : 1;  /**< BIST status for the LUE per-RMC RFT RAM. Only valid if ASE_BIST_STATUS0[LUE_RMC_NDONE<3>] == 0. */
	uint64_t reserved_40_49               : 10;
	uint64_t lue_rmc2_kdt_status          : 4;  /**< BIST status for the LUE per-RMC KDT RAMs. Only valid if ASE_BIST_STATUS0[LUE_RMC_NDONE<2>] == 0. */
	uint64_t lue_rmc2_rul_status          : 1;  /**< BIST status for the LUE per-RMC RUL RAM. Only valid if ASE_BIST_STATUS0[LUE_RMC_NDONE<2>] == 0. */
	uint64_t lue_rmc2_rft_status          : 1;  /**< BIST status for the LUE per-RMC RFT RAM. Only valid if ASE_BIST_STATUS0[LUE_RMC_NDONE<2>] == 0. */
	uint64_t reserved_24_33               : 10;
	uint64_t lue_rmc1_kdt_status          : 4;  /**< BIST status for the LUE per-RMC KDT RAMs. Only valid if ASE_BIST_STATUS0[LUE_RMC_NDONE<1>] == 0. */
	uint64_t lue_rmc1_rul_status          : 1;  /**< BIST status for the LUE per-RMC RUL RAM. Only valid if ASE_BIST_STATUS0[LUE_RMC_NDONE<1>] == 0. */
	uint64_t lue_rmc1_rft_status          : 1;  /**< BIST status for the LUE per-RMC RFT RAM. Only valid if ASE_BIST_STATUS0[LUE_RMC_NDONE<1>] == 0. */
	uint64_t reserved_8_17                : 10;
	uint64_t lue_rmc0_kdt_status          : 4;  /**< BIST status for the LUE per-RMC KDT RAMs. Only valid if ASE_BIST_STATUS0[LUE_RMC_NDONE<0>] == 0. */
	uint64_t lue_rmc0_rul_status          : 1;  /**< BIST status for the LUE per-RMC RUL RAM. Only valid if ASE_BIST_STATUS0[LUE_RMC_NDONE<0>] == 0. */
	uint64_t lue_rmc0_rft_status          : 1;  /**< BIST status for the LUE per-RMC RFT RAM. Only valid if ASE_BIST_STATUS0[LUE_RMC_NDONE<0>] == 0. */
	uint64_t reserved_0_1                 : 2;
#else
	uint64_t reserved_0_1                 : 2;
	uint64_t lue_rmc0_rft_status          : 1;
	uint64_t lue_rmc0_rul_status          : 1;
	uint64_t lue_rmc0_kdt_status          : 4;
	uint64_t reserved_8_17                : 10;
	uint64_t lue_rmc1_rft_status          : 1;
	uint64_t lue_rmc1_rul_status          : 1;
	uint64_t lue_rmc1_kdt_status          : 4;
	uint64_t reserved_24_33               : 10;
	uint64_t lue_rmc2_rft_status          : 1;
	uint64_t lue_rmc2_rul_status          : 1;
	uint64_t lue_rmc2_kdt_status          : 4;
	uint64_t reserved_40_49               : 10;
	uint64_t lue_rmc3_rft_status          : 1;
	uint64_t lue_rmc3_rul_status          : 1;
	uint64_t lue_rmc3_kdt_status          : 4;
	uint64_t reserved_56_63               : 8;
#endif
	} s;
	struct cvmx_ase_bist_status1_s        cn78xx;
	struct cvmx_ase_bist_status1_s        cn78xxp1;
};
typedef union cvmx_ase_bist_status1 cvmx_ase_bist_status1_t;

/**
 * cvmx_ase_config
 *
 * This is the general configuration register for the ASE block.
 *
 */
union cvmx_ase_config {
	uint64_t u64;
	struct cvmx_ase_config_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t endian_mode                  : 2;  /**< See ASE_ENDIAN_E. Endian swapping only applies to lookup request header data and KEY_RSP
                                                         match result. */
	uint64_t reserved_2_4                 : 3;
	uint64_t div2_clken                   : 1;  /**< Enable conditional SCLK/2 in ASE. This only enables the SCLK/2 domain, not the SCLK. Turn
                                                         this on if you want to do lookup requests. */
	uint64_t div1_clken                   : 1;  /**< Enable conditional SCLK in ASE. This only enables the SCLK domain, not the SCLK/2. Turn
                                                         this on if you want to do lookup requests or if you want to access OSM. */
#else
	uint64_t div1_clken                   : 1;
	uint64_t div2_clken                   : 1;
	uint64_t reserved_2_4                 : 3;
	uint64_t endian_mode                  : 2;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_ase_config_s              cn78xx;
	struct cvmx_ase_config_s              cn78xxp1;
};
typedef union cvmx_ase_config cvmx_ase_config_t;

/**
 * cvmx_ase_ecc_ctl
 *
 * This register can be used to disable ECC checks, insert ECC errors.
 *
 * Fields *ECC_DIS disable SBE detection/correction and DBE detection. If ECC_DIS is 0x1, then no
 * errors are detected.
 *
 * Fields *ECC_FLIP_SYND flip the syndrome<1:0> bits to generate 1-bit/2-bits error for testing.
 * _ 0x0 = normal operation.
 * _ 0x1 = SBE on bit<0>.
 * _ 0x2 = SBE on bit<1>.
 * _ 0x3 = DBE on bit<1:0>.
 */
union cvmx_ase_ecc_ctl {
	uint64_t u64;
	struct cvmx_ase_ecc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t lue_kdt_ecc_flip_synd        : 2;  /**< Flip syndrome in LUE RMC key data transfer buffer. */
	uint64_t lue_rul_ecc_flip_synd        : 2;  /**< Flip syndrome in LUE RMC buffer aligner wrapper rule FIFO. */
	uint64_t lue_rft_ecc_flip_synd        : 2;  /**< Flip syndrome in LUE HST and RMC rule format tables. */
	uint64_t lue_tat_ecc_flip_synd        : 2;  /**< Flip syndrome in LUE HST ruleDB access table. */
	uint64_t lue_kdb_ecc_flip_synd        : 2;  /**< Flip syndrome in LUE KRQ key data buffer. */
	uint64_t reserved_37_43               : 7;
	uint64_t lue_kdt_ecc_dis              : 1;  /**< Disable ECC for LUE RMC key data transfer buffer. */
	uint64_t lue_rul_ecc_dis              : 1;  /**< Disable ECC for LUE RMC buffer aligner wrapper rule FIFO. */
	uint64_t lue_rft_ecc_dis              : 1;  /**< Disable ECC for LUE HST and RMC rule format tables. */
	uint64_t lue_tat_ecc_dis              : 1;  /**< Disable ECC for LUE HST ruleDB access table. */
	uint64_t lue_kdb_ecc_dis              : 1;  /**< Disable ECC for LUE KRQ key data buffer. */
	uint64_t reserved_26_31               : 6;
	uint64_t lop_txb_ecc_flip_synd        : 2;  /**< Flip syndrome in LOP TXBUFF data memory. */
	uint64_t reserved_17_23               : 7;
	uint64_t lop_txb_ecc_dis              : 1;  /**< Disable ECC for LOP TXBUFF data memory. */
	uint64_t reserved_14_15               : 2;
	uint64_t lip_newq_ecc_flip_synd       : 2;  /**< Flip syndrome in LIP new queue. */
	uint64_t lip_pht_ecc_flip_synd        : 2;  /**< Flip syndrome in LIP packet header table. */
	uint64_t lip_gdt_ecc_flip_synd        : 2;  /**< Flip syndrome in LIP group definition table. */
	uint64_t lip_isf_ecc_flip_synd        : 2;  /**< Flip syndrome in LIP input skid FIFO. */
	uint64_t reserved_4_5                 : 2;
	uint64_t lip_newq_ecc_dis             : 1;  /**< Disable ECC for LIP new queue. */
	uint64_t lip_pht_ecc_dis              : 1;  /**< Disable ECC for LIP packet header table. */
	uint64_t lip_gdt_ecc_dis              : 1;  /**< Disable ECC for LIP group definition table. */
	uint64_t lip_isf_ecc_dis              : 1;  /**< Disable ECC for LIP input skid FIFO. */
#else
	uint64_t lip_isf_ecc_dis              : 1;
	uint64_t lip_gdt_ecc_dis              : 1;
	uint64_t lip_pht_ecc_dis              : 1;
	uint64_t lip_newq_ecc_dis             : 1;
	uint64_t reserved_4_5                 : 2;
	uint64_t lip_isf_ecc_flip_synd        : 2;
	uint64_t lip_gdt_ecc_flip_synd        : 2;
	uint64_t lip_pht_ecc_flip_synd        : 2;
	uint64_t lip_newq_ecc_flip_synd       : 2;
	uint64_t reserved_14_15               : 2;
	uint64_t lop_txb_ecc_dis              : 1;
	uint64_t reserved_17_23               : 7;
	uint64_t lop_txb_ecc_flip_synd        : 2;
	uint64_t reserved_26_31               : 6;
	uint64_t lue_kdb_ecc_dis              : 1;
	uint64_t lue_tat_ecc_dis              : 1;
	uint64_t lue_rft_ecc_dis              : 1;
	uint64_t lue_rul_ecc_dis              : 1;
	uint64_t lue_kdt_ecc_dis              : 1;
	uint64_t reserved_37_43               : 7;
	uint64_t lue_kdb_ecc_flip_synd        : 2;
	uint64_t lue_tat_ecc_flip_synd        : 2;
	uint64_t lue_rft_ecc_flip_synd        : 2;
	uint64_t lue_rul_ecc_flip_synd        : 2;
	uint64_t lue_kdt_ecc_flip_synd        : 2;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_ase_ecc_ctl_s             cn78xx;
	struct cvmx_ase_ecc_ctl_s             cn78xxp1;
};
typedef union cvmx_ase_ecc_ctl cvmx_ase_ecc_ctl_t;

/**
 * cvmx_ase_ecc_int
 *
 * This register contains the interrupt status for ECC failures. In all cases below, except
 * LUE_KDT_*, any request that generates an error has its response marked as errored. The
 * LUE_KDT_DBE error is not indicated in the response packet; the only indication of this error
 * is the interrupt mechanism.
 *
 * For all the LUE* errors below, additional information can be obtained by reading the
 * ASE_LUE_ERROR_LOG. For all the LIP* /LOP* errors below, additional information can be obtained
 * by reading ASE_LUF_ERROR_LOG.
 */
union cvmx_ase_ecc_int {
	uint64_t u64;
	struct cvmx_ase_ecc_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t lue_kdt_dbe                  : 1;  /**< Detected double-bit error on LUE RMC key data transfer buffer. */
	uint64_t lue_kdt_sbe                  : 1;  /**< Detected and corrected single-bit error on LUE RMC key data transfer buffer. */
	uint64_t lue_rul_dbe                  : 1;  /**< Detected double-bit error on LUE RMC buffer aligner wrapper rule FIFO. */
	uint64_t lue_rul_sbe                  : 1;  /**< Detected and corrected single-bit error on LUE RMC buffer aligner wrapper rule FIFO. */
	uint64_t reserved_38_39               : 2;
	uint64_t lue_rft_dbe                  : 1;  /**< Detected double-bit error on LUE HST and RMC rule format tables. This bit is not set for
                                                         software accesses to the RFT; it is only set for lookup accesses. */
	uint64_t lue_rft_sbe                  : 1;  /**< Detected and corrected single-bit error on LUE HST and RMC rule format tables. This bit is
                                                         not set for software accesses to the RFT; it only gets set for lookup accesses. */
	uint64_t lue_tat_dbe                  : 1;  /**< Detected double-bit error on LUE HST ruleDB access table. This bit is not set for software
                                                         accesses to the TAT; it only gets set for lookup accesses. It is expected that error
                                                         recovery will
                                                         require resetting the ASE and loading corrected software into the TAT. */
	uint64_t lue_tat_sbe                  : 1;  /**< Detected and corrected single-bit error on LUE HST ruleDB access table. This bit is not
                                                         set for software accesses to the TAT; it only gets set for lookup accesses. */
	uint64_t lue_kdb_dbe                  : 1;  /**< Detected double-bit error on LUE KRQ key data buffer. */
	uint64_t lue_kdb_sbe                  : 1;  /**< Detected and corrected single-bit error on LUE KRQ key data buffer. */
	uint64_t reserved_18_31               : 14;
	uint64_t lop_txb_dbe                  : 1;  /**< Detected double-bit error on LOP's TXBUFF RAM read operation. */
	uint64_t lop_txb_sbe                  : 1;  /**< Detected and corrected single-bit error on LOP's TXBUFF RAM read operation. */
	uint64_t reserved_8_15                : 8;
	uint64_t lip_newq_dbe                 : 1;  /**< Detected double-bit error on LIP's new queue. */
	uint64_t lip_newq_sbe                 : 1;  /**< Detected and corrected single-bit error on LIP's new queue. */
	uint64_t lip_pht_dbe                  : 1;  /**< Detected double-bit error on LIP's packet header table. This bit is not set for software
                                                         accesses to the GDT; it only gets set for lookup accesses. */
	uint64_t lip_pht_sbe                  : 1;  /**< Detected and corrected single-bit error on LIP's packet header table. This bit is not set
                                                         for software accesses to the GDT; it only gets set for lookup accesses. */
	uint64_t lip_gdt_dbe                  : 1;  /**< Detected double-bit error on LIP's group definition table. This bit is not set for
                                                         software accesses to the GDT; it only gets set for lookup accesses. */
	uint64_t lip_gdt_sbe                  : 1;  /**< Detected and corrected single-bit error on LIP's group definition table. This bit is not
                                                         set for software accesses to the GDT; it only gets set for lookup accesses. */
	uint64_t lip_isf_dbe                  : 1;  /**< Detected double-bit error on LIP's input skid FIFO. */
	uint64_t lip_isf_sbe                  : 1;  /**< Detected and corrected single-bit error on LIP's input skid FIFO. */
#else
	uint64_t lip_isf_sbe                  : 1;
	uint64_t lip_isf_dbe                  : 1;
	uint64_t lip_gdt_sbe                  : 1;
	uint64_t lip_gdt_dbe                  : 1;
	uint64_t lip_pht_sbe                  : 1;
	uint64_t lip_pht_dbe                  : 1;
	uint64_t lip_newq_sbe                 : 1;
	uint64_t lip_newq_dbe                 : 1;
	uint64_t reserved_8_15                : 8;
	uint64_t lop_txb_sbe                  : 1;
	uint64_t lop_txb_dbe                  : 1;
	uint64_t reserved_18_31               : 14;
	uint64_t lue_kdb_sbe                  : 1;
	uint64_t lue_kdb_dbe                  : 1;
	uint64_t lue_tat_sbe                  : 1;
	uint64_t lue_tat_dbe                  : 1;
	uint64_t lue_rft_sbe                  : 1;
	uint64_t lue_rft_dbe                  : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t lue_rul_sbe                  : 1;
	uint64_t lue_rul_dbe                  : 1;
	uint64_t lue_kdt_sbe                  : 1;
	uint64_t lue_kdt_dbe                  : 1;
	uint64_t reserved_44_63               : 20;
#endif
	} s;
	struct cvmx_ase_ecc_int_s             cn78xx;
	struct cvmx_ase_ecc_int_s             cn78xxp1;
};
typedef union cvmx_ase_ecc_int cvmx_ase_ecc_int_t;

/**
 * cvmx_ase_gen_int
 *
 * This register contains the interrupt status for general ASE interrupts. Errors reported in bit
 * positions <39:32>, <7:2>, and <0> are most likely due to software programming errors.
 *
 * In all LUE* cases below, any request that generates an error has its response marked as
 * errored. These LUE* interrupts are for diagnostic use, not for error handling. For all the
 * LUE* errors below, additional information can be obtained by reading ASE_LUE_ERROR_LOG.
 */
union cvmx_ase_gen_int {
	uint64_t u64;
	struct cvmx_ase_gen_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t lue_rme_fatal                : 1;  /**< One or more of the lookup engines detected a fatal error. It is expected that error
                                                         recovery will require resetting the ASE and loading corrected software into OSM. */
	uint64_t lue_invalid_req              : 1;  /**< Insufficient key data was provided for a new lookup request. It is expected that error
                                                         recovery will require resetting the ASE and loading corrected software into OSM. */
	uint64_t lue_hr_err_log               : 1;  /**< An error occurred for a host request and generated a host response with error. */
	uint64_t reserved_35_36               : 2;
	uint64_t lue_tic_bad_write            : 1;  /**< A data load to the TIC was prevented that would have caused a wrap condition. Either the
                                                         TAT row pointed
                                                         to by the TIC entry was invalid, or the starting TAT row and the increment value pointed
                                                         beyond the end of the TAT. It is expected that error recovery will require loading
                                                         correct software into the TIC. */
	uint64_t lue_tic_multi_hit            : 1;  /**< A TIC lookup request resulted in multiple entries reporting a hit. It is expected that
                                                         error recovery will require resetting the ASE and loading corrected software into the TIC. */
	uint64_t lue_tic_miss                 : 1;  /**< A TIC lookup request did not match a valid entry. It is expected that error recovery will
                                                         require resetting the ASE and loading corrected software into the TIC. */
	uint64_t reserved_8_31                : 24;
	uint64_t lip_tbf_missing_eop          : 1;  /**< The incoming TBL command did not indicate EOP on the correct beat, or the incoming lookup
                                                         command did not indicate EOP before the 12th beat. The request will be marked FATAL. */
	uint64_t lip_tbf_early_eop            : 1;  /**< The incoming TBL write command did not have enough write data beats to match the command.
                                                         The write operation is marked FATAL. */
	uint64_t lip_obf_missing_eop          : 1;  /**< The incoming OSM command did not indicate EOP on the correct beat. The request is marked FATAL. */
	uint64_t lip_obf_early_eop            : 1;  /**< The incoming OSM write command did not have enough write data beats to match the command.
                                                         The write operation is marked FATAL. */
	uint64_t lip_obf_drop_unkn_cmd        : 1;  /**< The incoming control word at LIP OSM bypass splitter does not decode to a valid command.
                                                         The packet is dropped since we can't trust the command to figure out what kind of response
                                                         to send to LAP. We depend on LAP timeouts to inform software. */
	uint64_t lip_obf_drop_malformed       : 1;  /**< LIP OSM bypass splitter dropped a beat because it was expecting a start-of-packet beat and
                                                         didn't see the SOP indication, or it saw both SOP and EOP indication. */
	uint64_t lip_obf_drop_cmd_dbe         : 1;  /**< LIP OSM bypass splitter sees the incoming control word is marked as having a double-bit
                                                         error. The packet is dropped since we can't trust the LID to send even an error response
                                                         to LAP. We depend on LAP timeouts to inform software. */
	uint64_t lip_isf_drop_full            : 1;  /**< LIP input skid FIFO dropped a beat because it was full. This only happens if LAP issues
                                                         request beats but has no ase__lap1_credit<0>s to do so; this indicates LAP credits are
                                                         misprogrammed. If this interrupt fires, the software has to reset LAP1 and ASE to recover,
                                                         as the credits are out of sync. */
#else
	uint64_t lip_isf_drop_full            : 1;
	uint64_t lip_obf_drop_cmd_dbe         : 1;
	uint64_t lip_obf_drop_malformed       : 1;
	uint64_t lip_obf_drop_unkn_cmd        : 1;
	uint64_t lip_obf_early_eop            : 1;
	uint64_t lip_obf_missing_eop          : 1;
	uint64_t lip_tbf_early_eop            : 1;
	uint64_t lip_tbf_missing_eop          : 1;
	uint64_t reserved_8_31                : 24;
	uint64_t lue_tic_miss                 : 1;
	uint64_t lue_tic_multi_hit            : 1;
	uint64_t lue_tic_bad_write            : 1;
	uint64_t reserved_35_36               : 2;
	uint64_t lue_hr_err_log               : 1;
	uint64_t lue_invalid_req              : 1;
	uint64_t lue_rme_fatal                : 1;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_ase_gen_int_s             cn78xx;
	struct cvmx_ase_gen_int_s             cn78xxp1;
};
typedef union cvmx_ase_gen_int cvmx_ase_gen_int_t;

/**
 * cvmx_ase_lip_config
 *
 * This register provides configuration for the LIP.
 *
 */
union cvmx_ase_lip_config {
	uint64_t u64;
	struct cvmx_ase_lip_config_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t drop_xoff_en                 : 1;  /**< This feature should remain disabled. */
	uint64_t gen_xon_en                   : 1;  /**< If enabled, the LIP generates XON indication to the LAP when lookup requests are
                                                         backpressured. If disabled, the LIP does not assert XON. */
	uint64_t reserved_1_1                 : 1;
	uint64_t hst_osm_hw_ecc_bypass        : 1;  /**< If enabled, host accesses to the OSM memory bypass hardware ECC generation and
                                                         calculation/correction/detection. If disabled, host accesses use hardware ECC generation
                                                         and calculation/correction/detection. */
#else
	uint64_t hst_osm_hw_ecc_bypass        : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t gen_xon_en                   : 1;
	uint64_t drop_xoff_en                 : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_ase_lip_config_s          cn78xx;
	struct cvmx_ase_lip_config_s          cn78xxp1;
};
typedef union cvmx_ase_lip_config cvmx_ase_lip_config_t;

/**
 * cvmx_ase_lip_spare
 *
 * Spare.
 *
 */
union cvmx_ase_lip_spare {
	uint64_t u64;
	struct cvmx_ase_lip_spare_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_ase_lip_spare_s           cn78xx;
	struct cvmx_ase_lip_spare_s           cn78xxp1;
};
typedef union cvmx_ase_lip_spare cvmx_ase_lip_spare_t;

/**
 * cvmx_ase_lop_config
 *
 * This register provides configuration for the LOP.
 *
 */
union cvmx_ase_lop_config {
	uint64_t u64;
	struct cvmx_ase_lop_config_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t rsp_pri                      : 4;  /**< Response output priority as follows:
                                                         <7> = LUE key response.
                                                         <6> = LUF table response.
                                                         <5> = LUE table response.
                                                         <4> = OSM write/read response.
                                                         There are two priority levels per response type:
                                                         0 = Lower priority. Round robin is used among the responses with higher priority to send
                                                         back to the LAP or CSR. When there is no response with higher priority left, round robin
                                                         is used to choose a response with lower priority to send back to LAP or CSR.
                                                         1 = Higher priority. */
	uint64_t reserved_1_3                 : 3;
	uint64_t rsp_dis                      : 1;  /**< If set, the LOP does not send response(s) to the LAP/CSR. It is only used for diagnosis
                                                         purposes. For example, it can be used to build up backpressure to LUE/LIP/LAP/OSM. In
                                                         normal operation, it must not be set. */
#else
	uint64_t rsp_dis                      : 1;
	uint64_t reserved_1_3                 : 3;
	uint64_t rsp_pri                      : 4;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_ase_lop_config_s          cn78xx;
	struct cvmx_ase_lop_config_s          cn78xxp1;
};
typedef union cvmx_ase_lop_config cvmx_ase_lop_config_t;

/**
 * cvmx_ase_lop_spare
 *
 * Spare.
 *
 */
union cvmx_ase_lop_spare {
	uint64_t u64;
	struct cvmx_ase_lop_spare_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_ase_lop_spare_s           cn78xx;
	struct cvmx_ase_lop_spare_s           cn78xxp1;
};
typedef union cvmx_ase_lop_spare cvmx_ase_lop_spare_t;

/**
 * cvmx_ase_lue_config
 *
 * This register provides configuration for the LUE.
 *
 */
union cvmx_ase_lue_config {
	uint64_t u64;
	struct cvmx_ase_lue_config_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_34_63               : 30;
	uint64_t pfcache_en                   : 1;  /**< Enable bucket entry PFLEN caching. When clear, if PFLEN < OSM-line, PFLEN BE caching is
                                                         disabled, and a BEREQ is made for each PFLEN group of BEs processed. When set, if PFLEN <
                                                         OSM-line, only a single BEREQ is made for the OSM-line which caches all of the BEs. */
	uint64_t pfab_en                      : 1;  /**< Enable bucket entry prefetch phase A/B request scheme. When clear, each bucket walk engine
                                                         is allowed to have a maximum of 8 outstanding rule read requests in progress at a time.
                                                         When set, each bucket walk engine is allowed to have a maximum of 16 outstanding rule read
                                                         requests in progress at a time, split into two groups of 8 (Phases A and B). After the
                                                         initial 8 bucket entries, the next set of [up to] 8 bucket entries are speculatively read
                                                         and submitted to the rule walk engine. Subsequent speculative read operations are
                                                         performed once all outstanding requests for a phase have completed. */
	uint64_t reserved_20_31               : 12;
	uint64_t twc_strspsta_rr              : 1;  /**< Within the TWC block, configures the arbiter which selects between pending TWE or BWE
                                                         STRSPs. When clear, fixed priority arbitration is selected, which gives BWEs higher
                                                         priority over TWEs. When set, round robin arbitration is selected which ensures fairness
                                                         across the TWE and BWE STRSPs. */
	uint64_t tta_req_rr                   : 1;  /**< Within the TTA blocks, configures the arbiter which selects between host access requests
                                                         and lookup requests. When configured for fixed priority, host accesses have higher
                                                         priority. If enabled, use round robin. If disabled, use fixed priority. */
	uint64_t rft_req_rr                   : 1;  /**< Within the RFT access logic, configures the arbiter which selects between host access
                                                         requests and lookup requests. When configured for fixed priority, host accesses have
                                                         higher priority. If enabled, use round-robin. If disabled, use fixed priority. */
	uint64_t reserved_4_16                : 13;
	uint64_t rme_enable                   : 4;  /**< Each bit, when set, enables rule processing by a local rule match engine. */
#else
	uint64_t rme_enable                   : 4;
	uint64_t reserved_4_16                : 13;
	uint64_t rft_req_rr                   : 1;
	uint64_t tta_req_rr                   : 1;
	uint64_t twc_strspsta_rr              : 1;
	uint64_t reserved_20_31               : 12;
	uint64_t pfab_en                      : 1;
	uint64_t pfcache_en                   : 1;
	uint64_t reserved_34_63               : 30;
#endif
	} s;
	struct cvmx_ase_lue_config_s          cn78xx;
	struct cvmx_ase_lue_config_s          cn78xxp1;
};
typedef union cvmx_ase_lue_config cvmx_ase_lue_config_t;

/**
 * cvmx_ase_lue_dbg_ctl0
 *
 * We are not rewiring the NSP's 16-bit debug bus. Instead we are duplicating that mux 4 times to
 * give CNXXXX better observability.
 * LUE DBGCTX is a DOR daisy-chained through the TWE and BWE engines, it can't be moved to a
 * straight CNXXXX debug bus without rewriting the whole thing.
 * This register selects engines for debug observations for the LUE's four 16-bit debug muxes and
 * selects context for observation.
 */
union cvmx_ase_lue_dbg_ctl0 {
	uint64_t u64;
	struct cvmx_ase_lue_dbg_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_63_63               : 1;
	uint64_t sel3                         : 7;  /**< Debug select for LUE's mux 3. */
	uint64_t reserved_55_55               : 1;
	uint64_t sel2                         : 7;  /**< Debug select for LUE's mux 2. */
	uint64_t reserved_47_47               : 1;
	uint64_t sel1                         : 7;  /**< Debug select for LUE's mux 1. */
	uint64_t reserved_39_39               : 1;
	uint64_t sel0                         : 7;  /**< Debug select for LUE's mux 0. */
	uint64_t reserved_20_31               : 12;
	uint64_t ctx_col_dbg                  : 4;  /**< Context column debug. 32-bit column of context information to display in the ASE_LUE_CTX
                                                         debug field.
                                                         _ TWE: Valid column values 0-12.
                                                         _ BWE: Valid column values 0-2.
                                                         _ RWE: Valid column values 0-8. */
	uint64_t reserved_13_15               : 3;
	uint64_t ctx_eng_dbg                  : 5;  /**< Engine ID from which context information will be made available in the ASE_LUE_CTX debug
                                                         field. Must be 0 to 19. */
	uint64_t reserved_2_7                 : 6;
	uint64_t ctx_src_dbg                  : 2;  /**< Engine type from which context information will be made available in the ASE_LUE_CTX debug
                                                         field.
                                                         0 = Tree walk engine.
                                                         1 = Bucket walk engine.
                                                         2 = Rule walk engine.
                                                         3 = Reserved. */
#else
	uint64_t ctx_src_dbg                  : 2;
	uint64_t reserved_2_7                 : 6;
	uint64_t ctx_eng_dbg                  : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t ctx_col_dbg                  : 4;
	uint64_t reserved_20_31               : 12;
	uint64_t sel0                         : 7;
	uint64_t reserved_39_39               : 1;
	uint64_t sel1                         : 7;
	uint64_t reserved_47_47               : 1;
	uint64_t sel2                         : 7;
	uint64_t reserved_55_55               : 1;
	uint64_t sel3                         : 7;
	uint64_t reserved_63_63               : 1;
#endif
	} s;
	struct cvmx_ase_lue_dbg_ctl0_s        cn78xx;
	struct cvmx_ase_lue_dbg_ctl0_s        cn78xxp1;
};
typedef union cvmx_ase_lue_dbg_ctl0 cvmx_ase_lue_dbg_ctl0_t;

/**
 * cvmx_ase_lue_dbg_ctl1
 *
 * The per-engine filtering from NSP is not really worth moving to DTX-style addressing.
 * We are not rewiring the NSP's 16-bit debug bus. Instead we are duplicating that mux 4 times to
 * give CNXXXX better observability.
 * This register selects engines for debug observations for the LUE's four 16-bit debug muxes.
 */
union cvmx_ase_lue_dbg_ctl1 {
	uint64_t u64;
	struct cvmx_ase_lue_dbg_ctl1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t rmc_dbg3                     : 2;  /**< Mux 3. Selects which of the four RMC modules will present signals on the debug bus when a
                                                         RMC signal group is selected for observation. */
	uint64_t reserved_58_59               : 2;
	uint64_t rmc_dbg2                     : 2;  /**< Mux 2. Selects which of the four RMC modules will present signals on the debug bus when a
                                                         RMC signal group is selected for observation. */
	uint64_t reserved_54_55               : 2;
	uint64_t rmc_dbg1                     : 2;  /**< Mux 1. Selects which of the four RMC modules will present signals on the debug bus when a
                                                         RMC signal group is selected for observation. */
	uint64_t reserved_50_51               : 2;
	uint64_t rmc_dbg0                     : 2;  /**< Mux 0. Selects which of the four RMC modules will present signals on the debug bus when a
                                                         RMC signal group is selected for observation. */
	uint64_t reserved_29_47               : 19;
	uint64_t eng_id_dbg3                  : 5;  /**< Mux 3. Selects which tree walk engine or bucket walk engine will present signals on the
                                                         debug bus when a TWC or BWC signal group is selected for observation. Valid values are
                                                         0-19. */
	uint64_t reserved_21_23               : 3;
	uint64_t eng_id_dbg2                  : 5;  /**< Mux 2. Selects which tree walk engine or bucket walk engine will present signals on the
                                                         debug bus when a TWC or BWC signal group is selected for observation. Valid values are
                                                         0-19. */
	uint64_t reserved_13_15               : 3;
	uint64_t eng_id_dbg1                  : 5;  /**< Mux 1. Selects which tree walk engine or bucket walk engine will present signals on the
                                                         debug bus when a TWC or BWC signal group is selected for observation. Valid values are
                                                         0-19. */
	uint64_t reserved_5_7                 : 3;
	uint64_t eng_id_dbg0                  : 5;  /**< Mux 0. Selects which tree walk engine or bucket walk engine will present signals on the
                                                         debug bus when a TWC or BWC signal group is selected for observation. Valid values are
                                                         0-19. */
#else
	uint64_t eng_id_dbg0                  : 5;
	uint64_t reserved_5_7                 : 3;
	uint64_t eng_id_dbg1                  : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t eng_id_dbg2                  : 5;
	uint64_t reserved_21_23               : 3;
	uint64_t eng_id_dbg3                  : 5;
	uint64_t reserved_29_47               : 19;
	uint64_t rmc_dbg0                     : 2;
	uint64_t reserved_50_51               : 2;
	uint64_t rmc_dbg1                     : 2;
	uint64_t reserved_54_55               : 2;
	uint64_t rmc_dbg2                     : 2;
	uint64_t reserved_58_59               : 2;
	uint64_t rmc_dbg3                     : 2;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_ase_lue_dbg_ctl1_s        cn78xx;
	struct cvmx_ase_lue_dbg_ctl1_s        cn78xxp1;
};
typedef union cvmx_ase_lue_dbg_ctl1 cvmx_ase_lue_dbg_ctl1_t;

/**
 * cvmx_ase_lue_error_log
 *
 * This register logs information to help diagnose LUE errors indicated in ASE_*_INT[LUE*].
 * Information is only logged to this register if the bit corresponding to the ERROR_ID is set in
 * ASE_LUE_ERROR_LOG_ENABLE. The contents of this register are invalid if no fields are set in
 * ASE_*_INT[LUE*]. The contents of this register are retained until all the bits in the
 * ASE_*_INT[LUE*] are cleared, or an error occurs that is of higher-priority than the error for
 * which information is currently logged by this CSR.
 *
 * The priority of the error is encoded by the enumerated values in ASE_LUE_ERROR_ID_E. The
 * highest priority error is KDT_DBE, the lowest is RFT_SBE. For RFT errors, if multiple errors
 * of equal weight are reported during a clock cycle, the error on the local RFT is reported with
 * highest priority, followed by RMC0, RMC1, RMC2, RMC3. Only interrupts listed in
 * ASE_LUE_ERROR_ID_E log errors.
 */
union cvmx_ase_lue_error_log {
	uint64_t u64;
	struct cvmx_ase_lue_error_log_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t error_id                     : 6;  /**< Type of error logged. See ASE_LUE_ERROR_ID_E. If [ERROR_ID] = HR_ERR_LOG, see
                                                         [HR_ERR_ID] for how to decode [DATA]. Otherwise, [ERROR_ID] indicates how to
                                                         decode [DATA]. */
	uint64_t reserved_54_55               : 2;
	uint64_t hr_err_id                    : 6;  /**< Type of host error logged. See ASE_LUE_ERROR_ID_E. Ignore HR_ERR_ID if ERROR_ID !=
                                                         HR_ERR_LOG. Indicates how to decode [DATA] if [ERROR_ID] == HR_ERR_LOG. */
	uint64_t data                         : 48; /**< Error logging information. The information in this field takes on different meanings
                                                         depending on the type of error that is latched in the ASE_*_INT[LUE*] fields. Decode this
                                                         field based on ERROR_ID and HR_ERR_ID:
                                                         _ TIC_MISS or TIC_MULTI_HIT, see ASE_LUE_ERROR_LOG_TIC_S.
                                                         _ TIC_BAD_WRITE, see ASE_LUE_ERROR_LOG_TIC_BAD_WRITE_S.
                                                         _ INVALID_TBL_ACC, see ASE_LUE_ERROR_LOG_INVTBLACC_S.
                                                         _ INVALID_REQ, see ASE_LUE_ERROR_LOG_INVREQ_S.
                                                         _ RME_FATAL, see ASE_LUE_ERROR_LOG_RME_FATAL_S.
                                                         _ KDB_*BE, see ASE_LUE_ERROR_LOG_KDB_ECC_S.
                                                         _ TAT_*BE, see ASE_LUE_ERROR_LOG_TAT_ECC_S.
                                                         _ RFT_*BE: see ASE_LUE_ERROR_LOG_RFT_ECC_S.
                                                         _ RUL_*BE, see ASE_LUE_ERROR_LOG_RUL_ECC_S.
                                                         _ KDT_*BE, see ASE_LUE_ERROR_LOG_KDT_ECC_S. */
#else
	uint64_t data                         : 48;
	uint64_t hr_err_id                    : 6;
	uint64_t reserved_54_55               : 2;
	uint64_t error_id                     : 6;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_ase_lue_error_log_s       cn78xx;
	struct cvmx_ase_lue_error_log_s       cn78xxp1;
};
typedef union cvmx_ase_lue_error_log cvmx_ase_lue_error_log_t;

/**
 * cvmx_ase_lue_error_log_enable
 *
 * Each field in this register, when set, allows the corresponding field in the ASE_*_INT[LUE*]
 * to log information in ASE_LUE_ERROR_LOG upon assertion.
 */
union cvmx_ase_lue_error_log_enable {
	uint64_t u64;
	struct cvmx_ase_lue_error_log_enable_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t kdt_dbe                      : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::KDT_DBE errors. */
	uint64_t reserved_52_52               : 1;
	uint64_t rul_dbe                      : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::RUL_DBE errors. */
	uint64_t rft_dbe                      : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::RFT_DBE errors. */
	uint64_t tat_dbe                      : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::TAT_DBE errors. */
	uint64_t kdb_dbe                      : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::KDB_DBE errors. */
	uint64_t reserved_37_47               : 11;
	uint64_t kdt_sbe                      : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::KDT_SBE errors. */
	uint64_t rul_sbe                      : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::RUL_SBE errors. */
	uint64_t rft_sbe                      : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::RFT_SBE errors. */
	uint64_t tat_sbe                      : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::TAT_SBE errors. */
	uint64_t kdb_sbe                      : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::KDB_SBE errors. */
	uint64_t reserved_8_31                : 24;
	uint64_t rme_fatal                    : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::RME_FATAL errors. */
	uint64_t invalid_req                  : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::INVALID_REQ errors. */
	uint64_t hr_err_log                   : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::HR_ERR_LOG errors. */
	uint64_t reserved_3_4                 : 2;
	uint64_t tic_bad_write                : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::TIC_BAD_WRITE errors. */
	uint64_t tic_multi_hit                : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::TIC_MULTI_HIT errors. */
	uint64_t tic_miss                     : 1;  /**< Enables logging for ASE_LUE_ERROR_ID_E::TIC_MISS errors. */
#else
	uint64_t tic_miss                     : 1;
	uint64_t tic_multi_hit                : 1;
	uint64_t tic_bad_write                : 1;
	uint64_t reserved_3_4                 : 2;
	uint64_t hr_err_log                   : 1;
	uint64_t invalid_req                  : 1;
	uint64_t rme_fatal                    : 1;
	uint64_t reserved_8_31                : 24;
	uint64_t kdb_sbe                      : 1;
	uint64_t tat_sbe                      : 1;
	uint64_t rft_sbe                      : 1;
	uint64_t rul_sbe                      : 1;
	uint64_t kdt_sbe                      : 1;
	uint64_t reserved_37_47               : 11;
	uint64_t kdb_dbe                      : 1;
	uint64_t tat_dbe                      : 1;
	uint64_t rft_dbe                      : 1;
	uint64_t rul_dbe                      : 1;
	uint64_t reserved_52_52               : 1;
	uint64_t kdt_dbe                      : 1;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_ase_lue_error_log_enable_s cn78xx;
	struct cvmx_ase_lue_error_log_enable_s cn78xxp1;
};
typedef union cvmx_ase_lue_error_log_enable cvmx_ase_lue_error_log_enable_t;

/**
 * cvmx_ase_lue_perf_filt
 *
 * This register contains filters for performance counter events.
 *
 */
union cvmx_ase_lue_perf_filt {
	uint64_t u64;
	struct cvmx_ase_lue_perf_filt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_49_63               : 15;
	uint64_t hst_tta_tic_id               : 9;  /**< Lookups to the TTA in the HST module will trigger a performance event if the lookup is for
                                                         the TIC ID value indicated in this field. See ASE_TBLDAT_TIC_S for description of TIC_ID. */
	uint64_t reserved_38_39               : 2;
	uint64_t hst_rft_kftidx               : 6;  /**< Lookups to the RFT in the HST module will trigger a performance event if the lookup is for
                                                         the KFTIDX value indicated in this field. */
	uint64_t reserved_26_31               : 6;
	uint64_t sel_all_perf                 : 1;  /**< Disable filtering. When set, overrides the setting of SEL_ID_PERF for some performance
                                                         counter events. This field is used by the TWC, BWC, and STR modules. */
	uint64_t sel_id_perf                  : 1;  /**< Selects how the value in the ENG_KID_ID_PERF field is interpreted. This field is used by
                                                         the TWC, BWC, and STR modules. */
	uint64_t reserved_22_23               : 2;
	uint64_t eng_kid_id_perf              : 6;  /**< When SEL_ID_PERF is clear, this field is interpreted as an 8-bit KID. When SEL_ID_PERF is
                                                         set, the lower five bits of this field are interpreted as the Engine ID and the upper
                                                         three bits are ignored, and valid values are 0-19. This field is used by the TWC, BWC, and
                                                         STR modules. */
	uint64_t reserved_14_15               : 2;
	uint64_t rmc_perf                     : 2;  /**< RMC to monitor for performance events. */
	uint64_t reserved_11_11               : 1;
	uint64_t perf_bwc_eng                 : 5;  /**< BWE to monitor for performance events. Valid values are 0-19. */
	uint64_t reserved_5_5                 : 1;
	uint64_t perf_twc_eng                 : 5;  /**< TWE to monitor for performance events. Valid values are 0-19. */
#else
	uint64_t perf_twc_eng                 : 5;
	uint64_t reserved_5_5                 : 1;
	uint64_t perf_bwc_eng                 : 5;
	uint64_t reserved_11_11               : 1;
	uint64_t rmc_perf                     : 2;
	uint64_t reserved_14_15               : 2;
	uint64_t eng_kid_id_perf              : 6;
	uint64_t reserved_22_23               : 2;
	uint64_t sel_id_perf                  : 1;
	uint64_t sel_all_perf                 : 1;
	uint64_t reserved_26_31               : 6;
	uint64_t hst_rft_kftidx               : 6;
	uint64_t reserved_38_39               : 2;
	uint64_t hst_tta_tic_id               : 9;
	uint64_t reserved_49_63               : 15;
#endif
	} s;
	struct cvmx_ase_lue_perf_filt_s       cn78xx;
	struct cvmx_ase_lue_perf_filt_s       cn78xxp1;
};
typedef union cvmx_ase_lue_perf_filt cvmx_ase_lue_perf_filt_t;

/**
 * cvmx_ase_lue_performance_control#
 *
 * A write operation to LUE_PERFORMANCE_CONTROL*, which sets the ENABLE field to 0x1
 * must not change the values of any other fields in the CSR.
 */
union cvmx_ase_lue_performance_controlx {
	uint64_t u64;
	struct cvmx_ase_lue_performance_controlx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t frozen                       : 1;  /**< Indicates that the counter is frozen (i.e one shot event occurred) and remains frozen
                                                         until the clear bit written. */
	uint64_t clear                        : 1;  /**< Writing 1 to this bit generates a hardware pulse that clears the LUE_PERFORMANCE_COUNTER
                                                         and [FROZEN]. */
	uint64_t enable                       : 1;  /**< Enable the counter. This bit is set to 1 to use the corresponding counter. */
	uint64_t reserved_27_28               : 2;
	uint64_t mode                         : 3;  /**< Performance counter mode.
                                                         Bit<24>:
                                                         1 = Event counted [SEL0] | [SEL1[ | [SEL2].
                                                         0 = Event counted [SEL0] & [SEL1] & [SEL2].
                                                         Bits<26:25>:
                                                         0x0 = Positive edge.
                                                         0x1 = Negative edge.
                                                         0x2 = Level.
                                                         0x3 = One shot. */
	uint64_t sel2                         : 8;  /**< Performance counter event select, third mux. */
	uint64_t sel1                         : 8;  /**< Performance counter event select, second mux. */
	uint64_t sel0                         : 8;  /**< Performance counter event select, first mux. */
#else
	uint64_t sel0                         : 8;
	uint64_t sel1                         : 8;
	uint64_t sel2                         : 8;
	uint64_t mode                         : 3;
	uint64_t reserved_27_28               : 2;
	uint64_t enable                       : 1;
	uint64_t clear                        : 1;
	uint64_t frozen                       : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_ase_lue_performance_controlx_s cn78xx;
	struct cvmx_ase_lue_performance_controlx_s cn78xxp1;
};
typedef union cvmx_ase_lue_performance_controlx cvmx_ase_lue_performance_controlx_t;

/**
 * cvmx_ase_lue_performance_control0
 *
 * A write operation to LUE_PERFORMANCE_CONTROL*, which sets the ENABLE field to 0x1
 * must not change the values of any other fields in the CSR.
 */
union cvmx_ase_lue_performance_control0 {
	uint64_t u64;
	struct cvmx_ase_lue_performance_control0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t frozen                       : 1;  /**< Indicates that the counter is frozen (i.e one shot event occurred) and remains frozen
                                                         until the clear bit written. */
	uint64_t clear                        : 1;  /**< Writing 1 to this bit generates a hardware pulse that clears the LUE_PERFORMANCE_COUNTER
                                                         and field FROZEN of this register. */
	uint64_t enable                       : 1;  /**< Enable the counter. This bit is set to 1 to use the corresponding counter. */
	uint64_t global_stop                  : 1;  /**< Writing a 1 to this bit stops all the counters in the group of eight counters. This bit is
                                                         only implemented in the first control register of a counter group. */
	uint64_t reserved_27_27               : 1;
	uint64_t mode                         : 3;  /**< Performance counter mode.
                                                         Bit<24>:
                                                         1 = Event counted [SEL0].
                                                         0 = Event counted [SEL0] & [SEL1] & [SEL2].
                                                         Bits<26:25>:
                                                         0x0 = Pos edge.
                                                         0x1 = Neg edge.
                                                         0x2 = Level.
                                                         0x3 = One shot. */
	uint64_t sel2                         : 8;  /**< Performance counter event select, third mux. */
	uint64_t sel1                         : 8;  /**< Performance counter event select, second mux. */
	uint64_t sel0                         : 8;  /**< Performance counter event select, first mux. */
#else
	uint64_t sel0                         : 8;
	uint64_t sel1                         : 8;
	uint64_t sel2                         : 8;
	uint64_t mode                         : 3;
	uint64_t reserved_27_27               : 1;
	uint64_t global_stop                  : 1;
	uint64_t enable                       : 1;
	uint64_t clear                        : 1;
	uint64_t frozen                       : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_ase_lue_performance_control0_s cn78xx;
	struct cvmx_ase_lue_performance_control0_s cn78xxp1;
};
typedef union cvmx_ase_lue_performance_control0 cvmx_ase_lue_performance_control0_t;

/**
 * cvmx_ase_lue_performance_control1
 *
 * A write operation to LUE_PERFORMANCE_CONTROL*, which sets the ENABLE or
 * GLOBAL_ENABLE fields to 0x1 must not change the values of any other fields in the
 * CSR.
 */
union cvmx_ase_lue_performance_control1 {
	uint64_t u64;
	struct cvmx_ase_lue_performance_control1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t frozen                       : 1;  /**< Indicates that the counter is frozen (i.e one shot event occurred) and remains frozen
                                                         until the clear bit written. */
	uint64_t clear                        : 1;  /**< Writing 1 to this bit generates a hardware pulse that clears the LUE_PERFORMANCE_COUNTER
                                                         and field FROZEN of this register. */
	uint64_t enable                       : 1;  /**< Enable the counter. This bit is set to 1 to use the corresponding counter. */
	uint64_t global_enable                : 1;  /**< Writing a 1 to this bit starts all the counters in the group of eight counters. This bit
                                                         is only implemented in the second control register of a counter group. Counters are
                                                         enabled by the OR of the individual [ENABLE]s and [GLOBAL_ENABLE]. */
	uint64_t reserved_27_27               : 1;
	uint64_t mode                         : 3;  /**< Performance counter mode.
                                                         Bit<24>:
                                                         1 = Event counted [SEL0] | [SEL1] | [SEL2].
                                                         0 = Event counted [SEL0] & [SEL1] & [SEL2].
                                                         Bits<26:25>:
                                                         0x0 = Positive edge.
                                                         0x1 = Negative edge.
                                                         0x2 = Level.
                                                         0x3 = One shot. */
	uint64_t sel2                         : 8;  /**< Performance counter event select, third mux. */
	uint64_t sel1                         : 8;  /**< Performance counter event select, second mux. */
	uint64_t sel0                         : 8;  /**< Performance counter event select, first mux. */
#else
	uint64_t sel0                         : 8;
	uint64_t sel1                         : 8;
	uint64_t sel2                         : 8;
	uint64_t mode                         : 3;
	uint64_t reserved_27_27               : 1;
	uint64_t global_enable                : 1;
	uint64_t enable                       : 1;
	uint64_t clear                        : 1;
	uint64_t frozen                       : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_ase_lue_performance_control1_s cn78xx;
	struct cvmx_ase_lue_performance_control1_s cn78xxp1;
};
typedef union cvmx_ase_lue_performance_control1 cvmx_ase_lue_performance_control1_t;

/**
 * cvmx_ase_lue_performance_counter#
 */
union cvmx_ase_lue_performance_counterx {
	uint64_t u64;
	struct cvmx_ase_lue_performance_counterx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t perf_cnt                     : 48; /**< Reflect the value of the performance counter. See ASE_LUE_PERFORMANCE_CONTROL* registers. */
#else
	uint64_t perf_cnt                     : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_ase_lue_performance_counterx_s cn78xx;
	struct cvmx_ase_lue_performance_counterx_s cn78xxp1;
};
typedef union cvmx_ase_lue_performance_counterx cvmx_ase_lue_performance_counterx_t;

/**
 * cvmx_ase_lue_spare
 *
 * Spare.
 *
 */
union cvmx_ase_lue_spare {
	uint64_t u64;
	struct cvmx_ase_lue_spare_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_ase_lue_spare_s           cn78xx;
	struct cvmx_ase_lue_spare_s           cn78xxp1;
};
typedef union cvmx_ase_lue_spare cvmx_ase_lue_spare_t;

/**
 * cvmx_ase_lue_twe_bwe_enable
 *
 * This register enables the tree/bucket walk engines.
 *
 */
union cvmx_ase_lue_twe_bwe_enable {
	uint64_t u64;
	struct cvmx_ase_lue_twe_bwe_enable_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_52_63               : 12;
	uint64_t bwe_en                       : 20; /**< Each bit enables a bucket walk engine to accept a thread graduation from a TWE. Meaning of
                                                         enable bits: [BWE19 ... BWE0]. */
	uint64_t reserved_20_31               : 12;
	uint64_t twe_en                       : 20; /**< Each bit enables a tree walk engine to accept new lookup requests. Meaning of enable bits:
                                                         [TWE19 ... TWE0]. */
#else
	uint64_t twe_en                       : 20;
	uint64_t reserved_20_31               : 12;
	uint64_t bwe_en                       : 20;
	uint64_t reserved_52_63               : 12;
#endif
	} s;
	struct cvmx_ase_lue_twe_bwe_enable_s  cn78xx;
	struct cvmx_ase_lue_twe_bwe_enable_s  cn78xxp1;
};
typedef union cvmx_ase_lue_twe_bwe_enable cvmx_ase_lue_twe_bwe_enable_t;

/**
 * cvmx_ase_luf_error_log
 *
 * The information logged in this register helps diagnose lookup front end (LUF) (look up
 * input/output pre/postprocessor) errors as indicated in ASE_*_INT[LIP* /LOP*]. The contents of
 * this CSR are invalid if no fields are set in ASE_*_INT[LIP* /LOP*]. The contents of this CSR
 * are retained until all the bits in the ASE_*_INT[LIP* /LOP*] are cleared, or an error occurs
 * that is of higher-priority than the error for which information is currently logged by this
 * CSR. The priority of the error is encoded by the enumerated values in ASE_LUF_ERROR_ID_E. Only
 * interrupts listed in ASE_LUF_ERROR_ID_E log errors.
 */
union cvmx_ase_luf_error_log {
	uint64_t u64;
	struct cvmx_ase_luf_error_log_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t error_id                     : 4;  /**< Type of error logged. See ASE_LUF_ERROR_ID_E. Also indicates how to decode the DATA field. */
	uint64_t data                         : 56; /**< Error logging information. The information in this field takes on different meanings
                                                         depending on the type of error that is latched in the ASE_*_INT[LIP* /LOP*] fields.
                                                         Decode this field based on ERROR_ID.
                                                         For LIP_*_*BE, see ASE_LUF_ERROR_LOG_LIP_ECC_S.
                                                         For LOP_TXB_*BE, see ASE_LUF_ERROR_LOG_LOP_ECC_S. */
#else
	uint64_t data                         : 56;
	uint64_t error_id                     : 4;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_ase_luf_error_log_s       cn78xx;
	struct cvmx_ase_luf_error_log_s       cn78xxp1;
};
typedef union cvmx_ase_luf_error_log cvmx_ase_luf_error_log_t;

/**
 * cvmx_ase_sft_rst
 *
 * This register allows soft reset.
 *
 */
union cvmx_ase_sft_rst {
	uint64_t u64;
	struct cvmx_ase_sft_rst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t busy                         : 1;  /**< When 1, ASE is busy completing reset. No access except the reading of this bit should
                                                         occur to the ASE until this is clear. */
	uint64_t reserved_1_62                : 62;
	uint64_t rst                          : 1;  /**< When set to 1 by software, ASE gets a short reset pulse (32 cycles in duration).
                                                         Everything but the RSL interface is reset (including CSRs). Hardware clears this bit when
                                                         the reset is complete. */
#else
	uint64_t rst                          : 1;
	uint64_t reserved_1_62                : 62;
	uint64_t busy                         : 1;
#endif
	} s;
	struct cvmx_ase_sft_rst_s             cn78xx;
	struct cvmx_ase_sft_rst_s             cn78xxp1;
};
typedef union cvmx_ase_sft_rst cvmx_ase_sft_rst_t;

/**
 * cvmx_ase_spare
 *
 * Spare.
 *
 */
union cvmx_ase_spare {
	uint64_t u64;
	struct cvmx_ase_spare_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_ase_spare_s               cn78xx;
	struct cvmx_ase_spare_s               cn78xxp1;
};
typedef union cvmx_ase_spare cvmx_ase_spare_t;

#endif
