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
 * cvmx-sli-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon sli.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_SLI_DEFS_H__
#define __CVMX_SLI_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_BIST_STATUS CVMX_SLI_BIST_STATUS_FUNC()
static inline uint64_t CVMX_SLI_BIST_STATUS_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000580ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000028580ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000000580ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028580ull;
			break;
	}
	cvmx_warn("CVMX_SLI_BIST_STATUS not supported on this chip\n");
	return 0x0000000000028580ull;
}
#else
#define CVMX_SLI_BIST_STATUS CVMX_SLI_BIST_STATUS_FUNC()
static inline uint64_t CVMX_SLI_BIST_STATUS_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000580ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000028580ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000580ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028580ull;
	}
	return 0x0000000000028580ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_CIU_INT_ENB CVMX_SLI_CIU_INT_ENB_FUNC()
static inline uint64_t CVMX_SLI_CIU_INT_ENB_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SLI_CIU_INT_ENB not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F0000027110ull);
}
#else
#define CVMX_SLI_CIU_INT_ENB (CVMX_ADD_IO_SEG(0x00011F0000027110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_CIU_INT_SUM CVMX_SLI_CIU_INT_SUM_FUNC()
static inline uint64_t CVMX_SLI_CIU_INT_SUM_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SLI_CIU_INT_SUM not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F0000027100ull);
}
#else
#define CVMX_SLI_CIU_INT_SUM (CVMX_ADD_IO_SEG(0x00011F0000027100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_CTL_PORTX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000050ull + ((offset) & 1) * 16;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000000000010050ull + ((offset) & 3) * 16;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return 0x0000000000000050ull + ((offset) & 3) * 16;
			break;

			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000000000286E0ull + ((offset) & 3) * 16;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000000000006E0ull + ((offset) & 3) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return 0x00000000000286E0ull + ((offset) & 3) * 16;
			break;
	}
	cvmx_warn("CVMX_SLI_CTL_PORTX (offset = %lu) not supported on this chip\n", offset);
	return 0x00000000000286E0ull + ((offset) & 3) * 16;
}
#else
static inline uint64_t CVMX_SLI_CTL_PORTX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000050ull + (offset) * 16;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000000000010050ull + (offset) * 16;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000050ull + (offset) * 16;

		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000000000286E0ull + (offset) * 16;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000000000006E0ull + (offset) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000286E0ull + (offset) * 16;
	}
	return 0x00000000000286E0ull + (offset) * 16;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_CTL_STATUS CVMX_SLI_CTL_STATUS_FUNC()
static inline uint64_t CVMX_SLI_CTL_STATUS_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000570ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000028570ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000000570ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028570ull;
			break;
	}
	cvmx_warn("CVMX_SLI_CTL_STATUS not supported on this chip\n");
	return 0x0000000000028570ull;
}
#else
#define CVMX_SLI_CTL_STATUS CVMX_SLI_CTL_STATUS_FUNC()
static inline uint64_t CVMX_SLI_CTL_STATUS_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000570ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000028570ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000570ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028570ull;
	}
	return 0x0000000000028570ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_DATA_OUT_CNT CVMX_SLI_DATA_OUT_CNT_FUNC()
static inline uint64_t CVMX_SLI_DATA_OUT_CNT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000005F0ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x00000000000285F0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x00000000000005F0ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000285F0ull;
			break;
	}
	cvmx_warn("CVMX_SLI_DATA_OUT_CNT not supported on this chip\n");
	return 0x00000000000285F0ull;
}
#else
#define CVMX_SLI_DATA_OUT_CNT CVMX_SLI_DATA_OUT_CNT_FUNC()
static inline uint64_t CVMX_SLI_DATA_OUT_CNT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000005F0ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000000000285F0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000000000005F0ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000285F0ull;
	}
	return 0x00000000000285F0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_DBG_DATA CVMX_SLI_DBG_DATA_FUNC()
static inline uint64_t CVMX_SLI_DBG_DATA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_DBG_DATA not supported on this chip\n");
	return 0x0000000000000310ull;
}
#else
#define CVMX_SLI_DBG_DATA (0x0000000000000310ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_DBG_SELECT CVMX_SLI_DBG_SELECT_FUNC()
static inline uint64_t CVMX_SLI_DBG_SELECT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_DBG_SELECT not supported on this chip\n");
	return 0x0000000000000300ull;
}
#else
#define CVMX_SLI_DBG_SELECT (0x0000000000000300ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_DMAX_CNT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000400ull + ((offset) & 1) * 16;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 1))
					return 0x0000000000028400ull + ((offset) & 1) * 16;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 1))
					return 0x0000000000000400ull + ((offset) & 1) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000028400ull + ((offset) & 1) * 16;
			break;
	}
	cvmx_warn("CVMX_SLI_DMAX_CNT (offset = %lu) not supported on this chip\n", offset);
	return 0x0000000000028400ull + ((offset) & 1) * 16;
}
#else
static inline uint64_t CVMX_SLI_DMAX_CNT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000400ull + (offset) * 16;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000028400ull + (offset) * 16;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000400ull + (offset) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028400ull + (offset) * 16;
	}
	return 0x0000000000028400ull + (offset) * 16;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_DMAX_INT_LEVEL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x00000000000003E0ull + ((offset) & 1) * 16;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 1))
					return 0x00000000000283E0ull + ((offset) & 1) * 16;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 1))
					return 0x00000000000003E0ull + ((offset) & 1) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x00000000000283E0ull + ((offset) & 1) * 16;
			break;
	}
	cvmx_warn("CVMX_SLI_DMAX_INT_LEVEL (offset = %lu) not supported on this chip\n", offset);
	return 0x00000000000283E0ull + ((offset) & 1) * 16;
}
#else
static inline uint64_t CVMX_SLI_DMAX_INT_LEVEL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000003E0ull + (offset) * 16;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000000000283E0ull + (offset) * 16;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000000000003E0ull + (offset) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000283E0ull + (offset) * 16;
	}
	return 0x00000000000283E0ull + (offset) * 16;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_DMAX_TIM(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000420ull + ((offset) & 1) * 16;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 1))
					return 0x0000000000028420ull + ((offset) & 1) * 16;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 1))
					return 0x0000000000000420ull + ((offset) & 1) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000028420ull + ((offset) & 1) * 16;
			break;
	}
	cvmx_warn("CVMX_SLI_DMAX_TIM (offset = %lu) not supported on this chip\n", offset);
	return 0x0000000000028420ull + ((offset) & 1) * 16;
}
#else
static inline uint64_t CVMX_SLI_DMAX_TIM(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000420ull + (offset) * 16;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000028420ull + (offset) * 16;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000420ull + (offset) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028420ull + (offset) * 16;
	}
	return 0x0000000000028420ull + (offset) * 16;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_INT_ENB_CIU CVMX_SLI_INT_ENB_CIU_FUNC()
static inline uint64_t CVMX_SLI_INT_ENB_CIU_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_INT_ENB_CIU not supported on this chip\n");
	return 0x0000000000003CD0ull;
}
#else
#define CVMX_SLI_INT_ENB_CIU (0x0000000000003CD0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_INT_ENB_PORTX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SLI_INT_ENB_PORTX(%lu) is invalid on this chip\n", offset);
	return 0x0000000000000340ull + ((offset) & 3) * 16;
}
#else
#define CVMX_SLI_INT_ENB_PORTX(offset) (0x0000000000000340ull + ((offset) & 3) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_INT_SUM CVMX_SLI_INT_SUM_FUNC()
static inline uint64_t CVMX_SLI_INT_SUM_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_INT_SUM not supported on this chip\n");
	return 0x0000000000000330ull;
}
#else
#define CVMX_SLI_INT_SUM (0x0000000000000330ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_LAST_WIN_RDATA0 CVMX_SLI_LAST_WIN_RDATA0_FUNC()
static inline uint64_t CVMX_SLI_LAST_WIN_RDATA0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_LAST_WIN_RDATA0 not supported on this chip\n");
	return 0x0000000000000600ull;
}
#else
#define CVMX_SLI_LAST_WIN_RDATA0 (0x0000000000000600ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_LAST_WIN_RDATA1 CVMX_SLI_LAST_WIN_RDATA1_FUNC()
static inline uint64_t CVMX_SLI_LAST_WIN_RDATA1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_LAST_WIN_RDATA1 not supported on this chip\n");
	return 0x0000000000000610ull;
}
#else
#define CVMX_SLI_LAST_WIN_RDATA1 (0x0000000000000610ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_LAST_WIN_RDATA2 CVMX_SLI_LAST_WIN_RDATA2_FUNC()
static inline uint64_t CVMX_SLI_LAST_WIN_RDATA2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_LAST_WIN_RDATA2 not supported on this chip\n");
	return 0x00000000000006C0ull;
}
#else
#define CVMX_SLI_LAST_WIN_RDATA2 (0x00000000000006C0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_LAST_WIN_RDATA3 CVMX_SLI_LAST_WIN_RDATA3_FUNC()
static inline uint64_t CVMX_SLI_LAST_WIN_RDATA3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_LAST_WIN_RDATA3 not supported on this chip\n");
	return 0x00000000000006D0ull;
}
#else
#define CVMX_SLI_LAST_WIN_RDATA3 (0x00000000000006D0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MACX_PFX_DMA_VF_INT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 1)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_SLI_MACX_PFX_DMA_VF_INT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return 0x0000000000027280ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16;
}
#else
#define CVMX_SLI_MACX_PFX_DMA_VF_INT(offset, block_id) (0x0000000000027280ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MACX_PFX_DMA_VF_INT_ENB(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 1)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_SLI_MACX_PFX_DMA_VF_INT_ENB(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return 0x0000000000027500ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16;
}
#else
#define CVMX_SLI_MACX_PFX_DMA_VF_INT_ENB(offset, block_id) (0x0000000000027500ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MACX_PFX_FLR_VF_INT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 1)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_SLI_MACX_PFX_FLR_VF_INT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return 0x0000000000027400ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16;
}
#else
#define CVMX_SLI_MACX_PFX_FLR_VF_INT(offset, block_id) (0x0000000000027400ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MACX_PFX_INT_ENB(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 1)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_SLI_MACX_PFX_INT_ENB(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return 0x0000000000027080ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16;
}
#else
#define CVMX_SLI_MACX_PFX_INT_ENB(offset, block_id) (0x0000000000027080ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MACX_PFX_INT_SUM(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 1)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_SLI_MACX_PFX_INT_SUM(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return 0x0000000000027000ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16;
}
#else
#define CVMX_SLI_MACX_PFX_INT_SUM(offset, block_id) (0x0000000000027000ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MACX_PFX_MBOX_INT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 1)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_SLI_MACX_PFX_MBOX_INT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return 0x0000000000027380ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16;
}
#else
#define CVMX_SLI_MACX_PFX_MBOX_INT(offset, block_id) (0x0000000000027380ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MACX_PFX_PKT_VF_INT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 1)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_SLI_MACX_PFX_PKT_VF_INT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return 0x0000000000027300ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16;
}
#else
#define CVMX_SLI_MACX_PFX_PKT_VF_INT(offset, block_id) (0x0000000000027300ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MACX_PFX_PKT_VF_INT_ENB(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 1)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_SLI_MACX_PFX_PKT_VF_INT_ENB(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return 0x0000000000027580ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16;
}
#else
#define CVMX_SLI_MACX_PFX_PKT_VF_INT_ENB(offset, block_id) (0x0000000000027580ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MACX_PFX_PP_VF_INT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 1)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_SLI_MACX_PFX_PP_VF_INT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return 0x0000000000027200ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16;
}
#else
#define CVMX_SLI_MACX_PFX_PP_VF_INT(offset, block_id) (0x0000000000027200ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MACX_PFX_PP_VF_INT_ENB(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 1)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_SLI_MACX_PFX_PP_VF_INT_ENB(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return 0x0000000000027480ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16;
}
#else
#define CVMX_SLI_MACX_PFX_PP_VF_INT_ENB(offset, block_id) (0x0000000000027480ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MAC_CREDIT_CNT CVMX_SLI_MAC_CREDIT_CNT_FUNC()
static inline uint64_t CVMX_SLI_MAC_CREDIT_CNT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003D70ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000023D70ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000003D70ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023D70ull;
			break;
	}
	cvmx_warn("CVMX_SLI_MAC_CREDIT_CNT not supported on this chip\n");
	return 0x0000000000023D70ull;
}
#else
#define CVMX_SLI_MAC_CREDIT_CNT CVMX_SLI_MAC_CREDIT_CNT_FUNC()
static inline uint64_t CVMX_SLI_MAC_CREDIT_CNT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003D70ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000023D70ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000003D70ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023D70ull;
	}
	return 0x0000000000023D70ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MAC_CREDIT_CNT2 CVMX_SLI_MAC_CREDIT_CNT2_FUNC()
static inline uint64_t CVMX_SLI_MAC_CREDIT_CNT2_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			return 0x0000000000013E10ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000023E10ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000003E10ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023E10ull;
			break;
	}
	cvmx_warn("CVMX_SLI_MAC_CREDIT_CNT2 not supported on this chip\n");
	return 0x0000000000023E10ull;
}
#else
#define CVMX_SLI_MAC_CREDIT_CNT2 CVMX_SLI_MAC_CREDIT_CNT2_FUNC()
static inline uint64_t CVMX_SLI_MAC_CREDIT_CNT2_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			return 0x0000000000013E10ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000023E10ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000003E10ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023E10ull;
	}
	return 0x0000000000023E10ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MAC_NUMBER CVMX_SLI_MAC_NUMBER_FUNC()
static inline uint64_t CVMX_SLI_MAC_NUMBER_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003E00ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000020050ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000003E00ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000020050ull;
			break;
	}
	cvmx_warn("CVMX_SLI_MAC_NUMBER not supported on this chip\n");
	return 0x0000000000020050ull;
}
#else
#define CVMX_SLI_MAC_NUMBER CVMX_SLI_MAC_NUMBER_FUNC()
static inline uint64_t CVMX_SLI_MAC_NUMBER_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003E00ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000020050ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000003E00ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000020050ull;
	}
	return 0x0000000000020050ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MEM_ACCESS_CTL CVMX_SLI_MEM_ACCESS_CTL_FUNC()
static inline uint64_t CVMX_SLI_MEM_ACCESS_CTL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000002F0ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x00000000000282F0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x00000000000002F0ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000282F0ull;
			break;
	}
	cvmx_warn("CVMX_SLI_MEM_ACCESS_CTL not supported on this chip\n");
	return 0x00000000000282F0ull;
}
#else
#define CVMX_SLI_MEM_ACCESS_CTL CVMX_SLI_MEM_ACCESS_CTL_FUNC()
static inline uint64_t CVMX_SLI_MEM_ACCESS_CTL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000002F0ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000000000282F0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000000000002F0ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000282F0ull;
	}
	return 0x00000000000282F0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MEM_ACCESS_SUBIDX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if (((offset >= 12) && (offset <= 27)))
				return 0x00000000000000E0ull + ((offset) & 31) * 16 - 16*12;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if (((offset >= 12) && (offset <= 27)))
					return 0x00000000000280E0ull + ((offset) & 31) * 16 - 16*12;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if (((offset >= 12) && (offset <= 27)))
					return 0x00000000000000E0ull + ((offset) & 31) * 16 - 16*12;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if (((offset >= 12) && (offset <= 27)))
				return 0x00000000000280E0ull + ((offset) & 31) * 16 - 16*12;
			break;
	}
	cvmx_warn("CVMX_SLI_MEM_ACCESS_SUBIDX (offset = %lu) not supported on this chip\n", offset);
	return 0x00000000000280E0ull + ((offset) & 31) * 16 - 16*12;
}
#else
static inline uint64_t CVMX_SLI_MEM_ACCESS_SUBIDX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000000E0ull + (offset) * 16 - 16*12;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000000000280E0ull + (offset) * 16 - 16*12;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000000000000E0ull + (offset) * 16 - 16*12;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000280E0ull + (offset) * 16 - 16*12;
	}
	return 0x00000000000280E0ull + (offset) * 16 - 16*12;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MEM_CTL CVMX_SLI_MEM_CTL_FUNC()
static inline uint64_t CVMX_SLI_MEM_CTL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x00000000000285E0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x00000000000005E0ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000285E0ull;
			break;

			break;
	}
	cvmx_warn("CVMX_SLI_MEM_CTL not supported on this chip\n");
	return 0x00000000000285E0ull;
}
#else
#define CVMX_SLI_MEM_CTL CVMX_SLI_MEM_CTL_FUNC()
static inline uint64_t CVMX_SLI_MEM_CTL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000000000285E0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000000000005E0ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000285E0ull;

	}
	return 0x00000000000285E0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MEM_INT_SUM CVMX_SLI_MEM_INT_SUM_FUNC()
static inline uint64_t CVMX_SLI_MEM_INT_SUM_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x00000000000285D0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x00000000000005D0ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000285D0ull;
			break;

			break;
	}
	cvmx_warn("CVMX_SLI_MEM_INT_SUM not supported on this chip\n");
	return 0x00000000000285D0ull;
}
#else
#define CVMX_SLI_MEM_INT_SUM CVMX_SLI_MEM_INT_SUM_FUNC()
static inline uint64_t CVMX_SLI_MEM_INT_SUM_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000000000285D0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000000000005D0ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000285D0ull;

	}
	return 0x00000000000285D0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MSIXX_TABLE_ADDR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 64))
					return 0x0000000000000000ull + ((offset) & 127) * 16;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 64))
					return 0x0000000000006000ull + ((offset) & 127) * 16;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 64))
				return 0x0000000000000000ull + ((offset) & 127) * 16;
			break;

			break;
	}
	cvmx_warn("CVMX_SLI_MSIXX_TABLE_ADDR (offset = %lu) not supported on this chip\n", offset);
	return 0x0000000000000000ull + ((offset) & 127) * 16;
}
#else
static inline uint64_t CVMX_SLI_MSIXX_TABLE_ADDR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000000000ull + (offset) * 16;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000006000ull + (offset) * 16;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000000ull + (offset) * 16;

	}
	return 0x0000000000000000ull + (offset) * 16;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MSIXX_TABLE_DATA(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 64))
					return 0x0000000000000008ull + ((offset) & 127) * 16;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 64))
					return 0x0000000000006008ull + ((offset) & 127) * 16;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 64))
				return 0x0000000000000008ull + ((offset) & 127) * 16;
			break;

			break;
	}
	cvmx_warn("CVMX_SLI_MSIXX_TABLE_DATA (offset = %lu) not supported on this chip\n", offset);
	return 0x0000000000000008ull + ((offset) & 127) * 16;
}
#else
static inline uint64_t CVMX_SLI_MSIXX_TABLE_DATA(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000000008ull + (offset) * 16;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000006008ull + (offset) * 16;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000008ull + (offset) * 16;

	}
	return 0x0000000000000008ull + (offset) * 16;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MSIX_MACX_PF_TABLE_ADDR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3)))))
		cvmx_warn("CVMX_SLI_MSIX_MACX_PF_TABLE_ADDR(%lu) is invalid on this chip\n", offset);
	return 0x0000000000007C00ull + ((offset) & 3) * 16;
}
#else
#define CVMX_SLI_MSIX_MACX_PF_TABLE_ADDR(offset) (0x0000000000007C00ull + ((offset) & 3) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MSIX_MACX_PF_TABLE_DATA(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3)))))
		cvmx_warn("CVMX_SLI_MSIX_MACX_PF_TABLE_DATA(%lu) is invalid on this chip\n", offset);
	return 0x0000000000007C08ull + ((offset) & 3) * 16;
}
#else
#define CVMX_SLI_MSIX_MACX_PF_TABLE_DATA(offset) (0x0000000000007C08ull + ((offset) & 3) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSIX_PBA0 CVMX_SLI_MSIX_PBA0_FUNC()
static inline uint64_t CVMX_SLI_MSIX_PBA0_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000001000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000007000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001000ull;
			break;

			break;
	}
	cvmx_warn("CVMX_SLI_MSIX_PBA0 not supported on this chip\n");
	return 0x0000000000001000ull;
}
#else
#define CVMX_SLI_MSIX_PBA0 CVMX_SLI_MSIX_PBA0_FUNC()
static inline uint64_t CVMX_SLI_MSIX_PBA0_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000001000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000007000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001000ull;

	}
	return 0x0000000000001000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSIX_PBA1 CVMX_SLI_MSIX_PBA1_FUNC()
static inline uint64_t CVMX_SLI_MSIX_PBA1_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000001008ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000007010ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001008ull;
			break;

			break;
	}
	cvmx_warn("CVMX_SLI_MSIX_PBA1 not supported on this chip\n");
	return 0x0000000000001008ull;
}
#else
#define CVMX_SLI_MSIX_PBA1 CVMX_SLI_MSIX_PBA1_FUNC()
static inline uint64_t CVMX_SLI_MSIX_PBA1_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000001008ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000007010ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001008ull;

	}
	return 0x0000000000001008ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_ENB0 CVMX_SLI_MSI_ENB0_FUNC()
static inline uint64_t CVMX_SLI_MSI_ENB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_ENB0 not supported on this chip\n");
	return 0x0000000000003C50ull;
}
#else
#define CVMX_SLI_MSI_ENB0 (0x0000000000003C50ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_ENB1 CVMX_SLI_MSI_ENB1_FUNC()
static inline uint64_t CVMX_SLI_MSI_ENB1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_ENB1 not supported on this chip\n");
	return 0x0000000000003C60ull;
}
#else
#define CVMX_SLI_MSI_ENB1 (0x0000000000003C60ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_ENB2 CVMX_SLI_MSI_ENB2_FUNC()
static inline uint64_t CVMX_SLI_MSI_ENB2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_ENB2 not supported on this chip\n");
	return 0x0000000000003C70ull;
}
#else
#define CVMX_SLI_MSI_ENB2 (0x0000000000003C70ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_ENB3 CVMX_SLI_MSI_ENB3_FUNC()
static inline uint64_t CVMX_SLI_MSI_ENB3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_ENB3 not supported on this chip\n");
	return 0x0000000000003C80ull;
}
#else
#define CVMX_SLI_MSI_ENB3 (0x0000000000003C80ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_RCV0 CVMX_SLI_MSI_RCV0_FUNC()
static inline uint64_t CVMX_SLI_MSI_RCV0_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003C10ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000023C10ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000003C10ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023C10ull;
			break;
	}
	cvmx_warn("CVMX_SLI_MSI_RCV0 not supported on this chip\n");
	return 0x0000000000023C10ull;
}
#else
#define CVMX_SLI_MSI_RCV0 CVMX_SLI_MSI_RCV0_FUNC()
static inline uint64_t CVMX_SLI_MSI_RCV0_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003C10ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000023C10ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000003C10ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023C10ull;
	}
	return 0x0000000000023C10ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_RCV1 CVMX_SLI_MSI_RCV1_FUNC()
static inline uint64_t CVMX_SLI_MSI_RCV1_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003C20ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000023C20ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000003C20ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023C20ull;
			break;
	}
	cvmx_warn("CVMX_SLI_MSI_RCV1 not supported on this chip\n");
	return 0x0000000000023C20ull;
}
#else
#define CVMX_SLI_MSI_RCV1 CVMX_SLI_MSI_RCV1_FUNC()
static inline uint64_t CVMX_SLI_MSI_RCV1_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003C20ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000023C20ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000003C20ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023C20ull;
	}
	return 0x0000000000023C20ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_RCV2 CVMX_SLI_MSI_RCV2_FUNC()
static inline uint64_t CVMX_SLI_MSI_RCV2_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003C30ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000023C30ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000003C30ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023C30ull;
			break;
	}
	cvmx_warn("CVMX_SLI_MSI_RCV2 not supported on this chip\n");
	return 0x0000000000023C30ull;
}
#else
#define CVMX_SLI_MSI_RCV2 CVMX_SLI_MSI_RCV2_FUNC()
static inline uint64_t CVMX_SLI_MSI_RCV2_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003C30ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000023C30ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000003C30ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023C30ull;
	}
	return 0x0000000000023C30ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_RCV3 CVMX_SLI_MSI_RCV3_FUNC()
static inline uint64_t CVMX_SLI_MSI_RCV3_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003C40ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000023C40ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000003C40ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023C40ull;
			break;
	}
	cvmx_warn("CVMX_SLI_MSI_RCV3 not supported on this chip\n");
	return 0x0000000000023C40ull;
}
#else
#define CVMX_SLI_MSI_RCV3 CVMX_SLI_MSI_RCV3_FUNC()
static inline uint64_t CVMX_SLI_MSI_RCV3_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003C40ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000023C40ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000003C40ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023C40ull;
	}
	return 0x0000000000023C40ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_RD_MAP CVMX_SLI_MSI_RD_MAP_FUNC()
static inline uint64_t CVMX_SLI_MSI_RD_MAP_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003CA0ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000023CA0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000003CA0ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023CA0ull;
			break;
	}
	cvmx_warn("CVMX_SLI_MSI_RD_MAP not supported on this chip\n");
	return 0x0000000000023CA0ull;
}
#else
#define CVMX_SLI_MSI_RD_MAP CVMX_SLI_MSI_RD_MAP_FUNC()
static inline uint64_t CVMX_SLI_MSI_RD_MAP_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003CA0ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000023CA0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000003CA0ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023CA0ull;
	}
	return 0x0000000000023CA0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1C_ENB0 CVMX_SLI_MSI_W1C_ENB0_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1C_ENB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1C_ENB0 not supported on this chip\n");
	return 0x0000000000003CF0ull;
}
#else
#define CVMX_SLI_MSI_W1C_ENB0 (0x0000000000003CF0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1C_ENB1 CVMX_SLI_MSI_W1C_ENB1_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1C_ENB1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1C_ENB1 not supported on this chip\n");
	return 0x0000000000003D00ull;
}
#else
#define CVMX_SLI_MSI_W1C_ENB1 (0x0000000000003D00ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1C_ENB2 CVMX_SLI_MSI_W1C_ENB2_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1C_ENB2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1C_ENB2 not supported on this chip\n");
	return 0x0000000000003D10ull;
}
#else
#define CVMX_SLI_MSI_W1C_ENB2 (0x0000000000003D10ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1C_ENB3 CVMX_SLI_MSI_W1C_ENB3_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1C_ENB3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1C_ENB3 not supported on this chip\n");
	return 0x0000000000003D20ull;
}
#else
#define CVMX_SLI_MSI_W1C_ENB3 (0x0000000000003D20ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1S_ENB0 CVMX_SLI_MSI_W1S_ENB0_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1S_ENB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1S_ENB0 not supported on this chip\n");
	return 0x0000000000003D30ull;
}
#else
#define CVMX_SLI_MSI_W1S_ENB0 (0x0000000000003D30ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1S_ENB1 CVMX_SLI_MSI_W1S_ENB1_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1S_ENB1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1S_ENB1 not supported on this chip\n");
	return 0x0000000000003D40ull;
}
#else
#define CVMX_SLI_MSI_W1S_ENB1 (0x0000000000003D40ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1S_ENB2 CVMX_SLI_MSI_W1S_ENB2_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1S_ENB2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1S_ENB2 not supported on this chip\n");
	return 0x0000000000003D50ull;
}
#else
#define CVMX_SLI_MSI_W1S_ENB2 (0x0000000000003D50ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1S_ENB3 CVMX_SLI_MSI_W1S_ENB3_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1S_ENB3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1S_ENB3 not supported on this chip\n");
	return 0x0000000000003D60ull;
}
#else
#define CVMX_SLI_MSI_W1S_ENB3 (0x0000000000003D60ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_WR_MAP CVMX_SLI_MSI_WR_MAP_FUNC()
static inline uint64_t CVMX_SLI_MSI_WR_MAP_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003C90ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000023C90ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000003C90ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023C90ull;
			break;
	}
	cvmx_warn("CVMX_SLI_MSI_WR_MAP not supported on this chip\n");
	return 0x0000000000023C90ull;
}
#else
#define CVMX_SLI_MSI_WR_MAP CVMX_SLI_MSI_WR_MAP_FUNC()
static inline uint64_t CVMX_SLI_MSI_WR_MAP_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003C90ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000023C90ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000003C90ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023C90ull;
	}
	return 0x0000000000023C90ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_NQM_RSP_ERR_SND_DBG CVMX_SLI_NQM_RSP_ERR_SND_DBG_FUNC()
static inline uint64_t CVMX_SLI_NQM_RSP_ERR_SND_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SLI_NQM_RSP_ERR_SND_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F0000028800ull);
}
#else
#define CVMX_SLI_NQM_RSP_ERR_SND_DBG (CVMX_ADD_IO_SEG(0x00011F0000028800ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PCIE_MSI_RCV CVMX_SLI_PCIE_MSI_RCV_FUNC()
static inline uint64_t CVMX_SLI_PCIE_MSI_RCV_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003CB0ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000023CB0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000003CB0ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023CB0ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PCIE_MSI_RCV not supported on this chip\n");
	return 0x0000000000023CB0ull;
}
#else
#define CVMX_SLI_PCIE_MSI_RCV CVMX_SLI_PCIE_MSI_RCV_FUNC()
static inline uint64_t CVMX_SLI_PCIE_MSI_RCV_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003CB0ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000023CB0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000003CB0ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023CB0ull;
	}
	return 0x0000000000023CB0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PCIE_MSI_RCV_B1 CVMX_SLI_PCIE_MSI_RCV_B1_FUNC()
static inline uint64_t CVMX_SLI_PCIE_MSI_RCV_B1_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000650ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000028650ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000000650ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028650ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PCIE_MSI_RCV_B1 not supported on this chip\n");
	return 0x0000000000028650ull;
}
#else
#define CVMX_SLI_PCIE_MSI_RCV_B1 CVMX_SLI_PCIE_MSI_RCV_B1_FUNC()
static inline uint64_t CVMX_SLI_PCIE_MSI_RCV_B1_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000650ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000028650ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000650ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028650ull;
	}
	return 0x0000000000028650ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PCIE_MSI_RCV_B2 CVMX_SLI_PCIE_MSI_RCV_B2_FUNC()
static inline uint64_t CVMX_SLI_PCIE_MSI_RCV_B2_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000660ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000028660ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000000660ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028660ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PCIE_MSI_RCV_B2 not supported on this chip\n");
	return 0x0000000000028660ull;
}
#else
#define CVMX_SLI_PCIE_MSI_RCV_B2 CVMX_SLI_PCIE_MSI_RCV_B2_FUNC()
static inline uint64_t CVMX_SLI_PCIE_MSI_RCV_B2_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000660ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000028660ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000660ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028660ull;
	}
	return 0x0000000000028660ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PCIE_MSI_RCV_B3 CVMX_SLI_PCIE_MSI_RCV_B3_FUNC()
static inline uint64_t CVMX_SLI_PCIE_MSI_RCV_B3_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000670ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000028670ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000000670ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028670ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PCIE_MSI_RCV_B3 not supported on this chip\n");
	return 0x0000000000028670ull;
}
#else
#define CVMX_SLI_PCIE_MSI_RCV_B3 CVMX_SLI_PCIE_MSI_RCV_B3_FUNC()
static inline uint64_t CVMX_SLI_PCIE_MSI_RCV_B3_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000670ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000028670ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000670ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028670ull;
	}
	return 0x0000000000028670ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_CNTS(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return 0x0000000000002400ull + ((offset) & 31) * 16;
			break;

			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return 0x00000000000100B0ull + ((offset) & 63) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return 0x0000000000002400ull + ((offset) & 63) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 63))
				return 0x00000000000100B0ull + ((offset) & 63) * 0x20000ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PKTX_CNTS (offset = %lu) not supported on this chip\n", offset);
	return 0x00000000000100B0ull + ((offset) & 63) * 0x20000ull;
}
#else
static inline uint64_t CVMX_SLI_PKTX_CNTS(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002400ull + (offset) * 16;

		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000000000100B0ull + (offset) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000002400ull + (offset) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000100B0ull + (offset) * 0x20000ull;
	}
	return 0x00000000000100B0ull + (offset) * 0x20000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_ERROR_INFO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SLI_PKTX_ERROR_INFO(%lu) is invalid on this chip\n", offset);
	return 0x00000000000100C0ull + ((offset) & 63) * 0x20000ull;
}
#else
#define CVMX_SLI_PKTX_ERROR_INFO(offset) (0x00000000000100C0ull + ((offset) & 63) * 0x20000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_INPUT_CONTROL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return 0x0000000000010000ull + ((offset) & 63) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return 0x0000000000004000ull + ((offset) & 63) * 16;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 63))
				return 0x0000000000010000ull + ((offset) & 63) * 0x20000ull;
			break;

			break;
	}
	cvmx_warn("CVMX_SLI_PKTX_INPUT_CONTROL (offset = %lu) not supported on this chip\n", offset);
	return 0x0000000000010000ull + ((offset) & 63) * 0x20000ull;
}
#else
static inline uint64_t CVMX_SLI_PKTX_INPUT_CONTROL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000010000ull + (offset) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000004000ull + (offset) * 16;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000010000ull + (offset) * 0x20000ull;

	}
	return 0x0000000000010000ull + (offset) * 0x20000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_INSTR_BADDR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return 0x0000000000002800ull + ((offset) & 31) * 16;
			break;

			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return 0x0000000000010010ull + ((offset) & 63) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return 0x0000000000002800ull + ((offset) & 63) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 63))
				return 0x0000000000010010ull + ((offset) & 63) * 0x20000ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PKTX_INSTR_BADDR (offset = %lu) not supported on this chip\n", offset);
	return 0x0000000000010010ull + ((offset) & 63) * 0x20000ull;
}
#else
static inline uint64_t CVMX_SLI_PKTX_INSTR_BADDR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002800ull + (offset) * 16;

		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000010010ull + (offset) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000002800ull + (offset) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000010010ull + (offset) * 0x20000ull;
	}
	return 0x0000000000010010ull + (offset) * 0x20000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_INSTR_BAOFF_DBELL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return 0x0000000000002C00ull + ((offset) & 31) * 16;
			break;

			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return 0x0000000000010020ull + ((offset) & 63) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return 0x0000000000002C00ull + ((offset) & 63) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 63))
				return 0x0000000000010020ull + ((offset) & 63) * 0x20000ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PKTX_INSTR_BAOFF_DBELL (offset = %lu) not supported on this chip\n", offset);
	return 0x0000000000010020ull + ((offset) & 63) * 0x20000ull;
}
#else
static inline uint64_t CVMX_SLI_PKTX_INSTR_BAOFF_DBELL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002C00ull + (offset) * 16;

		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000010020ull + (offset) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000002C00ull + (offset) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000010020ull + (offset) * 0x20000ull;
	}
	return 0x0000000000010020ull + (offset) * 0x20000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_INSTR_FIFO_RSIZE(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return 0x0000000000003000ull + ((offset) & 31) * 16;
			break;

			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return 0x0000000000010030ull + ((offset) & 63) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return 0x0000000000003000ull + ((offset) & 63) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 63))
				return 0x0000000000010030ull + ((offset) & 63) * 0x20000ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PKTX_INSTR_FIFO_RSIZE (offset = %lu) not supported on this chip\n", offset);
	return 0x0000000000010030ull + ((offset) & 63) * 0x20000ull;
}
#else
static inline uint64_t CVMX_SLI_PKTX_INSTR_FIFO_RSIZE(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003000ull + (offset) * 16;

		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000010030ull + (offset) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000003000ull + (offset) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000010030ull + (offset) * 0x20000ull;
	}
	return 0x0000000000010030ull + (offset) * 0x20000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_INSTR_HEADER(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SLI_PKTX_INSTR_HEADER(%lu) is invalid on this chip\n", offset);
	return 0x0000000000003400ull + ((offset) & 31) * 16;
}
#else
#define CVMX_SLI_PKTX_INSTR_HEADER(offset) (0x0000000000003400ull + ((offset) & 31) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_INT_LEVELS(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return 0x00000000000100A0ull + ((offset) & 63) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return 0x0000000000004400ull + ((offset) & 63) * 16;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 63))
				return 0x00000000000100A0ull + ((offset) & 63) * 0x20000ull;
			break;

			break;
	}
	cvmx_warn("CVMX_SLI_PKTX_INT_LEVELS (offset = %lu) not supported on this chip\n", offset);
	return 0x00000000000100A0ull + ((offset) & 63) * 0x20000ull;
}
#else
static inline uint64_t CVMX_SLI_PKTX_INT_LEVELS(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000000000100A0ull + (offset) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000004400ull + (offset) * 16;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000100A0ull + (offset) * 0x20000ull;

	}
	return 0x00000000000100A0ull + (offset) * 0x20000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_IN_BP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SLI_PKTX_IN_BP(%lu) is invalid on this chip\n", offset);
	return 0x0000000000003800ull + ((offset) & 31) * 16;
}
#else
#define CVMX_SLI_PKTX_IN_BP(offset) (0x0000000000003800ull + ((offset) & 31) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_MBOX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SLI_PKTX_MBOX_INT(%lu) is invalid on this chip\n", offset);
	return 0x0000000000010210ull + ((offset) & 63) * 0x20000ull;
}
#else
#define CVMX_SLI_PKTX_MBOX_INT(offset) (0x0000000000010210ull + ((offset) & 63) * 0x20000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_OUTPUT_CONTROL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return 0x0000000000010050ull + ((offset) & 63) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return 0x0000000000004800ull + ((offset) & 63) * 16;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 63))
				return 0x0000000000010050ull + ((offset) & 63) * 0x20000ull;
			break;

			break;
	}
	cvmx_warn("CVMX_SLI_PKTX_OUTPUT_CONTROL (offset = %lu) not supported on this chip\n", offset);
	return 0x0000000000010050ull + ((offset) & 63) * 0x20000ull;
}
#else
static inline uint64_t CVMX_SLI_PKTX_OUTPUT_CONTROL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000010050ull + (offset) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000004800ull + (offset) * 16;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000010050ull + (offset) * 0x20000ull;

	}
	return 0x0000000000010050ull + (offset) * 0x20000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_OUT_SIZE(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return 0x0000000000000C00ull + ((offset) & 31) * 16;
			break;

			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return 0x0000000000010060ull + ((offset) & 63) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return 0x0000000000000C00ull + ((offset) & 63) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 63))
				return 0x0000000000010060ull + ((offset) & 63) * 0x20000ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PKTX_OUT_SIZE (offset = %lu) not supported on this chip\n", offset);
	return 0x0000000000010060ull + ((offset) & 63) * 0x20000ull;
}
#else
static inline uint64_t CVMX_SLI_PKTX_OUT_SIZE(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000C00ull + (offset) * 16;

		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000010060ull + (offset) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000C00ull + (offset) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000010060ull + (offset) * 0x20000ull;
	}
	return 0x0000000000010060ull + (offset) * 0x20000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_PF_VF_MBOX_SIGX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 1)) && ((block_id <= 63)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id <= 63)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 1)) && ((block_id <= 63))))))
		cvmx_warn("CVMX_SLI_PKTX_PF_VF_MBOX_SIGX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return 0x0000000000010200ull + (((offset) & 1) + ((block_id) & 63) * 0x4000ull) * 8;
}
#else
#define CVMX_SLI_PKTX_PF_VF_MBOX_SIGX(offset, block_id) (0x0000000000010200ull + (((offset) & 1) + ((block_id) & 63) * 0x4000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_SLIST_BADDR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return 0x0000000000001400ull + ((offset) & 31) * 16;
			break;

			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return 0x0000000000010070ull + ((offset) & 63) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return 0x0000000000001400ull + ((offset) & 63) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 63))
				return 0x0000000000010070ull + ((offset) & 63) * 0x20000ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PKTX_SLIST_BADDR (offset = %lu) not supported on this chip\n", offset);
	return 0x0000000000010070ull + ((offset) & 63) * 0x20000ull;
}
#else
static inline uint64_t CVMX_SLI_PKTX_SLIST_BADDR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001400ull + (offset) * 16;

		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000010070ull + (offset) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000001400ull + (offset) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000010070ull + (offset) * 0x20000ull;
	}
	return 0x0000000000010070ull + (offset) * 0x20000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_SLIST_BAOFF_DBELL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return 0x0000000000001800ull + ((offset) & 31) * 16;
			break;

			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return 0x0000000000010080ull + ((offset) & 63) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return 0x0000000000001800ull + ((offset) & 63) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 63))
				return 0x0000000000010080ull + ((offset) & 63) * 0x20000ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PKTX_SLIST_BAOFF_DBELL (offset = %lu) not supported on this chip\n", offset);
	return 0x0000000000010080ull + ((offset) & 63) * 0x20000ull;
}
#else
static inline uint64_t CVMX_SLI_PKTX_SLIST_BAOFF_DBELL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001800ull + (offset) * 16;

		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000010080ull + (offset) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000001800ull + (offset) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000010080ull + (offset) * 0x20000ull;
	}
	return 0x0000000000010080ull + (offset) * 0x20000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_SLIST_FIFO_RSIZE(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return 0x0000000000001C00ull + ((offset) & 31) * 16;
			break;

			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return 0x0000000000010090ull + ((offset) & 63) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return 0x0000000000001C00ull + ((offset) & 63) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 63))
				return 0x0000000000010090ull + ((offset) & 63) * 0x20000ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PKTX_SLIST_FIFO_RSIZE (offset = %lu) not supported on this chip\n", offset);
	return 0x0000000000010090ull + ((offset) & 63) * 0x20000ull;
}
#else
static inline uint64_t CVMX_SLI_PKTX_SLIST_FIFO_RSIZE(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001C00ull + (offset) * 16;

		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000010090ull + (offset) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000001C00ull + (offset) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000010090ull + (offset) * 0x20000ull;
	}
	return 0x0000000000010090ull + (offset) * 0x20000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_VF_INT_SUM(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SLI_PKTX_VF_INT_SUM(%lu) is invalid on this chip\n", offset);
	return 0x00000000000100D0ull + ((offset) & 63) * 0x20000ull;
}
#else
#define CVMX_SLI_PKTX_VF_INT_SUM(offset) (0x00000000000100D0ull + ((offset) & 63) * 0x20000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_VF_SIG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63)))))
		cvmx_warn("CVMX_SLI_PKTX_VF_SIG(%lu) is invalid on this chip\n", offset);
	return 0x0000000000004C00ull + ((offset) & 63) * 16;
}
#else
#define CVMX_SLI_PKTX_VF_SIG(offset) (0x0000000000004C00ull + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_BIST_STATUS CVMX_SLI_PKT_BIST_STATUS_FUNC()
static inline uint64_t CVMX_SLI_PKT_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SLI_PKT_BIST_STATUS not supported on this chip\n");
	return 0x0000000000029220ull;
}
#else
#define CVMX_SLI_PKT_BIST_STATUS (0x0000000000029220ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_CNT_INT CVMX_SLI_PKT_CNT_INT_FUNC()
static inline uint64_t CVMX_SLI_PKT_CNT_INT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001130ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000029130ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000001130ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000029130ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PKT_CNT_INT not supported on this chip\n");
	return 0x0000000000029130ull;
}
#else
#define CVMX_SLI_PKT_CNT_INT CVMX_SLI_PKT_CNT_INT_FUNC()
static inline uint64_t CVMX_SLI_PKT_CNT_INT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001130ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000029130ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000001130ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000029130ull;
	}
	return 0x0000000000029130ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_CNT_INT_ENB CVMX_SLI_PKT_CNT_INT_ENB_FUNC()
static inline uint64_t CVMX_SLI_PKT_CNT_INT_ENB_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_CNT_INT_ENB not supported on this chip\n");
	return 0x0000000000001150ull;
}
#else
#define CVMX_SLI_PKT_CNT_INT_ENB (0x0000000000001150ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_CTL CVMX_SLI_PKT_CTL_FUNC()
static inline uint64_t CVMX_SLI_PKT_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_CTL not supported on this chip\n");
	return 0x0000000000001220ull;
}
#else
#define CVMX_SLI_PKT_CTL (0x0000000000001220ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_DATA_OUT_ES CVMX_SLI_PKT_DATA_OUT_ES_FUNC()
static inline uint64_t CVMX_SLI_PKT_DATA_OUT_ES_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_DATA_OUT_ES not supported on this chip\n");
	return 0x00000000000010B0ull;
}
#else
#define CVMX_SLI_PKT_DATA_OUT_ES (0x00000000000010B0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_DATA_OUT_NS CVMX_SLI_PKT_DATA_OUT_NS_FUNC()
static inline uint64_t CVMX_SLI_PKT_DATA_OUT_NS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_DATA_OUT_NS not supported on this chip\n");
	return 0x00000000000010A0ull;
}
#else
#define CVMX_SLI_PKT_DATA_OUT_NS (0x00000000000010A0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_DATA_OUT_ROR CVMX_SLI_PKT_DATA_OUT_ROR_FUNC()
static inline uint64_t CVMX_SLI_PKT_DATA_OUT_ROR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_DATA_OUT_ROR not supported on this chip\n");
	return 0x0000000000001090ull;
}
#else
#define CVMX_SLI_PKT_DATA_OUT_ROR (0x0000000000001090ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_DPADDR CVMX_SLI_PKT_DPADDR_FUNC()
static inline uint64_t CVMX_SLI_PKT_DPADDR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_DPADDR not supported on this chip\n");
	return 0x0000000000001080ull;
}
#else
#define CVMX_SLI_PKT_DPADDR (0x0000000000001080ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_GBL_CONTROL CVMX_SLI_PKT_GBL_CONTROL_FUNC()
static inline uint64_t CVMX_SLI_PKT_GBL_CONTROL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SLI_PKT_GBL_CONTROL not supported on this chip\n");
	return 0x0000000000029210ull;
}
#else
#define CVMX_SLI_PKT_GBL_CONTROL (0x0000000000029210ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_INPUT_CONTROL CVMX_SLI_PKT_INPUT_CONTROL_FUNC()
static inline uint64_t CVMX_SLI_PKT_INPUT_CONTROL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_INPUT_CONTROL not supported on this chip\n");
	return 0x0000000000001170ull;
}
#else
#define CVMX_SLI_PKT_INPUT_CONTROL (0x0000000000001170ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_INSTR_ENB CVMX_SLI_PKT_INSTR_ENB_FUNC()
static inline uint64_t CVMX_SLI_PKT_INSTR_ENB_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_INSTR_ENB not supported on this chip\n");
	return 0x0000000000001000ull;
}
#else
#define CVMX_SLI_PKT_INSTR_ENB (0x0000000000001000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_INSTR_RD_SIZE CVMX_SLI_PKT_INSTR_RD_SIZE_FUNC()
static inline uint64_t CVMX_SLI_PKT_INSTR_RD_SIZE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_INSTR_RD_SIZE not supported on this chip\n");
	return 0x00000000000011A0ull;
}
#else
#define CVMX_SLI_PKT_INSTR_RD_SIZE (0x00000000000011A0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_INSTR_SIZE CVMX_SLI_PKT_INSTR_SIZE_FUNC()
static inline uint64_t CVMX_SLI_PKT_INSTR_SIZE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_INSTR_SIZE not supported on this chip\n");
	return 0x0000000000001020ull;
}
#else
#define CVMX_SLI_PKT_INSTR_SIZE (0x0000000000001020ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_INT CVMX_SLI_PKT_INT_FUNC()
static inline uint64_t CVMX_SLI_PKT_INT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000029160ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000001160ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000029160ull;
			break;

			break;
	}
	cvmx_warn("CVMX_SLI_PKT_INT not supported on this chip\n");
	return 0x0000000000029160ull;
}
#else
#define CVMX_SLI_PKT_INT CVMX_SLI_PKT_INT_FUNC()
static inline uint64_t CVMX_SLI_PKT_INT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000029160ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000001160ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000029160ull;

	}
	return 0x0000000000029160ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_INT_LEVELS CVMX_SLI_PKT_INT_LEVELS_FUNC()
static inline uint64_t CVMX_SLI_PKT_INT_LEVELS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_INT_LEVELS not supported on this chip\n");
	return 0x0000000000001120ull;
}
#else
#define CVMX_SLI_PKT_INT_LEVELS (0x0000000000001120ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_IN_BP CVMX_SLI_PKT_IN_BP_FUNC()
static inline uint64_t CVMX_SLI_PKT_IN_BP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_IN_BP not supported on this chip\n");
	return 0x0000000000001210ull;
}
#else
#define CVMX_SLI_PKT_IN_BP (0x0000000000001210ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKT_IN_DONEX_CNTS(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return 0x0000000000002000ull + ((offset) & 31) * 16;
			break;

			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return 0x0000000000010040ull + ((offset) & 63) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return 0x0000000000002000ull + ((offset) & 63) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 63))
				return 0x0000000000010040ull + ((offset) & 63) * 0x20000ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PKT_IN_DONEX_CNTS (offset = %lu) not supported on this chip\n", offset);
	return 0x0000000000010040ull + ((offset) & 63) * 0x20000ull;
}
#else
static inline uint64_t CVMX_SLI_PKT_IN_DONEX_CNTS(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002000ull + (offset) * 16;

		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000010040ull + (offset) * 0x20000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000002000ull + (offset) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000010040ull + (offset) * 0x20000ull;
	}
	return 0x0000000000010040ull + (offset) * 0x20000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_IN_INSTR_COUNTS CVMX_SLI_PKT_IN_INSTR_COUNTS_FUNC()
static inline uint64_t CVMX_SLI_PKT_IN_INSTR_COUNTS_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001200ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000029200ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000001200ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000029200ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PKT_IN_INSTR_COUNTS not supported on this chip\n");
	return 0x0000000000029200ull;
}
#else
#define CVMX_SLI_PKT_IN_INSTR_COUNTS CVMX_SLI_PKT_IN_INSTR_COUNTS_FUNC()
static inline uint64_t CVMX_SLI_PKT_IN_INSTR_COUNTS_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001200ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000029200ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000001200ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000029200ull;
	}
	return 0x0000000000029200ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_IN_INT CVMX_SLI_PKT_IN_INT_FUNC()
static inline uint64_t CVMX_SLI_PKT_IN_INT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000029150ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000001150ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000029150ull;
			break;

			break;
	}
	cvmx_warn("CVMX_SLI_PKT_IN_INT not supported on this chip\n");
	return 0x0000000000029150ull;
}
#else
#define CVMX_SLI_PKT_IN_INT CVMX_SLI_PKT_IN_INT_FUNC()
static inline uint64_t CVMX_SLI_PKT_IN_INT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000029150ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000001150ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000029150ull;

	}
	return 0x0000000000029150ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_IN_JABBER CVMX_SLI_PKT_IN_JABBER_FUNC()
static inline uint64_t CVMX_SLI_PKT_IN_JABBER_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SLI_PKT_IN_JABBER not supported on this chip\n");
	return 0x0000000000029170ull;
}
#else
#define CVMX_SLI_PKT_IN_JABBER (0x0000000000029170ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_IN_PCIE_PORT CVMX_SLI_PKT_IN_PCIE_PORT_FUNC()
static inline uint64_t CVMX_SLI_PKT_IN_PCIE_PORT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_IN_PCIE_PORT not supported on this chip\n");
	return 0x00000000000011B0ull;
}
#else
#define CVMX_SLI_PKT_IN_PCIE_PORT (0x00000000000011B0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_IPTR CVMX_SLI_PKT_IPTR_FUNC()
static inline uint64_t CVMX_SLI_PKT_IPTR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_IPTR not supported on this chip\n");
	return 0x0000000000001070ull;
}
#else
#define CVMX_SLI_PKT_IPTR (0x0000000000001070ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_MAC0_SIG0 CVMX_SLI_PKT_MAC0_SIG0_FUNC()
static inline uint64_t CVMX_SLI_PKT_MAC0_SIG0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_SLI_PKT_MAC0_SIG0 not supported on this chip\n");
	return 0x0000000000001300ull;
}
#else
#define CVMX_SLI_PKT_MAC0_SIG0 (0x0000000000001300ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_MAC0_SIG1 CVMX_SLI_PKT_MAC0_SIG1_FUNC()
static inline uint64_t CVMX_SLI_PKT_MAC0_SIG1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_SLI_PKT_MAC0_SIG1 not supported on this chip\n");
	return 0x0000000000001310ull;
}
#else
#define CVMX_SLI_PKT_MAC0_SIG1 (0x0000000000001310ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_MAC1_SIG0 CVMX_SLI_PKT_MAC1_SIG0_FUNC()
static inline uint64_t CVMX_SLI_PKT_MAC1_SIG0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_SLI_PKT_MAC1_SIG0 not supported on this chip\n");
	return 0x0000000000001320ull;
}
#else
#define CVMX_SLI_PKT_MAC1_SIG0 (0x0000000000001320ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_MAC1_SIG1 CVMX_SLI_PKT_MAC1_SIG1_FUNC()
static inline uint64_t CVMX_SLI_PKT_MAC1_SIG1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_SLI_PKT_MAC1_SIG1 not supported on this chip\n");
	return 0x0000000000001330ull;
}
#else
#define CVMX_SLI_PKT_MAC1_SIG1 (0x0000000000001330ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKT_MACX_PFX_RINFO(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 1)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_SLI_PKT_MACX_PFX_RINFO(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return 0x0000000000029030ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16;
}
#else
#define CVMX_SLI_PKT_MACX_PFX_RINFO(offset, block_id) (0x0000000000029030ull + (((offset) & 1) + ((block_id) & 3) * 0x2ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKT_MACX_RINFO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3)))))
		cvmx_warn("CVMX_SLI_PKT_MACX_RINFO(%lu) is invalid on this chip\n", offset);
	return 0x0000000000001030ull + ((offset) & 3) * 16;
}
#else
#define CVMX_SLI_PKT_MACX_RINFO(offset) (0x0000000000001030ull + ((offset) & 3) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_MEM_CTL CVMX_SLI_PKT_MEM_CTL_FUNC()
static inline uint64_t CVMX_SLI_PKT_MEM_CTL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000029120ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000001120ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000029120ull;
			break;

			break;
	}
	cvmx_warn("CVMX_SLI_PKT_MEM_CTL not supported on this chip\n");
	return 0x0000000000029120ull;
}
#else
#define CVMX_SLI_PKT_MEM_CTL CVMX_SLI_PKT_MEM_CTL_FUNC()
static inline uint64_t CVMX_SLI_PKT_MEM_CTL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000029120ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000001120ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000029120ull;

	}
	return 0x0000000000029120ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_OUTPUT_WMARK CVMX_SLI_PKT_OUTPUT_WMARK_FUNC()
static inline uint64_t CVMX_SLI_PKT_OUTPUT_WMARK_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001180ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000029180ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000001180ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000029180ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PKT_OUTPUT_WMARK not supported on this chip\n");
	return 0x0000000000029180ull;
}
#else
#define CVMX_SLI_PKT_OUTPUT_WMARK CVMX_SLI_PKT_OUTPUT_WMARK_FUNC()
static inline uint64_t CVMX_SLI_PKT_OUTPUT_WMARK_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001180ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000029180ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000001180ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000029180ull;
	}
	return 0x0000000000029180ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_OUT_BMODE CVMX_SLI_PKT_OUT_BMODE_FUNC()
static inline uint64_t CVMX_SLI_PKT_OUT_BMODE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_OUT_BMODE not supported on this chip\n");
	return 0x00000000000010D0ull;
}
#else
#define CVMX_SLI_PKT_OUT_BMODE (0x00000000000010D0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_OUT_BP_EN CVMX_SLI_PKT_OUT_BP_EN_FUNC()
static inline uint64_t CVMX_SLI_PKT_OUT_BP_EN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_SLI_PKT_OUT_BP_EN not supported on this chip\n");
	return 0x0000000000001240ull;
}
#else
#define CVMX_SLI_PKT_OUT_BP_EN (0x0000000000001240ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_OUT_BP_EN2_W1C CVMX_SLI_PKT_OUT_BP_EN2_W1C_FUNC()
static inline uint64_t CVMX_SLI_PKT_OUT_BP_EN2_W1C_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SLI_PKT_OUT_BP_EN2_W1C not supported on this chip\n");
	return 0x0000000000029290ull;
}
#else
#define CVMX_SLI_PKT_OUT_BP_EN2_W1C (0x0000000000029290ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_OUT_BP_EN2_W1S CVMX_SLI_PKT_OUT_BP_EN2_W1S_FUNC()
static inline uint64_t CVMX_SLI_PKT_OUT_BP_EN2_W1S_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_SLI_PKT_OUT_BP_EN2_W1S not supported on this chip\n");
	return 0x0000000000029270ull;
}
#else
#define CVMX_SLI_PKT_OUT_BP_EN2_W1S (0x0000000000029270ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_OUT_BP_EN_W1C CVMX_SLI_PKT_OUT_BP_EN_W1C_FUNC()
static inline uint64_t CVMX_SLI_PKT_OUT_BP_EN_W1C_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SLI_PKT_OUT_BP_EN_W1C not supported on this chip\n");
	return 0x0000000000029280ull;
}
#else
#define CVMX_SLI_PKT_OUT_BP_EN_W1C (0x0000000000029280ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_OUT_BP_EN_W1S CVMX_SLI_PKT_OUT_BP_EN_W1S_FUNC()
static inline uint64_t CVMX_SLI_PKT_OUT_BP_EN_W1S_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SLI_PKT_OUT_BP_EN_W1S not supported on this chip\n");
	return 0x0000000000029260ull;
}
#else
#define CVMX_SLI_PKT_OUT_BP_EN_W1S (0x0000000000029260ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_OUT_ENB CVMX_SLI_PKT_OUT_ENB_FUNC()
static inline uint64_t CVMX_SLI_PKT_OUT_ENB_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_OUT_ENB not supported on this chip\n");
	return 0x0000000000001010ull;
}
#else
#define CVMX_SLI_PKT_OUT_ENB (0x0000000000001010ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_PCIE_PORT CVMX_SLI_PKT_PCIE_PORT_FUNC()
static inline uint64_t CVMX_SLI_PKT_PCIE_PORT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_PCIE_PORT not supported on this chip\n");
	return 0x00000000000010E0ull;
}
#else
#define CVMX_SLI_PKT_PCIE_PORT (0x00000000000010E0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_PKIND_VALID CVMX_SLI_PKT_PKIND_VALID_FUNC()
static inline uint64_t CVMX_SLI_PKT_PKIND_VALID_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SLI_PKT_PKIND_VALID not supported on this chip\n");
	return 0x0000000000029190ull;
}
#else
#define CVMX_SLI_PKT_PKIND_VALID (0x0000000000029190ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_PORT_IN_RST CVMX_SLI_PKT_PORT_IN_RST_FUNC()
static inline uint64_t CVMX_SLI_PKT_PORT_IN_RST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_PORT_IN_RST not supported on this chip\n");
	return 0x00000000000011F0ull;
}
#else
#define CVMX_SLI_PKT_PORT_IN_RST (0x00000000000011F0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_RING_RST CVMX_SLI_PKT_RING_RST_FUNC()
static inline uint64_t CVMX_SLI_PKT_RING_RST_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x00000000000291E0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x00000000000011E0ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000291E0ull;
			break;

			break;
	}
	cvmx_warn("CVMX_SLI_PKT_RING_RST not supported on this chip\n");
	return 0x00000000000291E0ull;
}
#else
#define CVMX_SLI_PKT_RING_RST CVMX_SLI_PKT_RING_RST_FUNC()
static inline uint64_t CVMX_SLI_PKT_RING_RST_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000000000291E0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000000000011E0ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000291E0ull;

	}
	return 0x00000000000291E0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_SLIST_ES CVMX_SLI_PKT_SLIST_ES_FUNC()
static inline uint64_t CVMX_SLI_PKT_SLIST_ES_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_SLIST_ES not supported on this chip\n");
	return 0x0000000000001050ull;
}
#else
#define CVMX_SLI_PKT_SLIST_ES (0x0000000000001050ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_SLIST_NS CVMX_SLI_PKT_SLIST_NS_FUNC()
static inline uint64_t CVMX_SLI_PKT_SLIST_NS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_SLIST_NS not supported on this chip\n");
	return 0x0000000000001040ull;
}
#else
#define CVMX_SLI_PKT_SLIST_NS (0x0000000000001040ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_SLIST_ROR CVMX_SLI_PKT_SLIST_ROR_FUNC()
static inline uint64_t CVMX_SLI_PKT_SLIST_ROR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_SLIST_ROR not supported on this chip\n");
	return 0x0000000000001030ull;
}
#else
#define CVMX_SLI_PKT_SLIST_ROR (0x0000000000001030ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_TIME_INT CVMX_SLI_PKT_TIME_INT_FUNC()
static inline uint64_t CVMX_SLI_PKT_TIME_INT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001140ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000029140ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000001140ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000029140ull;
			break;
	}
	cvmx_warn("CVMX_SLI_PKT_TIME_INT not supported on this chip\n");
	return 0x0000000000029140ull;
}
#else
#define CVMX_SLI_PKT_TIME_INT CVMX_SLI_PKT_TIME_INT_FUNC()
static inline uint64_t CVMX_SLI_PKT_TIME_INT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001140ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000029140ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000001140ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000029140ull;
	}
	return 0x0000000000029140ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_TIME_INT_ENB CVMX_SLI_PKT_TIME_INT_ENB_FUNC()
static inline uint64_t CVMX_SLI_PKT_TIME_INT_ENB_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_TIME_INT_ENB not supported on this chip\n");
	return 0x0000000000001160ull;
}
#else
#define CVMX_SLI_PKT_TIME_INT_ENB (0x0000000000001160ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PORTX_PKIND(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SLI_PORTX_PKIND(%lu) is invalid on this chip\n", offset);
	return 0x0000000000000800ull + ((offset) & 31) * 16;
}
#else
#define CVMX_SLI_PORTX_PKIND(offset) (0x0000000000000800ull + ((offset) & 31) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PP_PKT_CSR_CONTROL CVMX_SLI_PP_PKT_CSR_CONTROL_FUNC()
static inline uint64_t CVMX_SLI_PP_PKT_CSR_CONTROL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SLI_PP_PKT_CSR_CONTROL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F00000282D0ull);
}
#else
#define CVMX_SLI_PP_PKT_CSR_CONTROL (CVMX_ADD_IO_SEG(0x00011F00000282D0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_S2C_END_MERGE CVMX_SLI_S2C_END_MERGE_FUNC()
static inline uint64_t CVMX_SLI_S2C_END_MERGE_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return CVMX_ADD_IO_SEG(0x00011F0000025000ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return CVMX_ADD_IO_SEG(0x00011F0000015000ull);
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011F0000025000ull);
			break;

			break;
	}
	cvmx_warn("CVMX_SLI_S2C_END_MERGE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F0000025000ull);
}
#else
#define CVMX_SLI_S2C_END_MERGE CVMX_SLI_S2C_END_MERGE_FUNC()
static inline uint64_t CVMX_SLI_S2C_END_MERGE_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x00011F0000025000ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x00011F0000015000ull);
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011F0000025000ull);

	}
	return CVMX_ADD_IO_SEG(0x00011F0000025000ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_S2M_PORTX_CTL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000003D80ull + ((offset) & 1) * 16;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return 0x0000000000003D80ull + ((offset) & 3) * 16;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000000000013D80ull + ((offset) & 3) * 16;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000000000023D80ull + ((offset) & 3) * 16;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000000000003D80ull + ((offset) & 3) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return 0x0000000000023D80ull + ((offset) & 3) * 16;
			break;
	}
	cvmx_warn("CVMX_SLI_S2M_PORTX_CTL (offset = %lu) not supported on this chip\n", offset);
	return 0x0000000000023D80ull + ((offset) & 3) * 16;
}
#else
static inline uint64_t CVMX_SLI_S2M_PORTX_CTL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003D80ull + (offset) * 16;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000003D80ull + (offset) * 16;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000000000013D80ull + (offset) * 16;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000023D80ull + (offset) * 16;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000003D80ull + (offset) * 16;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000023D80ull + (offset) * 16;
	}
	return 0x0000000000023D80ull + (offset) * 16;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_SCRATCH_1 CVMX_SLI_SCRATCH_1_FUNC()
static inline uint64_t CVMX_SLI_SCRATCH_1_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000003C0ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x00000000000283C0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x00000000000003C0ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000283C0ull;
			break;
	}
	cvmx_warn("CVMX_SLI_SCRATCH_1 not supported on this chip\n");
	return 0x00000000000283C0ull;
}
#else
#define CVMX_SLI_SCRATCH_1 CVMX_SLI_SCRATCH_1_FUNC()
static inline uint64_t CVMX_SLI_SCRATCH_1_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000003C0ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000000000283C0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000000000003C0ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000283C0ull;
	}
	return 0x00000000000283C0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_SCRATCH_2 CVMX_SLI_SCRATCH_2_FUNC()
static inline uint64_t CVMX_SLI_SCRATCH_2_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000003D0ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x00000000000283D0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x00000000000003D0ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000283D0ull;
			break;
	}
	cvmx_warn("CVMX_SLI_SCRATCH_2 not supported on this chip\n");
	return 0x00000000000283D0ull;
}
#else
#define CVMX_SLI_SCRATCH_2 CVMX_SLI_SCRATCH_2_FUNC()
static inline uint64_t CVMX_SLI_SCRATCH_2_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000003D0ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000000000283D0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000000000003D0ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000283D0ull;
	}
	return 0x00000000000283D0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_STATE1 CVMX_SLI_STATE1_FUNC()
static inline uint64_t CVMX_SLI_STATE1_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000620ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000028620ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000000620ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028620ull;
			break;
	}
	cvmx_warn("CVMX_SLI_STATE1 not supported on this chip\n");
	return 0x0000000000028620ull;
}
#else
#define CVMX_SLI_STATE1 CVMX_SLI_STATE1_FUNC()
static inline uint64_t CVMX_SLI_STATE1_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000620ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000028620ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000620ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028620ull;
	}
	return 0x0000000000028620ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_STATE2 CVMX_SLI_STATE2_FUNC()
static inline uint64_t CVMX_SLI_STATE2_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000630ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000028630ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000000630ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028630ull;
			break;
	}
	cvmx_warn("CVMX_SLI_STATE2 not supported on this chip\n");
	return 0x0000000000028630ull;
}
#else
#define CVMX_SLI_STATE2 CVMX_SLI_STATE2_FUNC()
static inline uint64_t CVMX_SLI_STATE2_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000630ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000028630ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000630ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028630ull;
	}
	return 0x0000000000028630ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_STATE3 CVMX_SLI_STATE3_FUNC()
static inline uint64_t CVMX_SLI_STATE3_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000640ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000028640ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000000640ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028640ull;
			break;
	}
	cvmx_warn("CVMX_SLI_STATE3 not supported on this chip\n");
	return 0x0000000000028640ull;
}
#else
#define CVMX_SLI_STATE3 CVMX_SLI_STATE3_FUNC()
static inline uint64_t CVMX_SLI_STATE3_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000640ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000028640ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000640ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000028640ull;
	}
	return 0x0000000000028640ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_TX_PIPE CVMX_SLI_TX_PIPE_FUNC()
static inline uint64_t CVMX_SLI_TX_PIPE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SLI_TX_PIPE not supported on this chip\n");
	return 0x0000000000001230ull;
}
#else
#define CVMX_SLI_TX_PIPE (0x0000000000001230ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_WINDOW_CTL CVMX_SLI_WINDOW_CTL_FUNC()
static inline uint64_t CVMX_SLI_WINDOW_CTL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000002E0ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x00000000000282E0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x00000000000002E0ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000282E0ull;
			break;
	}
	cvmx_warn("CVMX_SLI_WINDOW_CTL not supported on this chip\n");
	return 0x00000000000282E0ull;
}
#else
#define CVMX_SLI_WINDOW_CTL CVMX_SLI_WINDOW_CTL_FUNC()
static inline uint64_t CVMX_SLI_WINDOW_CTL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000002E0ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000000000282E0ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000000000002E0ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000000000282E0ull;
	}
	return 0x00000000000282E0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_WIN_RD_ADDR CVMX_SLI_WIN_RD_ADDR_FUNC()
static inline uint64_t CVMX_SLI_WIN_RD_ADDR_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000010ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000020010ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000000010ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000020010ull;
			break;
	}
	cvmx_warn("CVMX_SLI_WIN_RD_ADDR not supported on this chip\n");
	return 0x0000000000020010ull;
}
#else
#define CVMX_SLI_WIN_RD_ADDR CVMX_SLI_WIN_RD_ADDR_FUNC()
static inline uint64_t CVMX_SLI_WIN_RD_ADDR_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000010ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000020010ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000010ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000020010ull;
	}
	return 0x0000000000020010ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_WIN_RD_DATA CVMX_SLI_WIN_RD_DATA_FUNC()
static inline uint64_t CVMX_SLI_WIN_RD_DATA_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000040ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000020040ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000000040ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000020040ull;
			break;
	}
	cvmx_warn("CVMX_SLI_WIN_RD_DATA not supported on this chip\n");
	return 0x0000000000020040ull;
}
#else
#define CVMX_SLI_WIN_RD_DATA CVMX_SLI_WIN_RD_DATA_FUNC()
static inline uint64_t CVMX_SLI_WIN_RD_DATA_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000040ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000020040ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000040ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000020040ull;
	}
	return 0x0000000000020040ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_WIN_WR_ADDR CVMX_SLI_WIN_WR_ADDR_FUNC()
static inline uint64_t CVMX_SLI_WIN_WR_ADDR_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000020000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000000000ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000020000ull;
			break;
	}
	cvmx_warn("CVMX_SLI_WIN_WR_ADDR not supported on this chip\n");
	return 0x0000000000020000ull;
}
#else
#define CVMX_SLI_WIN_WR_ADDR CVMX_SLI_WIN_WR_ADDR_FUNC()
static inline uint64_t CVMX_SLI_WIN_WR_ADDR_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000020000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000000ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000020000ull;
	}
	return 0x0000000000020000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_WIN_WR_DATA CVMX_SLI_WIN_WR_DATA_FUNC()
static inline uint64_t CVMX_SLI_WIN_WR_DATA_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000020ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000020020ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000000020ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000020020ull;
			break;
	}
	cvmx_warn("CVMX_SLI_WIN_WR_DATA not supported on this chip\n");
	return 0x0000000000020020ull;
}
#else
#define CVMX_SLI_WIN_WR_DATA CVMX_SLI_WIN_WR_DATA_FUNC()
static inline uint64_t CVMX_SLI_WIN_WR_DATA_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000020ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000020020ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000020ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000020020ull;
	}
	return 0x0000000000020020ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_WIN_WR_MASK CVMX_SLI_WIN_WR_MASK_FUNC()
static inline uint64_t CVMX_SLI_WIN_WR_MASK_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000030ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return 0x0000000000020030ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return 0x0000000000000030ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000020030ull;
			break;
	}
	cvmx_warn("CVMX_SLI_WIN_WR_MASK not supported on this chip\n");
	return 0x0000000000020030ull;
}
#else
#define CVMX_SLI_WIN_WR_MASK CVMX_SLI_WIN_WR_MASK_FUNC()
static inline uint64_t CVMX_SLI_WIN_WR_MASK_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000030ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000000000020030ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000000000000030ull;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000000000020030ull;
	}
	return 0x0000000000020030ull;
}
#endif

/**
 * cvmx_sli_bist_status
 *
 * This register contains results from BIST runs of MAC's memories: 0 = pass (or BIST in
 * progress/never run), 1 = fail.
 */
union cvmx_sli_bist_status {
	uint64_t u64;
	struct cvmx_sli_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t ncb_req                      : 1;  /**< BIST status for IOI Request FIFO. */
	uint64_t n2p0_c                       : 1;  /**< BIST status for N2P Port0 cmd. */
	uint64_t n2p0_o                       : 1;  /**< BIST status for N2P Port0 data. */
	uint64_t n2p1_c                       : 1;  /**< BIST status for N2P Port1 cmd. */
	uint64_t n2p1_o                       : 1;  /**< BIST status for N2P Port1 data. */
	uint64_t cpl_p0                       : 1;  /**< BIST status for CPL Port 0. */
	uint64_t cpl_p1                       : 1;  /**< BIST status for CPL Port 1. */
	uint64_t reserved_19_24               : 6;
	uint64_t p2n0_c0                      : 1;  /**< BIST status for P2N port0 C0. */
	uint64_t p2n0_c1                      : 1;  /**< BIST status for P2N port0 C1. */
	uint64_t p2n0_n                       : 1;  /**< BIST status for P2N port0 N. */
	uint64_t p2n0_p0                      : 1;  /**< BIST status for P2N port0 P0. */
	uint64_t p2n0_p1                      : 1;  /**< BIST status for P2N port0 P1. */
	uint64_t p2n1_c0                      : 1;  /**< BIST status for P2N port1 C0. */
	uint64_t p2n1_c1                      : 1;  /**< BIST status for P2N port1 C1. */
	uint64_t p2n1_n                       : 1;  /**< BIST status for P2N port1 N. */
	uint64_t p2n1_p0                      : 1;  /**< BIST status for P2N port1 P0. */
	uint64_t p2n1_p1                      : 1;  /**< BIST status for P2N port1 P1. */
	uint64_t reserved_6_8                 : 3;
	uint64_t dsi1_1                       : 1;  /**< BIST status for DSI1 memory 1. */
	uint64_t dsi1_0                       : 1;  /**< BIST status for DSI1 memory 0. */
	uint64_t dsi0_1                       : 1;  /**< BIST status for DSI0 memory 1. */
	uint64_t dsi0_0                       : 1;  /**< BIST status for DSI0 memory 0. */
	uint64_t msi                          : 1;  /**< BIST status for MSI memory map. */
	uint64_t ncb_cmd                      : 1;  /**< BIST status for IOI outbound commands. */
#else
	uint64_t ncb_cmd                      : 1;
	uint64_t msi                          : 1;
	uint64_t dsi0_0                       : 1;
	uint64_t dsi0_1                       : 1;
	uint64_t dsi1_0                       : 1;
	uint64_t dsi1_1                       : 1;
	uint64_t reserved_6_8                 : 3;
	uint64_t p2n1_p1                      : 1;
	uint64_t p2n1_p0                      : 1;
	uint64_t p2n1_n                       : 1;
	uint64_t p2n1_c1                      : 1;
	uint64_t p2n1_c0                      : 1;
	uint64_t p2n0_p1                      : 1;
	uint64_t p2n0_p0                      : 1;
	uint64_t p2n0_n                       : 1;
	uint64_t p2n0_c1                      : 1;
	uint64_t p2n0_c0                      : 1;
	uint64_t reserved_19_24               : 6;
	uint64_t cpl_p1                       : 1;
	uint64_t cpl_p0                       : 1;
	uint64_t n2p1_o                       : 1;
	uint64_t n2p1_c                       : 1;
	uint64_t n2p0_o                       : 1;
	uint64_t n2p0_c                       : 1;
	uint64_t ncb_req                      : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_bist_status_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t n2p0_c                       : 1;  /**< BIST Status for N2P Port0 Cmd */
	uint64_t n2p0_o                       : 1;  /**< BIST Status for N2P Port0 Data */
	uint64_t reserved_27_28               : 2;
	uint64_t cpl_p0                       : 1;  /**< BIST Status for CPL Port 0 */
	uint64_t cpl_p1                       : 1;  /**< BIST Status for CPL Port 1 */
	uint64_t reserved_19_24               : 6;
	uint64_t p2n0_c0                      : 1;  /**< BIST Status for P2N Port0 C0 */
	uint64_t p2n0_c1                      : 1;  /**< BIST Status for P2N Port0 C1 */
	uint64_t p2n0_n                       : 1;  /**< BIST Status for P2N Port0 N */
	uint64_t p2n0_p0                      : 1;  /**< BIST Status for P2N Port0 P0 */
	uint64_t p2n0_p1                      : 1;  /**< BIST Status for P2N Port0 P1 */
	uint64_t p2n1_c0                      : 1;  /**< BIST Status for P2N Port1 C0 */
	uint64_t p2n1_c1                      : 1;  /**< BIST Status for P2N Port1 C1 */
	uint64_t p2n1_n                       : 1;  /**< BIST Status for P2N Port1 N */
	uint64_t p2n1_p0                      : 1;  /**< BIST Status for P2N Port1 P0 */
	uint64_t p2n1_p1                      : 1;  /**< BIST Status for P2N Port1 P1 */
	uint64_t reserved_6_8                 : 3;
	uint64_t dsi1_1                       : 1;  /**< BIST Status for DSI1 Memory 1 */
	uint64_t dsi1_0                       : 1;  /**< BIST Status for DSI1 Memory 0 */
	uint64_t dsi0_1                       : 1;  /**< BIST Status for DSI0 Memory 1 */
	uint64_t dsi0_0                       : 1;  /**< BIST Status for DSI0 Memory 0 */
	uint64_t msi                          : 1;  /**< BIST Status for MSI Memory Map */
	uint64_t ncb_cmd                      : 1;  /**< BIST Status for NCB Outbound Commands */
#else
	uint64_t ncb_cmd                      : 1;
	uint64_t msi                          : 1;
	uint64_t dsi0_0                       : 1;
	uint64_t dsi0_1                       : 1;
	uint64_t dsi1_0                       : 1;
	uint64_t dsi1_1                       : 1;
	uint64_t reserved_6_8                 : 3;
	uint64_t p2n1_p1                      : 1;
	uint64_t p2n1_p0                      : 1;
	uint64_t p2n1_n                       : 1;
	uint64_t p2n1_c1                      : 1;
	uint64_t p2n1_c0                      : 1;
	uint64_t p2n0_p1                      : 1;
	uint64_t p2n0_p0                      : 1;
	uint64_t p2n0_n                       : 1;
	uint64_t p2n0_c1                      : 1;
	uint64_t p2n0_c0                      : 1;
	uint64_t reserved_19_24               : 6;
	uint64_t cpl_p1                       : 1;
	uint64_t cpl_p0                       : 1;
	uint64_t reserved_27_28               : 2;
	uint64_t n2p0_o                       : 1;
	uint64_t n2p0_c                       : 1;
	uint64_t reserved_31_63               : 33;
#endif
	} cn61xx;
	struct cvmx_sli_bist_status_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t n2p0_c                       : 1;  /**< BIST Status for N2P Port0 Cmd */
	uint64_t n2p0_o                       : 1;  /**< BIST Status for N2P Port0 Data */
	uint64_t n2p1_c                       : 1;  /**< BIST Status for N2P Port1 Cmd */
	uint64_t n2p1_o                       : 1;  /**< BIST Status for N2P Port1 Data */
	uint64_t cpl_p0                       : 1;  /**< BIST Status for CPL Port 0 */
	uint64_t cpl_p1                       : 1;  /**< BIST Status for CPL Port 1 */
	uint64_t reserved_19_24               : 6;
	uint64_t p2n0_c0                      : 1;  /**< BIST Status for P2N Port0 C0 */
	uint64_t p2n0_c1                      : 1;  /**< BIST Status for P2N Port0 C1 */
	uint64_t p2n0_n                       : 1;  /**< BIST Status for P2N Port0 N */
	uint64_t p2n0_p0                      : 1;  /**< BIST Status for P2N Port0 P0 */
	uint64_t p2n0_p1                      : 1;  /**< BIST Status for P2N Port0 P1 */
	uint64_t p2n1_c0                      : 1;  /**< BIST Status for P2N Port1 C0 */
	uint64_t p2n1_c1                      : 1;  /**< BIST Status for P2N Port1 C1 */
	uint64_t p2n1_n                       : 1;  /**< BIST Status for P2N Port1 N */
	uint64_t p2n1_p0                      : 1;  /**< BIST Status for P2N Port1 P0 */
	uint64_t p2n1_p1                      : 1;  /**< BIST Status for P2N Port1 P1 */
	uint64_t reserved_6_8                 : 3;
	uint64_t dsi1_1                       : 1;  /**< BIST Status for DSI1 Memory 1 */
	uint64_t dsi1_0                       : 1;  /**< BIST Status for DSI1 Memory 0 */
	uint64_t dsi0_1                       : 1;  /**< BIST Status for DSI0 Memory 1 */
	uint64_t dsi0_0                       : 1;  /**< BIST Status for DSI0 Memory 0 */
	uint64_t msi                          : 1;  /**< BIST Status for MSI Memory Map */
	uint64_t ncb_cmd                      : 1;  /**< BIST Status for NCB Outbound Commands */
#else
	uint64_t ncb_cmd                      : 1;
	uint64_t msi                          : 1;
	uint64_t dsi0_0                       : 1;
	uint64_t dsi0_1                       : 1;
	uint64_t dsi1_0                       : 1;
	uint64_t dsi1_1                       : 1;
	uint64_t reserved_6_8                 : 3;
	uint64_t p2n1_p1                      : 1;
	uint64_t p2n1_p0                      : 1;
	uint64_t p2n1_n                       : 1;
	uint64_t p2n1_c1                      : 1;
	uint64_t p2n1_c0                      : 1;
	uint64_t p2n0_p1                      : 1;
	uint64_t p2n0_p0                      : 1;
	uint64_t p2n0_n                       : 1;
	uint64_t p2n0_c1                      : 1;
	uint64_t p2n0_c0                      : 1;
	uint64_t reserved_19_24               : 6;
	uint64_t cpl_p1                       : 1;
	uint64_t cpl_p0                       : 1;
	uint64_t n2p1_o                       : 1;
	uint64_t n2p1_c                       : 1;
	uint64_t n2p0_o                       : 1;
	uint64_t n2p0_c                       : 1;
	uint64_t reserved_31_63               : 33;
#endif
	} cn63xx;
	struct cvmx_sli_bist_status_cn63xx    cn63xxp1;
	struct cvmx_sli_bist_status_cn61xx    cn66xx;
	struct cvmx_sli_bist_status_s         cn68xx;
	struct cvmx_sli_bist_status_s         cn68xxp1;
	struct cvmx_sli_bist_status_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t n2p0_c                       : 1;  /**< BIST Status for N2P Port0 Cmd */
	uint64_t n2p0_o                       : 1;  /**< BIST Status for N2P Port0 Data */
	uint64_t reserved_27_28               : 2;
	uint64_t cpl_p0                       : 1;  /**< BIST Status for CPL Port 0 */
	uint64_t cpl_p1                       : 1;  /**< BIST Status for CPL Port 1 */
	uint64_t reserved_19_24               : 6;
	uint64_t p2n0_c0                      : 1;  /**< BIST Status for P2N Port0 C0 */
	uint64_t reserved_17_17               : 1;
	uint64_t p2n0_n                       : 1;  /**< BIST Status for P2N Port0 N */
	uint64_t p2n0_p0                      : 1;  /**< BIST Status for P2N Port0 P0 */
	uint64_t reserved_14_14               : 1;
	uint64_t p2n1_c0                      : 1;  /**< BIST Status for P2N Port1 C0 */
	uint64_t reserved_12_12               : 1;
	uint64_t p2n1_n                       : 1;  /**< BIST Status for P2N Port1 N */
	uint64_t p2n1_p0                      : 1;  /**< BIST Status for P2N Port1 P0 */
	uint64_t reserved_6_9                 : 4;
	uint64_t dsi1_1                       : 1;  /**< BIST Status for DSI1 Memory 1 */
	uint64_t dsi1_0                       : 1;  /**< BIST Status for DSI1 Memory 0 */
	uint64_t dsi0_1                       : 1;  /**< BIST Status for DSI0 Memory 1 */
	uint64_t dsi0_0                       : 1;  /**< BIST Status for DSI0 Memory 0 */
	uint64_t msi                          : 1;  /**< BIST Status for MSI Memory Map */
	uint64_t ncb_cmd                      : 1;  /**< BIST Status for NCB Outbound Commands */
#else
	uint64_t ncb_cmd                      : 1;
	uint64_t msi                          : 1;
	uint64_t dsi0_0                       : 1;
	uint64_t dsi0_1                       : 1;
	uint64_t dsi1_0                       : 1;
	uint64_t dsi1_1                       : 1;
	uint64_t reserved_6_9                 : 4;
	uint64_t p2n1_p0                      : 1;
	uint64_t p2n1_n                       : 1;
	uint64_t reserved_12_12               : 1;
	uint64_t p2n1_c0                      : 1;
	uint64_t reserved_14_14               : 1;
	uint64_t p2n0_p0                      : 1;
	uint64_t p2n0_n                       : 1;
	uint64_t reserved_17_17               : 1;
	uint64_t p2n0_c0                      : 1;
	uint64_t reserved_19_24               : 6;
	uint64_t cpl_p1                       : 1;
	uint64_t cpl_p0                       : 1;
	uint64_t reserved_27_28               : 2;
	uint64_t n2p0_o                       : 1;
	uint64_t n2p0_c                       : 1;
	uint64_t reserved_31_63               : 33;
#endif
	} cn70xx;
	struct cvmx_sli_bist_status_cn70xx    cn70xxp1;
	struct cvmx_sli_bist_status_s         cn73xx;
	struct cvmx_sli_bist_status_s         cn78xx;
	struct cvmx_sli_bist_status_s         cn78xxp1;
	struct cvmx_sli_bist_status_cn61xx    cnf71xx;
	struct cvmx_sli_bist_status_s         cnf75xx;
};
typedef union cvmx_sli_bist_status cvmx_sli_bist_status_t;

/**
 * cvmx_sli_ciu_int_enb
 *
 * Interrupt enable register for a given SLI_CIU_INT_SUM register.
 *
 */
union cvmx_sli_ciu_int_enb {
	uint64_t u64;
	struct cvmx_sli_ciu_int_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_51_63               : 13;
	uint64_t m3_un_wi                     : 1;  /**< Enables SLI_CIU_INT_SUM[M3_UN_WI] to generate an interrupt to the CIU. */
	uint64_t m3_un_b0                     : 1;  /**< Enables SLI_CIU_INT_SUM[M3_UN_B0] to generate an interrupt to the CIU. */
	uint64_t m3_up_wi                     : 1;  /**< Enables SLI_CIU_INT_SUM[M3_UP_WI] to generate an interrupt to the CIU. */
	uint64_t m3_up_b0                     : 1;  /**< Enables SLI_CIU_INT_SUM[M3_UP_B0] to generate an interrupt to the CIU. */
	uint64_t m2_un_wi                     : 1;  /**< Enables SLI_CIU_INT_SUM[M2_UN_WI] to generate an interrupt to the CIU. */
	uint64_t m2_un_b0                     : 1;  /**< Enables SLI_CIU_INT_SUM[M2_UN_B0] to generate an interrupt to the CIU. */
	uint64_t m2_up_wi                     : 1;  /**< Enables SLI_CIU_INT_SUM[M2_UP_WI] to generate an interrupt to the CIU. */
	uint64_t m2_up_b0                     : 1;  /**< Enables SLI_CIU_INT_SUM[M2_UP_B0] to generate an interrupt to the CIU. */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_CIU_INT_SUM[M1_UN_WI] to generate an interrupt to the CIU. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_CIU_INT_SUM[M1_UN_B0] to generate an interrupt to the CIU. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_CIU_INT_SUM[M1_UP_WI] to generate an interrupt to the CIU. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_CIU_INT_SUM[M1_UP_B0] to generate an interrupt to the CIU. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_CIU_INT_SUM[M0_UN_WI] to generate an interrupt to the CIU. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_CIU_INT_SUM[M0_UN_B0] to generate an interrupt to the CIU. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_CIU_INT_SUM[M0_UP_WI] to generate an interrupt to the CIU. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_CIU_INT_SUM[M0_UP_B0] to generate an interrupt to the CIU. */
	uint64_t m3p0_pppf_err                : 1;  /**< Enables SLI_CIU_INT_SUM[M3P0_PPPF_ERR] to generate an interrupt to the CIU. */
	uint64_t m3p0_pktpf_err               : 1;  /**< Enables SLI_CIU_INT_SUM[M3P0_PKTPF_ERR] to generate an interrupt to the CIU. */
	uint64_t m3p0_dmapf_err               : 1;  /**< Enables SLI_CIU_INT_SUM[M3P0_DMAPF_ERR] to generate an interrupt to the CIU. */
	uint64_t m2p0_pppf_err                : 1;  /**< Enables SLI_CIU_INT_SUM[M2P0_PPPF_ERR] to generate an interrupt to the CIU. */
	uint64_t m2p0_ppvf_err                : 1;  /**< Enables SLI_CIU_INT_SUM[M2P0_PPVF_ERR] to generate an interrupt to the CIU. */
	uint64_t m2p0_pktpf_err               : 1;  /**< Enables SLI_CIU_INT_SUM[M2P0_PKTPF_ERR] to generate an interrupt to the CIU. */
	uint64_t m2p0_pktvf_err               : 1;  /**< Enables SLI_CIU_INT_SUM[M2P0_PKTVF_ERR] to generate an interrupt to the CIU. */
	uint64_t m2p0_dmapf_err               : 1;  /**< Enables SLI_CIU_INT_SUM[M2P0_DMAPF_ERR] to generate an interrupt to the CIU. */
	uint64_t m2p0_dmavf_err               : 1;  /**< Enables SLI_CIU_INT_SUM[M2P0_DMAVF_ERR] to generate an interrupt to the CIU. */
	uint64_t m1p0_pppf_err                : 1;  /**< Enables SLI_CIU_INT_SUM[M1P0_PPPF_ERR] to generate an interrupt to the CIU. */
	uint64_t m1p0_pktpf_err               : 1;  /**< Enables SLI_CIU_INT_SUM[M1P0_PKTPF_ERR] to generate an interrupt to the CIU. */
	uint64_t m1p0_dmapf_err               : 1;  /**< Enables SLI_CIU_INT_SUM[M1P0_DMAPF_ERR] to generate an interrupt to the CIU. */
	uint64_t m0p1_pppf_err                : 1;  /**< Not used by hardware. */
	uint64_t m0p1_ppvf_err                : 1;  /**< Not used by hardware. */
	uint64_t m0p1_pktpf_err               : 1;  /**< Not used by hardware. */
	uint64_t m0p1_pktvf_err               : 1;  /**< Not used by hardware. */
	uint64_t m0p1_dmapf_err               : 1;  /**< Not used by hardware. */
	uint64_t m0p1_dmavf_err               : 1;  /**< Not used by hardware. */
	uint64_t m0p0_pppf_err                : 1;  /**< Enables SLI_CIU_INT_SUM[M0P0_PPPF_ERR] to generate an interrupt to the CIU. */
	uint64_t m0p0_ppvf_err                : 1;  /**< Enables SLI_CIU_INT_SUM[M0P0_PPVF_ERR] to generate an interrupt to the CIU. */
	uint64_t m0p0_pktpf_err               : 1;  /**< Enables SLI_CIU_INT_SUM[M0P0_PKTPF_ERR] to generate an interrupt to the CIU. */
	uint64_t m0p0_pktvf_err               : 1;  /**< Enables SLI_CIU_INT_SUM[M0P0_PKTVF_ERR] to generate an interrupt to the CIU. */
	uint64_t m0p0_dmapf_err               : 1;  /**< Enables SLI_CIU_INT_SUM[M0P0_DMAPF_ERR] to generate an interrupt to the CIU. */
	uint64_t m0p0_dmavf_err               : 1;  /**< Enables SLI_CIU_INT_SUM[M0P0_DMAVF_ERR] to generate an interrupt to the CIU. */
	uint64_t m2v0_flr                     : 1;  /**< Enables SLI_CIU_INT_SUM[M2V0_FLR] to generate an interrupt to the CIU. */
	uint64_t m2p0_flr                     : 1;  /**< Enables SLI_CIU_INT_SUM[M2P0_FLR] to generate an interrupt to the CIU. */
	uint64_t reserved_5_8                 : 4;
	uint64_t m0v1_flr                     : 1;  /**< Not used by hardware. */
	uint64_t m0p1_flr                     : 1;  /**< Not used by hardware. */
	uint64_t m0v0_flr                     : 1;  /**< Enables SLI_CIU_INT_SUM[M0V0_FLR] to generate an interrupt to the CIU. */
	uint64_t m0p0_flr                     : 1;  /**< Enables SLI_CIU_INT_SUM[M0P0_FLR] to generate an interrupt to the CIU. */
	uint64_t rml_to                       : 1;  /**< Enables SLI_CIU_INT_SUM[RML_TO] to generate an interrupt to the CIU. */
#else
	uint64_t rml_to                       : 1;
	uint64_t m0p0_flr                     : 1;
	uint64_t m0v0_flr                     : 1;
	uint64_t m0p1_flr                     : 1;
	uint64_t m0v1_flr                     : 1;
	uint64_t reserved_5_8                 : 4;
	uint64_t m2p0_flr                     : 1;
	uint64_t m2v0_flr                     : 1;
	uint64_t m0p0_dmavf_err               : 1;
	uint64_t m0p0_dmapf_err               : 1;
	uint64_t m0p0_pktvf_err               : 1;
	uint64_t m0p0_pktpf_err               : 1;
	uint64_t m0p0_ppvf_err                : 1;
	uint64_t m0p0_pppf_err                : 1;
	uint64_t m0p1_dmavf_err               : 1;
	uint64_t m0p1_dmapf_err               : 1;
	uint64_t m0p1_pktvf_err               : 1;
	uint64_t m0p1_pktpf_err               : 1;
	uint64_t m0p1_ppvf_err                : 1;
	uint64_t m0p1_pppf_err                : 1;
	uint64_t m1p0_dmapf_err               : 1;
	uint64_t m1p0_pktpf_err               : 1;
	uint64_t m1p0_pppf_err                : 1;
	uint64_t m2p0_dmavf_err               : 1;
	uint64_t m2p0_dmapf_err               : 1;
	uint64_t m2p0_pktvf_err               : 1;
	uint64_t m2p0_pktpf_err               : 1;
	uint64_t m2p0_ppvf_err                : 1;
	uint64_t m2p0_pppf_err                : 1;
	uint64_t m3p0_dmapf_err               : 1;
	uint64_t m3p0_pktpf_err               : 1;
	uint64_t m3p0_pppf_err                : 1;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t reserved_51_63               : 13;
#endif
	} s;
	struct cvmx_sli_ciu_int_enb_s         cn73xx;
	struct cvmx_sli_ciu_int_enb_s         cn78xx;
	struct cvmx_sli_ciu_int_enb_s         cnf75xx;
};
typedef union cvmx_sli_ciu_int_enb cvmx_sli_ciu_int_enb_t;

/**
 * cvmx_sli_ciu_int_sum
 *
 * The fields in this register are set when an interrupt condition occurs; write 1 to clear.
 * A bit set in this register will send and interrupt to CIU
 */
union cvmx_sli_ciu_int_sum {
	uint64_t u64;
	struct cvmx_sli_ciu_int_sum_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_51_63               : 13;
	uint64_t m3_un_wi                     : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, received unsupported N-TLP for window register from MAC 3. This occurs when
                                                         the window
                                                         registers are disabled and a window register access occurs.
                                                         This can only be set by a PF and not a VF access.
                                                         This bit is set when SLI_MAC()_PF()_INT_SUM[UN_WI]
                                                         Throws SLI_INTSN_E::SLI_INT_M3_UN_WI. */
	uint64_t m3_un_b0                     : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, received unsupported N-TLP for Bar 0 from MAC 3. This occurs when the BAR 0
                                                         address space
                                                         is disabled.
                                                         This can only be set by a PF and not a VF access.
                                                         This bit is set when SLI_MAC()_PF()_INT_SUM[UN_B0]
                                                         Throws SLI_INTSN_E::SLI_INT_M3_UN_B0. */
	uint64_t m3_up_wi                     : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, received unsupported P-TLP for window register from MAC 3. This occurs when
                                                         the window
                                                         registers are disabled and a window register access occurs.
                                                         This can only be set by a PF and not a VF access.
                                                         Throws SLI_INTSN_E::SLI_INT_M3_UP_WI. */
	uint64_t m3_up_b0                     : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, received unsupported P-TLP for Bar 0 from MAC 3. This occurs when the BAR 0
                                                         address space
                                                         This can only be set by a PF and not a VF access.
                                                         Throws SLI_INTSN_E::SLI_INT_M3_UP_B0. */
	uint64_t m2_un_wi                     : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, received unsupported N-TLP for window register from MAC 2. This occurs when
                                                         the window
                                                         registers are disabled and a window register access occurs.
                                                         This can only be set by a PF and not a VF access.
                                                         Throws SLI_INTSN_E::SLI_INT_M2_UN_WI. */
	uint64_t m2_un_b0                     : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, received unsupported N-TLP for Bar 0 from MAC 2. This occurs when the BAR 0
                                                         address space
                                                         is disabled.
                                                         This can only be set by a PF and not a VF access.
                                                         Throws SLI_INTSN_E::SLI_INT_M2_UN_B0. */
	uint64_t m2_up_wi                     : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, received unsupported P-TLP for window register from MAC 2. This occurs when
                                                         the window
                                                         registers are disabled and a window register access occurs.
                                                         This can only be set by a PF and not a VF access.
                                                         Throws SLI_INTSN_E::SLI_INT_M2_UP_WI. */
	uint64_t m2_up_b0                     : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, received unsupported P-TLP for Bar 0 from MAC 2. This occurs when the BAR 0
                                                         address space
                                                         This can only be set by a PF and not a VF access.
                                                         Throws SLI_INTSN_E::SLI_INT_M2_UP_B0. */
	uint64_t m1_un_wi                     : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, received unsupported N-TLP for window register from MAC 1. This occurs when
                                                         the window
                                                         registers are disabled and a window register access occurs.
                                                         This can only be set by a PF and not a VF access.
                                                         Throws SLI_INTSN_E::SLI_INT_M1_UN_WI. */
	uint64_t m1_un_b0                     : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, received unsupported N-TLP for Bar 0 from MAC 1. This occurs when the BAR 0
                                                         address space
                                                         is disabled.
                                                         This can only be set by a PF and not a VF access.
                                                         Throws SLI_INTSN_E::SLI_INT_M1_UN_B0. */
	uint64_t m1_up_wi                     : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, received unsupported P-TLP for window register from MAC 1. This occurs when
                                                         the window
                                                         registers are disabled and a window register access occurs.
                                                         This can only be set by a PF and not a VF access.
                                                         Throws SLI_INTSN_E::SLI_INT_M1_UP_WI. */
	uint64_t m1_up_b0                     : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, received unsupported P-TLP for Bar 0 from MAC 1. This occurs when the BAR 0
                                                         address space
                                                         This can only be set by a PF and not a VF access.
                                                         Throws SLI_INTSN_E::SLI_INT_M1_UP_B0. */
	uint64_t m0_un_wi                     : 1;  /**< Received unsupported N-TLP for window register from MAC 0. This occurs when the window
                                                         registers are disabled and a window register access occurs.
                                                         This can only be set by a PF and not a VF access.
                                                         Throws SLI_INTSN_E::SLI_INT_M0_UN_WI. */
	uint64_t m0_un_b0                     : 1;  /**< Received unsupported N-TLP for Bar 0 from MAC 0. This occurs when the BAR 0 address space
                                                         is disabled.
                                                         This can only be set by a PF and not a VF access.
                                                         Throws SLI_INTSN_E::SLI_INT_M0_UN_B0. */
	uint64_t m0_up_wi                     : 1;  /**< Received unsupported P-TLP for window register from MAC 0. This occurs when the window
                                                         registers are disabled and a window register access occurs.
                                                         This can only be set by a PF and not a VF access.
                                                         Throws SLI_INTSN_E::SLI_INT_M0_UP_WI. */
	uint64_t m0_up_b0                     : 1;  /**< Received unsupported P-TLP for Bar 0 from MAC 0. This occurs when the BAR 0 address space
                                                         This can only be set by a PF and not a VF access.
                                                         Throws SLI_INTSN_E::SLI_INT_M0_UP_B0. */
	uint64_t m3p0_pppf_err                : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, on SLI_PORT_E::SRIO1 PF0, when an error response is received for a PF PP
                                                         transaction read,
                                                         this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_M3P0_PPPF_ERR. */
	uint64_t m3p0_pktpf_err               : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, on SLI_PORT_E::SRIO1 PF0, When an error response is received for a PF packet
                                                         transaction
                                                         read or a doorbell overflow for a ring associated with this PF occurs, this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_M3P0_PKTPF_ERR. */
	uint64_t m3p0_dmapf_err               : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, on SLI_PORT_E::SRIO1 PF0, when an error response is received for a PF DMA
                                                         transaction
                                                         read,
                                                         this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_M3P0_DMAPF_ERR. */
	uint64_t m2p0_pppf_err                : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, on SLI_PORT_E::SRIO0 PF0, when an error response is received for a PF PP
                                                         transaction read,
                                                         this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_M2P0_PPPF_ERR. */
	uint64_t m2p0_ppvf_err                : 1;  /**< This interrupt should not occur.
                                                         Throws SLI_INTSN_E::SLI_INT_M2P0_PPVF_ERR. */
	uint64_t m2p0_pktpf_err               : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, on SLI_PORT_E::SRIO0 PF0, When an error response is received for a PF packet
                                                         transaction
                                                         read or a doorbell overflow for a ring associated with this PF occurs, this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_M2P0_PKTPF_ERR. */
	uint64_t m2p0_pktvf_err               : 1;  /**< This interrupt should not occur.
                                                         Throws SLI_INTSN_E::SLI_INT_M2P0_PKTVF_ERR. */
	uint64_t m2p0_dmapf_err               : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, on SLI_PORT_E::SRIO0 PF0, when an error response is received for a PF DMA
                                                         transaction
                                                         read,
                                                         this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_M2P0_DMAPF_ERR. */
	uint64_t m2p0_dmavf_err               : 1;  /**< This interrupt should not occur.
                                                         Throws SLI_INTSN_E::SLI_INT_M2P0_DMAVF_ERR. */
	uint64_t m1p0_pppf_err                : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, on SLI_PORT_E::PEM1 PF0, when an error response is received for a PF PP
                                                         transaction read,
                                                         this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_M1P0_PPPF_ERR. */
	uint64_t m1p0_pktpf_err               : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, on SLI_PORT_E::PEM1 PF0, When an error response is received for a PF packet
                                                         transaction
                                                         read or a doorbell overflow for a ring associated with this PF occurs, this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_M1P0_PKTPF_ERR. */
	uint64_t m1p0_dmapf_err               : 1;  /**< For CNF73XX, this interrupt should not occur.
                                                         For CNF75XX, on SLI_PORT_E::PEM1 PF0, when an error response is received for a PF DMA
                                                         transaction read,
                                                         this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_M1P0_DMAPF_ERR. */
	uint64_t m0p1_pppf_err                : 1;  /**< This interrupt cannot occur.
                                                         Throws SLI_INTSN_E::SLI_INT_M0P1_PPPF_ERR. */
	uint64_t m0p1_ppvf_err                : 1;  /**< This interrupt cannot occur.
                                                         Throws SLI_INTSN_E::SLI_INT_M0P1_PPVF_ERR. */
	uint64_t m0p1_pktpf_err               : 1;  /**< This interrupt cannot occur.
                                                         Throws SLI_INTSN_E::SLI_INT_M0P1_PKTPF_ERR. */
	uint64_t m0p1_pktvf_err               : 1;  /**< This interrupt cannot occur.
                                                         Throws SLI_INTSN_E::SLI_INT_M0P1_PKTVF_ERR. */
	uint64_t m0p1_dmapf_err               : 1;  /**< This interrupt cannot occur.
                                                         Throws SLI_INTSN_E::SLI_INT_M0P1_DMAPF_ERR. */
	uint64_t m0p1_dmavf_err               : 1;  /**< This interrupt cannot occur.
                                                         Throws SLI_INTSN_E::SLI_INT_M0P1_DMAVF_ERR. */
	uint64_t m0p0_pppf_err                : 1;  /**< On SLI_PORT_E::PEM0 PF0, when an error response is received for a PF PP transaction read,
                                                         this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_M0P0_PPPF_ERR. */
	uint64_t m0p0_ppvf_err                : 1;  /**< On SLI_PORT_E::PEM0 PF0, when an error response is received for a VF PP transaction read,
                                                         this bit is set. This bit should be cleared and followed by a read to
                                                         SLI_MAC()_PF()_PP_VF_INT to discover which VF is set.
                                                         Throws SLI_INTSN_E::SLI_INT_M0P0_PPVF_ERR. */
	uint64_t m0p0_pktpf_err               : 1;  /**< On SLI_PORT_E::PEM0 PF0, When an error response is received for a PF packet transaction
                                                         read or a doorbell overflow for a ring associated with this PF occurs, this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_M0P0_PKTPF_ERR. */
	uint64_t m0p0_pktvf_err               : 1;  /**< On SLI_PORT_E::PEM0 PF0, when an error response is received for a VF PP transaction read,
                                                         a doorbell overflow for a ring associated with a VF occurs or an illegal memory access
                                                         from a VF occurs, this bit is set. This bit should be cleared and followed by a read to
                                                         SLI_MAC()_PF()_PKT_VF_INT to discover which VF is set.
                                                         Throws SLI_INTSN_E::SLI_INT_M0P0_PKTVF_ERR. */
	uint64_t m0p0_dmapf_err               : 1;  /**< On SLI_PORT_E::PEM0 PF0, when an error response is received for a PF DMA transaction read,
                                                         this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_M0P0_DMAPF_ERR. */
	uint64_t m0p0_dmavf_err               : 1;  /**< When an error response is received for a VF DMA transaction read on SLI_PORT_E::PEM0 PF0,
                                                         this bit is set. This bit should be cleared and followed by a read to
                                                         SLI_MAC()_PF()_DMA_VF_INT is required to discover which VF is set.
                                                         Throws SLI_INTSN_E::SLI_INT_M0P0_DMAVF_ERR. */
	uint64_t m2v0_flr                     : 1;  /**< This interrupt should not occur.
                                                         Throws SLI_INTSN_E::SLI_INT_M2V0_FLR. */
	uint64_t m2p0_flr                     : 1;  /**< A FLR occurred for SLI_PORT_E::PEM2 PF0. This bit should be cleared and followed by
                                                         a read to SLI_MAC()_PF()_FLR_VF_INT to discover which VF experienced the FLR.
                                                         Throws SLI_INTSN_E::SLI_INT_M2P0_FLR. */
	uint64_t reserved_5_8                 : 4;
	uint64_t m0v1_flr                     : 1;  /**< This interrupt cannot occur.
                                                         Throws SLI_INTSN_E::SLI_INT_M0V1_FLR. */
	uint64_t m0p1_flr                     : 1;  /**< This interrupt cannot occur.
                                                         Throws SLI_INTSN_E::SLI_INT_M0P1_FLR. */
	uint64_t m0v0_flr                     : 1;  /**< An FLR occurred for a VF on SLI_PORT_E::PEM0 PF0.
                                                         This bit should be cleared and followed by a read to SLI_MAC()_PF()_FLR_VF_INT to discover
                                                         which VF experienced the FLR.
                                                         Throws SLI_INTSN_E::SLI_INT_M0V0_FLR. */
	uint64_t m0p0_flr                     : 1;  /**< A FLR occurred for SLI_PORT_E::PEM0 PF0.
                                                         Throws SLI_INTSN_E::SLI_INT_M0P0_FLR. */
	uint64_t rml_to                       : 1;  /**< A read or write transfer to a RSL that did not complete within
                                                         SLI_WINDOW_CTL[TIME] coprocessor-clock cycles
                                                         Throws SLI_INTSN_E::SLI_INT_RML_TO. */
#else
	uint64_t rml_to                       : 1;
	uint64_t m0p0_flr                     : 1;
	uint64_t m0v0_flr                     : 1;
	uint64_t m0p1_flr                     : 1;
	uint64_t m0v1_flr                     : 1;
	uint64_t reserved_5_8                 : 4;
	uint64_t m2p0_flr                     : 1;
	uint64_t m2v0_flr                     : 1;
	uint64_t m0p0_dmavf_err               : 1;
	uint64_t m0p0_dmapf_err               : 1;
	uint64_t m0p0_pktvf_err               : 1;
	uint64_t m0p0_pktpf_err               : 1;
	uint64_t m0p0_ppvf_err                : 1;
	uint64_t m0p0_pppf_err                : 1;
	uint64_t m0p1_dmavf_err               : 1;
	uint64_t m0p1_dmapf_err               : 1;
	uint64_t m0p1_pktvf_err               : 1;
	uint64_t m0p1_pktpf_err               : 1;
	uint64_t m0p1_ppvf_err                : 1;
	uint64_t m0p1_pppf_err                : 1;
	uint64_t m1p0_dmapf_err               : 1;
	uint64_t m1p0_pktpf_err               : 1;
	uint64_t m1p0_pppf_err                : 1;
	uint64_t m2p0_dmavf_err               : 1;
	uint64_t m2p0_dmapf_err               : 1;
	uint64_t m2p0_pktvf_err               : 1;
	uint64_t m2p0_pktpf_err               : 1;
	uint64_t m2p0_ppvf_err                : 1;
	uint64_t m2p0_pppf_err                : 1;
	uint64_t m3p0_dmapf_err               : 1;
	uint64_t m3p0_pktpf_err               : 1;
	uint64_t m3p0_pppf_err                : 1;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t reserved_51_63               : 13;
#endif
	} s;
	struct cvmx_sli_ciu_int_sum_s         cn73xx;
	struct cvmx_sli_ciu_int_sum_s         cn78xx;
	struct cvmx_sli_ciu_int_sum_s         cnf75xx;
};
typedef union cvmx_sli_ciu_int_sum cvmx_sli_ciu_int_sum_t;

/**
 * cvmx_sli_ctl_port#
 *
 * These registers contains control information for access to ports. Indexed by SLI_PORT_E.
 * Note: SLI_CTL_PORT0 controls PF0.
 */
union cvmx_sli_ctl_portx {
	uint64_t u64;
	struct cvmx_sli_ctl_portx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_22_63               : 42;
	uint64_t intd                         : 1;  /**< When '0' Intd wire asserted. Before mapping. */
	uint64_t intc                         : 1;  /**< When '0' Intc wire asserted. Before mapping. */
	uint64_t intb                         : 1;  /**< When '0' Intb wire asserted. Before mapping. */
	uint64_t inta                         : 1;  /**< When '0' Inta wire asserted. Before mapping. */
	uint64_t dis_port                     : 1;  /**< When set, the output to the MAC is disabled. This occurs when the MAC reset line
                                                         transitions from deasserted to asserted. Writing a 1 to this location clears this
                                                         condition when the MAC is no longer in reset and the output to the MAC is at the beginning
                                                         of a transfer. */
	uint64_t waitl_com                    : 1;  /**< When set to 1, causes the SLI to wait for a commit from the L2C before sending additional
                                                         completions to the L2C from the MAC.
                                                         Set this for more conservative behavior. Clear this for more aggressive, higher-
                                                         performance behavior. */
	uint64_t intd_map                     : 2;  /**< Maps INTD to INTA(00), INTB(01), INTC(10) or
                                                         INTD (11). */
	uint64_t intc_map                     : 2;  /**< Maps INTC to INTA(00), INTB(01), INTC(10) or
                                                         INTD (11). */
	uint64_t intb_map                     : 2;  /**< Maps INTB to INTA(00), INTB(01), INTC(10) or
                                                         INTD (11). */
	uint64_t inta_map                     : 2;  /**< Maps INTA to INTA(00), INTB(01), INTC(10) or
                                                         INTD (11). */
	uint64_t ctlp_ro                      : 1;  /**< Relaxed ordering enable for completion TLPS */
	uint64_t reserved_6_6                 : 1;
	uint64_t ptlp_ro                      : 1;  /**< Relaxed ordering enable for posted TLPS */
	uint64_t reserved_1_4                 : 4;
	uint64_t wait_com                     : 1;  /**< Wait for commit. When set to 1, causes the SLI to wait for a commit from the L2C before
                                                         sending additional stores to the L2C from the MAC. The SLI requests a commit on the last
                                                         store if more than one STORE operation is required on the IOI. Most applications will not
                                                         notice a difference, so this bit should not be set. Setting the bit is more conservative
                                                         on ordering, lower performance. */
#else
	uint64_t wait_com                     : 1;
	uint64_t reserved_1_4                 : 4;
	uint64_t ptlp_ro                      : 1;
	uint64_t reserved_6_6                 : 1;
	uint64_t ctlp_ro                      : 1;
	uint64_t inta_map                     : 2;
	uint64_t intb_map                     : 2;
	uint64_t intc_map                     : 2;
	uint64_t intd_map                     : 2;
	uint64_t waitl_com                    : 1;
	uint64_t dis_port                     : 1;
	uint64_t inta                         : 1;
	uint64_t intb                         : 1;
	uint64_t intc                         : 1;
	uint64_t intd                         : 1;
	uint64_t reserved_22_63               : 42;
#endif
	} s;
	struct cvmx_sli_ctl_portx_s           cn61xx;
	struct cvmx_sli_ctl_portx_s           cn63xx;
	struct cvmx_sli_ctl_portx_s           cn63xxp1;
	struct cvmx_sli_ctl_portx_s           cn66xx;
	struct cvmx_sli_ctl_portx_s           cn68xx;
	struct cvmx_sli_ctl_portx_s           cn68xxp1;
	struct cvmx_sli_ctl_portx_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_22_63               : 42;
	uint64_t intd                         : 1;  /**< When '0' Intd wire asserted. Before mapping. */
	uint64_t intc                         : 1;  /**< When '0' Intc wire asserted. Before mapping. */
	uint64_t intb                         : 1;  /**< When '0' Intb wire asserted. Before mapping. */
	uint64_t inta                         : 1;  /**< When '0' Inta wire asserted. Before mapping. */
	uint64_t dis_port                     : 1;  /**< When set the output to the MAC is disabled. This
                                                         occurs when the MAC reset line transitions from
                                                         de-asserted to asserted. Writing a '1' to this
                                                         location will clear this condition when the MAC is
                                                         no longer in reset and the output to the MAC is at
                                                         the begining of a transfer. */
	uint64_t waitl_com                    : 1;  /**< When set '1' casues the SLI to wait for a commit
                                                         from the L2C before sending additional completions
                                                         to the L2C from a MAC.
                                                         Set this for more conservative behavior. Clear
                                                         this for more aggressive, higher-performance
                                                         behavior */
	uint64_t intd_map                     : 2;  /**< Maps INTD to INTA(00), INTB(01), INTC(10) or
                                                         INTD (11). */
	uint64_t intc_map                     : 2;  /**< Maps INTC to INTA(00), INTB(01), INTC(10) or
                                                         INTD (11). */
	uint64_t intb_map                     : 2;  /**< Maps INTB to INTA(00), INTB(01), INTC(10) or
                                                         INTD (11). */
	uint64_t inta_map                     : 2;  /**< Maps INTA to INTA(00), INTB(01), INTC(10) or
                                                         INTD (11). */
	uint64_t ctlp_ro                      : 1;  /**< Relaxed ordering enable for Completion TLPS. */
	uint64_t reserved_6_6                 : 1;
	uint64_t ptlp_ro                      : 1;  /**< Relaxed ordering enable for Posted TLPS. */
	uint64_t reserved_4_1                 : 4;
	uint64_t wait_com                     : 1;  /**< When set '1' casues the SLI to wait for a commit
                                                         from the L2C before sending additional stores to
                                                         the L2C from a MAC.
                                                         The SLI will request a commit on the last store
                                                         if more than one STORE operation is required on
                                                         the NCB.
                                                         Most applications will not notice a difference, so
                                                         should not set this bit. Setting the bit is more
                                                         conservative on ordering, lower performance */
#else
	uint64_t wait_com                     : 1;
	uint64_t reserved_4_1                 : 4;
	uint64_t ptlp_ro                      : 1;
	uint64_t reserved_6_6                 : 1;
	uint64_t ctlp_ro                      : 1;
	uint64_t inta_map                     : 2;
	uint64_t intb_map                     : 2;
	uint64_t intc_map                     : 2;
	uint64_t intd_map                     : 2;
	uint64_t waitl_com                    : 1;
	uint64_t dis_port                     : 1;
	uint64_t inta                         : 1;
	uint64_t intb                         : 1;
	uint64_t intc                         : 1;
	uint64_t intd                         : 1;
	uint64_t reserved_22_63               : 42;
#endif
	} cn70xx;
	struct cvmx_sli_ctl_portx_cn70xx      cn70xxp1;
	struct cvmx_sli_ctl_portx_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t dis_port                     : 1;  /**< When set, the output to the MAC is disabled. This occurs when the MAC reset line
                                                         transitions from deasserted to asserted. Writing a 1 to this location clears this
                                                         condition when the MAC is no longer in reset and the output to the MAC is at the beginning
                                                         of a transfer. */
	uint64_t waitl_com                    : 1;  /**< When set to 1, causes the SLI to wait for a commit from the L2C before sending additional
                                                         completions to the L2C from the MAC.
                                                         Set this for more conservative behavior. Clear this for more aggressive, higher-
                                                         performance behavior. */
	uint64_t reserved_8_15                : 8;
	uint64_t ctlp_ro                      : 1;  /**< Relaxed ordering enable for completion TLPS */
	uint64_t reserved_6_6                 : 1;
	uint64_t ptlp_ro                      : 1;  /**< Relaxed ordering enable for posted TLPS */
	uint64_t reserved_1_4                 : 4;
	uint64_t wait_com                     : 1;  /**< Wait for commit. When set to 1, causes the SLI to wait for a commit from the L2C before
                                                         sending additional stores to the L2C from the MAC. The SLI requests a commit on the last
                                                         store if more than one STORE operation is required on the IOI. Most applications will not
                                                         notice a difference, so this bit should not be set. Setting the bit is more conservative
                                                         on ordering, lower performance. */
#else
	uint64_t wait_com                     : 1;
	uint64_t reserved_1_4                 : 4;
	uint64_t ptlp_ro                      : 1;
	uint64_t reserved_6_6                 : 1;
	uint64_t ctlp_ro                      : 1;
	uint64_t reserved_8_15                : 8;
	uint64_t waitl_com                    : 1;
	uint64_t dis_port                     : 1;
	uint64_t reserved_18_63               : 46;
#endif
	} cn73xx;
	struct cvmx_sli_ctl_portx_cn73xx      cn78xx;
	struct cvmx_sli_ctl_portx_cn73xx      cn78xxp1;
	struct cvmx_sli_ctl_portx_s           cnf71xx;
	struct cvmx_sli_ctl_portx_cn73xx      cnf75xx;
};
typedef union cvmx_sli_ctl_portx cvmx_sli_ctl_portx_t;

/**
 * cvmx_sli_ctl_status
 *
 * This register contains control and status for SLI. Write operations to this register are not
 * ordered with write/read operations to the MAC memory space. To ensure that a write has
 * completed, software must read the register before making an access (i.e. MAC memory space)
 * that requires the value of this register to be updated.
 */
union cvmx_sli_ctl_status {
	uint64_t u64;
	struct cvmx_sli_ctl_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t m2s1_ncbi                    : 4;  /**< For CNF73XX, this field is reserved.
                                                         For CNF75XX, contains the IOBI that traffic (inbound BAR1/BAR2 posted writes, inbound
                                                         BAR1/BAR2
                                                         non-posted reads, outbound BAR1/BAR2 completions, and inbound CPU completions)
                                                         from SLI_PORT_E::SRIO0 and SLI_PORT_E::SRIO1 is placed on. Values 2-15 are reserved. */
	uint64_t m2s0_ncbi                    : 4;  /**< Contains the IOBI that traffic  (inbound BAR1/BAR2 posted writes, inbound BAR1/BAR2
                                                         non-posted reads, outbound BAR1/BAR2 completions, and inbound CPU completions)
                                                         from SLI_PORT_E::PEM0 and SLI_PORT_E::PEM1 is placed on.  Values 2-15 are reserved. */
	uint64_t oci_id                       : 4;  /**< The CCPI ID. */
	uint64_t p1_ntags                     : 6;  /**< Number of tags available for MAC port 1.
                                                         In RC mode, one tag is needed for each outbound TLP that requires a CPL TLP.
                                                         In EP mode, the number of tags required for a TLP request is 1 per 64-bytes of CPL data +
                                                         1.
                                                         This field should only be written as part of a reset sequence and before issuing any read
                                                         operations, CFGs, or I/O transactions from the core(s). */
	uint64_t p0_ntags                     : 6;  /**< Number of tags available for MAC port 0.
                                                         In RC mode, one tag is needed for each outbound TLP that requires a CPL TLP.
                                                         In EP mode, the number of tags required for a TLP request is 1 per 64-bytes of CPL data +
                                                         1.
                                                         This field should only be written as part of a reset sequence and before issuing any read
                                                         operations, CFGs, or I/O transactions from the core(s). */
	uint64_t chip_rev                     : 8;  /**< Chip revision level. */
#else
	uint64_t chip_rev                     : 8;
	uint64_t p0_ntags                     : 6;
	uint64_t p1_ntags                     : 6;
	uint64_t oci_id                       : 4;
	uint64_t m2s0_ncbi                    : 4;
	uint64_t m2s1_ncbi                    : 4;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_ctl_status_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t p0_ntags                     : 6;  /**< Number of tags available for outbound TLPs to the
                                                         MACS. One tag is needed for each outbound TLP that
                                                         requires a CPL TLP.
                                                         This field should only be written as part of
                                                         reset sequence, before issuing any reads, CFGs, or
                                                         IO transactions from the core(s). */
	uint64_t chip_rev                     : 8;  /**< The chip revision. */
#else
	uint64_t chip_rev                     : 8;
	uint64_t p0_ntags                     : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} cn61xx;
	struct cvmx_sli_ctl_status_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t p1_ntags                     : 6;  /**< Number of tags available for MAC Port1.
                                                         In RC mode 1 tag is needed for each outbound TLP
                                                         that requires a CPL TLP. In Endpoint mode the
                                                         number of tags required for a TLP request is
                                                         1 per 64-bytes of CPL data + 1.
                                                         This field should only be written as part of
                                                         reset sequence, before issuing any reads, CFGs, or
                                                         IO transactions from the core(s). */
	uint64_t p0_ntags                     : 6;  /**< Number of tags available for MAC Port0.
                                                         In RC mode 1 tag is needed for each outbound TLP
                                                         that requires a CPL TLP. In Endpoint mode the
                                                         number of tags required for a TLP request is
                                                         1 per 64-bytes of CPL data + 1.
                                                         This field should only be written as part of
                                                         reset sequence, before issuing any reads, CFGs, or
                                                         IO transactions from the core(s). */
	uint64_t chip_rev                     : 8;  /**< The chip revision. */
#else
	uint64_t chip_rev                     : 8;
	uint64_t p0_ntags                     : 6;
	uint64_t p1_ntags                     : 6;
	uint64_t reserved_20_63               : 44;
#endif
	} cn63xx;
	struct cvmx_sli_ctl_status_cn63xx     cn63xxp1;
	struct cvmx_sli_ctl_status_cn61xx     cn66xx;
	struct cvmx_sli_ctl_status_cn63xx     cn68xx;
	struct cvmx_sli_ctl_status_cn63xx     cn68xxp1;
	struct cvmx_sli_ctl_status_cn63xx     cn70xx;
	struct cvmx_sli_ctl_status_cn63xx     cn70xxp1;
	struct cvmx_sli_ctl_status_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t m2s1_ncbi                    : 4;  /**< Contains the IOBI that traffic (inbound BAR1/BAR2 posted writes, inbound BAR1/BAR2
                                                         non-posted reads, outbound BAR1/BAR2 completions, and inbound CPU completions)
                                                         from SLI_PORT_E::PEM2 and SLI_PORT_E::PEM3 is placed on. Values 2-15 are reserved. */
	uint64_t m2s0_ncbi                    : 4;  /**< Contains the IOBI that traffic  (inbound BAR1/BAR2 posted writes, inbound BAR1/BAR2
                                                         non-posted reads, outbound BAR1/BAR2 completions, and inbound CPU completions)
                                                         from SLI_PORT_E::PEM0 and SLI_PORT_E::PEM1 is placed on.  Values 2-15 are reserved. */
	uint64_t reserved_20_23               : 4;
	uint64_t p1_ntags                     : 6;  /**< Number of tags available for MAC port 1.
                                                         In RC mode, one tag is needed for each outbound TLP that requires a CPL TLP.
                                                         In EP mode, the number of tags required for a TLP request is 1 per 64-bytes of CPL data +
                                                         1.
                                                         This field should only be written as part of a reset sequence and before issuing any read
                                                         operations, CFGs, or I/O transactions from the core(s). */
	uint64_t p0_ntags                     : 6;  /**< Number of tags available for MAC port 0.
                                                         In RC mode, one tag is needed for each outbound TLP that requires a CPL TLP.
                                                         In EP mode, the number of tags required for a TLP request is 1 per 64-bytes of CPL data +
                                                         1.
                                                         This field should only be written as part of a reset sequence and before issuing any read
                                                         operations, CFGs, or I/O transactions from the core(s). */
	uint64_t chip_rev                     : 8;  /**< Chip revision level. */
#else
	uint64_t chip_rev                     : 8;
	uint64_t p0_ntags                     : 6;
	uint64_t p1_ntags                     : 6;
	uint64_t reserved_20_23               : 4;
	uint64_t m2s0_ncbi                    : 4;
	uint64_t m2s1_ncbi                    : 4;
	uint64_t reserved_32_63               : 32;
#endif
	} cn73xx;
	struct cvmx_sli_ctl_status_s          cn78xx;
	struct cvmx_sli_ctl_status_s          cn78xxp1;
	struct cvmx_sli_ctl_status_cn61xx     cnf71xx;
	struct cvmx_sli_ctl_status_cn73xx     cnf75xx;
};
typedef union cvmx_sli_ctl_status cvmx_sli_ctl_status_t;

/**
 * cvmx_sli_data_out_cnt
 *
 * This register contains the EXEC data out FIFO count and the data unload counter.
 *
 */
union cvmx_sli_data_out_cnt {
	uint64_t u64;
	struct cvmx_sli_data_out_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t p1_ucnt                      : 16; /**< For CNF73XX, reserved.
                                                         For CNF75XX, FIFO1 unload count. This counter is incremented by 1 every time a word is removed from
                                                         data out FIFO1, whose count is shown in P1_FCNT. */
	uint64_t p1_fcnt                      : 6;  /**< For CNF73XX, reserved.
                                                         For CNF75XX, FIFO1 data out count. Number of address data words presently
                                                         buffered in the FIFO1. MACs
                                                         associated with FIFO1: SRIO0, SRIO1. */
	uint64_t p0_ucnt                      : 16; /**< FIFO0 unload count. This counter is incremented by 1 every time a word is removed from
                                                         data out FIFO0, whose count is shown in P0_FCNT. */
	uint64_t p0_fcnt                      : 6;  /**< FIFO0 data out count. Number of address data words presently buffered in the FIFO0. MACs
                                                         associated with FIFO0: PCIe0, PCIe1. */
#else
	uint64_t p0_fcnt                      : 6;
	uint64_t p0_ucnt                      : 16;
	uint64_t p1_fcnt                      : 6;
	uint64_t p1_ucnt                      : 16;
	uint64_t reserved_44_63               : 20;
#endif
	} s;
	struct cvmx_sli_data_out_cnt_s        cn61xx;
	struct cvmx_sli_data_out_cnt_s        cn63xx;
	struct cvmx_sli_data_out_cnt_s        cn63xxp1;
	struct cvmx_sli_data_out_cnt_s        cn66xx;
	struct cvmx_sli_data_out_cnt_s        cn68xx;
	struct cvmx_sli_data_out_cnt_s        cn68xxp1;
	struct cvmx_sli_data_out_cnt_s        cn70xx;
	struct cvmx_sli_data_out_cnt_s        cn70xxp1;
	struct cvmx_sli_data_out_cnt_s        cn73xx;
	struct cvmx_sli_data_out_cnt_s        cn78xx;
	struct cvmx_sli_data_out_cnt_s        cn78xxp1;
	struct cvmx_sli_data_out_cnt_s        cnf71xx;
	struct cvmx_sli_data_out_cnt_s        cnf75xx;
};
typedef union cvmx_sli_data_out_cnt cvmx_sli_data_out_cnt_t;

/**
 * cvmx_sli_dbg_data
 *
 * SLI_DBG_DATA = SLI Debug Data Register
 *
 * Value returned on the debug-data lines from the RSLs
 */
union cvmx_sli_dbg_data {
	uint64_t u64;
	struct cvmx_sli_dbg_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t dsel_ext                     : 1;  /**< Allows changes in the external pins to set the
                                                         debug select value. */
	uint64_t data                         : 17; /**< Value on the debug data lines. */
#else
	uint64_t data                         : 17;
	uint64_t dsel_ext                     : 1;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_sli_dbg_data_s            cn61xx;
	struct cvmx_sli_dbg_data_s            cn63xx;
	struct cvmx_sli_dbg_data_s            cn63xxp1;
	struct cvmx_sli_dbg_data_s            cn66xx;
	struct cvmx_sli_dbg_data_s            cn68xx;
	struct cvmx_sli_dbg_data_s            cn68xxp1;
	struct cvmx_sli_dbg_data_s            cnf71xx;
};
typedef union cvmx_sli_dbg_data cvmx_sli_dbg_data_t;

/**
 * cvmx_sli_dbg_select
 *
 * SLI_DBG_SELECT = Debug Select Register
 *
 * Contains the debug select value last written to the RSLs.
 */
union cvmx_sli_dbg_select {
	uint64_t u64;
	struct cvmx_sli_dbg_select_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t adbg_sel                     : 1;  /**< When set '1' the SLI_DBG_DATA[DATA] will only be
                                                         loaded when SLI_DBG_DATA[DATA] bit [16] is a '1'.
                                                         When the debug data comes from an Async-RSL bit
                                                         16 is used to tell that the data present is valid. */
	uint64_t dbg_sel                      : 32; /**< When this register is written the RML will write
                                                         all "F"s to the previous RTL to disable it from
                                                         sending Debug-Data. The RML will then send a write
                                                         to the new RSL with the supplied Debug-Select
                                                         value. Because it takes time for the new Debug
                                                         Select value to take effect and the requested
                                                         Debug-Data to return, time is needed to the new
                                                         Debug-Data to arrive.  The inititator of the Debug
                                                         Select should issue a read to a CSR before reading
                                                         the Debug Data (this read could also be to the
                                                         SLI_DBG_DATA but the returned value for the first
                                                         read will return NS data. */
#else
	uint64_t dbg_sel                      : 32;
	uint64_t adbg_sel                     : 1;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_sli_dbg_select_s          cn61xx;
	struct cvmx_sli_dbg_select_s          cn63xx;
	struct cvmx_sli_dbg_select_s          cn63xxp1;
	struct cvmx_sli_dbg_select_s          cn66xx;
	struct cvmx_sli_dbg_select_s          cn68xx;
	struct cvmx_sli_dbg_select_s          cn68xxp1;
	struct cvmx_sli_dbg_select_s          cnf71xx;
};
typedef union cvmx_sli_dbg_select cvmx_sli_dbg_select_t;

/**
 * cvmx_sli_dma#_cnt
 *
 * These registers contain the DMA count values.
 *
 */
union cvmx_sli_dmax_cnt {
	uint64_t u64;
	struct cvmx_sli_dmax_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t cnt                          : 32; /**< The DMA counter. SLI/DPI hardware subtracts the written value from
                                                         the counter whenever software writes this CSR. SLI/DPI hardware increments this
                                                         counter after completing an OUTBOUND or EXTERNAL-ONLY DMA instruction
                                                         with DPI_DMA_INSTR_HDR_S[CA] set DPI_DMA_INSTR_HDR_S[CSEL] equal to this
                                                         CSR index. These increments may cause interrupts.
                                                         See SLI_DMA()_INT_LEVEL and SLI_MAC()_PF()_INT_SUM[DCNT,DTIME]. */
#else
	uint64_t cnt                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_dmax_cnt_s            cn61xx;
	struct cvmx_sli_dmax_cnt_s            cn63xx;
	struct cvmx_sli_dmax_cnt_s            cn63xxp1;
	struct cvmx_sli_dmax_cnt_s            cn66xx;
	struct cvmx_sli_dmax_cnt_s            cn68xx;
	struct cvmx_sli_dmax_cnt_s            cn68xxp1;
	struct cvmx_sli_dmax_cnt_s            cn70xx;
	struct cvmx_sli_dmax_cnt_s            cn70xxp1;
	struct cvmx_sli_dmax_cnt_s            cn73xx;
	struct cvmx_sli_dmax_cnt_s            cn78xx;
	struct cvmx_sli_dmax_cnt_s            cn78xxp1;
	struct cvmx_sli_dmax_cnt_s            cnf71xx;
	struct cvmx_sli_dmax_cnt_s            cnf75xx;
};
typedef union cvmx_sli_dmax_cnt cvmx_sli_dmax_cnt_t;

/**
 * cvmx_sli_dma#_int_level
 *
 * These registers contain the thresholds for DMA count and timer interrupts.
 *
 */
union cvmx_sli_dmax_int_level {
	uint64_t u64;
	struct cvmx_sli_dmax_int_level_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t time                         : 32; /**< Whenever the SLI_DMA()_TIM[TIM] timer exceeds this value,
                                                         SLI_MAC()_PF()_INT_SUM[DTIME<x>] is set. The SLI_DMA()_TIM[TIM] timer
                                                         increments every SLI clock whenever SLI_DMA()_CNT[CNT] != 0, and is cleared
                                                         when SLI_MAC()_PF()_INT_SUM[DTIME<x>] is written with one. */
	uint64_t cnt                          : 32; /**< Whenever SLI_DMA()_CNT[CNT] exceeds this value, SLI_MAC()_PF()_INT_SUM[DCNT<x>]
                                                         is set. */
#else
	uint64_t cnt                          : 32;
	uint64_t time                         : 32;
#endif
	} s;
	struct cvmx_sli_dmax_int_level_s      cn61xx;
	struct cvmx_sli_dmax_int_level_s      cn63xx;
	struct cvmx_sli_dmax_int_level_s      cn63xxp1;
	struct cvmx_sli_dmax_int_level_s      cn66xx;
	struct cvmx_sli_dmax_int_level_s      cn68xx;
	struct cvmx_sli_dmax_int_level_s      cn68xxp1;
	struct cvmx_sli_dmax_int_level_s      cn70xx;
	struct cvmx_sli_dmax_int_level_s      cn70xxp1;
	struct cvmx_sli_dmax_int_level_s      cn73xx;
	struct cvmx_sli_dmax_int_level_s      cn78xx;
	struct cvmx_sli_dmax_int_level_s      cn78xxp1;
	struct cvmx_sli_dmax_int_level_s      cnf71xx;
	struct cvmx_sli_dmax_int_level_s      cnf75xx;
};
typedef union cvmx_sli_dmax_int_level cvmx_sli_dmax_int_level_t;

/**
 * cvmx_sli_dma#_tim
 *
 * These registers contain the DMA timer values.
 *
 */
union cvmx_sli_dmax_tim {
	uint64_t u64;
	struct cvmx_sli_dmax_tim_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t tim                          : 32; /**< The DMA timer value. The timer increments when
                                                         SLI_DMA()_CNT[CNT]!=0 and clears when SLI_DMA()_CNT[CNT]=0. */
#else
	uint64_t tim                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_dmax_tim_s            cn61xx;
	struct cvmx_sli_dmax_tim_s            cn63xx;
	struct cvmx_sli_dmax_tim_s            cn63xxp1;
	struct cvmx_sli_dmax_tim_s            cn66xx;
	struct cvmx_sli_dmax_tim_s            cn68xx;
	struct cvmx_sli_dmax_tim_s            cn68xxp1;
	struct cvmx_sli_dmax_tim_s            cn70xx;
	struct cvmx_sli_dmax_tim_s            cn70xxp1;
	struct cvmx_sli_dmax_tim_s            cn73xx;
	struct cvmx_sli_dmax_tim_s            cn78xx;
	struct cvmx_sli_dmax_tim_s            cn78xxp1;
	struct cvmx_sli_dmax_tim_s            cnf71xx;
	struct cvmx_sli_dmax_tim_s            cnf75xx;
};
typedef union cvmx_sli_dmax_tim cvmx_sli_dmax_tim_t;

/**
 * cvmx_sli_int_enb_ciu
 *
 * Used to enable the various interrupting conditions of SLI
 *
 */
union cvmx_sli_int_enb_ciu {
	uint64_t u64;
	struct cvmx_sli_int_enb_ciu_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t pipe_err                     : 1;  /**< Illegal packet csr address. */
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t sprt3_err                    : 1;  /**< Error Response received on SLI port 3. */
	uint64_t sprt2_err                    : 1;  /**< Error Response received on SLI port 2. */
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_29_31               : 3;
	uint64_t mio_int2                     : 1;  /**< Enables SLI_INT_SUM[28] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Reserved. */
	uint64_t m2_un_b0                     : 1;  /**< Reserved. */
	uint64_t m2_up_wi                     : 1;  /**< Reserved. */
	uint64_t m2_up_b0                     : 1;  /**< Reserved. */
	uint64_t reserved_18_19               : 2;
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt on the RSL. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt on the RSL. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt on the RSL. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt on the RSL. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t reserved_18_19               : 2;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t mio_int2                     : 1;
	uint64_t reserved_29_31               : 3;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t pipe_err                     : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_sli_int_enb_ciu_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t sprt3_err                    : 1;  /**< Error Response received on SLI port 3. */
	uint64_t sprt2_err                    : 1;  /**< Error Response received on SLI port 2. */
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_28_31               : 4;
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Reserved. */
	uint64_t m2_un_b0                     : 1;  /**< Reserved. */
	uint64_t m2_up_wi                     : 1;  /**< Reserved. */
	uint64_t m2_up_b0                     : 1;  /**< Reserved. */
	uint64_t reserved_18_19               : 2;
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt on the RSL. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt on the RSL. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt on the RSL. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt on the RSL. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t reserved_18_19               : 2;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t reserved_28_31               : 4;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn61xx;
	struct cvmx_sli_int_enb_ciu_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t reserved_58_59               : 2;
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_18_31               : 14;
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt on the RSL. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt on the RSL. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt on the RSL. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt on the RSL. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t reserved_18_31               : 14;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t reserved_58_59               : 2;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn63xx;
	struct cvmx_sli_int_enb_ciu_cn63xx    cn63xxp1;
	struct cvmx_sli_int_enb_ciu_cn61xx    cn66xx;
	struct cvmx_sli_int_enb_ciu_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t pipe_err                     : 1;  /**< Illegal packet csr address. */
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t reserved_58_59               : 2;
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t reserved_51_51               : 1;
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_18_31               : 14;
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt on the RSL. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt on the RSL. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt on the RSL. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt on the RSL. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t reserved_18_31               : 14;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t reserved_58_59               : 2;
	uint64_t ill_pad                      : 1;
	uint64_t pipe_err                     : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} cn68xx;
	struct cvmx_sli_int_enb_ciu_cn68xx    cn68xxp1;
	struct cvmx_sli_int_enb_ciu_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_63_61               : 3;
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t sprt3_err                    : 1;  /**< Error Response received on SLI port 3. */
	uint64_t sprt2_err                    : 1;  /**< Error Response received on SLI port 2. */
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_47_38               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_31_29               : 3;
	uint64_t mio_int2                     : 1;  /**< Enables SLI_INT_SUM[28] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Enables SLI_INT_SUM[23] to generate an
                                                         interrupt on the RSL. */
	uint64_t m2_un_b0                     : 1;  /**< Enables SLI_INT_SUM[22] to generate an
                                                         interrupt on the RSL. */
	uint64_t m2_up_wi                     : 1;  /**< Enables SLI_INT_SUM[21] to generate an
                                                         interrupt on the RSL. */
	uint64_t m2_up_b0                     : 1;  /**< Enables SLI_INT_SUM[20] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_19_18               : 2;
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_7_6                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt on the RSL. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt on the RSL. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt on the RSL. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt on the RSL. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_7_6                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t reserved_19_18               : 2;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t mio_int2                     : 1;
	uint64_t reserved_31_29               : 3;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_47_38               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_63_61               : 3;
#endif
	} cn70xx;
	struct cvmx_sli_int_enb_ciu_cn70xx    cn70xxp1;
	struct cvmx_sli_int_enb_ciu_cn61xx    cnf71xx;
};
typedef union cvmx_sli_int_enb_ciu cvmx_sli_int_enb_ciu_t;

/**
 * cvmx_sli_int_enb_port#
 *
 * When a field in this register is set, and a corresponding interrupt condition asserts in
 * SLI_INT_SUM, an interrupt is generated. Interrupts can be sent to PCIe0 or PCIe1.
 */
union cvmx_sli_int_enb_portx {
	uint64_t u64;
	struct cvmx_sli_int_enb_portx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t pipe_err                     : 1;  /**< Out of range PIPE value. */
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t sprt3_err                    : 1;  /**< Error Response received on SLI port 3. */
	uint64_t sprt2_err                    : 1;  /**< Error Response received on SLI port 2. */
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_30_31               : 2;
	uint64_t mac2_int                     : 1;  /**< Enables SLI_INT_SUM[29] to generate an
                                                         interrupt to the PCIE-Port2 for MSI/inta.
                                                         SLI_INT_ENB_PORT2[MAC0_INT] sould NEVER be set.
                                                         SLI_INT_ENB_PORT2[MAC1_INT] sould NEVER be set. */
	uint64_t reserved_28_28               : 1;
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Reserved. */
	uint64_t m2_un_b0                     : 1;  /**< Reserved. */
	uint64_t m2_up_wi                     : 1;  /**< Reserved. */
	uint64_t m2_up_b0                     : 1;  /**< Reserved. */
	uint64_t mac1_int                     : 1;  /**< Enables SLI_INT_SUM[19] to generate an
                                                         interrupt to the PCIE-Port1 for MSI/inta.
                                                         The valuse of this bit has NO effect on PCIE Port0.
                                                         SLI_INT_ENB_PORT0[MAC1_INT] sould NEVER be set. */
	uint64_t mac0_int                     : 1;  /**< Enables SLI_INT_SUM[18] to generate an
                                                         interrupt to the PCIE-Port0 for MSI/inta.
                                                         The valus of this bit has NO effect on PCIE Port1.
                                                         SLI_INT_ENB_PORT1[MAC0_INT] sould NEVER be set. */
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT0[MIO_INT1] should NEVER be set. */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT1[MIO_INT0] should NEVER be set. */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t mio_int3                     : 1;  /**< Enables SLI_INT_SUM[MIO_INT3] to generate an interrupt to the MAC core for MSI/INTA.
                                                         MIO_INT3 should only be set in SLI_INT_ENB_PORT3. */
	uint64_t reserved_6_6                 : 1;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_6                 : 1;
	uint64_t mio_int3                     : 1;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t mac2_int                     : 1;
	uint64_t reserved_30_31               : 2;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t pipe_err                     : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_sli_int_enb_portx_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t sprt3_err                    : 1;  /**< Error Response received on SLI port 3. */
	uint64_t sprt2_err                    : 1;  /**< Error Response received on SLI port 2. */
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_28_31               : 4;
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Reserved. */
	uint64_t m2_un_b0                     : 1;  /**< Reserved. */
	uint64_t m2_up_wi                     : 1;  /**< Reserved. */
	uint64_t m2_up_b0                     : 1;  /**< Reserved. */
	uint64_t mac1_int                     : 1;  /**< Enables SLI_INT_SUM[19] to generate an
                                                         interrupt to the PCIE-Port1 for MSI/inta.
                                                         The valuse of this bit has NO effect on PCIE Port0.
                                                         SLI_INT_ENB_PORT0[MAC1_INT] sould NEVER be set. */
	uint64_t mac0_int                     : 1;  /**< Enables SLI_INT_SUM[18] to generate an
                                                         interrupt to the PCIE-Port0 for MSI/inta.
                                                         The valus of this bit has NO effect on PCIE Port1.
                                                         SLI_INT_ENB_PORT1[MAC0_INT] sould NEVER be set. */
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT0[MIO_INT1] should NEVER be set. */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT1[MIO_INT0] should NEVER be set. */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t reserved_28_31               : 4;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn61xx;
	struct cvmx_sli_int_enb_portx_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t reserved_58_59               : 2;
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_20_31               : 12;
	uint64_t mac1_int                     : 1;  /**< Enables SLI_INT_SUM[19] to generate an
                                                         interrupt to the PCIE-Port1 for MSI/inta.
                                                         The valuse of this bit has NO effect on PCIE Port0.
                                                         SLI_INT_ENB_PORT0[MAC1_INT] sould NEVER be set. */
	uint64_t mac0_int                     : 1;  /**< Enables SLI_INT_SUM[18] to generate an
                                                         interrupt to the PCIE-Port0 for MSI/inta.
                                                         The valus of this bit has NO effect on PCIE Port1.
                                                         SLI_INT_ENB_PORT1[MAC0_INT] sould NEVER be set. */
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT0[MIO_INT1] should NEVER be set. */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT1[MIO_INT0] should NEVER be set. */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t reserved_20_31               : 12;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t reserved_58_59               : 2;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn63xx;
	struct cvmx_sli_int_enb_portx_cn63xx  cn63xxp1;
	struct cvmx_sli_int_enb_portx_cn61xx  cn66xx;
	struct cvmx_sli_int_enb_portx_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t pipe_err                     : 1;  /**< Out of range PIPE value. */
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t reserved_58_59               : 2;
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t reserved_51_51               : 1;
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_20_31               : 12;
	uint64_t mac1_int                     : 1;  /**< Enables SLI_INT_SUM[19] to generate an
                                                         interrupt to the PCIE-Port1 for MSI/inta.
                                                         The valuse of this bit has NO effect on PCIE Port0.
                                                         SLI_INT_ENB_PORT0[MAC1_INT] sould NEVER be set. */
	uint64_t mac0_int                     : 1;  /**< Enables SLI_INT_SUM[18] to generate an
                                                         interrupt to the PCIE-Port0 for MSI/inta.
                                                         The valus of this bit has NO effect on PCIE Port1.
                                                         SLI_INT_ENB_PORT1[MAC0_INT] sould NEVER be set. */
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT0[MIO_INT1] should NEVER be set. */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT1[MIO_INT0] should NEVER be set. */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t reserved_20_31               : 12;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t reserved_58_59               : 2;
	uint64_t ill_pad                      : 1;
	uint64_t pipe_err                     : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} cn68xx;
	struct cvmx_sli_int_enb_portx_cn68xx  cn68xxp1;
	struct cvmx_sli_int_enb_portx_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_63_61               : 3;
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t sprt3_err                    : 1;  /**< Error Response received on SLI port 3. */
	uint64_t sprt2_err                    : 1;  /**< Error Response received on SLI port 2. */
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_47_38               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_31_30               : 2;
	uint64_t mac2_int                     : 1;  /**< Enables SLI_INT_SUM[29] to generate an
                                                         interrupt to the PCIE-Port2 for MSI/inta.
                                                         SLI_INT_ENB_PORT2[MAC0_INT] sould NEVER be set.
                                                         SLI_INT_ENB_PORT2[MAC1_INT] sould NEVER be set. */
	uint64_t mio_int2                     : 1;  /**< Enables SLI_INT_SUM[28] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT2[MIO_INT2] should NEVER be set. */
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Enables SLI_INT_SUM[23] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m2_un_b0                     : 1;  /**< Enables SLI_INT_SUM[22] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m2_up_wi                     : 1;  /**< Enables SLI_INT_SUM[21] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m2_up_b0                     : 1;  /**< Enables SLI_INT_SUM[20] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t mac1_int                     : 1;  /**< Enables SLI_INT_SUM[19] to generate an
                                                         interrupt to the PCIE-Port1 for MSI/inta.
                                                         The valuse of this bit has NO effect on PCIE Port0.
                                                         SLI_INT_ENB_PORT0[MAC1_INT] sould NEVER be set. */
	uint64_t mac0_int                     : 1;  /**< Enables SLI_INT_SUM[18] to generate an
                                                         interrupt to the PCIE-Port0 for MSI/inta.
                                                         The valus of this bit has NO effect on PCIE Port1.
                                                         SLI_INT_ENB_PORT1[MAC0_INT] sould NEVER be set. */
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT0[MIO_INT1] should NEVER be set. */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT1[MIO_INT0] should NEVER be set. */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_7_6                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_7_6                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t mio_int2                     : 1;
	uint64_t mac2_int                     : 1;
	uint64_t reserved_31_30               : 2;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_47_38               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_63_61               : 3;
#endif
	} cn70xx;
	struct cvmx_sli_int_enb_portx_cn70xx  cn70xxp1;
	struct cvmx_sli_int_enb_portx_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t sprt3_err                    : 1;  /**< Enables SLI_INT_SUM[SPRT3_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t sprt2_err                    : 1;  /**< Enables SLI_INT_SUM[SPRT2_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t sprt1_err                    : 1;  /**< Enables SLI_INT_SUM[SPRT1_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t sprt0_err                    : 1;  /**< Enables SLI_INT_SUM[SPRT0_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t pins_err                     : 1;  /**< Enables SLI_INT_SUM[PINS_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t pop_err                      : 1;  /**< Enables SLI_INT_SUM[POP_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t pdi_err                      : 1;  /**< Enables SLI_INT_SUM[PDI_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t pgl_err                      : 1;  /**< Enables SLI_INT_SUM[PGL_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t reserved_50_51               : 2;
	uint64_t psldbof                      : 1;  /**< Enables SLI_INT_SUM[PSLDBOF] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t pidbof                       : 1;  /**< Enables SLI_INT_SUM[PIDBOF] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< Enables SLI_INT_SUM[DTIME] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t dcnt                         : 2;  /**< Enables SLI_INT_SUM[DCNT] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t dmafi                        : 2;  /**< Enables SLI_INT_SUM[DMAFI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t reserved_29_31               : 3;
	uint64_t vf_err                       : 1;  /**< Illegal access from VF */
	uint64_t m3_un_wi                     : 1;  /**< Enables SLI_INT_SUM[M3_UN_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m3_un_b0                     : 1;  /**< Enables SLI_INT_SUM[M3_UN_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m3_up_wi                     : 1;  /**< Enables SLI_INT_SUM[M3_UP_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m3_up_b0                     : 1;  /**< Enables SLI_INT_SUM[M3_UP_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m2_un_wi                     : 1;  /**< Enables SLI_INT_SUM[M2_UN_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m2_un_b0                     : 1;  /**< Enables SLI_INT_SUM[M2_UN_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m2_up_wi                     : 1;  /**< Enables SLI_INT_SUM[M2_UP_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m2_up_b0                     : 1;  /**< Enables SLI_INT_SUM[M2_UP_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t reserved_18_19               : 2;
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[MIO_INT1] to generate an interrupt to the MAC core for MSI/INTA.
                                                         MIO_INT1 should only be set in SLI_INT_ENB_PORT1. */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[MIO_INT0] to generate an interrupt to the MAC core for MSI/INTA.
                                                         MIO_INT0 should only be set in SLI_INT_ENB_PORT0. */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[M1_UN_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[M1_UN_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[M1_UP_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[M1_UP_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[M0_UN_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[M0_UN_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[M0_UP_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[M0_UP_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t mio_int3                     : 1;  /**< Enables SLI_INT_SUM[MIO_INT3] to generate an interrupt to the MAC core for MSI/INTA.
                                                         MIO_INT3 should only be set in SLI_INT_ENB_PORT3. */
	uint64_t mio_int2                     : 1;  /**< Enables SLI_INT_SUM[MIO_INT2] to generate an interrupt to the MAC core for MSI/INTA.
                                                         MIO_INT2 should only be set in SLI_INT_ENB_PORT2. */
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[PTIME] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[PCNT] to generate an interrupt to the PCIE core for MSI/INTA. */
	uint64_t reserved_1_3                 : 3;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[RML_TO] to generate an interrupt to the PCIE core for MSI/INTA. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_3                 : 3;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t mio_int2                     : 1;
	uint64_t mio_int3                     : 1;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t reserved_18_19               : 2;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t vf_err                       : 1;
	uint64_t reserved_29_31               : 3;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t reserved_60_63               : 4;
#endif
	} cn78xxp1;
	struct cvmx_sli_int_enb_portx_cn61xx  cnf71xx;
};
typedef union cvmx_sli_int_enb_portx cvmx_sli_int_enb_portx_t;

/**
 * cvmx_sli_int_sum
 *
 * The fields in this register are set when an interrupt condition occurs; write 1 to clear. All
 * fields of the register are valid when a PF reads the register. Not available to VFs, and
 * writes by the
 * VF do not modify the register.
 */
union cvmx_sli_int_sum {
	uint64_t u64;
	struct cvmx_sli_int_sum_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t pipe_err                     : 1;  /**< Set when a PIPE value outside range is received. */
	uint64_t ill_pad                      : 1;  /**< Set when a BAR0 address R/W falls into theaddress
                                                         range of the Packet-CSR, but for an unused
                                                         address. */
	uint64_t sprt3_err                    : 1;  /**< Reserved. */
	uint64_t sprt2_err                    : 1;  /**< Reserved. */
	uint64_t sprt1_err                    : 1;  /**< When an error response received on SLI port 1
                                                         this bit is set. */
	uint64_t sprt0_err                    : 1;  /**< When an error response received on SLI port 0
                                                         this bit is set. */
	uint64_t pins_err                     : 1;  /**< When a read error occurs on a packet instruction
                                                         this bit is set. */
	uint64_t pop_err                      : 1;  /**< When a read error occurs on a packet scatter
                                                         pointer pair this bit is set. */
	uint64_t pdi_err                      : 1;  /**< When a read error occurs on a packet data read
                                                         this bit is set. */
	uint64_t pgl_err                      : 1;  /**< When a read error occurs on a packet gather list
                                                         read this bit is set. */
	uint64_t pin_bp                       : 1;  /**< Packet input count has exceeded the WMARK.
                                                         See SLI_PKT_IN_BP */
	uint64_t pout_err                     : 1;  /**< Set when PKO sends packet data with the error bit
                                                         set. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PSLDBOF] */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PIDBOF] */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< Whenever SLI_DMAx_CNT[CNT] is not 0, the
                                                         SLI_DMAx_TIM[TIM] timer increments every SLI
                                                         clock.
                                                         DTIME[x] is set whenever SLI_DMAx_TIM[TIM] >
                                                         SLI_DMAx_INT_LEVEL[TIME].
                                                         DTIME[x] is normally cleared by clearing
                                                         SLI_DMAx_CNT[CNT] (which also clears
                                                         SLI_DMAx_TIM[TIM]). */
	uint64_t dcnt                         : 2;  /**< DCNT[x] is set whenever SLI_DMAx_CNT[CNT] >
                                                         SLI_DMAx_INT_LEVEL[CNT].
                                                         DCNT[x] is normally cleared by decreasing
                                                         SLI_DMAx_CNT[CNT]. */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts. */
	uint64_t reserved_30_31               : 2;
	uint64_t mac2_int                     : 1;  /**< Interrupt from MAC2.
                                                         See PEM2_INT_SUM (enabled by PEM2_INT_ENB_INT) */
	uint64_t reserved_28_28               : 1;
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Reserved. */
	uint64_t m2_un_b0                     : 1;  /**< Reserved. */
	uint64_t m2_up_wi                     : 1;  /**< Reserved. */
	uint64_t m2_up_b0                     : 1;  /**< Reserved. */
	uint64_t mac1_int                     : 1;  /**< Interrupt from MAC1.
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB_INT) */
	uint64_t mac0_int                     : 1;  /**< Interrupt from MAC0.
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB_INT) */
	uint64_t mio_int1                     : 1;  /**< Interrupt from MIO for PORT 1.
                                                         See CIU_INT33_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT33_EN0, CIU_INT33_EN1) */
	uint64_t mio_int0                     : 1;  /**< Interrupt from MIO for PORT 0.
                                                         See CIU_INT32_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT32_EN0, CIU_INT32_EN1) */
	uint64_t m1_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m1_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t mio_int3                     : 1;  /**< CIU interrupt output for MAC 3. A copy of CIU3_DEST(x)_IO_INT[INTR],
                                                         where x=CIU_DEST_IO_E::PEM(3) (i.e. x=4). */
	uint64_t reserved_6_6                 : 1;
	uint64_t ptime                        : 1;  /**< Packet Timer has an interrupt. Which rings can
                                                         be found in SLI_PKT_TIME_INT. */
	uint64_t pcnt                         : 1;  /**< Packet Counter has an interrupt. Which rings can
                                                         be found in SLI_PKT_CNT_INT. */
	uint64_t iob2big                      : 1;  /**< A requested IOBDMA is to large. */
	uint64_t bar0_to                      : 1;  /**< BAR0 R/W to a NCB device did not receive
                                                         read-data/commit in 0xffff core clocks. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< A read or write transfer did not complete
                                                         within 0xffff core clocks. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_6                 : 1;
	uint64_t mio_int3                     : 1;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t mac2_int                     : 1;
	uint64_t reserved_30_31               : 2;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t pipe_err                     : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_sli_int_sum_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Set when a BAR0 address R/W falls into theaddress
                                                         range of the Packet-CSR, but for an unused
                                                         address. */
	uint64_t sprt3_err                    : 1;  /**< Reserved. */
	uint64_t sprt2_err                    : 1;  /**< Reserved. */
	uint64_t sprt1_err                    : 1;  /**< When an error response received on SLI port 1
                                                         this bit is set. */
	uint64_t sprt0_err                    : 1;  /**< When an error response received on SLI port 0
                                                         this bit is set. */
	uint64_t pins_err                     : 1;  /**< When a read error occurs on a packet instruction
                                                         this bit is set. */
	uint64_t pop_err                      : 1;  /**< When a read error occurs on a packet scatter
                                                         pointer pair this bit is set. */
	uint64_t pdi_err                      : 1;  /**< When a read error occurs on a packet data read
                                                         this bit is set. */
	uint64_t pgl_err                      : 1;  /**< When a read error occurs on a packet gather list
                                                         read this bit is set. */
	uint64_t pin_bp                       : 1;  /**< Packet input count has exceeded the WMARK.
                                                         See SLI_PKT_IN_BP */
	uint64_t pout_err                     : 1;  /**< Set when PKO sends packet data with the error bit
                                                         set. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PSLDBOF] */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PIDBOF] */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< Whenever SLI_DMAx_CNT[CNT] is not 0, the
                                                         SLI_DMAx_TIM[TIM] timer increments every SLI
                                                         clock.
                                                         DTIME[x] is set whenever SLI_DMAx_TIM[TIM] >
                                                         SLI_DMAx_INT_LEVEL[TIME].
                                                         DTIME[x] is normally cleared by clearing
                                                         SLI_DMAx_CNT[CNT] (which also clears
                                                         SLI_DMAx_TIM[TIM]). */
	uint64_t dcnt                         : 2;  /**< DCNT[x] is set whenever SLI_DMAx_CNT[CNT] >
                                                         SLI_DMAx_INT_LEVEL[CNT].
                                                         DCNT[x] is normally cleared by decreasing
                                                         SLI_DMAx_CNT[CNT]. */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts. */
	uint64_t reserved_28_31               : 4;
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Reserved. */
	uint64_t m2_un_b0                     : 1;  /**< Reserved. */
	uint64_t m2_up_wi                     : 1;  /**< Reserved. */
	uint64_t m2_up_b0                     : 1;  /**< Reserved. */
	uint64_t mac1_int                     : 1;  /**< Interrupt from MAC1.
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB_INT) */
	uint64_t mac0_int                     : 1;  /**< Interrupt from MAC0.
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB_INT) */
	uint64_t mio_int1                     : 1;  /**< Interrupt from MIO for PORT 1.
                                                         See CIU_INT33_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT33_EN0, CIU_INT33_EN1) */
	uint64_t mio_int0                     : 1;  /**< Interrupt from MIO for PORT 0.
                                                         See CIU_INT32_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT32_EN0, CIU_INT32_EN1) */
	uint64_t m1_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m1_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Packet Timer has an interrupt. Which rings can
                                                         be found in SLI_PKT_TIME_INT. */
	uint64_t pcnt                         : 1;  /**< Packet Counter has an interrupt. Which rings can
                                                         be found in SLI_PKT_CNT_INT. */
	uint64_t iob2big                      : 1;  /**< A requested IOBDMA is to large. */
	uint64_t bar0_to                      : 1;  /**< BAR0 R/W to a NCB device did not receive
                                                         read-data/commit in 0xffff core clocks. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< A read or write transfer did not complete
                                                         within 0xffff core clocks. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t reserved_28_31               : 4;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn61xx;
	struct cvmx_sli_int_sum_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Set when a BAR0 address R/W falls into theaddress
                                                         range of the Packet-CSR, but for an unused
                                                         address. */
	uint64_t reserved_58_59               : 2;
	uint64_t sprt1_err                    : 1;  /**< When an error response received on SLI port 1
                                                         this bit is set. */
	uint64_t sprt0_err                    : 1;  /**< When an error response received on SLI port 0
                                                         this bit is set. */
	uint64_t pins_err                     : 1;  /**< When a read error occurs on a packet instruction
                                                         this bit is set. */
	uint64_t pop_err                      : 1;  /**< When a read error occurs on a packet scatter
                                                         pointer pair this bit is set. */
	uint64_t pdi_err                      : 1;  /**< When a read error occurs on a packet data read
                                                         this bit is set. */
	uint64_t pgl_err                      : 1;  /**< When a read error occurs on a packet gather list
                                                         read this bit is set. */
	uint64_t pin_bp                       : 1;  /**< Packet input count has exceeded the WMARK.
                                                         See SLI_PKT_IN_BP */
	uint64_t pout_err                     : 1;  /**< Set when PKO sends packet data with the error bit
                                                         set. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PSLDBOF] */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PIDBOF] */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< Whenever SLI_DMAx_CNT[CNT] is not 0, the
                                                         SLI_DMAx_TIM[TIM] timer increments every SLI
                                                         clock.
                                                         DTIME[x] is set whenever SLI_DMAx_TIM[TIM] >
                                                         SLI_DMAx_INT_LEVEL[TIME].
                                                         DTIME[x] is normally cleared by clearing
                                                         SLI_DMAx_CNT[CNT] (which also clears
                                                         SLI_DMAx_TIM[TIM]). */
	uint64_t dcnt                         : 2;  /**< DCNT[x] is set whenever SLI_DMAx_CNT[CNT] >
                                                         SLI_DMAx_INT_LEVEL[CNT].
                                                         DCNT[x] is normally cleared by decreasing
                                                         SLI_DMAx_CNT[CNT]. */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts. */
	uint64_t reserved_20_31               : 12;
	uint64_t mac1_int                     : 1;  /**< Interrupt from MAC1.
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB_INT) */
	uint64_t mac0_int                     : 1;  /**< Interrupt from MAC0.
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB_INT) */
	uint64_t mio_int1                     : 1;  /**< Interrupt from MIO for PORT 1.
                                                         See CIU_INT33_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT33_EN0, CIU_INT33_EN1) */
	uint64_t mio_int0                     : 1;  /**< Interrupt from MIO for PORT 0.
                                                         See CIU_INT32_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT32_EN0, CIU_INT32_EN1) */
	uint64_t m1_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m1_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Packet Timer has an interrupt. Which rings can
                                                         be found in SLI_PKT_TIME_INT. */
	uint64_t pcnt                         : 1;  /**< Packet Counter has an interrupt. Which rings can
                                                         be found in SLI_PKT_CNT_INT. */
	uint64_t iob2big                      : 1;  /**< A requested IOBDMA is to large. */
	uint64_t bar0_to                      : 1;  /**< BAR0 R/W to a NCB device did not receive
                                                         read-data/commit in 0xffff core clocks. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< A read or write transfer did not complete
                                                         within 0xffff core clocks. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t reserved_20_31               : 12;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t reserved_58_59               : 2;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn63xx;
	struct cvmx_sli_int_sum_cn63xx        cn63xxp1;
	struct cvmx_sli_int_sum_cn61xx        cn66xx;
	struct cvmx_sli_int_sum_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t pipe_err                     : 1;  /**< Set when a PIPE value outside range is received. */
	uint64_t ill_pad                      : 1;  /**< Set when a BAR0 address R/W falls into theaddress
                                                         range of the Packet-CSR, but for an unused
                                                         address. */
	uint64_t reserved_58_59               : 2;
	uint64_t sprt1_err                    : 1;  /**< When an error response received on SLI port 1
                                                         this bit is set. */
	uint64_t sprt0_err                    : 1;  /**< When an error response received on SLI port 0
                                                         this bit is set. */
	uint64_t pins_err                     : 1;  /**< When a read error occurs on a packet instruction
                                                         this bit is set. */
	uint64_t pop_err                      : 1;  /**< When a read error occurs on a packet scatter
                                                         pointer pair this bit is set. */
	uint64_t pdi_err                      : 1;  /**< When a read error occurs on a packet data read
                                                         this bit is set. */
	uint64_t pgl_err                      : 1;  /**< When a read error occurs on a packet gather list
                                                         read this bit is set. */
	uint64_t reserved_51_51               : 1;
	uint64_t pout_err                     : 1;  /**< Set when PKO sends packet data with the error bit
                                                         set. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PSLDBOF] */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PIDBOF] */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< Whenever SLI_DMAx_CNT[CNT] is not 0, the
                                                         SLI_DMAx_TIM[TIM] timer increments every SLI
                                                         clock.
                                                         DTIME[x] is set whenever SLI_DMAx_TIM[TIM] >
                                                         SLI_DMAx_INT_LEVEL[TIME].
                                                         DTIME[x] is normally cleared by clearing
                                                         SLI_DMAx_CNT[CNT] (which also clears
                                                         SLI_DMAx_TIM[TIM]). */
	uint64_t dcnt                         : 2;  /**< DCNT[x] is set whenever SLI_DMAx_CNT[CNT] >
                                                         SLI_DMAx_INT_LEVEL[CNT].
                                                         DCNT[x] is normally cleared by decreasing
                                                         SLI_DMAx_CNT[CNT]. */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts. */
	uint64_t reserved_20_31               : 12;
	uint64_t mac1_int                     : 1;  /**< Interrupt from MAC1.
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB_INT) */
	uint64_t mac0_int                     : 1;  /**< Interrupt from MAC0.
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB_INT) */
	uint64_t mio_int1                     : 1;  /**< Interrupt from MIO for PORT 1.
                                                         See CIU_INT33_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT33_EN0, CIU_INT33_EN1) */
	uint64_t mio_int0                     : 1;  /**< Interrupt from MIO for PORT 0.
                                                         See CIU_INT32_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT32_EN0, CIU_INT32_EN1) */
	uint64_t m1_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m1_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Packet Timer has an interrupt. Which rings can
                                                         be found in SLI_PKT_TIME_INT. */
	uint64_t pcnt                         : 1;  /**< Packet Counter has an interrupt. Which rings can
                                                         be found in SLI_PKT_CNT_INT. */
	uint64_t iob2big                      : 1;  /**< A requested IOBDMA is to large. */
	uint64_t bar0_to                      : 1;  /**< BAR0 R/W to a NCB device did not receive
                                                         read-data/commit in 0xffff core clocks. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< A read or write transfer did not complete
                                                         within 0xffff core clocks. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t reserved_20_31               : 12;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t reserved_58_59               : 2;
	uint64_t ill_pad                      : 1;
	uint64_t pipe_err                     : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} cn68xx;
	struct cvmx_sli_int_sum_cn68xx        cn68xxp1;
	struct cvmx_sli_int_sum_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Set when a BAR0 address R/W falls into theaddress
                                                         range of the Packet-CSR, but for an unused
                                                         address. */
	uint64_t sprt3_err                    : 1;  /**< Reserved. */
	uint64_t sprt2_err                    : 1;  /**< When an error response received on SLI port 2
                                                         this bit is set. */
	uint64_t sprt1_err                    : 1;  /**< When an error response received on SLI port 1
                                                         this bit is set. */
	uint64_t sprt0_err                    : 1;  /**< When an error response received on SLI port 0
                                                         this bit is set. */
	uint64_t pins_err                     : 1;  /**< When a read error occurs on a packet instruction
                                                         this bit is set. */
	uint64_t pop_err                      : 1;  /**< When a read error occurs on a packet scatter
                                                         pointer pair this bit is set. */
	uint64_t pdi_err                      : 1;  /**< When a read error occurs on a packet data read
                                                         this bit is set. */
	uint64_t pgl_err                      : 1;  /**< When a read error occurs on a packet gather list
                                                         read this bit is set. */
	uint64_t pin_bp                       : 1;  /**< Packet input count has exceeded the WMARK.
                                                         See SLI_PKT_IN_BP */
	uint64_t pout_err                     : 1;  /**< Set when PKO sends packet data with the error bit
                                                         set. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PSLDBOF] */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PIDBOF] */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< Whenever SLI_DMAx_CNT[CNT] is not 0, the
                                                         SLI_DMAx_TIM[TIM] timer increments every SLI
                                                         clock.
                                                         DTIME[x] is set whenever SLI_DMAx_TIM[TIM] >
                                                         SLI_DMAx_INT_LEVEL[TIME].
                                                         DTIME[x] is normally cleared by clearing
                                                         SLI_DMAx_CNT[CNT] (which also clears
                                                         SLI_DMAx_TIM[TIM]). */
	uint64_t dcnt                         : 2;  /**< DCNT[x] is set whenever SLI_DMAx_CNT[CNT] >
                                                         SLI_DMAx_INT_LEVEL[CNT].
                                                         DCNT[x] is normally cleared by decreasing
                                                         SLI_DMAx_CNT[CNT]. */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts. */
	uint64_t reserved_30_31               : 2;
	uint64_t mac2_int                     : 1;  /**< Interrupt from MAC2.
                                                         See PEM2_INT_SUM (enabled by PEM2_INT_ENB_INT) */
	uint64_t mio_int2                     : 1;  /**< Interrupt from MIO for PORT 2.
                                                         See CIU_INT32_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT32_EN0, CIU_INT32_EN1) */
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m2_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m2_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m2_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t mac1_int                     : 1;  /**< Interrupt from MAC1.
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB_INT) */
	uint64_t mac0_int                     : 1;  /**< Interrupt from MAC0.
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB_INT) */
	uint64_t mio_int1                     : 1;  /**< Interrupt from MIO for PORT 1.
                                                         See CIU_INT33_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT33_EN0, CIU_INT33_EN1) */
	uint64_t mio_int0                     : 1;  /**< Interrupt from MIO for PORT 0.
                                                         See CIU_INT32_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT32_EN0, CIU_INT32_EN1) */
	uint64_t m1_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m1_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Packet Timer has an interrupt. Which rings can
                                                         be found in SLI_PKT_TIME_INT. */
	uint64_t pcnt                         : 1;  /**< Packet Counter has an interrupt. Which rings can
                                                         be found in SLI_PKT_CNT_INT. */
	uint64_t iob2big                      : 1;  /**< A requested IOBDMA is to large. */
	uint64_t bar0_to                      : 1;  /**< BAR0 R/W to a NCB device did not receive
                                                         read-data/commit in 0xffff core clocks. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< A read or write transfer did not complete
                                                         within 0xffff core clocks. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t mio_int2                     : 1;
	uint64_t mac2_int                     : 1;
	uint64_t reserved_30_31               : 2;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn70xx;
	struct cvmx_sli_int_sum_cn70xx        cn70xxp1;
	struct cvmx_sli_int_sum_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t sprt3_err                    : 1;  /**< SLI/DPI sets [SPRT3_ERR] when it receives an error response on a completion for a
                                                         transaction mastered by DPI from MAC3/FPORT=3. When MAC3 is PEM3 (PCIe),
                                                         the error response may be due to a UR or CA completion, or a timeout. DPI
                                                         masters all transactions created to service all of DPI Instruction (packet) input,
                                                         DPI packet output, and DPI DMA Instructions.
                                                         Throws SLI_INTSN_E::SLI_INT_SPRT3_ERR. */
	uint64_t sprt2_err                    : 1;  /**< SLI/DPI sets [SPRT2_ERR] when it receives an error response on a completion for a
                                                         read mastered by DPI from MAC2/FPORT=2. When MAC2 is PEM2 (PCIe),
                                                         the error response may be due to a UR or CA completion, or a timeout. DPI
                                                         masters all transactions created to service all of DPI Instruction (packet) input,
                                                         DPI packet output, and DPI DMA Instructions.
                                                         Throws SLI_INTSN_E::SLI_INT_SPRT2_ERR. */
	uint64_t sprt1_err                    : 1;  /**< SLI/DPI sets [SPRT1_ERR] when it receives an error response on a completion for a
                                                         read mastered by DPI from MAC1/FPORT=1. When MAC1 is PEM1 (PCIe),
                                                         the error response may be due to a UR or CA completion, or a timeout. DPI
                                                         masters all transactions created to service all of DPI Instruction (packet) input,
                                                         DPI packet output, and DPI DMA Instructions.
                                                         Throws SLI_INTSN_E::SLI_INT_SPRT1_ERR. */
	uint64_t sprt0_err                    : 1;  /**< SLI/DPI sets [SPRT0_ERR] when it receives an error response on a completion for a
                                                         read mastered by DPI from MAC0/FPORT=0. When MAC0 is PEM0 (PCIe),
                                                         the error response may be due to a UR or CA completion, or a timeout. DPI
                                                         masters all transactions created to service all of DPI Instruction (packet) input,
                                                         DPI packet output, and DPI DMA Instructions.
                                                         Throws SLI_INTSN_E::SLI_INT_SPRT0_ERR. */
	uint64_t pins_err                     : 1;  /**< Packet instruction read error. SLI/DPI sets [PINS_ERR] when it receives an error
                                                         response to a DPI Instruction (packet) input instruction read. Whenever
                                                         SLI/DPI sets [PINS_ERR], it also will have set one of [SPRT*_ERR].
                                                         Throws SLI_INTSN_E::SLI_INT_PINS_ERR. */
	uint64_t pop_err                      : 1;  /**< Packet pointer pair error. SLI/DPI sets [POP_ERR] when it receives
                                                         an error response on a DPI packet output buffer/info pointer pair read.
                                                         Whenever SLI/DPI sets [POP_ERR], it also will have set one of [SPRT*_ERR].
                                                         Throws SLI_INTSN_E::SLI_INT_POP_ERR. */
	uint64_t pdi_err                      : 1;  /**< Packet data read error. SLI/DPI sets [PDI_ERR] when it receives an error
                                                         response to a DPI Instruction (packet) input packet data read. Whenever
                                                         SLI/DPI sets [PDI_ERR], it also will have set one of [SPRT*_ERR].
                                                         Throws SLI_INTSN_E::SLI_INT_PDI_ERR. */
	uint64_t pgl_err                      : 1;  /**< Packet gather list read error. SLI/DPI sets [PDI_ERR] when it receives an error
                                                         response to a DPI Instruction (packet) input indirect gather list read. Whenever
                                                         SLI/DPI sets [PGL_ERR], it also will have set one of [SPRT*_ERR].
                                                         Throws SLI_INTSN_E::SLI_INT_PGL_ERR. */
	uint64_t reserved_50_51               : 2;
	uint64_t psldbof                      : 1;  /**< Packet output doorbell count overflowed. SLI/DPI sets [PSLDBOF]
                                                         whenever a DPI packet output doorbell overflows.
                                                         Throws SLI_INTSN_E::SLI_INT_PSLDBOF. */
	uint64_t pidbof                       : 1;  /**< Packet instruction input doorbell count overflowed. SLI/DPI sets
                                                         [PIDBOF] whenever a DPI Instruction (packet) input doorbell overflows.
                                                         Throws SLI_INTSN_E::SLI_INT_PIDBOF. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< Whenever SLI_DMA()_CNT[CNT] is not 0, the SLI_DMA()_TIM[TIM] timer increments
                                                         every SLI clock. DTIME<x> is set whenever SLI_DMA()_TIM[TIM] >
                                                         SLI_DMA()_INT_LEVEL[TIME]. DTIME<x> is normally cleared by clearing
                                                         SLI_DMA()_CNT[CNT] (which also clears SLI_DMA()_TIM[TIM]). Throws
                                                         SLI_INTSN_E::SLI_INT_DTIME0/1. */
	uint64_t dcnt                         : 2;  /**< DCNT<x> is set whenever SLI_DMAx_CNT[CNT] > SLI_DMA()_INT_LEVEL[CNT]. DCNT<x> is
                                                         normally cleared by decreasing SLI_DMA()_CNT[CNT]. Throws SLI_INTSN_E::SLI_INT_DCNT0/1. */
	uint64_t dmafi                        : 2;  /**< DMA set forced interrupts. Set by SLI/DPI after completing a DPI DMA
                                                         Instruction with DPI_DMA_INSTR_HDR_S[FI] set.
                                                         Throws SLI_INTSN_E::SLI_INT_DMAFI0/1. */
	uint64_t reserved_29_31               : 3;
	uint64_t vf_err                       : 1;  /**< Illegal BAR0 access from a virtual function. SLI/DPI may set [VF_ERR] whenever a
                                                         virtual function accesses a VF BAR0 address/CSR that it shouldn't have.
                                                         Throws SLI_INTSN_E::SLI_INT_VF_ERR. */
	uint64_t m3_un_wi                     : 1;  /**< Received unsupported N-TLP for window register from MAC 3. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M3_UN_WI. */
	uint64_t m3_un_b0                     : 1;  /**< Received unsupported N-TLP for Bar0 from MAC 3. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M3_UN_B0. */
	uint64_t m3_up_wi                     : 1;  /**< Received unsupported P-TLP for window register from MAC 3. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M3_UP_WI. */
	uint64_t m3_up_b0                     : 1;  /**< Received unsupported P-TLP for Bar0 from MAC 3. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M3_UP_B0. */
	uint64_t m2_un_wi                     : 1;  /**< Received unsupported N-TLP for window register from MAC 2. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M2_UN_WI. */
	uint64_t m2_un_b0                     : 1;  /**< Received unsupported N-TLP for Bar0 from MAC 2. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M2_UN_B0. */
	uint64_t m2_up_wi                     : 1;  /**< Received unsupported P-TLP for window register from MAC 2. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M2_UP_WI. */
	uint64_t m2_up_b0                     : 1;  /**< Received unsupported P-TLP for Bar0 from MAC 2. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M2_UP_B0. */
	uint64_t reserved_18_19               : 2;
	uint64_t mio_int1                     : 1;  /**< CIU interrupt output for MAC 1. A copy of CIU3_DEST(x)_IO_INT[INTR],
                                                         where x=CIU_DEST_IO_E::PEM(1) (i.e. x=2). */
	uint64_t mio_int0                     : 1;  /**< CIU interrupt output for MAC 0. A copy of CIU3_DEST(x)_IO_INT[INTR],
                                                         where x=CIU_DEST_IO_E::PEM(0) (i.e. x=1). */
	uint64_t m1_un_wi                     : 1;  /**< Received unsupported N-TLP for window register from MAC 1. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M1_UN_WI. */
	uint64_t m1_un_b0                     : 1;  /**< Received unsupported N-TLP for Bar 0 from MAC 1. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M1_UN_B0. */
	uint64_t m1_up_wi                     : 1;  /**< Received unsupported P-TLP for window register from MAC 1. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M1_UP_WI. */
	uint64_t m1_up_b0                     : 1;  /**< Received unsupported P-TLP for Bar 0 from MAC 1. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M1_UP_B0. */
	uint64_t m0_un_wi                     : 1;  /**< Received unsupported N-TLP for window register from MAC 0. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M0_UP_WI. */
	uint64_t m0_un_b0                     : 1;  /**< Received unsupported N-TLP for Bar 0 from MAC 0. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M0_UP_B0. */
	uint64_t m0_up_wi                     : 1;  /**< Received unsupported P-TLP for window register from MAC 0. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M0_UP_WI. */
	uint64_t m0_up_b0                     : 1;  /**< Received unsupported P-TLP for Bar 0 from MAC 0. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M0_UP_B0. */
	uint64_t mio_int3                     : 1;  /**< CIU interrupt output for MAC 3. A copy of CIU3_DEST(x)_IO_INT[INTR],
                                                         where x=CIU_DEST_IO_E::PEM(3) (i.e. x=4). */
	uint64_t mio_int2                     : 1;  /**< CIU interrupt output for MAC 2. A copy of CIU3_DEST(x)_IO_INT[INTR],
                                                         where x=CIU_DEST_IO_E::PEM(2) (i.e. x=3). */
	uint64_t ptime                        : 1;  /**< Packet timer has an interrupt. Asserts if, for any i, both
                                                         SLI_PKT_TIME_INT<i> and SLI_PKT(i)_OUTPUT_CONTROL[TENB] are set.
                                                         [PTIME] assertion throws SLI_INTSN_E::SLI_INT_PTIME to CIU. */
	uint64_t pcnt                         : 1;  /**< Packet counter has an interrupt. Asserts if, for any i, both
                                                         SLI_PKT_CNT_INT<i> and SLI_PKT(i)_OUTPUT_CONTROL[CENB] are set.
                                                         [PCNT] assertion throws SLI_INTSN_E::SLI_INT_PCNT to CIU. */
	uint64_t reserved_1_3                 : 3;
	uint64_t rml_to                       : 1;  /**< A read or write transfer to a RSL that did not complete within
                                                         SLI_WINDOW_CTL[TIME] coprocessor-clock cycles, or a notification from the CCPI
                                                         that is has sent a previously written command and can take another within
                                                         SLI_WINDOW_CTL[OCX_TIME].
                                                         Throws SLI_INTSN_E::SLI_INT_RML_TO. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_3                 : 3;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t mio_int2                     : 1;
	uint64_t mio_int3                     : 1;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t reserved_18_19               : 2;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t vf_err                       : 1;
	uint64_t reserved_29_31               : 3;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t reserved_60_63               : 4;
#endif
	} cn78xxp1;
	struct cvmx_sli_int_sum_cn61xx        cnf71xx;
};
typedef union cvmx_sli_int_sum cvmx_sli_int_sum_t;

/**
 * cvmx_sli_last_win_rdata0
 *
 * The data from the last initiated window read by MAC 0.
 *
 */
union cvmx_sli_last_win_rdata0 {
	uint64_t u64;
	struct cvmx_sli_last_win_rdata0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Last window read data. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_last_win_rdata0_s     cn61xx;
	struct cvmx_sli_last_win_rdata0_s     cn63xx;
	struct cvmx_sli_last_win_rdata0_s     cn63xxp1;
	struct cvmx_sli_last_win_rdata0_s     cn66xx;
	struct cvmx_sli_last_win_rdata0_s     cn68xx;
	struct cvmx_sli_last_win_rdata0_s     cn68xxp1;
	struct cvmx_sli_last_win_rdata0_s     cn70xx;
	struct cvmx_sli_last_win_rdata0_s     cn70xxp1;
	struct cvmx_sli_last_win_rdata0_s     cnf71xx;
};
typedef union cvmx_sli_last_win_rdata0 cvmx_sli_last_win_rdata0_t;

/**
 * cvmx_sli_last_win_rdata1
 *
 * The data from the last initiated window read by MAC 1.
 *
 */
union cvmx_sli_last_win_rdata1 {
	uint64_t u64;
	struct cvmx_sli_last_win_rdata1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Last window read data. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_last_win_rdata1_s     cn61xx;
	struct cvmx_sli_last_win_rdata1_s     cn63xx;
	struct cvmx_sli_last_win_rdata1_s     cn63xxp1;
	struct cvmx_sli_last_win_rdata1_s     cn66xx;
	struct cvmx_sli_last_win_rdata1_s     cn68xx;
	struct cvmx_sli_last_win_rdata1_s     cn68xxp1;
	struct cvmx_sli_last_win_rdata1_s     cn70xx;
	struct cvmx_sli_last_win_rdata1_s     cn70xxp1;
	struct cvmx_sli_last_win_rdata1_s     cnf71xx;
};
typedef union cvmx_sli_last_win_rdata1 cvmx_sli_last_win_rdata1_t;

/**
 * cvmx_sli_last_win_rdata2
 *
 * The data from the last initiated window read by MAC 2.
 *
 */
union cvmx_sli_last_win_rdata2 {
	uint64_t u64;
	struct cvmx_sli_last_win_rdata2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Last window read data. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_last_win_rdata2_s     cn61xx;
	struct cvmx_sli_last_win_rdata2_s     cn66xx;
	struct cvmx_sli_last_win_rdata2_s     cn70xx;
	struct cvmx_sli_last_win_rdata2_s     cn70xxp1;
	struct cvmx_sli_last_win_rdata2_s     cnf71xx;
};
typedef union cvmx_sli_last_win_rdata2 cvmx_sli_last_win_rdata2_t;

/**
 * cvmx_sli_last_win_rdata3
 *
 * The data from the last initiated window read by MAC 3.
 *
 */
union cvmx_sli_last_win_rdata3 {
	uint64_t u64;
	struct cvmx_sli_last_win_rdata3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Last window read data. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_last_win_rdata3_s     cn61xx;
	struct cvmx_sli_last_win_rdata3_s     cn66xx;
	struct cvmx_sli_last_win_rdata3_s     cn70xx;
	struct cvmx_sli_last_win_rdata3_s     cn70xxp1;
	struct cvmx_sli_last_win_rdata3_s     cnf71xx;
};
typedef union cvmx_sli_last_win_rdata3 cvmx_sli_last_win_rdata3_t;

/**
 * cvmx_sli_mac#_pf#_dma_vf_int
 *
 * When an error response is received for a VF DMA transaction read, the appropriate VF indexed
 * bit is set.  The appropriate PF should read the appropriate register.
 * Indexed by (MAC index) SLI_PORT_E.
 * This CSR array is valid only for SLI_PORT_E::PEM0 PF0.
 */
union cvmx_sli_macx_pfx_dma_vf_int {
	uint64_t u64;
	struct cvmx_sli_macx_pfx_dma_vf_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t vf_int                       : 64; /**< When an error response is received for a VF DMA transaction read, the appropriate VF
                                                         indexed bit is set. */
#else
	uint64_t vf_int                       : 64;
#endif
	} s;
	struct cvmx_sli_macx_pfx_dma_vf_int_s cn73xx;
	struct cvmx_sli_macx_pfx_dma_vf_int_s cn78xx;
	struct cvmx_sli_macx_pfx_dma_vf_int_s cnf75xx;
};
typedef union cvmx_sli_macx_pfx_dma_vf_int cvmx_sli_macx_pfx_dma_vf_int_t;

/**
 * cvmx_sli_mac#_pf#_dma_vf_int_enb
 *
 * Indexed by (MAC index) SLI_PORT_E.
 * This CSR array is valid only for SLI_PORT_E::PEM0 PF0.
 */
union cvmx_sli_macx_pfx_dma_vf_int_enb {
	uint64_t u64;
	struct cvmx_sli_macx_pfx_dma_vf_int_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t vf_int_enb                   : 64; /**< Enables DMA interrupts for the corresponding VF. */
#else
	uint64_t vf_int_enb                   : 64;
#endif
	} s;
	struct cvmx_sli_macx_pfx_dma_vf_int_enb_s cn73xx;
	struct cvmx_sli_macx_pfx_dma_vf_int_enb_s cn78xx;
	struct cvmx_sli_macx_pfx_dma_vf_int_enb_s cnf75xx;
};
typedef union cvmx_sli_macx_pfx_dma_vf_int_enb cvmx_sli_macx_pfx_dma_vf_int_enb_t;

/**
 * cvmx_sli_mac#_pf#_flr_vf_int
 *
 * Indexed by (MAC index) SLI_PORT_E.
 * This CSR array is valid only for SLI_PORT_E::PEM0 PF0.
 */
union cvmx_sli_macx_pfx_flr_vf_int {
	uint64_t u64;
	struct cvmx_sli_macx_pfx_flr_vf_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t vf_int                       : 64; /**< When an VF causes a flr the appropriate VF indexed bit is set. */
#else
	uint64_t vf_int                       : 64;
#endif
	} s;
	struct cvmx_sli_macx_pfx_flr_vf_int_s cn73xx;
	struct cvmx_sli_macx_pfx_flr_vf_int_s cn78xx;
	struct cvmx_sli_macx_pfx_flr_vf_int_s cnf75xx;
};
typedef union cvmx_sli_macx_pfx_flr_vf_int cvmx_sli_macx_pfx_flr_vf_int_t;

/**
 * cvmx_sli_mac#_pf#_int_enb
 *
 * Interrupt enable register for a given PF SLI_MAC()_PF()_INT_SUM register.
 * Indexed by (MAC index) SLI_PORT_E.
 */
union cvmx_sli_macx_pfx_int_enb {
	uint64_t u64;
	struct cvmx_sli_macx_pfx_int_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pppf_err                     : 1;  /**< Enables SLI_MAC()_PF()_INT_SUM[PPPF_ERR] to generate an interrupt to the MAC core
                                                         for MSI/INTA. */
	uint64_t ppvf_err                     : 1;  /**< Enables SLI_MAC()_PF()_INT_SUM[PPVF_ERR] to generate an interrupt to the MAC core
                                                         for MSI/INTA.
                                                         Note: the corresponding interrupt will only ever be set for SR-IOV PF's:
                                                         SLI_PORT_E::PEM0 PF0. */
	uint64_t pktpf_err                    : 1;  /**< Enables SLI_MAC()_PF()_INT_SUM[PKTPF_ERR] to generate an interrupt to the MAC core
                                                         for MSI/INTA. */
	uint64_t pktvf_err                    : 1;  /**< Enables SLI_MAC()_PF()_INT_SUM[PKTVF_ERR] to generate an interrupt to the MAC core
                                                         for MSI/INTA.
                                                         Note: the corresponding interrupt will only ever be set for SR-IOV PF's:
                                                         SLI_PORT_E::PEM0 PF0. */
	uint64_t dmapf_err                    : 1;  /**< Enables SLI_MAC()_PF()_INT_SUM[DMAPF_ERR] to generate an interrupt to the MAC core
                                                         for MSI/INTA. */
	uint64_t dmavf_err                    : 1;  /**< Enables SLI_MAC()_PF()_INT_SUM[DMAVF_ERR] to generate an interrupt to the MAC core
                                                         for MSI/INTA.
                                                         Note: the corresponding interrupt will only ever be set for SR-IOV PF's:
                                                         SLI_PORT_E::PEM0 PF0. */
	uint64_t vf_mbox                      : 1;  /**< Enables SLI_MAC()_PF()_INT_SUM[VF_MBOX] to generate an interrupt to the MAC core
                                                         for MSI/INTA.
                                                         Note: the corresponding interrupt will only ever be set for SR-IOV PF's:
                                                         SLI_PORT_E::PEM0 PF0. */
	uint64_t reserved_38_56               : 19;
	uint64_t dtime                        : 2;  /**< Enables SLI_MAC()_PF()_INT_SUM[DTIME] to generate an interrupt to the MAC core for
                                                         MSI/INTA. */
	uint64_t dcnt                         : 2;  /**< Enables SLI_MAC()_PF()_INT_SUM[DCNT] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t dmafi                        : 2;  /**< Enables SLI_MAC()_PF()_INT_SUM[DMAFI] to generate an interrupt to the MAC core for
                                                         MSI/INTA. */
	uint64_t reserved_12_31               : 20;
	uint64_t un_wi                        : 1;  /**< Enables SLI_MAC()_PF()_INT_SUM[UN_WI] to generate an interrupt to the MAC core for
                                                         MSI/INTA. */
	uint64_t un_b0                        : 1;  /**< Enables SLI_MAC()_PF()_INT_SUM[UN_B0] to generate an interrupt to the MAC core for
                                                         MSI/INTA. */
	uint64_t up_wi                        : 1;  /**< Enables SLI_MAC()_PF()_INT_SUM[UP_WI] to generate an interrupt to the MAC core for
                                                         MSI/INTA. */
	uint64_t up_b0                        : 1;  /**< Enables SLI_MAC()_PF()_INT_SUM[UP_B0] to generate an interrupt to the MAC core for
                                                         MSI/INTA. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_MAC()_PF()_INT_SUM[PTIME] to generate an interrupt to the MAC core for
                                                         MSI/INTA. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_MAC()_PF()_INT_SUM[PCNT] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t reserved_2_3                 : 2;
	uint64_t mio_int                      : 1;  /**< Enables SLI_MAC()_PF()_INT_SUM[MIO_INT] to generate an interrupt to the MAC core
                                                         for MSI/INTA. */
	uint64_t rml_to                       : 1;  /**< Enables SLI_MAC()_PF()_INT_SUM[RML_TO] to generate an interrupt to the MAC core
                                                         for MSI/INTA. */
#else
	uint64_t rml_to                       : 1;
	uint64_t mio_int                      : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t up_b0                        : 1;
	uint64_t up_wi                        : 1;
	uint64_t un_b0                        : 1;
	uint64_t un_wi                        : 1;
	uint64_t reserved_12_31               : 20;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_56               : 19;
	uint64_t vf_mbox                      : 1;
	uint64_t dmavf_err                    : 1;
	uint64_t dmapf_err                    : 1;
	uint64_t pktvf_err                    : 1;
	uint64_t pktpf_err                    : 1;
	uint64_t ppvf_err                     : 1;
	uint64_t pppf_err                     : 1;
#endif
	} s;
	struct cvmx_sli_macx_pfx_int_enb_s    cn73xx;
	struct cvmx_sli_macx_pfx_int_enb_s    cn78xx;
	struct cvmx_sli_macx_pfx_int_enb_s    cnf75xx;
};
typedef union cvmx_sli_macx_pfx_int_enb cvmx_sli_macx_pfx_int_enb_t;

/**
 * cvmx_sli_mac#_pf#_int_sum
 *
 * Interrupt summary register for a given PF. Indexed (MAC index) by SLI_PORT_E.
 * The fields in this register are set when an interrupt condition occurs; write 1 to clear.
 */
union cvmx_sli_macx_pfx_int_sum {
	uint64_t u64;
	struct cvmx_sli_macx_pfx_int_sum_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pppf_err                     : 1;  /**< When an error response is received for a PF PP transaction read, this bit is set. */
	uint64_t ppvf_err                     : 1;  /**< When an error response is received for a VF PP transaction read, this bit is set.
                                                         A subsequent read to SLI_MAC()_PF()_PP_VF_INT is required to discover which VF.
                                                         Note: this will only ever be set for SR-IOV PF's: SLI_PORT_E::PEM0 PF0. */
	uint64_t pktpf_err                    : 1;  /**< This bit is set when any of the following events occur
                                                         1) An error response is received for PF packet transaction read.
                                                         2) A doorbell overflow for a ring associated with this PF occurs in
                                                            SLI_PKT()_INSTR_BAOFF_DBELL or SLI_PKT()_SLIST_BAOFF_DBELL.
                                                         3) A packet was received from PKO for ring X, SLI_PKT_GBL_CONTROL[NOPTR_D] = 0
                                                            and SLI_PKTX_SLIST_BAOFF_DBELL[DBELL] = 0. */
	uint64_t pktvf_err                    : 1;  /**< This bit is set when any of the following events occur
                                                         1) An error response is received for VF packet transaction read.
                                                         2) A doorbell overflow for a ring associated with a VF occurs in
                                                            SLI_PKT()_INSTR_BAOFF_DBELL or SLI_PKT()_SLIST_BAOFF_DBELL.
                                                         3) A packet was received from PKO for ring X, SLI_PKT_GBL_CONTROL[NOPTR_D] = 0
                                                            and SLI_PKTX_SLIST_BAOFF_DBELL[DBELL] = 0.
                                                         4) An illegal bar0 bar1 bar2 memory access from a VF occurs.
                                                         A subsequent read to SLI_MAC()_PF()_PKT_VF_INT is required to discover which VF.
                                                         Note: this will only ever be set for SR-IOV PF's: SLI_PORT_E::PEM0 PF0. */
	uint64_t dmapf_err                    : 1;  /**< When an error response is received for a PF DMA transaction read, this bit is set. */
	uint64_t dmavf_err                    : 1;  /**< When an error response is received for a VF DMA transaction read, this bit is set.
                                                         A subsequent read to SLI_MAC()_PF()_DMA_VF_INT is required to discover which VF.
                                                         Note: this will only ever be set for SR-IOV PF's: SLI_PORT_E::PEM0 PF0. */
	uint64_t vf_mbox                      : 1;  /**< When an VF wants to communicate to a PF it writes its SLI_PKT_PF_MBOX_SIG2 register
                                                         causing
                                                         this bit to be set.
                                                         A subsequent read to SLI_MAC()_PF()_MBOX_INT is required to discover which VF.
                                                         Note: this will only ever be set for SR-IOV PF's: SLI_PORT_E::PEM0 PF0. */
	uint64_t reserved_38_56               : 19;
	uint64_t dtime                        : 2;  /**< Whenever SLI_DMA()_CNT[CNT] is not 0, the SLI_DMA()_TIM[TIM] timer increments
                                                         every SLI clock. DTIME<x> is set whenever SLI_DMA()_TIM[TIM] >
                                                         SLI_DMA()_INT_LEVEL[TIME]. DTIME<x> is normally cleared by clearing
                                                         SLI_DMA()_CNT[CNT] (which also clears SLI_DMA()_TIM[TIM]). */
	uint64_t dcnt                         : 2;  /**< DCNT<x> is set whenever SLI_DMAx_CNT[CNT] > SLI_DMA()_INT_LEVEL[CNT]. DCNT<x> is
                                                         normally cleared by decreasing SLI_DMA()_CNT[CNT]. */
	uint64_t dmafi                        : 2;  /**< DMA set forced interrupts. Set by SLI/DPI after completing a DPI DMA
                                                         Instruction with DPI_DMA_INSTR_HDR_S[FI] set. */
	uint64_t reserved_12_31               : 20;
	uint64_t un_wi                        : 1;  /**< Received unsupported N-TLP for window register from MAC. This occurs when the window
                                                         registers are disabled and a window register access occurs.
                                                         This can only be set by a PF and not a VF access. */
	uint64_t un_b0                        : 1;  /**< Received unsupported N-TLP for Bar 0 from MAC. This occurs when the BAR 0 address space
                                                         is disabled.
                                                         This can only be set by a PF and not a VF access. */
	uint64_t up_wi                        : 1;  /**< Received unsupported P-TLP for window register from MAC. This occurs when the window
                                                         registers are disabled and a window register access occurs.
                                                         This can only be set by a PF and not a VF access. */
	uint64_t up_b0                        : 1;  /**< Received unsupported P-TLP for Bar 0 from MAC. This occurs when the BAR 0 address space
                                                         This can only be set by a PF and not a VF access. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< The preferred method to service PTIME interrupts is through MSIX rings.  This bit is only
                                                         used
                                                         for legacy code.
                                                         Packet timer has an interrupt. The specific rings can be found in SLI_PKT_TIME_INT. */
	uint64_t pcnt                         : 1;  /**< The preferred method to service PCNT interrupts is through MSIX rings.  This bit is only
                                                         used
                                                         for legacy code.
                                                         Packet counter has an interrupt. The specific rings can be found in SLI_PKT_CNT_INT. */
	uint64_t reserved_2_3                 : 2;
	uint64_t mio_int                      : 1;  /**< Interrupt from CIU for this PF. CIU can output an interrupt for each each MAC
                                                         included in the CIU_DEST_IO_E enumeration. SLI can send the CIU interrupt for a
                                                         MAC out its PF0. [MIO_INT] bits are copies of corresponding CIU3_DEST()_IO_INT[INTR]
                                                         bits.
                                                         [MIO_INT] cannot assert for SLI_PORT_E::SRIO0 PF0 and SLI_PORT_E::SRIO1 PF0. */
	uint64_t rml_to                       : 1;  /**< A read or write transfer to a RSL that did not complete within
                                                         SLI_WINDOW_CTL[TIME] coprocessor-clock cycles. */
#else
	uint64_t rml_to                       : 1;
	uint64_t mio_int                      : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t up_b0                        : 1;
	uint64_t up_wi                        : 1;
	uint64_t un_b0                        : 1;
	uint64_t un_wi                        : 1;
	uint64_t reserved_12_31               : 20;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_56               : 19;
	uint64_t vf_mbox                      : 1;
	uint64_t dmavf_err                    : 1;
	uint64_t dmapf_err                    : 1;
	uint64_t pktvf_err                    : 1;
	uint64_t pktpf_err                    : 1;
	uint64_t ppvf_err                     : 1;
	uint64_t pppf_err                     : 1;
#endif
	} s;
	struct cvmx_sli_macx_pfx_int_sum_s    cn73xx;
	struct cvmx_sli_macx_pfx_int_sum_s    cn78xx;
	struct cvmx_sli_macx_pfx_int_sum_s    cnf75xx;
};
typedef union cvmx_sli_macx_pfx_int_sum cvmx_sli_macx_pfx_int_sum_t;

/**
 * cvmx_sli_mac#_pf#_mbox_int
 *
 * When a VF to PF MBOX write occurs the appropriate bit is set.
 * Indexed by (MAC index) SLI_PORT_E.
 * This CSR array is valid only for SLI_PORT_E::PEM0 PF0.
 */
union cvmx_sli_macx_pfx_mbox_int {
	uint64_t u64;
	struct cvmx_sli_macx_pfx_mbox_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t vf_int                       : 64; /**< When an VF wants to communicate to a PF it writes its SLI_PKT()_PF_VF_MBOX_SIG()
                                                         register the appropriate ring indexed bit is set.  The PF should then read  the
                                                         appropriate SLI_PKT()_PF_VF_MBOX_SIG() indexed register. */
#else
	uint64_t vf_int                       : 64;
#endif
	} s;
	struct cvmx_sli_macx_pfx_mbox_int_s   cn73xx;
	struct cvmx_sli_macx_pfx_mbox_int_s   cn78xx;
	struct cvmx_sli_macx_pfx_mbox_int_s   cnf75xx;
};
typedef union cvmx_sli_macx_pfx_mbox_int cvmx_sli_macx_pfx_mbox_int_t;

/**
 * cvmx_sli_mac#_pf#_pkt_vf_int
 *
 * When an error response is received for a VF PP transaction read, a doorbell
 * overflow for a ring associated with a VF occurs or an illegal memory access from a VF occurs,
 * the appropriate VF indexed bit is set.  The appropriate PF should read the appropriate
 * register.
 * Indexed by (MAC index) SLI_PORT_E.
 * This CSR array is valid only for SLI_PORT_E::PEM0 PF0.
 */
union cvmx_sli_macx_pfx_pkt_vf_int {
	uint64_t u64;
	struct cvmx_sli_macx_pfx_pkt_vf_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t vf_int                       : 64; /**< When an error response is received for a VF PKT transaction read, the appropriate VF
                                                         indexed bit is set. */
#else
	uint64_t vf_int                       : 64;
#endif
	} s;
	struct cvmx_sli_macx_pfx_pkt_vf_int_s cn73xx;
	struct cvmx_sli_macx_pfx_pkt_vf_int_s cn78xx;
	struct cvmx_sli_macx_pfx_pkt_vf_int_s cnf75xx;
};
typedef union cvmx_sli_macx_pfx_pkt_vf_int cvmx_sli_macx_pfx_pkt_vf_int_t;

/**
 * cvmx_sli_mac#_pf#_pkt_vf_int_enb
 *
 * Indexed by (MAC index) SLI_PORT_E.
 * This CSR array is valid only for SLI_PORT_E::PEM0 PF0.
 */
union cvmx_sli_macx_pfx_pkt_vf_int_enb {
	uint64_t u64;
	struct cvmx_sli_macx_pfx_pkt_vf_int_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t vf_int_enb                   : 64; /**< Enables PKT interrupts for the corresponding VF. */
#else
	uint64_t vf_int_enb                   : 64;
#endif
	} s;
	struct cvmx_sli_macx_pfx_pkt_vf_int_enb_s cn73xx;
	struct cvmx_sli_macx_pfx_pkt_vf_int_enb_s cn78xx;
	struct cvmx_sli_macx_pfx_pkt_vf_int_enb_s cnf75xx;
};
typedef union cvmx_sli_macx_pfx_pkt_vf_int_enb cvmx_sli_macx_pfx_pkt_vf_int_enb_t;

/**
 * cvmx_sli_mac#_pf#_pp_vf_int
 *
 * When an error response is received for a VF PP transaction read, the appropriate VF indexed
 * bit is set.  The appropriate PF should read the appropriate register.
 * Indexed by (MAC index) SLI_PORT_E.
 * This CSR array is valid only for SLI_PORT_E::PEM0 PF0.
 */
union cvmx_sli_macx_pfx_pp_vf_int {
	uint64_t u64;
	struct cvmx_sli_macx_pfx_pp_vf_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t vf_int                       : 64; /**< When an error response is received for a VF PP transaction read, the appropriate VF
                                                         indexed bit is set. */
#else
	uint64_t vf_int                       : 64;
#endif
	} s;
	struct cvmx_sli_macx_pfx_pp_vf_int_s  cn73xx;
	struct cvmx_sli_macx_pfx_pp_vf_int_s  cn78xx;
	struct cvmx_sli_macx_pfx_pp_vf_int_s  cnf75xx;
};
typedef union cvmx_sli_macx_pfx_pp_vf_int cvmx_sli_macx_pfx_pp_vf_int_t;

/**
 * cvmx_sli_mac#_pf#_pp_vf_int_enb
 *
 * Indexed by (MAC index) SLI_PORT_E.
 * This CSR array is valid only for SLI_PORT_E::PEM0 PF0.
 */
union cvmx_sli_macx_pfx_pp_vf_int_enb {
	uint64_t u64;
	struct cvmx_sli_macx_pfx_pp_vf_int_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t vf_int_enb                   : 64; /**< Enables PP interrupts for the corresponding VF. */
#else
	uint64_t vf_int_enb                   : 64;
#endif
	} s;
	struct cvmx_sli_macx_pfx_pp_vf_int_enb_s cn73xx;
	struct cvmx_sli_macx_pfx_pp_vf_int_enb_s cn78xx;
	struct cvmx_sli_macx_pfx_pp_vf_int_enb_s cnf75xx;
};
typedef union cvmx_sli_macx_pfx_pp_vf_int_enb cvmx_sli_macx_pfx_pp_vf_int_enb_t;

/**
 * cvmx_sli_mac_credit_cnt
 *
 * This register contains the number of credits for the MAC port FIFOs used by the SLI. This
 * value needs to be set before S2M traffic flow starts. A write operation to this register
 * causes the credit counts in the SLI for the MAC ports to be reset to the value in this
 * register if the corresponding disable bit in this register is set to 0.
 */
union cvmx_sli_mac_credit_cnt {
	uint64_t u64;
	struct cvmx_sli_mac_credit_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t p1_c_d                       : 1;  /**< When set, does not allow writing of P1_CCNT. */
	uint64_t p1_n_d                       : 1;  /**< When set, does not allow writing of P1_NCNT. */
	uint64_t p1_p_d                       : 1;  /**< When set, does not allow writing of P1_PCNT. */
	uint64_t p0_c_d                       : 1;  /**< When set, does not allow writing of P0_CCNT. */
	uint64_t p0_n_d                       : 1;  /**< When set, does not allow writing of P0_NCNT. */
	uint64_t p0_p_d                       : 1;  /**< When set, does not allow writing of P0_PCNT. */
	uint64_t p1_ccnt                      : 8;  /**< Port 1 C-TLP FIFO credits. Legal values are 0x25 to 0x80. */
	uint64_t p1_ncnt                      : 8;  /**< Port 1 N-TLP FIFO credits. Legal values are 0x5 to 0x20. */
	uint64_t p1_pcnt                      : 8;  /**< Port 1 P-TLP FIFO credits. Legal values are 0x25 to 0x80. */
	uint64_t p0_ccnt                      : 8;  /**< Port 0 C-TLP FIFO credits. Legal values are 0x25 to 0x80. */
	uint64_t p0_ncnt                      : 8;  /**< Port 0 N-TLP FIFO credits. Legal values are 0x5 to 0x20. */
	uint64_t p0_pcnt                      : 8;  /**< Port 0 P-TLP FIFO credits. Legal values are 0x25 to 0x80. */
#else
	uint64_t p0_pcnt                      : 8;
	uint64_t p0_ncnt                      : 8;
	uint64_t p0_ccnt                      : 8;
	uint64_t p1_pcnt                      : 8;
	uint64_t p1_ncnt                      : 8;
	uint64_t p1_ccnt                      : 8;
	uint64_t p0_p_d                       : 1;
	uint64_t p0_n_d                       : 1;
	uint64_t p0_c_d                       : 1;
	uint64_t p1_p_d                       : 1;
	uint64_t p1_n_d                       : 1;
	uint64_t p1_c_d                       : 1;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_sli_mac_credit_cnt_s      cn61xx;
	struct cvmx_sli_mac_credit_cnt_s      cn63xx;
	struct cvmx_sli_mac_credit_cnt_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t p1_ccnt                      : 8;  /**< Port1 C-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
	uint64_t p1_ncnt                      : 8;  /**< Port1 N-TLP FIFO Credits.
                                                         Legal values are 0x5 to 0x10. */
	uint64_t p1_pcnt                      : 8;  /**< Port1 P-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
	uint64_t p0_ccnt                      : 8;  /**< Port0 C-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
	uint64_t p0_ncnt                      : 8;  /**< Port0 N-TLP FIFO Credits.
                                                         Legal values are 0x5 to 0x10. */
	uint64_t p0_pcnt                      : 8;  /**< Port0 P-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
#else
	uint64_t p0_pcnt                      : 8;
	uint64_t p0_ncnt                      : 8;
	uint64_t p0_ccnt                      : 8;
	uint64_t p1_pcnt                      : 8;
	uint64_t p1_ncnt                      : 8;
	uint64_t p1_ccnt                      : 8;
	uint64_t reserved_48_63               : 16;
#endif
	} cn63xxp1;
	struct cvmx_sli_mac_credit_cnt_s      cn66xx;
	struct cvmx_sli_mac_credit_cnt_s      cn68xx;
	struct cvmx_sli_mac_credit_cnt_s      cn68xxp1;
	struct cvmx_sli_mac_credit_cnt_s      cn70xx;
	struct cvmx_sli_mac_credit_cnt_s      cn70xxp1;
	struct cvmx_sli_mac_credit_cnt_s      cn73xx;
	struct cvmx_sli_mac_credit_cnt_s      cn78xx;
	struct cvmx_sli_mac_credit_cnt_s      cn78xxp1;
	struct cvmx_sli_mac_credit_cnt_s      cnf71xx;
	struct cvmx_sli_mac_credit_cnt_s      cnf75xx;
};
typedef union cvmx_sli_mac_credit_cnt cvmx_sli_mac_credit_cnt_t;

/**
 * cvmx_sli_mac_credit_cnt2
 *
 * This register contains the number of credits for the MAC port FIFOs (for MACs 2 and 3) used by
 * the SLI. This value must be set before S2M traffic flow starts. A write to this register
 * causes the credit counts in the SLI for the MAC ports to be reset to the value in this
 * register.
 */
union cvmx_sli_mac_credit_cnt2 {
	uint64_t u64;
	struct cvmx_sli_mac_credit_cnt2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t p3_c_d                       : 1;  /**< When set, does not allow writing of P3_CCNT. */
	uint64_t p3_n_d                       : 1;  /**< When set, does not allow writing of P3_NCNT. */
	uint64_t p3_p_d                       : 1;  /**< When set, does not allow writing of P3_PCNT. */
	uint64_t p2_c_d                       : 1;  /**< When set, does not allow writing of P2_CCNT. */
	uint64_t p2_n_d                       : 1;  /**< When set, does not allow writing of P2_NCNT. */
	uint64_t p2_p_d                       : 1;  /**< When set, does not allow writing of P2_PCNT. */
	uint64_t p3_ccnt                      : 8;  /**< Port 3 C-TLP FIFO credits. Legal values are 0x25 to 0x80. */
	uint64_t p3_ncnt                      : 8;  /**< Port 3 N-TLP FIFO credits. Legal values are 0x5 to 0x20. */
	uint64_t p3_pcnt                      : 8;  /**< Port 3 P-TLP FIFO credits. Legal values are 0x25 to 0x80. */
	uint64_t p2_ccnt                      : 8;  /**< Port 2 C-TLP FIFO credits. Legal values are 0x25 to 0x80. */
	uint64_t p2_ncnt                      : 8;  /**< Port 2 N-TLP FIFO credits. Legal values are 0x5 to 0x20. */
	uint64_t p2_pcnt                      : 8;  /**< Port 2 P-TLP FIFO credits. Legal values are 0x25 to 0x80. */
#else
	uint64_t p2_pcnt                      : 8;
	uint64_t p2_ncnt                      : 8;
	uint64_t p2_ccnt                      : 8;
	uint64_t p3_pcnt                      : 8;
	uint64_t p3_ncnt                      : 8;
	uint64_t p3_ccnt                      : 8;
	uint64_t p2_p_d                       : 1;
	uint64_t p2_n_d                       : 1;
	uint64_t p2_c_d                       : 1;
	uint64_t p3_p_d                       : 1;
	uint64_t p3_n_d                       : 1;
	uint64_t p3_c_d                       : 1;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_sli_mac_credit_cnt2_s     cn61xx;
	struct cvmx_sli_mac_credit_cnt2_s     cn66xx;
	struct cvmx_sli_mac_credit_cnt2_s     cn70xx;
	struct cvmx_sli_mac_credit_cnt2_s     cn70xxp1;
	struct cvmx_sli_mac_credit_cnt2_s     cn73xx;
	struct cvmx_sli_mac_credit_cnt2_s     cn78xx;
	struct cvmx_sli_mac_credit_cnt2_s     cn78xxp1;
	struct cvmx_sli_mac_credit_cnt2_s     cnf71xx;
	struct cvmx_sli_mac_credit_cnt2_s     cnf75xx;
};
typedef union cvmx_sli_mac_credit_cnt2 cvmx_sli_mac_credit_cnt2_t;

/**
 * cvmx_sli_mac_number
 *
 * When read from a MAC port, this register returns the MAC's port number, otherwise returns zero.
 *
 */
union cvmx_sli_mac_number {
	uint64_t u64;
	struct cvmx_sli_mac_number_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t a_mode                       : 1;  /**< SLI in Authentik mode. */
	uint64_t num                          : 8;  /**< MAC number. Enumerated by SLI_PORT_E. */
#else
	uint64_t num                          : 8;
	uint64_t a_mode                       : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_sli_mac_number_s          cn61xx;
	struct cvmx_sli_mac_number_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t num                          : 8;  /**< The mac number. */
#else
	uint64_t num                          : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} cn63xx;
	struct cvmx_sli_mac_number_s          cn66xx;
	struct cvmx_sli_mac_number_cn63xx     cn68xx;
	struct cvmx_sli_mac_number_cn63xx     cn68xxp1;
	struct cvmx_sli_mac_number_s          cn70xx;
	struct cvmx_sli_mac_number_s          cn70xxp1;
	struct cvmx_sli_mac_number_s          cn73xx;
	struct cvmx_sli_mac_number_s          cn78xx;
	struct cvmx_sli_mac_number_s          cn78xxp1;
	struct cvmx_sli_mac_number_s          cnf71xx;
	struct cvmx_sli_mac_number_s          cnf75xx;
};
typedef union cvmx_sli_mac_number cvmx_sli_mac_number_t;

/**
 * cvmx_sli_mem_access_ctl
 *
 * This register contains control signals for access to the MAC address space.
 *
 */
union cvmx_sli_mem_access_ctl {
	uint64_t u64;
	struct cvmx_sli_mem_access_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t max_word                     : 4;  /**< Maximum number of words. Specifies the maximum number of words to merge into a single
                                                         write operation from the cores to the MAC. Legal values are 1 to 16, with 0 treated as 16. */
	uint64_t timer                        : 10; /**< Merge timer. When the SLI starts a core-to-MAC write, TIMER specifies the maximum wait, in
                                                         coprocessor-clock cycles, to merge additional write operations from the cores into one
                                                         large write. The values for this field range from 1 to 1024, with 0 treated as 1024. */
#else
	uint64_t timer                        : 10;
	uint64_t max_word                     : 4;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_sli_mem_access_ctl_s      cn61xx;
	struct cvmx_sli_mem_access_ctl_s      cn63xx;
	struct cvmx_sli_mem_access_ctl_s      cn63xxp1;
	struct cvmx_sli_mem_access_ctl_s      cn66xx;
	struct cvmx_sli_mem_access_ctl_s      cn68xx;
	struct cvmx_sli_mem_access_ctl_s      cn68xxp1;
	struct cvmx_sli_mem_access_ctl_s      cn70xx;
	struct cvmx_sli_mem_access_ctl_s      cn70xxp1;
	struct cvmx_sli_mem_access_ctl_s      cn73xx;
	struct cvmx_sli_mem_access_ctl_s      cn78xx;
	struct cvmx_sli_mem_access_ctl_s      cn78xxp1;
	struct cvmx_sli_mem_access_ctl_s      cnf71xx;
	struct cvmx_sli_mem_access_ctl_s      cnf75xx;
};
typedef union cvmx_sli_mem_access_ctl cvmx_sli_mem_access_ctl_t;

/**
 * cvmx_sli_mem_access_subid#
 *
 * These registers contains address index and control bits for access to memory from cores.
 *
 */
union cvmx_sli_mem_access_subidx {
	uint64_t u64;
	struct cvmx_sli_mem_access_subidx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t pvf                          : 16; /**< Selects the function number accessed by the reads/writes/atomics. DPI_DMA_FUNC_SEL_S
                                                         describes the format of this field. If [PVF]!=0, [PORT] must select a PCIe MAC that
                                                         supports more than one physical function and/or must select a PCIe MAC
                                                         with SR-IOV enabled. */
	uint64_t reserved_43_43               : 1;
	uint64_t zero                         : 1;  /**< Causes all byte read operations to be zero-length read operations. Returns 0s to the EXEC
                                                         for all read data. This must be zero for SRIO ports. */
	uint64_t port                         : 3;  /**< The MAC that the reads/writes/atomics are sent to. Enumerated by SLI_PORT_E. */
	uint64_t nmerge                       : 1;  /**< When set, no merging is allowed in this window. Applicable to writes only. */
	uint64_t esr                          : 2;  /**< Endian swap for read operations. ES<1:0> for read operations to this subID. ES<1:0> is the
                                                         endian-swap attribute for these MAC memory space read operations. Not used for write or
                                                         atomic operations.
                                                         Enumerated by SLI_ENDIANSWAP_E. */
	uint64_t esw                          : 2;  /**< Endian swap for write operations. ES<1:0> for write operations to this subID. ES<1:0> is
                                                         the endian-swap attribute for these MAC memory space write operations. Not used for reads
                                                         and IOBDMAs.
                                                         Enumerated by SLI_ENDIANSWAP_E. */
	uint64_t wtype                        : 2;  /**< Write type. ADDRTYPE<1:0> for write operations to this subID.
                                                         For PCIE:
                                                           * ADDRTYPE<0> is the relaxed-order attribute.
                                                           * ADDRTYPE<1> is the no-snoop attribute.
                                                         For SRIO:
                                                           * ADDRTYPE<1:0> help select an SRIO()_S2M_TYPE() entry.
                                                         Not used for reads and IOBDMAs. */
	uint64_t rtype                        : 2;  /**< Read type. ADDRTYPE<1:0> for read operations to this subID.
                                                         For PCIE:
                                                           * ADDRTYPE<0> is the relaxed-order attribute.
                                                           * ADDRTYPE<1> is the no-snoop attribute.
                                                         For SRIO:
                                                           * ADDRTYPE<1:0> help select an SRIO()_S2M_TYPE() entry.
                                                         Not used for writes and atomics. */
	uint64_t reserved_0_29                : 30;
#else
	uint64_t reserved_0_29                : 30;
	uint64_t rtype                        : 2;
	uint64_t wtype                        : 2;
	uint64_t esw                          : 2;
	uint64_t esr                          : 2;
	uint64_t nmerge                       : 1;
	uint64_t port                         : 3;
	uint64_t zero                         : 1;
	uint64_t reserved_43_43               : 1;
	uint64_t pvf                          : 16;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_sli_mem_access_subidx_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t zero                         : 1;  /**< Causes all byte reads to be zero length reads.
                                                         Returns to the EXEC a zero for all read data.
                                                         This must be zero for sRIO ports. */
	uint64_t port                         : 3;  /**< Physical MAC Port that reads/writes to
                                                         this subid are sent to. Must be <= 1, as there are
                                                         only two ports present. */
	uint64_t nmerge                       : 1;  /**< When set, no merging is allowed in this window. */
	uint64_t esr                          : 2;  /**< ES<1:0> for reads to this subid.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space reads. */
	uint64_t esw                          : 2;  /**< ES<1:0> for writes to this subid.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space writes. */
	uint64_t wtype                        : 2;  /**< ADDRTYPE<1:0> for writes to this subid
                                                         For PCIe:
                                                         - ADDRTYPE<0> is the relaxed-order attribute
                                                         - ADDRTYPE<1> is the no-snoop attribute
                                                         For sRIO:
                                                         - ADDRTYPE<1:0> help select an SRIO*_S2M_TYPE*
                                                           entry */
	uint64_t rtype                        : 2;  /**< ADDRTYPE<1:0> for reads to this subid
                                                         For PCIe:
                                                         - ADDRTYPE<0> is the relaxed-order attribute
                                                         - ADDRTYPE<1> is the no-snoop attribute
                                                         For sRIO:
                                                         - ADDRTYPE<1:0> help select an SRIO*_S2M_TYPE*
                                                           entry */
	uint64_t ba                           : 30; /**< Address Bits <63:34> for reads/writes that use
                                                         this subid. */
#else
	uint64_t ba                           : 30;
	uint64_t rtype                        : 2;
	uint64_t wtype                        : 2;
	uint64_t esw                          : 2;
	uint64_t esr                          : 2;
	uint64_t nmerge                       : 1;
	uint64_t port                         : 3;
	uint64_t zero                         : 1;
	uint64_t reserved_43_63               : 21;
#endif
	} cn61xx;
	struct cvmx_sli_mem_access_subidx_cn61xx cn63xx;
	struct cvmx_sli_mem_access_subidx_cn61xx cn63xxp1;
	struct cvmx_sli_mem_access_subidx_cn61xx cn66xx;
	struct cvmx_sli_mem_access_subidx_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t zero                         : 1;  /**< Causes all byte reads to be zero length reads.
                                                         Returns to the EXEC a zero for all read data.
                                                         This must be zero for sRIO ports. */
	uint64_t port                         : 3;  /**< Physical MAC Port that reads/writes to
                                                         this subid are sent to. Must be <= 1, as there are
                                                         only two ports present. */
	uint64_t nmerge                       : 1;  /**< When set, no merging is allowed in this window. */
	uint64_t esr                          : 2;  /**< ES<1:0> for reads to this subid.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space reads. */
	uint64_t esw                          : 2;  /**< ES<1:0> for writes to this subid.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space writes. */
	uint64_t wtype                        : 2;  /**< ADDRTYPE<1:0> for writes to this subid
                                                         For PCIe:
                                                         - ADDRTYPE<0> is the relaxed-order attribute
                                                         - ADDRTYPE<1> is the no-snoop attribute */
	uint64_t rtype                        : 2;  /**< ADDRTYPE<1:0> for reads to this subid
                                                         For PCIe:
                                                         - ADDRTYPE<0> is the relaxed-order attribute
                                                         - ADDRTYPE<1> is the no-snoop attribute */
	uint64_t ba                           : 28; /**< Address Bits <63:36> for reads/writes that use
                                                         this subid. */
	uint64_t reserved_0_1                 : 2;
#else
	uint64_t reserved_0_1                 : 2;
	uint64_t ba                           : 28;
	uint64_t rtype                        : 2;
	uint64_t wtype                        : 2;
	uint64_t esw                          : 2;
	uint64_t esr                          : 2;
	uint64_t nmerge                       : 1;
	uint64_t port                         : 3;
	uint64_t zero                         : 1;
	uint64_t reserved_43_63               : 21;
#endif
	} cn68xx;
	struct cvmx_sli_mem_access_subidx_cn68xx cn68xxp1;
	struct cvmx_sli_mem_access_subidx_cn61xx cn70xx;
	struct cvmx_sli_mem_access_subidx_cn61xx cn70xxp1;
	struct cvmx_sli_mem_access_subidx_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t pvf                          : 16; /**< Selects the function number accessed by the reads/writes/atomics. DPI_DMA_FUNC_SEL_S
                                                         describes the format of this field. If [PVF]!=0, [PORT] must select a PCIe MAC that
                                                         supports more than one physical function and/or must select a PCIe MAC
                                                         with SR-IOV enabled. */
	uint64_t reserved_43_43               : 1;
	uint64_t zero                         : 1;  /**< Causes all byte read operations to be zero-length read operations. Returns 0s to the EXEC
                                                         for all read data. */
	uint64_t port                         : 3;  /**< The MAC that the reads/writes/atomics are sent to. Enumerated by SLI_PORT_E. */
	uint64_t nmerge                       : 1;  /**< When set, no merging is allowed in this window. Applicable to writes only. */
	uint64_t esr                          : 2;  /**< Endian swap for read operations. ES<1:0> for read operations to this subID. ES<1:0> is the
                                                         endian-swap attribute for these MAC memory space read operations. Not used for write or
                                                         atomic operations.
                                                         Enumerated by SLI_ENDIANSWAP_E. */
	uint64_t esw                          : 2;  /**< Endian swap for write operations. ES<1:0> for write operations to this subID. ES<1:0> is
                                                         the endian-swap attribute for these MAC memory space write operations. Not used for reads
                                                         and IOBDMAs.
                                                         Enumerated by SLI_ENDIANSWAP_E. */
	uint64_t wtype                        : 2;  /**< Write type. ADDRTYPE<1:0> for write operations to this subID.
                                                         * ADDRTYPE<0> is the relaxed-order attribute.
                                                         * ADDRTYPE<1> is the no-snoop attribute.
                                                         Not used for reads and IOBDMAs. */
	uint64_t rtype                        : 2;  /**< Read type. ADDRTYPE<1:0> for read operations to this subID.
                                                         * ADDRTYPE<0> is the relaxed-order attribute.
                                                         * ADDRTYPE<1> is the no-snoop attribute.
                                                         Not used for writes and atomics. */
	uint64_t ba                           : 30; /**< Bus address. Address bits<63:34> for read/write/atomic operations. */
#else
	uint64_t ba                           : 30;
	uint64_t rtype                        : 2;
	uint64_t wtype                        : 2;
	uint64_t esw                          : 2;
	uint64_t esr                          : 2;
	uint64_t nmerge                       : 1;
	uint64_t port                         : 3;
	uint64_t zero                         : 1;
	uint64_t reserved_43_43               : 1;
	uint64_t pvf                          : 16;
	uint64_t reserved_60_63               : 4;
#endif
	} cn73xx;
	struct cvmx_sli_mem_access_subidx_cn73xx cn78xx;
	struct cvmx_sli_mem_access_subidx_cn61xx cn78xxp1;
	struct cvmx_sli_mem_access_subidx_cn61xx cnf71xx;
	struct cvmx_sli_mem_access_subidx_cn73xx cnf75xx;
};
typedef union cvmx_sli_mem_access_subidx cvmx_sli_mem_access_subidx_t;

/**
 * cvmx_sli_mem_ctl
 *
 * This register controls the ECC of the SLI memories.
 *
 */
union cvmx_sli_mem_ctl {
	uint64_t u64;
	struct cvmx_sli_mem_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t tlpn1_fs                     : 2;  /**< Used to flip the synd. for p2n1_tlp_n_fifo. */
	uint64_t tlpn1_ecc                    : 1;  /**< When set the for p2n1_tlp_n_fifo will have an ECC not generated and checked. */
	uint64_t tlpp1_fs                     : 2;  /**< Used to flip the synd. for p2n1_tlp_p_fifo. */
	uint64_t tlpp1_ecc                    : 1;  /**< When set the for p2n1_tlp_p_fifo will have an ECC not generated and checked. */
	uint64_t tlpc1_fs                     : 2;  /**< Used to flip the synd. for p2n1_tlp_cpl_fifo. */
	uint64_t tlpc1_ecc                    : 1;  /**< When set the for p2n1_tlp_cpl_fifo will have an ECC not generated and checked. */
	uint64_t tlpn0_fs                     : 2;  /**< Used to flip the synd. for p2n0_tlp_n_fifo. */
	uint64_t tlpn0_ecc                    : 1;  /**< When set the for p2n0_tlp_n_fifo will have an ECC not generated and checked. */
	uint64_t tlpp0_fs                     : 2;  /**< Used to flip the synd. for p2n0_tlp_p_fifo. */
	uint64_t tlpp0_ecc                    : 1;  /**< When set the for p2n0_tlp_p_fifo will have an ECC not generated and checked. */
	uint64_t tlpc0_fs                     : 2;  /**< Used to flip the synd. for p2n0_tlp_cpl_fifo. */
	uint64_t tlpc0_ecc                    : 1;  /**< When set the for p2n0_tlp_cpl_fifo will have an ECC not generated and checked. */
	uint64_t nppr_fs                      : 2;  /**< Used to flip the synd. for nod_pp_req_fifo. */
	uint64_t nppr_ecc                     : 1;  /**< When set the for nod_pp_req_fifo  will have an ECC not generated and checked. */
	uint64_t cpl1_fs                      : 2;  /**< Used to flip the synd. for cpl1_fifo. */
	uint64_t cpl1_ecc                     : 1;  /**< When the set cpl1_fifo will have an ECC not generated and checked. */
	uint64_t cpl0_fs                      : 2;  /**< Used to flip the synd. for the cpl0_fifo. */
	uint64_t cpl0_ecc                     : 1;  /**< When set the cpl0_fifo will have an ECC not generated and checked. */
#else
	uint64_t cpl0_ecc                     : 1;
	uint64_t cpl0_fs                      : 2;
	uint64_t cpl1_ecc                     : 1;
	uint64_t cpl1_fs                      : 2;
	uint64_t nppr_ecc                     : 1;
	uint64_t nppr_fs                      : 2;
	uint64_t tlpc0_ecc                    : 1;
	uint64_t tlpc0_fs                     : 2;
	uint64_t tlpp0_ecc                    : 1;
	uint64_t tlpp0_fs                     : 2;
	uint64_t tlpn0_ecc                    : 1;
	uint64_t tlpn0_fs                     : 2;
	uint64_t tlpc1_ecc                    : 1;
	uint64_t tlpc1_fs                     : 2;
	uint64_t tlpp1_ecc                    : 1;
	uint64_t tlpp1_fs                     : 2;
	uint64_t tlpn1_ecc                    : 1;
	uint64_t tlpn1_fs                     : 2;
	uint64_t reserved_27_63               : 37;
#endif
	} s;
	struct cvmx_sli_mem_ctl_s             cn73xx;
	struct cvmx_sli_mem_ctl_s             cn78xx;
	struct cvmx_sli_mem_ctl_s             cn78xxp1;
	struct cvmx_sli_mem_ctl_s             cnf75xx;
};
typedef union cvmx_sli_mem_ctl cvmx_sli_mem_ctl_t;

/**
 * cvmx_sli_mem_int_sum
 *
 * Set when an interrupt condition occurs; write one to clear.
 *
 */
union cvmx_sli_mem_int_sum {
	uint64_t u64;
	struct cvmx_sli_mem_int_sum_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t tlpn1_dbe                    : 1;  /**< Set when the p2n1_tlp_n_fifo has a DBE. */
	uint64_t tlpn1_sbe                    : 1;  /**< Set when the p2n1_tlp_n_fifo has a SBE. */
	uint64_t tlpp1_dbe                    : 1;  /**< Set when the p2n1_tlp_p_fifo has a DBE. */
	uint64_t tlpp1_sbe                    : 1;  /**< Set when the p2n1_tlp_p_fifo has a SBE. */
	uint64_t tlpc1_dbe                    : 1;  /**< Set when the p2n1_tlp_cpl_fifo has a DBE. */
	uint64_t tlpc1_sbe                    : 1;  /**< Set when the p2n1_tlp_cpl_fifo has a SBE. */
	uint64_t tlpn0_dbe                    : 1;  /**< Set when the p2n0_tlp_n_fifo has a DBE. */
	uint64_t tlpn0_sbe                    : 1;  /**< Set when the p2n0_tlp_n_fifo has a SBE. */
	uint64_t tlpp0_dbe                    : 1;  /**< Set when the p2n0_tlp_p_fifo has a DBE. */
	uint64_t tlpp0_sbe                    : 1;  /**< Set when the p2n0_tlp_p_fifo has a SBE. */
	uint64_t tlpc0_dbe                    : 1;  /**< Set when the p2n0_tlp_cpl_fifo has a DBE. */
	uint64_t tlpc0_sbe                    : 1;  /**< Set when the p2n0_tlp_cpl_fifo has a SBE. */
	uint64_t nppr_dbe                     : 1;  /**< Set when the nod_pp_req_fifo has a DBE. */
	uint64_t nppr_sbe                     : 1;  /**< Set when the nod_pp_req_fifo has a SBE. */
	uint64_t cpl1_dbe                     : 1;  /**< Set when the cpl1_fifo has a DBE. */
	uint64_t cpl1_sbe                     : 1;  /**< Set when the cpl1_fifo has a SBE. */
	uint64_t cpl0_dbe                     : 1;  /**< Set when the cpl0_fifo has a DBE. */
	uint64_t cpl0_sbe                     : 1;  /**< Set when the cpl0_fifo has a SBE. */
#else
	uint64_t cpl0_sbe                     : 1;
	uint64_t cpl0_dbe                     : 1;
	uint64_t cpl1_sbe                     : 1;
	uint64_t cpl1_dbe                     : 1;
	uint64_t nppr_sbe                     : 1;
	uint64_t nppr_dbe                     : 1;
	uint64_t tlpc0_sbe                    : 1;
	uint64_t tlpc0_dbe                    : 1;
	uint64_t tlpp0_sbe                    : 1;
	uint64_t tlpp0_dbe                    : 1;
	uint64_t tlpn0_sbe                    : 1;
	uint64_t tlpn0_dbe                    : 1;
	uint64_t tlpc1_sbe                    : 1;
	uint64_t tlpc1_dbe                    : 1;
	uint64_t tlpp1_sbe                    : 1;
	uint64_t tlpp1_dbe                    : 1;
	uint64_t tlpn1_sbe                    : 1;
	uint64_t tlpn1_dbe                    : 1;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_sli_mem_int_sum_s         cn73xx;
	struct cvmx_sli_mem_int_sum_s         cn78xx;
	struct cvmx_sli_mem_int_sum_s         cn78xxp1;
	struct cvmx_sli_mem_int_sum_s         cnf75xx;
};
typedef union cvmx_sli_mem_int_sum cvmx_sli_mem_int_sum_t;

/**
 * cvmx_sli_msi_enb0
 *
 * Used to enable the interrupt generation for the bits in the SLI_MSI_RCV0.
 *
 */
union cvmx_sli_msi_enb0 {
	uint64_t u64;
	struct cvmx_sli_msi_enb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t enb                          : 64; /**< Enables bit [63:0] of SLI_MSI_RCV0. */
#else
	uint64_t enb                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_enb0_s            cn61xx;
	struct cvmx_sli_msi_enb0_s            cn63xx;
	struct cvmx_sli_msi_enb0_s            cn63xxp1;
	struct cvmx_sli_msi_enb0_s            cn66xx;
	struct cvmx_sli_msi_enb0_s            cn68xx;
	struct cvmx_sli_msi_enb0_s            cn68xxp1;
	struct cvmx_sli_msi_enb0_s            cn70xx;
	struct cvmx_sli_msi_enb0_s            cn70xxp1;
	struct cvmx_sli_msi_enb0_s            cnf71xx;
};
typedef union cvmx_sli_msi_enb0 cvmx_sli_msi_enb0_t;

/**
 * cvmx_sli_msi_enb1
 *
 * Used to enable the interrupt generation for the bits in the SLI_MSI_RCV1.
 *
 */
union cvmx_sli_msi_enb1 {
	uint64_t u64;
	struct cvmx_sli_msi_enb1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t enb                          : 64; /**< Enables bit [63:0] of SLI_MSI_RCV1. */
#else
	uint64_t enb                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_enb1_s            cn61xx;
	struct cvmx_sli_msi_enb1_s            cn63xx;
	struct cvmx_sli_msi_enb1_s            cn63xxp1;
	struct cvmx_sli_msi_enb1_s            cn66xx;
	struct cvmx_sli_msi_enb1_s            cn68xx;
	struct cvmx_sli_msi_enb1_s            cn68xxp1;
	struct cvmx_sli_msi_enb1_s            cn70xx;
	struct cvmx_sli_msi_enb1_s            cn70xxp1;
	struct cvmx_sli_msi_enb1_s            cnf71xx;
};
typedef union cvmx_sli_msi_enb1 cvmx_sli_msi_enb1_t;

/**
 * cvmx_sli_msi_enb2
 *
 * Used to enable the interrupt generation for the bits in the SLI_MSI_RCV2.
 *
 */
union cvmx_sli_msi_enb2 {
	uint64_t u64;
	struct cvmx_sli_msi_enb2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t enb                          : 64; /**< Enables bit [63:0] of SLI_MSI_RCV2. */
#else
	uint64_t enb                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_enb2_s            cn61xx;
	struct cvmx_sli_msi_enb2_s            cn63xx;
	struct cvmx_sli_msi_enb2_s            cn63xxp1;
	struct cvmx_sli_msi_enb2_s            cn66xx;
	struct cvmx_sli_msi_enb2_s            cn68xx;
	struct cvmx_sli_msi_enb2_s            cn68xxp1;
	struct cvmx_sli_msi_enb2_s            cn70xx;
	struct cvmx_sli_msi_enb2_s            cn70xxp1;
	struct cvmx_sli_msi_enb2_s            cnf71xx;
};
typedef union cvmx_sli_msi_enb2 cvmx_sli_msi_enb2_t;

/**
 * cvmx_sli_msi_enb3
 *
 * Used to enable the interrupt generation for the bits in the SLI_MSI_RCV3.
 *
 */
union cvmx_sli_msi_enb3 {
	uint64_t u64;
	struct cvmx_sli_msi_enb3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t enb                          : 64; /**< Enables bit [63:0] of SLI_MSI_RCV3. */
#else
	uint64_t enb                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_enb3_s            cn61xx;
	struct cvmx_sli_msi_enb3_s            cn63xx;
	struct cvmx_sli_msi_enb3_s            cn63xxp1;
	struct cvmx_sli_msi_enb3_s            cn66xx;
	struct cvmx_sli_msi_enb3_s            cn68xx;
	struct cvmx_sli_msi_enb3_s            cn68xxp1;
	struct cvmx_sli_msi_enb3_s            cn70xx;
	struct cvmx_sli_msi_enb3_s            cn70xxp1;
	struct cvmx_sli_msi_enb3_s            cnf71xx;
};
typedef union cvmx_sli_msi_enb3 cvmx_sli_msi_enb3_t;

/**
 * cvmx_sli_msi_rcv0
 *
 * This register contains bits <63:0> of the 256 bits of MSI interrupts.
 *
 */
union cvmx_sli_msi_rcv0 {
	uint64_t u64;
	struct cvmx_sli_msi_rcv0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t intr                         : 64; /**< Bits 63-0 of the 256 bits of MSI interrupt. */
#else
	uint64_t intr                         : 64;
#endif
	} s;
	struct cvmx_sli_msi_rcv0_s            cn61xx;
	struct cvmx_sli_msi_rcv0_s            cn63xx;
	struct cvmx_sli_msi_rcv0_s            cn63xxp1;
	struct cvmx_sli_msi_rcv0_s            cn66xx;
	struct cvmx_sli_msi_rcv0_s            cn68xx;
	struct cvmx_sli_msi_rcv0_s            cn68xxp1;
	struct cvmx_sli_msi_rcv0_s            cn70xx;
	struct cvmx_sli_msi_rcv0_s            cn70xxp1;
	struct cvmx_sli_msi_rcv0_s            cn73xx;
	struct cvmx_sli_msi_rcv0_s            cn78xx;
	struct cvmx_sli_msi_rcv0_s            cn78xxp1;
	struct cvmx_sli_msi_rcv0_s            cnf71xx;
	struct cvmx_sli_msi_rcv0_s            cnf75xx;
};
typedef union cvmx_sli_msi_rcv0 cvmx_sli_msi_rcv0_t;

/**
 * cvmx_sli_msi_rcv1
 *
 * This register contains bits <127:64> of the 256 bits of MSI interrupts.
 *
 */
union cvmx_sli_msi_rcv1 {
	uint64_t u64;
	struct cvmx_sli_msi_rcv1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t intr                         : 64; /**< Bits 127-64 of the 256 bits of MSI interrupt. */
#else
	uint64_t intr                         : 64;
#endif
	} s;
	struct cvmx_sli_msi_rcv1_s            cn61xx;
	struct cvmx_sli_msi_rcv1_s            cn63xx;
	struct cvmx_sli_msi_rcv1_s            cn63xxp1;
	struct cvmx_sli_msi_rcv1_s            cn66xx;
	struct cvmx_sli_msi_rcv1_s            cn68xx;
	struct cvmx_sli_msi_rcv1_s            cn68xxp1;
	struct cvmx_sli_msi_rcv1_s            cn70xx;
	struct cvmx_sli_msi_rcv1_s            cn70xxp1;
	struct cvmx_sli_msi_rcv1_s            cn73xx;
	struct cvmx_sli_msi_rcv1_s            cn78xx;
	struct cvmx_sli_msi_rcv1_s            cn78xxp1;
	struct cvmx_sli_msi_rcv1_s            cnf71xx;
	struct cvmx_sli_msi_rcv1_s            cnf75xx;
};
typedef union cvmx_sli_msi_rcv1 cvmx_sli_msi_rcv1_t;

/**
 * cvmx_sli_msi_rcv2
 *
 * This register contains bits <191:128> of the 256 bits of MSI interrupts.
 *
 */
union cvmx_sli_msi_rcv2 {
	uint64_t u64;
	struct cvmx_sli_msi_rcv2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t intr                         : 64; /**< Bits 191-128 of the 256 bits of MSI interrupt. */
#else
	uint64_t intr                         : 64;
#endif
	} s;
	struct cvmx_sli_msi_rcv2_s            cn61xx;
	struct cvmx_sli_msi_rcv2_s            cn63xx;
	struct cvmx_sli_msi_rcv2_s            cn63xxp1;
	struct cvmx_sli_msi_rcv2_s            cn66xx;
	struct cvmx_sli_msi_rcv2_s            cn68xx;
	struct cvmx_sli_msi_rcv2_s            cn68xxp1;
	struct cvmx_sli_msi_rcv2_s            cn70xx;
	struct cvmx_sli_msi_rcv2_s            cn70xxp1;
	struct cvmx_sli_msi_rcv2_s            cn73xx;
	struct cvmx_sli_msi_rcv2_s            cn78xx;
	struct cvmx_sli_msi_rcv2_s            cn78xxp1;
	struct cvmx_sli_msi_rcv2_s            cnf71xx;
	struct cvmx_sli_msi_rcv2_s            cnf75xx;
};
typedef union cvmx_sli_msi_rcv2 cvmx_sli_msi_rcv2_t;

/**
 * cvmx_sli_msi_rcv3
 *
 * This register contains bits <255:192> of the 256 bits of MSI interrupts.
 *
 */
union cvmx_sli_msi_rcv3 {
	uint64_t u64;
	struct cvmx_sli_msi_rcv3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t intr                         : 64; /**< Bits 255-192 of the 256 bits of MSI interrupt. */
#else
	uint64_t intr                         : 64;
#endif
	} s;
	struct cvmx_sli_msi_rcv3_s            cn61xx;
	struct cvmx_sli_msi_rcv3_s            cn63xx;
	struct cvmx_sli_msi_rcv3_s            cn63xxp1;
	struct cvmx_sli_msi_rcv3_s            cn66xx;
	struct cvmx_sli_msi_rcv3_s            cn68xx;
	struct cvmx_sli_msi_rcv3_s            cn68xxp1;
	struct cvmx_sli_msi_rcv3_s            cn70xx;
	struct cvmx_sli_msi_rcv3_s            cn70xxp1;
	struct cvmx_sli_msi_rcv3_s            cn73xx;
	struct cvmx_sli_msi_rcv3_s            cn78xx;
	struct cvmx_sli_msi_rcv3_s            cn78xxp1;
	struct cvmx_sli_msi_rcv3_s            cnf71xx;
	struct cvmx_sli_msi_rcv3_s            cnf75xx;
};
typedef union cvmx_sli_msi_rcv3 cvmx_sli_msi_rcv3_t;

/**
 * cvmx_sli_msi_rd_map
 *
 * This register is used to read the mapping function of the SLI_PCIE_MSI_RCV to SLI_MSI_RCV
 * registers.
 */
union cvmx_sli_msi_rd_map {
	uint64_t u64;
	struct cvmx_sli_msi_rd_map_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t rd_int                       : 8;  /**< The value of the map at the location previously written to the MSI_INT field of this register. */
	uint64_t msi_int                      : 8;  /**< Selects the value that is received when the SLI_PCIE_MSI_RCV register is written. */
#else
	uint64_t msi_int                      : 8;
	uint64_t rd_int                       : 8;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_sli_msi_rd_map_s          cn61xx;
	struct cvmx_sli_msi_rd_map_s          cn63xx;
	struct cvmx_sli_msi_rd_map_s          cn63xxp1;
	struct cvmx_sli_msi_rd_map_s          cn66xx;
	struct cvmx_sli_msi_rd_map_s          cn68xx;
	struct cvmx_sli_msi_rd_map_s          cn68xxp1;
	struct cvmx_sli_msi_rd_map_s          cn70xx;
	struct cvmx_sli_msi_rd_map_s          cn70xxp1;
	struct cvmx_sli_msi_rd_map_s          cn73xx;
	struct cvmx_sli_msi_rd_map_s          cn78xx;
	struct cvmx_sli_msi_rd_map_s          cn78xxp1;
	struct cvmx_sli_msi_rd_map_s          cnf71xx;
	struct cvmx_sli_msi_rd_map_s          cnf75xx;
};
typedef union cvmx_sli_msi_rd_map cvmx_sli_msi_rd_map_t;

/**
 * cvmx_sli_msi_w1c_enb0
 *
 * Used to clear bits in SLI_MSI_ENB0.
 *
 */
union cvmx_sli_msi_w1c_enb0 {
	uint64_t u64;
	struct cvmx_sli_msi_w1c_enb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t clr                          : 64; /**< A write of '1' to a vector will clear the
                                                         cooresponding bit in SLI_MSI_ENB0.
                                                         A read to this address will return 0. */
#else
	uint64_t clr                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1c_enb0_s        cn61xx;
	struct cvmx_sli_msi_w1c_enb0_s        cn63xx;
	struct cvmx_sli_msi_w1c_enb0_s        cn63xxp1;
	struct cvmx_sli_msi_w1c_enb0_s        cn66xx;
	struct cvmx_sli_msi_w1c_enb0_s        cn68xx;
	struct cvmx_sli_msi_w1c_enb0_s        cn68xxp1;
	struct cvmx_sli_msi_w1c_enb0_s        cn70xx;
	struct cvmx_sli_msi_w1c_enb0_s        cn70xxp1;
	struct cvmx_sli_msi_w1c_enb0_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1c_enb0 cvmx_sli_msi_w1c_enb0_t;

/**
 * cvmx_sli_msi_w1c_enb1
 *
 * Used to clear bits in SLI_MSI_ENB1.
 *
 */
union cvmx_sli_msi_w1c_enb1 {
	uint64_t u64;
	struct cvmx_sli_msi_w1c_enb1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t clr                          : 64; /**< A write of '1' to a vector will clear the
                                                         cooresponding bit in SLI_MSI_ENB1.
                                                         A read to this address will return 0. */
#else
	uint64_t clr                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1c_enb1_s        cn61xx;
	struct cvmx_sli_msi_w1c_enb1_s        cn63xx;
	struct cvmx_sli_msi_w1c_enb1_s        cn63xxp1;
	struct cvmx_sli_msi_w1c_enb1_s        cn66xx;
	struct cvmx_sli_msi_w1c_enb1_s        cn68xx;
	struct cvmx_sli_msi_w1c_enb1_s        cn68xxp1;
	struct cvmx_sli_msi_w1c_enb1_s        cn70xx;
	struct cvmx_sli_msi_w1c_enb1_s        cn70xxp1;
	struct cvmx_sli_msi_w1c_enb1_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1c_enb1 cvmx_sli_msi_w1c_enb1_t;

/**
 * cvmx_sli_msi_w1c_enb2
 *
 * Used to clear bits in SLI_MSI_ENB2.
 *
 */
union cvmx_sli_msi_w1c_enb2 {
	uint64_t u64;
	struct cvmx_sli_msi_w1c_enb2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t clr                          : 64; /**< A write of '1' to a vector will clear the
                                                         cooresponding bit in SLI_MSI_ENB2.
                                                         A read to this address will return 0. */
#else
	uint64_t clr                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1c_enb2_s        cn61xx;
	struct cvmx_sli_msi_w1c_enb2_s        cn63xx;
	struct cvmx_sli_msi_w1c_enb2_s        cn63xxp1;
	struct cvmx_sli_msi_w1c_enb2_s        cn66xx;
	struct cvmx_sli_msi_w1c_enb2_s        cn68xx;
	struct cvmx_sli_msi_w1c_enb2_s        cn68xxp1;
	struct cvmx_sli_msi_w1c_enb2_s        cn70xx;
	struct cvmx_sli_msi_w1c_enb2_s        cn70xxp1;
	struct cvmx_sli_msi_w1c_enb2_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1c_enb2 cvmx_sli_msi_w1c_enb2_t;

/**
 * cvmx_sli_msi_w1c_enb3
 *
 * Used to clear bits in SLI_MSI_ENB3.
 *
 */
union cvmx_sli_msi_w1c_enb3 {
	uint64_t u64;
	struct cvmx_sli_msi_w1c_enb3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t clr                          : 64; /**< A write of '1' to a vector will clear the
                                                         cooresponding bit in SLI_MSI_ENB3.
                                                         A read to this address will return 0. */
#else
	uint64_t clr                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1c_enb3_s        cn61xx;
	struct cvmx_sli_msi_w1c_enb3_s        cn63xx;
	struct cvmx_sli_msi_w1c_enb3_s        cn63xxp1;
	struct cvmx_sli_msi_w1c_enb3_s        cn66xx;
	struct cvmx_sli_msi_w1c_enb3_s        cn68xx;
	struct cvmx_sli_msi_w1c_enb3_s        cn68xxp1;
	struct cvmx_sli_msi_w1c_enb3_s        cn70xx;
	struct cvmx_sli_msi_w1c_enb3_s        cn70xxp1;
	struct cvmx_sli_msi_w1c_enb3_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1c_enb3 cvmx_sli_msi_w1c_enb3_t;

/**
 * cvmx_sli_msi_w1s_enb0
 *
 * Used to set bits in SLI_MSI_ENB0.
 *
 */
union cvmx_sli_msi_w1s_enb0 {
	uint64_t u64;
	struct cvmx_sli_msi_w1s_enb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t set                          : 64; /**< A write of '1' to a vector will set the
                                                         cooresponding bit in SLI_MSI_ENB0.
                                                         A read to this address will return 0. */
#else
	uint64_t set                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1s_enb0_s        cn61xx;
	struct cvmx_sli_msi_w1s_enb0_s        cn63xx;
	struct cvmx_sli_msi_w1s_enb0_s        cn63xxp1;
	struct cvmx_sli_msi_w1s_enb0_s        cn66xx;
	struct cvmx_sli_msi_w1s_enb0_s        cn68xx;
	struct cvmx_sli_msi_w1s_enb0_s        cn68xxp1;
	struct cvmx_sli_msi_w1s_enb0_s        cn70xx;
	struct cvmx_sli_msi_w1s_enb0_s        cn70xxp1;
	struct cvmx_sli_msi_w1s_enb0_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1s_enb0 cvmx_sli_msi_w1s_enb0_t;

/**
 * cvmx_sli_msi_w1s_enb1
 *
 * SLI_MSI_W1S_ENB0 = SLI MSI Write 1 To Set Enable1
 * Used to set bits in SLI_MSI_ENB1.
 */
union cvmx_sli_msi_w1s_enb1 {
	uint64_t u64;
	struct cvmx_sli_msi_w1s_enb1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t set                          : 64; /**< A write of '1' to a vector will set the
                                                         cooresponding bit in SLI_MSI_ENB1.
                                                         A read to this address will return 0. */
#else
	uint64_t set                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1s_enb1_s        cn61xx;
	struct cvmx_sli_msi_w1s_enb1_s        cn63xx;
	struct cvmx_sli_msi_w1s_enb1_s        cn63xxp1;
	struct cvmx_sli_msi_w1s_enb1_s        cn66xx;
	struct cvmx_sli_msi_w1s_enb1_s        cn68xx;
	struct cvmx_sli_msi_w1s_enb1_s        cn68xxp1;
	struct cvmx_sli_msi_w1s_enb1_s        cn70xx;
	struct cvmx_sli_msi_w1s_enb1_s        cn70xxp1;
	struct cvmx_sli_msi_w1s_enb1_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1s_enb1 cvmx_sli_msi_w1s_enb1_t;

/**
 * cvmx_sli_msi_w1s_enb2
 *
 * Used to set bits in SLI_MSI_ENB2.
 *
 */
union cvmx_sli_msi_w1s_enb2 {
	uint64_t u64;
	struct cvmx_sli_msi_w1s_enb2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t set                          : 64; /**< A write of '1' to a vector will set the
                                                         cooresponding bit in SLI_MSI_ENB2.
                                                         A read to this address will return 0. */
#else
	uint64_t set                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1s_enb2_s        cn61xx;
	struct cvmx_sli_msi_w1s_enb2_s        cn63xx;
	struct cvmx_sli_msi_w1s_enb2_s        cn63xxp1;
	struct cvmx_sli_msi_w1s_enb2_s        cn66xx;
	struct cvmx_sli_msi_w1s_enb2_s        cn68xx;
	struct cvmx_sli_msi_w1s_enb2_s        cn68xxp1;
	struct cvmx_sli_msi_w1s_enb2_s        cn70xx;
	struct cvmx_sli_msi_w1s_enb2_s        cn70xxp1;
	struct cvmx_sli_msi_w1s_enb2_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1s_enb2 cvmx_sli_msi_w1s_enb2_t;

/**
 * cvmx_sli_msi_w1s_enb3
 *
 * Used to set bits in SLI_MSI_ENB3.
 *
 */
union cvmx_sli_msi_w1s_enb3 {
	uint64_t u64;
	struct cvmx_sli_msi_w1s_enb3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t set                          : 64; /**< A write of '1' to a vector will set the
                                                         cooresponding bit in SLI_MSI_ENB3.
                                                         A read to this address will return 0. */
#else
	uint64_t set                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1s_enb3_s        cn61xx;
	struct cvmx_sli_msi_w1s_enb3_s        cn63xx;
	struct cvmx_sli_msi_w1s_enb3_s        cn63xxp1;
	struct cvmx_sli_msi_w1s_enb3_s        cn66xx;
	struct cvmx_sli_msi_w1s_enb3_s        cn68xx;
	struct cvmx_sli_msi_w1s_enb3_s        cn68xxp1;
	struct cvmx_sli_msi_w1s_enb3_s        cn70xx;
	struct cvmx_sli_msi_w1s_enb3_s        cn70xxp1;
	struct cvmx_sli_msi_w1s_enb3_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1s_enb3 cvmx_sli_msi_w1s_enb3_t;

/**
 * cvmx_sli_msi_wr_map
 *
 * This register is used to write the mapping function of the SLI_PCIE_MSI_RCV to SLI_MSI_RCV
 * registers. At reset, the mapping function is one-to-one, that is MSI_INT 1 maps to CIU_INT 1,
 * 2 to 2, 3 to 3, etc.
 */
union cvmx_sli_msi_wr_map {
	uint64_t u64;
	struct cvmx_sli_msi_wr_map_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t ciu_int                      : 8;  /**< Selects which bit in the SLI_MSI_RCV() (0-255) will be set when the value specified in
                                                         the MSI_INT of this register is received during a write operation to the SLI_PCIE_MSI_RCV
                                                         register. */
	uint64_t msi_int                      : 8;  /**< Selects the value that is received when the SLI_PCIE_MSI_RCV register is written. */
#else
	uint64_t msi_int                      : 8;
	uint64_t ciu_int                      : 8;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_sli_msi_wr_map_s          cn61xx;
	struct cvmx_sli_msi_wr_map_s          cn63xx;
	struct cvmx_sli_msi_wr_map_s          cn63xxp1;
	struct cvmx_sli_msi_wr_map_s          cn66xx;
	struct cvmx_sli_msi_wr_map_s          cn68xx;
	struct cvmx_sli_msi_wr_map_s          cn68xxp1;
	struct cvmx_sli_msi_wr_map_s          cn70xx;
	struct cvmx_sli_msi_wr_map_s          cn70xxp1;
	struct cvmx_sli_msi_wr_map_s          cn73xx;
	struct cvmx_sli_msi_wr_map_s          cn78xx;
	struct cvmx_sli_msi_wr_map_s          cn78xxp1;
	struct cvmx_sli_msi_wr_map_s          cnf71xx;
	struct cvmx_sli_msi_wr_map_s          cnf75xx;
};
typedef union cvmx_sli_msi_wr_map cvmx_sli_msi_wr_map_t;

/**
 * cvmx_sli_msix#_table_addr
 *
 * The MSI-X table cannot be burst read or written.
 *
 * The combination of all MSI-X Tables contain (64 + 4) entries - one per
 * ring plus one per PF. (i.e. 64 plus one per valid SLI_MAC()_PF()_INT_SUM
 * present.)
 *
 * The MSI-X table for an individual PF has SLI_PKT_MAC()_PF()_RINFO[TRS]
 * entries for the rings associated to the PF (up to 64 max) plus one
 * more table entry for SLI_MAC()_PF()_INT_SUM. The
 * SLI_MAC()_PF()_INT_SUM-related MSI-X table entry is
 * always entry SLI_MSIX(SLI_PKT_MAC()_PF()_RINFO[TRS])_TABLE_ADDR and
 * always present and valid. All earlier SLI_MSIX()_TABLE_ADDR entries
 * correspond to rings. When SLI_PKT_MAC()_PF()_RINFO[NVFS]=0, SR-IOV
 * virtual functions cannot be used, and all SLI_PKT_MAC()_PF()_RINFO[TRS]+1
 * entries in the PF MSI-X table are present and valid for use by the PF.
 * When SLI_PKT_MAC()_PF()_RINFO[NVFS]!=0, SR-IOV virtual functions may
 * be used, and the first
 *   SLI_PKT_MAC()_PF()_RINFO[NVFS]*SLI_PKT_MAC()_PF()_RINFO[RPVF]
 * entries of the PF MSI-X table are present but not valid, and
 * should never be accessed by the PF.
 *
 * The MSI-X table for an individual VF has SLI_PKT_MAC()_PF()_RINFO[RPVF]
 * entries (up to 8 max), all valid, one per ring that the VF owns.
 */
union cvmx_sli_msixx_table_addr {
	uint64_t u64;
	struct cvmx_sli_msixx_table_addr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t addr                         : 64; /**< Message address. */
#else
	uint64_t addr                         : 64;
#endif
	} s;
	struct cvmx_sli_msixx_table_addr_s    cn73xx;
	struct cvmx_sli_msixx_table_addr_s    cn78xx;
	struct cvmx_sli_msixx_table_addr_s    cn78xxp1;
	struct cvmx_sli_msixx_table_addr_s    cnf75xx;
};
typedef union cvmx_sli_msixx_table_addr cvmx_sli_msixx_table_addr_t;

/**
 * cvmx_sli_msix#_table_data
 *
 * The MSI-X table cannot be burst read or written. VF/PF access is the same as
 * described for the SLI_MSIX()_TABLE_ADDR.
 */
union cvmx_sli_msixx_table_data {
	uint64_t u64;
	struct cvmx_sli_msixx_table_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t vector_ctl                   : 1;  /**< Message mask. */
	uint64_t data                         : 32; /**< Message data. */
#else
	uint64_t data                         : 32;
	uint64_t vector_ctl                   : 1;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_sli_msixx_table_data_s    cn73xx;
	struct cvmx_sli_msixx_table_data_s    cn78xx;
	struct cvmx_sli_msixx_table_data_s    cn78xxp1;
	struct cvmx_sli_msixx_table_data_s    cnf75xx;
};
typedef union cvmx_sli_msixx_table_data cvmx_sli_msixx_table_data_t;

/**
 * cvmx_sli_msix_mac#_pf_table_addr
 *
 * These registers shadow the four physical MSIX PF ERR entries.
 * Each MAC sees its entry at its own TRS offset.
 */
union cvmx_sli_msix_macx_pf_table_addr {
	uint64_t u64;
	struct cvmx_sli_msix_macx_pf_table_addr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t addr                         : 64; /**< Message address <63:0>. */
#else
	uint64_t addr                         : 64;
#endif
	} s;
	struct cvmx_sli_msix_macx_pf_table_addr_s cn78xxp1;
};
typedef union cvmx_sli_msix_macx_pf_table_addr cvmx_sli_msix_macx_pf_table_addr_t;

/**
 * cvmx_sli_msix_mac#_pf_table_data
 *
 * These registers shadow four physical MSIX PF ERR entries.
 * Each MAC sees its entry at its own TRS offset.
 */
union cvmx_sli_msix_macx_pf_table_data {
	uint64_t u64;
	struct cvmx_sli_msix_macx_pf_table_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t vector_ctl                   : 1;  /**< Message mask. */
	uint64_t data                         : 32; /**< Message data <31:0>. */
#else
	uint64_t data                         : 32;
	uint64_t vector_ctl                   : 1;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_sli_msix_macx_pf_table_data_s cn78xxp1;
};
typedef union cvmx_sli_msix_macx_pf_table_data cvmx_sli_msix_macx_pf_table_data_t;

/**
 * cvmx_sli_msix_pba0
 *
 * The MSI-X pending bit array cannot be burst read.
 * In SR-IOV mode, a VF will find its pending completion interrupts in bit
 * positions [(RPVF-1):0]. If RPVF<64, bits [63:RPVF] are returned as zero.
 *
 * Each VF can read their own pending completion interrupts based on the ring/VF
 * configuration. Therefore, a VF sees the PBA as smaller than what is shown below
 * (unless it owns all 64 entries).  Unassigned bits will return zeros.
 *
 * <pre>
 *    RPVF  Interrupts per VF   Pending bits returned
 *    ----  -----------------   ---------------------
 *      0            0          0
 *      1            1          MSG_PND0
 *      2            2          MSG_PND1  - MSG_PND0
 *      4            4          MSG_PND3  - MSG_PND0
 *      8            8          MSG_PND7  - MSG_PND0
 * </pre>
 *
 * If SLI_PKT_MAC()_PF()_RINFO[TRS]=63 (i.e. 64 total DPI Packet Rings configured), a PF will
 * find its pending completion interrupts in bit positions [63:0]. When
 * SLI_PKT_MAC()_PF()_RINFO[TRS]=63,
 * the PF will find its PCIe error interrupt in SLI_MSIX_PBA1, bit position 0.
 *
 * If SLI_PKT_MAC()_PF()_RINFO[TRS]<63 (i.e. 0, 1, 2, 4, or 8 rings configured), a PF will find
 * its ring pending completion interrupts in bit positions [TNR:0]. It will find its PCIe
 * error interrupt in bit position [(TNR+1)]. Bits [63:(TNR+2)] are returned as zero.
 * When SLI_PKT_MAC()_PF()_RINFO[TRS]<63, SLI_MSIX_PBA1 is not used and returns zeros.
 *
 * If SR-IOV Mode is off there is no virtual function support, but the PF can configure up to 65
 * entries (up to 64 DPI Packet Rings plus 1 PF ring) for itself.
 */
union cvmx_sli_msix_pba0 {
	uint64_t u64;
	struct cvmx_sli_msix_pba0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t msg_pnd                      : 64; /**< VF message pending vector. */
#else
	uint64_t msg_pnd                      : 64;
#endif
	} s;
	struct cvmx_sli_msix_pba0_s           cn73xx;
	struct cvmx_sli_msix_pba0_s           cn78xx;
	struct cvmx_sli_msix_pba0_s           cn78xxp1;
	struct cvmx_sli_msix_pba0_s           cnf75xx;
};
typedef union cvmx_sli_msix_pba0 cvmx_sli_msix_pba0_t;

/**
 * cvmx_sli_msix_pba1
 *
 * The MSI-X pending bit array cannot be burst read.
 *
 * PF_PND is assigned to PCIe related errors. The error bit can only be found in PBA1 when
 * SLI_PKT_MAC()_PF()_RINFO[TRS]=63 (i.e. 64 total DPI Packet Rings configured).
 *
 * This register is accessible by the PF. A read by a particular PF only
 * returns its own pending status. That is, any PF can read this register, but the hardware
 * ensures
 * that the PF only sees its own status.
 */
union cvmx_sli_msix_pba1 {
	uint64_t u64;
	struct cvmx_sli_msix_pba1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t pf_pnd                       : 1;  /**< PF message pending. */
#else
	uint64_t pf_pnd                       : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_sli_msix_pba1_s           cn73xx;
	struct cvmx_sli_msix_pba1_s           cn78xx;
	struct cvmx_sli_msix_pba1_s           cn78xxp1;
	struct cvmx_sli_msix_pba1_s           cnf75xx;
};
typedef union cvmx_sli_msix_pba1 cvmx_sli_msix_pba1_t;

/**
 * cvmx_sli_nqm_rsp_err_snd_dbg
 *
 * This register is for diagnostic use only.
 *
 */
union cvmx_sli_nqm_rsp_err_snd_dbg {
	uint64_t u64;
	struct cvmx_sli_nqm_rsp_err_snd_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t vf_index                     : 12; /**< On the SLI NQM_RSP_ERR interface, force-send this vf_index. For diagnostic use only. */
#else
	uint64_t vf_index                     : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_sli_nqm_rsp_err_snd_dbg_s cn73xx;
	struct cvmx_sli_nqm_rsp_err_snd_dbg_s cn78xx;
	struct cvmx_sli_nqm_rsp_err_snd_dbg_s cnf75xx;
};
typedef union cvmx_sli_nqm_rsp_err_snd_dbg cvmx_sli_nqm_rsp_err_snd_dbg_t;

/**
 * cvmx_sli_pcie_msi_rcv
 *
 * This is the register where MSI write operations are directed from the MAC.
 *
 */
union cvmx_sli_pcie_msi_rcv {
	uint64_t u64;
	struct cvmx_sli_pcie_msi_rcv_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t intr                         : 8;  /**< A write operation to this register results in a bit in one of the SLI_MSI_RCV()
                                                         registers being set. Which bit is set depends on the value previously written using the
                                                         SLI_MSI_WR_MAP register, or if not previously written, the reset value of the MAP. */
#else
	uint64_t intr                         : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_sli_pcie_msi_rcv_s        cn61xx;
	struct cvmx_sli_pcie_msi_rcv_s        cn63xx;
	struct cvmx_sli_pcie_msi_rcv_s        cn63xxp1;
	struct cvmx_sli_pcie_msi_rcv_s        cn66xx;
	struct cvmx_sli_pcie_msi_rcv_s        cn68xx;
	struct cvmx_sli_pcie_msi_rcv_s        cn68xxp1;
	struct cvmx_sli_pcie_msi_rcv_s        cn70xx;
	struct cvmx_sli_pcie_msi_rcv_s        cn70xxp1;
	struct cvmx_sli_pcie_msi_rcv_s        cn73xx;
	struct cvmx_sli_pcie_msi_rcv_s        cn78xx;
	struct cvmx_sli_pcie_msi_rcv_s        cn78xxp1;
	struct cvmx_sli_pcie_msi_rcv_s        cnf71xx;
	struct cvmx_sli_pcie_msi_rcv_s        cnf75xx;
};
typedef union cvmx_sli_pcie_msi_rcv cvmx_sli_pcie_msi_rcv_t;

/**
 * cvmx_sli_pcie_msi_rcv_b1
 *
 * This register is where MSI write operations are directed from the MAC. This register can be
 * used by the PCIe and SRIO MACs.
 */
union cvmx_sli_pcie_msi_rcv_b1 {
	uint64_t u64;
	struct cvmx_sli_pcie_msi_rcv_b1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t intr                         : 8;  /**< A write to this register results in a bit in one of the SLI_MSI_RCVn registers being set.
                                                         Which bit is set depends on what is previously written using SLI_MSI_WR_MAP. If nothing is
                                                         previously written, the bit set is the reset value of the MAP. */
	uint64_t reserved_0_7                 : 8;
#else
	uint64_t reserved_0_7                 : 8;
	uint64_t intr                         : 8;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn61xx;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn63xx;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn63xxp1;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn66xx;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn68xx;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn68xxp1;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn70xx;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn70xxp1;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn73xx;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn78xx;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn78xxp1;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cnf71xx;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cnf75xx;
};
typedef union cvmx_sli_pcie_msi_rcv_b1 cvmx_sli_pcie_msi_rcv_b1_t;

/**
 * cvmx_sli_pcie_msi_rcv_b2
 *
 * This register is where MSI write operations are directed from the MAC.  This register can be
 * used by PCIe and SRIO MACs.
 */
union cvmx_sli_pcie_msi_rcv_b2 {
	uint64_t u64;
	struct cvmx_sli_pcie_msi_rcv_b2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t intr                         : 8;  /**< A write to this register results in a bit in one of the SLI_MSI_RCVn registers being set.
                                                         Which bit is set depends on what is previously written using the SLI_MSI_WR_MAP register.
                                                         If nothing is previously written, the bit set is the reset value of the MAP. */
	uint64_t reserved_0_15                : 16;
#else
	uint64_t reserved_0_15                : 16;
	uint64_t intr                         : 8;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn61xx;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn63xx;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn63xxp1;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn66xx;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn68xx;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn68xxp1;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn70xx;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn70xxp1;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn73xx;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn78xx;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn78xxp1;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cnf71xx;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cnf75xx;
};
typedef union cvmx_sli_pcie_msi_rcv_b2 cvmx_sli_pcie_msi_rcv_b2_t;

/**
 * cvmx_sli_pcie_msi_rcv_b3
 *
 * This register is where MSI write operations are directed from the MAC. This register can be
 * used by PCIe and SRIO MACs.
 */
union cvmx_sli_pcie_msi_rcv_b3 {
	uint64_t u64;
	struct cvmx_sli_pcie_msi_rcv_b3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t intr                         : 8;  /**< A write to this register results in a bit in one of the SLI_MSI_RCVn registers being set.
                                                         Which bit is set depends on what is previously written using the SLI_MSI_WR_MAP register.
                                                         If nothing is previously written, the bit set is the reset value of the MAP. */
	uint64_t reserved_0_23                : 24;
#else
	uint64_t reserved_0_23                : 24;
	uint64_t intr                         : 8;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn61xx;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn63xx;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn63xxp1;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn66xx;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn68xx;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn68xxp1;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn70xx;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn70xxp1;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn73xx;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn78xx;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn78xxp1;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cnf71xx;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cnf75xx;
};
typedef union cvmx_sli_pcie_msi_rcv_b3 cvmx_sli_pcie_msi_rcv_b3_t;

/**
 * cvmx_sli_pkt#_cnts
 *
 * This register contains the counters for output rings.
 *
 */
union cvmx_sli_pktx_cnts {
	uint64_t u64;
	struct cvmx_sli_pktx_cnts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t po_int                       : 1;  /**< Packet output interrupt bit for the ring. [PO_INT] reads as one whenever:
                                                          * [CNT]   > SLI_PKT()_INT_LEVELS[CNT].
                                                          * Or, [TIMER] > SLI_PKT()_INT_LEVELS[TIME].
                                                         [PO_INT] can cause an MSI-X interrupt for ring, and its [CNT] ([TIMER]) component
                                                         can cause SLI_MAC()_PF()_INT_SUM[PCNT] (SLI_MAC()_PF()_INT_SUM[PTIME]) to
                                                         be set if
                                                         SLI_PKT()_OUTPUT_CONTROL[CENB] (SLI_PKT()_OUTPUT_CONTROL[TENB]) is set.
                                                         SLI_PKT_CNT_INT (SLI_PKT_TIME_INT) shows the [CNT] ([TIMER]) component
                                                         for this and all other rings. See also SLI_PKT_IN_DONE()_CNTS[PO_INT]. */
	uint64_t pi_int                       : 1;  /**< Packet input interrupt bit for the ring. A copy of SLI_PKT_IN_DONE()_CNTS[PI_INT]. */
	uint64_t mbox_int                     : 1;  /**< Reads corresponding bit in SLI_PKT()_MBOX_INT. */
	uint64_t resend                       : 1;  /**< A write of 1 will resend an MSI-X interrupt message if there is a pending interrupt in
                                                         [P0_INT], [PI_INT] or [MBOX_INT] for this ring after the write of [CNT] occurs.
                                                         [RESEND] and [CNT] must be written together with the assumption that the write of
                                                         [CNT] will clear the [PO_INT] interrupt bit. If the write of [CNT] does not cause
                                                         the [CNT] to drop below the thresholds another MSI-X message will be sent.
                                                         The [RESEND] bit will never effect INTA/B/C/D or MSI interrupt. */
	uint64_t reserved_54_59               : 6;
	uint64_t timer                        : 22; /**< Timer, incremented every 1024 coprocessor-clock cycles when [CNT] is
                                                         not zero. The hardware clears both [TIMER] and [PO_INT] when [CNT]
                                                         goes to 0. The first increment of this count can occur between 0 to
                                                         1023 coprocessor-clock cycles after [CNT] becomes nonzero. */
	uint64_t cnt                          : 32; /**< Ring counter. Hardware adds to [CNT] as it sends packets out. On a write
                                                         to this register, hardware subtracts the amount written to the [CNT] field from
                                                         [CNT], which will clear [PO_INT] if [CNT] becomes <= SLI_PKT()_INT_LEVELS[CNT].
                                                         When SLI_PKT()_OUTPUT_CONTROL[BMODE] is clear, the hardware adds 1
                                                         to [CNT] per packet. When SLI_PKT()_OUTPUT_CONTROL[BMODE] is set, the hardware
                                                         adds (packet_length+X) to [CNT] per packet. X is zero when info pointer mode
                                                         is used, 8 when info pointer mode is not used. */
#else
	uint64_t cnt                          : 32;
	uint64_t timer                        : 22;
	uint64_t reserved_54_59               : 6;
	uint64_t resend                       : 1;
	uint64_t mbox_int                     : 1;
	uint64_t pi_int                       : 1;
	uint64_t po_int                       : 1;
#endif
	} s;
	struct cvmx_sli_pktx_cnts_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t timer                        : 22; /**< Timer incremented every 1024 core clocks
                                                         when SLI_PKTS#_CNTS[CNT] is non zero. Field
                                                         cleared when SLI_PKTS#_CNTS[CNT] goes to 0.
                                                         Field is also cleared when SLI_PKT_TIME_INT is
                                                         cleared.
                                                         The first increment of this count can occur
                                                         between 0 to 1023 core clocks. */
	uint64_t cnt                          : 32; /**< ring counter. This field is incremented as
                                                         packets are sent out and decremented in response to
                                                         writes to this field.
                                                         When SLI_PKT_OUT_BMODE is '0' a value of 1 is
                                                         added to the register for each packet, when '1'
                                                         and the info-pointer is NOT used the length of the
                                                         packet plus 8 is added, when '1' and info-pointer
                                                         mode IS used the packet length is added to this
                                                         field. */
#else
	uint64_t cnt                          : 32;
	uint64_t timer                        : 22;
	uint64_t reserved_54_63               : 10;
#endif
	} cn61xx;
	struct cvmx_sli_pktx_cnts_cn61xx      cn63xx;
	struct cvmx_sli_pktx_cnts_cn61xx      cn63xxp1;
	struct cvmx_sli_pktx_cnts_cn61xx      cn66xx;
	struct cvmx_sli_pktx_cnts_cn61xx      cn68xx;
	struct cvmx_sli_pktx_cnts_cn61xx      cn68xxp1;
	struct cvmx_sli_pktx_cnts_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_63_54               : 10;
	uint64_t timer                        : 22; /**< "Timer incremented every 1024 core clocks
                                                         when SLI_PKTS#_CNTS[CNT] is non zero. Field
                                                         cleared when SLI_PKTS#_CNTS[CNT] goes to 0.
                                                         Field is also cleared when SLI_PKT_TIME_INT is
                                                         cleared.
                                                         The first increment of this count can occur
                                                         between 0 to 1023 core clocks." */
	uint64_t cnt                          : 32; /**< ring counter. This field is incremented as
                                                         packets are sent out and decremented in response to
                                                         writes to this field.
                                                         When SLI_PKT_OUT_BMODE is '0' a value of 1 is
                                                         added to the register for each packet, when '1'
                                                         and the info-pointer is NOT used the length of the
                                                         packet plus 8 is added, when '1' and info-pointer
                                                         mode IS used the packet length is added to this
                                                         field. */
#else
	uint64_t cnt                          : 32;
	uint64_t timer                        : 22;
	uint64_t reserved_63_54               : 10;
#endif
	} cn70xx;
	struct cvmx_sli_pktx_cnts_cn70xx      cn70xxp1;
	struct cvmx_sli_pktx_cnts_s           cn73xx;
	struct cvmx_sli_pktx_cnts_s           cn78xx;
	struct cvmx_sli_pktx_cnts_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t po_int                       : 1;  /**< Packet output interrupt bit for the ring (i). [PO_INT] reads as one whenever:
                                                          * [CNT]   > SLI_PKT(i)_INT_LEVELS[CNT], or
                                                          * [TIMER] > SLI_PKT(i)_INT_LEVELS[TIME]
                                                         [PO_INT] can cause an MSI-X interrupt for ring i, and its [CNT] ([TIMER]) component
                                                         can cause SLI_INT_SUM[PCNT] (SLI_INT_SUM[PTIME]) to be set if
                                                         SLI_PKT(i)_OUTPUT_CONTROL[CENB] (SLI_PKT(i)_OUTPUT_CONTROL[TENB]) is set.
                                                         SLI_PKT_CNT_INT (SLI_PKT_TIME_INT) shows the [CNT] ([TIMER]) component
                                                         for this and all other rings. See also SLI_PKT_IN_DONE(i)_CNTS[PO_INT]. */
	uint64_t pi_int                       : 1;  /**< Packet input interrupt bit for the ring. A copy of SLI_PKT_IN_DONE(0..63)_CNTS[PI_INT]. */
	uint64_t reserved_61_54               : 8;
	uint64_t timer                        : 22; /**< Timer, incremented every 1024 coprocessor-clock cycles when [CNT] is
                                                         not zero. The hardware clears both [TIMER] and [PO_INT] when [CNT]
                                                         goes to 0. The first increment of this count can occur between 0 to
                                                         1023 coprocessor-clock cycles after [CNT] becomes nonzero. */
	uint64_t cnt                          : 32; /**< Ring counter. Hardware adds to [CNT] as it sends packets out. On a write
                                                         to this register, hardware subtracts the amount written to the [CNT] field from
                                                         [CNT], which will clear [PO_INT] if [CNT] becomes <= SLI_PKT(i)_INT_LEVELS[CNT].
                                                         When SLI_PKT()_OUTPUT_CONTROL[BMODE] is clear, the hardware adds 1
                                                         to [CNT] per packet. When SLI_PKT()_OUTPUT_CONTROL[BMODE] is set, the hardware
                                                         adds (packet_length+X) to [CNT] per packet. X is zero when info pointer mode
                                                         is used, 8 when info pointer mode is not used. */
#else
	uint64_t cnt                          : 32;
	uint64_t timer                        : 22;
	uint64_t reserved_61_54               : 8;
	uint64_t pi_int                       : 1;
	uint64_t po_int                       : 1;
#endif
	} cn78xxp1;
	struct cvmx_sli_pktx_cnts_cn61xx      cnf71xx;
	struct cvmx_sli_pktx_cnts_s           cnf75xx;
};
typedef union cvmx_sli_pktx_cnts cvmx_sli_pktx_cnts_t;

/**
 * cvmx_sli_pkt#_error_info
 *
 * The fields in this register are set when an error conditions occur and can be cleared.
 * These fields are for information purpose only and do NOT generate interrupts.
 */
union cvmx_sli_pktx_error_info {
	uint64_t u64;
	struct cvmx_sli_pktx_error_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t osize_err                    : 1;  /**< A VF created a bad instruction that caused a gather inbound packet to exceed
                                                         64K bytes. */
	uint64_t nobdell_err                  : 1;  /**< A VF has no slists doorbell pointers for an outbound packet that is ready
                                                         to be sent. */
	uint64_t pins_err                     : 1;  /**< Packet instruction read error. When a read error occurs on a packet instruction, this bit
                                                         is set. */
	uint64_t pop_err                      : 1;  /**< Packet scatter pointer pair error. When a read error occurs on a packet scatter pointer
                                                         pair, this bit is set. */
	uint64_t pdi_err                      : 1;  /**< Packet data read error. When a read error occurs on a packet data read, this bit is set. */
	uint64_t pgl_err                      : 1;  /**< Packet gather list read error. When a read error occurs on a packet gather list read, this
                                                         bit is set. */
	uint64_t psldbof                      : 1;  /**< Packet scatter list doorbell count overflowed. Which doorbell can be found in
                                                         DPI_PINT_INFO[PSLDBOF]. */
	uint64_t pidbof                       : 1;  /**< Packet instruction doorbell count overflowed. Which doorbell can be found in
                                                         DPI_PINT_INFO[PIDBOF]. */
#else
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t nobdell_err                  : 1;
	uint64_t osize_err                    : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_sli_pktx_error_info_s     cn73xx;
	struct cvmx_sli_pktx_error_info_s     cn78xx;
	struct cvmx_sli_pktx_error_info_s     cnf75xx;
};
typedef union cvmx_sli_pktx_error_info cvmx_sli_pktx_error_info_t;

/**
 * cvmx_sli_pkt#_in_bp
 *
 * "SLI_PKT[0..31]_IN_BP = SLI Packet ring# Input Backpressure
 * The counters and thresholds for input packets to apply backpressure to processing of the
 * packets."
 */
union cvmx_sli_pktx_in_bp {
	uint64_t u64;
	struct cvmx_sli_pktx_in_bp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wmark                        : 32; /**< When CNT is greater than this threshold no more
                                                         packets will be processed for this ring.
                                                         When writing this field of the SLI_PKT#_IN_BP
                                                         register, use a 4-byte write so as to not write
                                                         any other field of this register. */
	uint64_t cnt                          : 32; /**< ring counter. This field is incremented by one
                                                         whenever OCTEON receives, buffers, and creates a
                                                         work queue entry for a packet that arrives by the
                                                         cooresponding input ring. A write to this field
                                                         will be subtracted from the field value.
                                                         When writing this field of the SLI_PKT#_IN_BP
                                                         register, use a 4-byte write so as to not write
                                                         any other field of this register. */
#else
	uint64_t cnt                          : 32;
	uint64_t wmark                        : 32;
#endif
	} s;
	struct cvmx_sli_pktx_in_bp_s          cn61xx;
	struct cvmx_sli_pktx_in_bp_s          cn63xx;
	struct cvmx_sli_pktx_in_bp_s          cn63xxp1;
	struct cvmx_sli_pktx_in_bp_s          cn66xx;
	struct cvmx_sli_pktx_in_bp_s          cn70xx;
	struct cvmx_sli_pktx_in_bp_s          cn70xxp1;
	struct cvmx_sli_pktx_in_bp_s          cnf71xx;
};
typedef union cvmx_sli_pktx_in_bp cvmx_sli_pktx_in_bp_t;

/**
 * cvmx_sli_pkt#_input_control
 *
 * This register is the control for read operations for gather list and instructions.
 *
 */
union cvmx_sli_pktx_input_control {
	uint64_t u64;
	struct cvmx_sli_pktx_input_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_55_63               : 9;
	uint64_t rpvf                         : 7;  /**< The number of rings assigned to this VF.
                                                         Read only copy of SLI_PKT_MAC()_PF()_RINFO[RPVF] */
	uint64_t reserved_31_47               : 17;
	uint64_t mac_num                      : 2;  /**< The MAC (PEM) that the physical function belongs to. Enumerated by SLI_PORT_E.
                                                         [MAC_NUM] is RO when accessed via BAR0 of a virtual function, and R/W otherwise.
                                                         [MAC_NUM] applies to the ring pair, which includes both this input
                                                         ring and to the output ring of the same index. */
	uint64_t quiet                        : 1;  /**< Asserted after a ring has gone into reset and the ring has met the conditions
                                                         of SLI_PKT_GBL_CONTROL[QTIME]. */
	uint64_t reserved_27_27               : 1;
	uint64_t rdsize                       : 2;  /**< Number of instructions to be read in one MAC read request. Two-bit values are:
                                                         0x0 = 1 instruction.
                                                         0x1 = 2 instructions.
                                                         0x2 = 3 instructions.
                                                         0x3 = 4 instructions. */
	uint64_t is_64b                       : 1;  /**< When IS_64B=1, instruction input ring i uses 64B versus 32B. */
	uint64_t rst                          : 1;  /**< Packet reset. When [RST]=1, the rings are in reset. [RST] can be set
                                                         by software writing a 1 to the field, by hardware upon receipt of an
                                                         FLR to an associated function, or by hardware when it receives an error
                                                         response for a read associated with the rings.
                                                         [RST] applies to both this input ring and to the output ring of the same
                                                         index.
                                                         Software must not clear [RST] from 1->0 until the [QUIET] bit is a 1.
                                                         A ring reset may clear all state associated with the input and
                                                         output rings, so software must completely re-initialize both
                                                         before reusing them.
                                                         See also SLI_PKT_RING_RST[RST]. */
	uint64_t enb                          : 1;  /**< Enable for the input ring i. Whenever [RST] is set, hardware forces
                                                         [ENB] clear.  Software can only write [ENB] to 1.  [ENB] can only be cleared
                                                         only by writing SLI_PKT()_INPUT_CONTROL[RST].  Once [ENB] is cleared software can
                                                         only write [ENB] to a 1 once [QUIET] is a 1.
                                                         In the PF, [ENB] is also SLI_PKT_INSTR_ENB<i>. */
	uint64_t pbp_dhi                      : 13; /**< Not used by hardware, but may be cleared by hardware when [RST] is set. */
	uint64_t d_nsr                        : 1;  /**< If [USE_CSR]=1 (DPTR Format 0), [D_NSR] is ADDRTYPE<1> for First Direct and
                                                         Gather DPTR reads. ADDRTYPE<1> is the no-snoop attribute for PCIe.
                                                         For CNF75XXX, ADDRTYPE<1> helps select an SRIO()_S2M_TYPE() entry with sRIO.
                                                         If [USE_CSR]=0 (DPTR Format 1), [D_NSR] is MACADD<61> for First Direct and
                                                         Gather DPTR reads. (ADDRTYPE<1> comes from DPTR<61> in these cases when
                                                         [USE_CSR]=0.) */
	uint64_t d_esr                        : 2;  /**< If [USE_CSR]=1 (DPTR Format 0), [D_ESR] is ES<1:0> for First Direct and Gather
                                                         DPTR reads. ES<1:0> is the endian-swap attribute for these MAC memory space
                                                         reads. Enumerated by SLI_ENDIANSWAP_E.
                                                         If [USE_CSR]=0 (DPTR Format 1), [D_NSR] is MACADD<63:62> for First Direct and
                                                         Gather DPTR reads. (ES<1:0> comes from DPTR<63:62> in these cases when
                                                         [USE_CSR]=0.) */
	uint64_t d_ror                        : 1;  /**< If [USE_CSR]=1 (DPTR Format 0), [D_ROR] is ADDRTYPE<0> for First Direct and
                                                         Gather DPTR reads. ADDRTYPE<0> is the relaxed-order attribute for PCIe.
                                                         For CNF75XX, ADDRTYPE<0> helps select an SRIO()_S2M_TYPE() entry with sRIO.
                                                         If [USE_CSR]=0 (DPTR Format 1), [D_NSR] is MACADD<60> for First Direct and
                                                         Gather DPTR reads. (ADDRTYPE<0> comes from DPTR<60> in these cases when
                                                         [USE_CSR]=0.) */
	uint64_t use_csr                      : 1;  /**< When set to 1, [D_ROR], [D_ESR], and [D_NSR] are used for ROR, ESR, and NSR,
                                                         respectively.
                                                         (DPTR Format 0) When clear to 0, <63:60> from a First Direct or Gather DPTR are
                                                         used.
                                                         (DPTR Format 1) The bits not used for ROR, ESR, and NSR become bits <63:60> of
                                                         the address used to fetch packet data. */
	uint64_t nsr                          : 1;  /**< [NSR] is ADDRTYPE<1> for input instruction reads (from
                                                         SLI_PKT()_INSTR_BADDR+) and First Indirect DPTR reads. ADDRTYPE<1>
                                                         is the no-snoop attribute for PCIe.
                                                         [NSR] helps select an SRIO()_S2M_TYPE() entry with sRIO. */
	uint64_t esr                          : 2;  /**< [ESR] is ES<1:0> for input instruction reads (from
                                                         SLI_PKT()_INSTR_BADDR+) and First Indirect DPTR reads. ES<1:0> is
                                                         the endian-swap attribute for these MAC memory space reads.
                                                         Enumerated by SLI_ENDIANSWAP_E. */
	uint64_t ror                          : 1;  /**< [ROR] is ADDRTYPE<0> for input instruction reads (from
                                                         SLI_PKT()_INSTR_BADDR+) and First Indirect DPTR reads.
                                                         ADDRTYPE<0> is the relaxed-order attribute for PCIe.
                                                         It helps select an SRIO()_S2M_TYPE() entry with sRIO. */
#else
	uint64_t ror                          : 1;
	uint64_t esr                          : 2;
	uint64_t nsr                          : 1;
	uint64_t use_csr                      : 1;
	uint64_t d_ror                        : 1;
	uint64_t d_esr                        : 2;
	uint64_t d_nsr                        : 1;
	uint64_t pbp_dhi                      : 13;
	uint64_t enb                          : 1;
	uint64_t rst                          : 1;
	uint64_t is_64b                       : 1;
	uint64_t rdsize                       : 2;
	uint64_t reserved_27_27               : 1;
	uint64_t quiet                        : 1;
	uint64_t mac_num                      : 2;
	uint64_t reserved_31_47               : 17;
	uint64_t rpvf                         : 7;
	uint64_t reserved_55_63               : 9;
#endif
	} s;
	struct cvmx_sli_pktx_input_control_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_55_63               : 9;
	uint64_t rpvf                         : 7;  /**< The number of rings assigned to this VF.
                                                         Read only copy of SLI_PKT_MAC()_PF()_RINFO[RPVF] */
	uint64_t pvf_num                      : 16; /**< The function that the ring belongs to. DPI_DMA_FUNC_SEL_S describes
                                                         the format of this field. If [PVF_NUM]!=0, [MAC_NUM] must select a
                                                         PCIe MAC that supports more than one physical function and/or must
                                                         select a PCIe MAC with SR-IOV enabled.
                                                         [PVF_NUM] configuration must match SLI_PKT_MAC()_PF()_RINFO
                                                         configuration.
                                                         [PVF_NUM] is RO when accessed via BAR0 of a virtual function, and R/W
                                                         otherwise.
                                                         [PVF_NUM] applies to the ring pair, which includes both this input
                                                         ring and to the output ring of the same index. */
	uint64_t reserved_31_31               : 1;
	uint64_t mac_num                      : 2;  /**< The MAC (PEM) that the physical function belongs to. Enumerated by SLI_PORT_E.
                                                         [MAC_NUM] is RO when accessed via BAR0 of a virtual function, and R/W otherwise.
                                                         [MAC_NUM] applies to the ring pair, which includes both this input
                                                         ring and to the output ring of the same index. */
	uint64_t quiet                        : 1;  /**< Asserted after a ring has gone into reset and the ring has met the conditions
                                                         of SLI_PKT_GBL_CONTROL[QTIME]. */
	uint64_t reserved_27_27               : 1;
	uint64_t rdsize                       : 2;  /**< Number of instructions to be read in one MAC read request. Two-bit values are:
                                                         0x0 = 1 instruction.
                                                         0x1 = 2 instructions.
                                                         0x2 = 3 instructions.
                                                         0x3 = 4 instructions. */
	uint64_t is_64b                       : 1;  /**< When IS_64B=1, instruction input ring i uses 64B versus 32B. */
	uint64_t rst                          : 1;  /**< Packet reset. When [RST]=1, the rings are in reset. [RST] can be set
                                                         by software writing a 1 to the field, by hardware upon receipt of an
                                                         FLR to an associated function, or by hardware when it receives an error
                                                         response for a read associated with the rings.
                                                         [RST] applies to both this input ring and to the output ring of the same
                                                         index.
                                                         Software must not clear [RST] from 1->0 until the [QUIET] bit is a 1.
                                                         A ring reset may clear all state associated with the input and
                                                         output rings, so software must completely re-initialize both
                                                         before reusing them.
                                                         See also SLI_PKT_RING_RST[RST]. */
	uint64_t enb                          : 1;  /**< Enable for the input ring i. Whenever [RST] is set, hardware forces
                                                         [ENB] clear.  Software can only write [ENB] to 1.  [ENB] can only be cleared
                                                         only by writing SLI_PKT()_INPUT_CONTROL[RST].  Once [ENB] is cleared software can
                                                         only write [ENB] to a 1 once [QUIET] is a 1.
                                                         In the PF, [ENB] is also SLI_PKT_INSTR_ENB<i>. */
	uint64_t pbp_dhi                      : 13; /**< Not used by hardware, but may be cleared by hardware when [RST] is set. */
	uint64_t d_nsr                        : 1;  /**< If [USE_CSR]=1, [D_NSR] is ADDRTYPE<1> for First Direct and Gather DPTR
                                                         reads. ADDRTYPE<1> is the no-snoop attribute for PCIe. (DPTR Format 0)
                                                         If [USE_CSR]=0, [D_NSR] is MACADD<61> for First Direct and Gather DPTR
                                                         reads. (ADDRTYPE<1> comes from DPTR<61> in these cases when [USE_CSR]=0.)
                                                         (DPTR Format 1) */
	uint64_t d_esr                        : 2;  /**< If [USE_CSR]=1 (DPTR Format 0), [D_ESR] is ES<1:0> for First Direct and Gather
                                                         DPTR reads. ES<1:0> is the endian-swap attribute for these MAC memory space
                                                         reads. Enumerated by SLI_ENDIANSWAP_E.
                                                         If [USE_CSR]=0 (DPTR Format 1), [D_NSR] is MACADD<63:62> for First Direct and
                                                         Gather DPTR reads. (ES<1:0> comes from DPTR<63:62> in these cases when
                                                         [USE_CSR]=0.) */
	uint64_t d_ror                        : 1;  /**< If [USE_CSR]=1, [D_ROR] is ADDRTYPE<0> for First Direct and Gather DPTR
                                                         reads. ADDRTYPE<0> is the relaxed-order attribute for PCIe. (DPTR Format 0)
                                                         If [USE_CSR]=0, [D_NSR] is MACADD<60> for First Direct and Gather DPTR
                                                         reads. (ADDRTYPE<0> comes from DPTR<60> in these cases when [USE_CSR]=0.)
                                                         (DPTR Format 1) */
	uint64_t use_csr                      : 1;  /**< When set to 1, [D_ROR], [D_ESR], and [D_NSR] are used for ROR, ESR, and NSR,
                                                         respectively.
                                                         (DPTR Format 0) When clear to 0, <63:60> from a First Direct or Gather DPTR are
                                                         used.
                                                         (DPTR Format 1) The bits not used for ROR, ESR, and NSR become bits <63:60> of
                                                         the address used to fetch packet data. */
	uint64_t nsr                          : 1;  /**< [NSR] is ADDRTYPE<1> for input instruction reads (from
                                                         SLI_PKT()_INSTR_BADDR+) and First Indirect DPTR reads. ADDRTYPE<1>
                                                         is the no-snoop attribute for PCIe. */
	uint64_t esr                          : 2;  /**< [ESR] is ES<1:0> for input instruction reads (from
                                                         SLI_PKT()_INSTR_BADDR+) and First Indirect DPTR reads. ES<1:0> is
                                                         the endian-swap attribute for these MAC memory space reads.
                                                         Enumerated by SLI_ENDIANSWAP_E. */
	uint64_t ror                          : 1;  /**< [ROR] is ADDRTYPE<0> for input instruction reads (from
                                                         SLI_PKT()_INSTR_BADDR+) and First Indirect DPTR reads.
                                                         ADDRTYPE<0> is the relaxed-order attribute for PCIe. */
#else
	uint64_t ror                          : 1;
	uint64_t esr                          : 2;
	uint64_t nsr                          : 1;
	uint64_t use_csr                      : 1;
	uint64_t d_ror                        : 1;
	uint64_t d_esr                        : 2;
	uint64_t d_nsr                        : 1;
	uint64_t pbp_dhi                      : 13;
	uint64_t enb                          : 1;
	uint64_t rst                          : 1;
	uint64_t is_64b                       : 1;
	uint64_t rdsize                       : 2;
	uint64_t reserved_27_27               : 1;
	uint64_t quiet                        : 1;
	uint64_t mac_num                      : 2;
	uint64_t reserved_31_31               : 1;
	uint64_t pvf_num                      : 16;
	uint64_t rpvf                         : 7;
	uint64_t reserved_55_63               : 9;
#endif
	} cn73xx;
	struct cvmx_sli_pktx_input_control_cn73xx cn78xx;
	struct cvmx_sli_pktx_input_control_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_39_63               : 25;
	uint64_t vf_num                       : 7;  /**< The function number that the ring belongs to. When [VF_NUM]=0, the physical
                                                         function controls the ring. When [VF_NUM]!=0, it must correctly indicate the
                                                         virtual function number that controls the ring. ([VF_NUM]=1 selects the first
                                                         virtual function within the selected physical function, [VF_NUM]=2 selects
                                                         the second virtual function within the selected physical function ...)
                                                         Legal values are 0..64.
                                                         [VF_NUM] is RO when accessed via BAR0 of a virtual function, and R/W otherwise.
                                                         [VF_NUM] applies to both this input ring and to the output ring of the same
                                                         index. */
	uint64_t reserved_31_31               : 1;
	uint64_t mac_num                      : 2;  /**< The MAC (PEM) that the physical function belongs to. Legal value are 0-3.
                                                         [MAC_NUM] is RO when accessed via BAR0 of a virtual function, and R/W otherwise.
                                                         [MAC_NUM] applies to both this input ring and to the output ring of the same
                                                         index. */
	uint64_t reserved_27_28               : 2;
	uint64_t rdsize                       : 2;  /**< Number of instructions to be read in one MAC read request for the 4 ports, 16 rings. Two
                                                         bit value are:
                                                         0x0 = 1 Instruction.
                                                         0x1 = 2 Instructions.
                                                         0x2 = 3 Instructions.
                                                         0x3 = 4 Instructions. */
	uint64_t is_64b                       : 1;  /**< When IS_64B=1, instruction input ring i uses 64B versus 32B. */
	uint64_t rst                          : 1;  /**< Packet reset. When [RST]=1, the rings are in reset. [RST] can be set
                                                         by software writing a 1 to the field, by hardware upon receipt of an
                                                         FLR to an associated function, or by hardware when it receives an error
                                                         response for a read associated with the rings.
                                                         [MAC_NUM] applies to both this input ring and to the output ring of the same
                                                         index.
                                                         Software must not clear [RST] from 1->0 until [RST] has been asserted
                                                         for at least 2ms. A ring reset may clear all state associated with the
                                                         input and output rings, so software must completely re-initialize both
                                                         before reusing them.
                                                         See also SLI_PKT_RING_RST[RST]. */
	uint64_t enb                          : 1;  /**< Enable for the input ring i. Whenever [RST] is set, hardware forces
                                                         [ENB] clear.
                                                         In the PF, [ENB] is also SLI_PKT_INSTR_ENB<i>. */
	uint64_t pbp_dhi                      : 13; /**< Not used by hardware, but may be cleared by hardware when [RST] is set. */
	uint64_t d_nsr                        : 1;  /**< If [USE_CSR]=1 (DPTR Format 0), [D_NSR] is ADDRTYPE<1> for First Direct and
                                                         Gather DPTR reads. ADDRTYPE<1> is the no-snoop attribute for PCIe.
                                                         If [USE_CSR]=0 (DPTR Format 1), [D_NSR] is MACADD<61> for First Direct and
                                                         Gather DPTR reads. (ADDRTYPE<1> comes from DPTR<61> in these cases when
                                                         [USE_CSR]=0.) */
	uint64_t d_esr                        : 2;  /**< If [USE_CSR]=1 (DPTR Format 0), [D_ESR] is ES<1:0> for First Direct and Gather
                                                         DPTR reads. ES<1:0> is the endian-swap attribute for these MAC memory space
                                                         reads. Enumerated by SLI_ENDIANSWAP_E.
                                                         If [USE_CSR]=0 (DPTR Format 1), [D_NSR] is MACADD<63:62> for First Direct and
                                                         Gather DPTR reads. (ES<1:0> comes from DPTR<63:62> in these cases when
                                                         [USE_CSR]=0.) */
	uint64_t d_ror                        : 1;  /**< If [USE_CSR]=1 (DPTR Format 0), [D_ROR] is ADDRTYPE<0> for First Direct and
                                                         Gather DPTR reads. ADDRTYPE<0> is the relaxed-order attribute for PCIe.
                                                         If [USE_CSR]=0 (DPTR Format 1), [D_NSR] is MACADD<60> for First Direct and
                                                         Gather DPTR reads. (ADDRTYPE<0> comes from DPTR<60> in these cases when
                                                         [USE_CSR]=0.) */
	uint64_t use_csr                      : 1;  /**< When set to 1, [D_ROR], [D_ESR], and [D_NSR] are used for ROR, ESR, and NSR,
                                                         respectively.
                                                         (DPTR Format 0) When clear to 0, <63:60> from a First Direct or Gather DPTR are
                                                         used.
                                                         (DPTR Format 1) The bits not used for ROR, ESR, and NSR become bits <63:60> of
                                                         the address used to fetch packet data. */
	uint64_t nsr                          : 1;  /**< [NSR] is ADDRTYPE<1> for input instruction reads (from
                                                         SLI_PKT()_INSTR_BADDR+) and First Indirect DPTR reads. ADDRTYPE<1>
                                                         is the no-snoop attribute for PCIe. */
	uint64_t esr                          : 2;  /**< [ESR] is ES<1:0> for input instruction reads (from
                                                         SLI_PKT()_INSTR_BADDR+) and First Indirect DPTR reads. ES<1:0> is
                                                         the endian-swap attribute for these MAC memory space reads.
                                                         Enumerated by SLI_ENDIANSWAP_E. */
	uint64_t ror                          : 1;  /**< [ROR] is ADDRTYPE<0> for input instruction reads (from
                                                         SLI_PKT()_INSTR_BADDR+) and First Indirect DPTR reads.
                                                         ADDRTYPE<0> is the relaxed-order attribute for PCIe. */
#else
	uint64_t ror                          : 1;
	uint64_t esr                          : 2;
	uint64_t nsr                          : 1;
	uint64_t use_csr                      : 1;
	uint64_t d_ror                        : 1;
	uint64_t d_esr                        : 2;
	uint64_t d_nsr                        : 1;
	uint64_t pbp_dhi                      : 13;
	uint64_t enb                          : 1;
	uint64_t rst                          : 1;
	uint64_t is_64b                       : 1;
	uint64_t rdsize                       : 2;
	uint64_t reserved_27_28               : 2;
	uint64_t mac_num                      : 2;
	uint64_t reserved_31_31               : 1;
	uint64_t vf_num                       : 7;
	uint64_t reserved_39_63               : 25;
#endif
	} cn78xxp1;
	struct cvmx_sli_pktx_input_control_cn73xx cnf75xx;
};
typedef union cvmx_sli_pktx_input_control cvmx_sli_pktx_input_control_t;

/**
 * cvmx_sli_pkt#_instr_baddr
 *
 * This register contains the start-of-instruction for input packets. The address must be
 * addressed-aligned to the size of the instruction.
 */
union cvmx_sli_pktx_instr_baddr {
	uint64_t u64;
	struct cvmx_sli_pktx_instr_baddr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t addr                         : 61; /**< Base address for instruction ring. [ADDR] Must be naturally-aligned to the
                                                         instruction size selected by SLI_PKT()_INPUT_CONTROL[IS_64B].  The hardware
                                                         ignores the bottom bits if not naturally aligned. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t addr                         : 61;
#endif
	} s;
	struct cvmx_sli_pktx_instr_baddr_s    cn61xx;
	struct cvmx_sli_pktx_instr_baddr_s    cn63xx;
	struct cvmx_sli_pktx_instr_baddr_s    cn63xxp1;
	struct cvmx_sli_pktx_instr_baddr_s    cn66xx;
	struct cvmx_sli_pktx_instr_baddr_s    cn68xx;
	struct cvmx_sli_pktx_instr_baddr_s    cn68xxp1;
	struct cvmx_sli_pktx_instr_baddr_s    cn70xx;
	struct cvmx_sli_pktx_instr_baddr_s    cn70xxp1;
	struct cvmx_sli_pktx_instr_baddr_s    cn73xx;
	struct cvmx_sli_pktx_instr_baddr_s    cn78xx;
	struct cvmx_sli_pktx_instr_baddr_s    cn78xxp1;
	struct cvmx_sli_pktx_instr_baddr_s    cnf71xx;
	struct cvmx_sli_pktx_instr_baddr_s    cnf75xx;
};
typedef union cvmx_sli_pktx_instr_baddr cvmx_sli_pktx_instr_baddr_t;

/**
 * cvmx_sli_pkt#_instr_baoff_dbell
 *
 * This register contains the doorbell and base address offset for the next read.
 *
 */
union cvmx_sli_pktx_instr_baoff_dbell {
	uint64_t u64;
	struct cvmx_sli_pktx_instr_baoff_dbell_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t aoff                         : 32; /**< The offset from the SLI_PKT()_INSTR_BADDR where the next instruction will be
                                                         read from.
                                                         A write of 0xFFFFFFFF to [DBELL] clears both [DBELL] and [AOFF]. */
	uint64_t dbell                        : 32; /**< Instruction doorbell count. Writes to this register increment [DBELL] by the
                                                         value written to [DBELL]. Hardware decrements [DBELL] as it reads instructions,
                                                         simultaneously also "incrementing" [AOFF].
                                                         A write of 0xFFFFFFFF to [DBELL] clears both [DBELL] and [AOFF].
                                                         Writes of non 0xFFFFFFFF are ignored if this ring is in reset. */
#else
	uint64_t dbell                        : 32;
	uint64_t aoff                         : 32;
#endif
	} s;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn61xx;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn63xx;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn63xxp1;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn66xx;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn68xx;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn68xxp1;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn70xx;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn70xxp1;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn73xx;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn78xx;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn78xxp1;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cnf71xx;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cnf75xx;
};
typedef union cvmx_sli_pktx_instr_baoff_dbell cvmx_sli_pktx_instr_baoff_dbell_t;

/**
 * cvmx_sli_pkt#_instr_fifo_rsize
 *
 * This register contains the FIFO field and ring size for instructions.
 *
 */
union cvmx_sli_pktx_instr_fifo_rsize {
	uint64_t u64;
	struct cvmx_sli_pktx_instr_fifo_rsize_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t max                          : 9;  /**< Max FIFO size. */
	uint64_t rrp                          : 9;  /**< FIFO read pointer. */
	uint64_t wrp                          : 9;  /**< FIFO write pointer. */
	uint64_t fcnt                         : 5;  /**< FIFO count. */
	uint64_t rsize                        : 32; /**< Instruction ring size.  Legal values have to be greater then 128.
                                                         Writes to [RSIZE] of less than 128 will set [RSIZE] to 128. */
#else
	uint64_t rsize                        : 32;
	uint64_t fcnt                         : 5;
	uint64_t wrp                          : 9;
	uint64_t rrp                          : 9;
	uint64_t max                          : 9;
#endif
	} s;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn61xx;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn63xx;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn63xxp1;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn66xx;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn68xx;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn68xxp1;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn70xx;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn70xxp1;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn73xx;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn78xx;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn78xxp1;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cnf71xx;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cnf75xx;
};
typedef union cvmx_sli_pktx_instr_fifo_rsize cvmx_sli_pktx_instr_fifo_rsize_t;

/**
 * cvmx_sli_pkt#_instr_header
 *
 * "SLI_PKT[0..31]_INSTR_HEADER = SLI Packet ring# Instruction Header.
 * VAlues used to build input packet header."
 */
union cvmx_sli_pktx_instr_header {
	uint64_t u64;
	struct cvmx_sli_pktx_instr_header_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t pbp                          : 1;  /**< Enable Packet-by-packet mode.
                                                         Allows DPI to generate PKT_INST_HDR[PM,SL]
                                                         differently per DPI instruction.
                                                         USE_IHDR must be set whenever PBP is set. */
	uint64_t reserved_38_42               : 5;
	uint64_t rparmode                     : 2;  /**< Parse Mode. Becomes PKT_INST_HDR[PM]
                                                         when DPI_INST_HDR[R]==1 and PBP==0 */
	uint64_t reserved_35_35               : 1;
	uint64_t rskp_len                     : 7;  /**< Skip Length. Becomes PKT_INST_HDR[SL]
                                                         when DPI_INST_HDR[R]==1 and PBP==0 */
	uint64_t rngrpext                     : 2;  /**< Becomes PKT_INST_HDR[GRPEXT]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rnqos                        : 1;  /**< Becomes PKT_INST_HDR[NQOS]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rngrp                        : 1;  /**< Becomes PKT_INST_HDR[NGRP]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rntt                         : 1;  /**< Becomes PKT_INST_HDR[NTT]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rntag                        : 1;  /**< Becomes PKT_INST_HDR[NTAG]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t use_ihdr                     : 1;  /**< When set '1' DPI always prepends a PKT_INST_HDR
                                                         as part of the packet data sent to PIP/IPD,
                                                         regardless of DPI_INST_HDR[R]. (DPI also always
                                                         prepends a PKT_INST_HDR when DPI_INST_HDR[R]=1.)
                                                         USE_IHDR must be set whenever PBP is set. */
	uint64_t reserved_16_20               : 5;
	uint64_t par_mode                     : 2;  /**< Parse Mode. Becomes PKT_INST_HDR[PM]
                                                         when DPI_INST_HDR[R]==0 and USE_IHDR==1 and PBP==0 */
	uint64_t reserved_13_13               : 1;
	uint64_t skp_len                      : 7;  /**< Skip Length. Becomes PKT_INST_HDR[SL]
                                                         when DPI_INST_HDR[R]==0 and USE_IHDR==1 and PBP==0 */
	uint64_t ngrpext                      : 2;  /**< Becomes PKT_INST_HDR[GRPEXT]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t nqos                         : 1;  /**< Becomes PKT_INST_HDR[NQOS]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t ngrp                         : 1;  /**< Becomes PKT_INST_HDR[NGRP]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t ntt                          : 1;  /**< Becomes PKT_INST_HDR[NTT]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t ntag                         : 1;  /**< Becomes PKT_INST_HDR[NTAG]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
#else
	uint64_t ntag                         : 1;
	uint64_t ntt                          : 1;
	uint64_t ngrp                         : 1;
	uint64_t nqos                         : 1;
	uint64_t ngrpext                      : 2;
	uint64_t skp_len                      : 7;
	uint64_t reserved_13_13               : 1;
	uint64_t par_mode                     : 2;
	uint64_t reserved_16_20               : 5;
	uint64_t use_ihdr                     : 1;
	uint64_t rntag                        : 1;
	uint64_t rntt                         : 1;
	uint64_t rngrp                        : 1;
	uint64_t rnqos                        : 1;
	uint64_t rngrpext                     : 2;
	uint64_t rskp_len                     : 7;
	uint64_t reserved_35_35               : 1;
	uint64_t rparmode                     : 2;
	uint64_t reserved_38_42               : 5;
	uint64_t pbp                          : 1;
	uint64_t reserved_44_63               : 20;
#endif
	} s;
	struct cvmx_sli_pktx_instr_header_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t pbp                          : 1;  /**< Enable Packet-by-packet mode.
                                                         Allows DPI to generate PKT_INST_HDR[PM,SL]
                                                         differently per DPI instruction.
                                                         USE_IHDR must be set whenever PBP is set. */
	uint64_t reserved_38_42               : 5;
	uint64_t rparmode                     : 2;  /**< Parse Mode. Becomes PKT_INST_HDR[PM]
                                                         when DPI_INST_HDR[R]==1 and PBP==0 */
	uint64_t reserved_35_35               : 1;
	uint64_t rskp_len                     : 7;  /**< Skip Length. Becomes PKT_INST_HDR[SL]
                                                         when DPI_INST_HDR[R]==1 and PBP==0 */
	uint64_t reserved_26_27               : 2;
	uint64_t rnqos                        : 1;  /**< Becomes PKT_INST_HDR[NQOS]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rngrp                        : 1;  /**< Becomes PKT_INST_HDR[NGRP]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rntt                         : 1;  /**< Becomes PKT_INST_HDR[NTT]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rntag                        : 1;  /**< Becomes PKT_INST_HDR[NTAG]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t use_ihdr                     : 1;  /**< When set '1' DPI always prepends a PKT_INST_HDR
                                                         as part of the packet data sent to PIP/IPD,
                                                         regardless of DPI_INST_HDR[R]. (DPI also always
                                                         prepends a PKT_INST_HDR when DPI_INST_HDR[R]=1.)
                                                         USE_IHDR must be set whenever PBP is set. */
	uint64_t reserved_16_20               : 5;
	uint64_t par_mode                     : 2;  /**< Parse Mode. Becomes PKT_INST_HDR[PM]
                                                         when DPI_INST_HDR[R]==0 and USE_IHDR==1 and PBP==0 */
	uint64_t reserved_13_13               : 1;
	uint64_t skp_len                      : 7;  /**< Skip Length. Becomes PKT_INST_HDR[SL]
                                                         when DPI_INST_HDR[R]==0 and USE_IHDR==1 and PBP==0 */
	uint64_t reserved_4_5                 : 2;
	uint64_t nqos                         : 1;  /**< Becomes PKT_INST_HDR[NQOS]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t ngrp                         : 1;  /**< Becomes PKT_INST_HDR[NGRP]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t ntt                          : 1;  /**< Becomes PKT_INST_HDR[NTT]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t ntag                         : 1;  /**< Becomes PKT_INST_HDR[NTAG]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
#else
	uint64_t ntag                         : 1;
	uint64_t ntt                          : 1;
	uint64_t ngrp                         : 1;
	uint64_t nqos                         : 1;
	uint64_t reserved_4_5                 : 2;
	uint64_t skp_len                      : 7;
	uint64_t reserved_13_13               : 1;
	uint64_t par_mode                     : 2;
	uint64_t reserved_16_20               : 5;
	uint64_t use_ihdr                     : 1;
	uint64_t rntag                        : 1;
	uint64_t rntt                         : 1;
	uint64_t rngrp                        : 1;
	uint64_t rnqos                        : 1;
	uint64_t reserved_26_27               : 2;
	uint64_t rskp_len                     : 7;
	uint64_t reserved_35_35               : 1;
	uint64_t rparmode                     : 2;
	uint64_t reserved_38_42               : 5;
	uint64_t pbp                          : 1;
	uint64_t reserved_44_63               : 20;
#endif
	} cn61xx;
	struct cvmx_sli_pktx_instr_header_cn61xx cn63xx;
	struct cvmx_sli_pktx_instr_header_cn61xx cn63xxp1;
	struct cvmx_sli_pktx_instr_header_cn61xx cn66xx;
	struct cvmx_sli_pktx_instr_header_s   cn68xx;
	struct cvmx_sli_pktx_instr_header_cn61xx cn68xxp1;
	struct cvmx_sli_pktx_instr_header_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t pbp                          : 1;  /**< Enable Packet-by-packet mode.
                                                         Allows DPI to generate PKT_INST_HDR[PM,SL]
                                                         differently per DPI instruction.
                                                         USE_IHDR must be set whenever PBP is set. */
	uint64_t reserved_38_42               : 5;
	uint64_t rparmode                     : 2;  /**< Parse Mode. Becomes PKT_INST_HDR[PM]
                                                         when DPI_INST_HDR[R]==1 and PBP==0 */
	uint64_t reserved_35_35               : 1;
	uint64_t rskp_len                     : 7;  /**< Skip Length. Becomes PKT_INST_HDR[SL]
                                                         when DPI_INST_HDR[R]==1 and PBP==0 */
	uint64_t reserved_27_26               : 2;
	uint64_t rnqos                        : 1;  /**< Becomes PKT_INST_HDR[NQOS]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rngrp                        : 1;  /**< Becomes PKT_INST_HDR[NGRP]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rntt                         : 1;  /**< Becomes PKT_INST_HDR[NTT]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rntag                        : 1;  /**< Becomes PKT_INST_HDR[NTAG]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t use_ihdr                     : 1;  /**< When set '1' DPI always prepends a PKT_INST_HDR
                                                         as part of the packet data sent to PIP/IPD,
                                                         regardless of DPI_INST_HDR[R]. (DPI also always
                                                         prepends a PKT_INST_HDR when DPI_INST_HDR[R]=1.)
                                                         USE_IHDR must be set whenever PBP is set. */
	uint64_t reserved_20_16               : 5;
	uint64_t par_mode                     : 2;  /**< Parse Mode. Becomes PKT_INST_HDR[PM]
                                                         when DPI_INST_HDR[R]==0 and USE_IHDR==1 and PBP==0 */
	uint64_t reserved_13_13               : 1;
	uint64_t skp_len                      : 7;  /**< Skip Length. Becomes PKT_INST_HDR[SL]
                                                         when DPI_INST_HDR[R]==0 and USE_IHDR==1 and PBP==0 */
	uint64_t reserved_5_4                 : 2;
	uint64_t nqos                         : 1;  /**< Becomes PKT_INST_HDR[NQOS]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t ngrp                         : 1;  /**< Becomes PKT_INST_HDR[NGRP]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t ntt                          : 1;  /**< Becomes PKT_INST_HDR[NTT]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t ntag                         : 1;  /**< Becomes PKT_INST_HDR[NTAG]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
#else
	uint64_t ntag                         : 1;
	uint64_t ntt                          : 1;
	uint64_t ngrp                         : 1;
	uint64_t nqos                         : 1;
	uint64_t reserved_5_4                 : 2;
	uint64_t skp_len                      : 7;
	uint64_t reserved_13_13               : 1;
	uint64_t par_mode                     : 2;
	uint64_t reserved_20_16               : 5;
	uint64_t use_ihdr                     : 1;
	uint64_t rntag                        : 1;
	uint64_t rntt                         : 1;
	uint64_t rngrp                        : 1;
	uint64_t rnqos                        : 1;
	uint64_t reserved_27_26               : 2;
	uint64_t rskp_len                     : 7;
	uint64_t reserved_35_35               : 1;
	uint64_t rparmode                     : 2;
	uint64_t reserved_38_42               : 5;
	uint64_t pbp                          : 1;
	uint64_t reserved_44_63               : 20;
#endif
	} cn70xx;
	struct cvmx_sli_pktx_instr_header_cn70xx cn70xxp1;
	struct cvmx_sli_pktx_instr_header_cn61xx cnf71xx;
};
typedef union cvmx_sli_pktx_instr_header cvmx_sli_pktx_instr_header_t;

/**
 * cvmx_sli_pkt#_int_levels
 *
 * This register contains output-packet interrupt levels.
 *
 */
union cvmx_sli_pktx_int_levels {
	uint64_t u64;
	struct cvmx_sli_pktx_int_levels_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t time                         : 22; /**< Output ring counter time interrupt threshold. SLI sets SLI_PKT()_CNTS[PO_INT]
                                                         (and SLI_PKT_TIME_INT<i> and SLI_PKT_INT<i>), and may cause an MSI-X, MSI, or
                                                         INTA/B/C/D interrupt, whenever SLI_PKT()_CNTS[TIMER] > [TIME].
                                                         Whenever software changes the value of [TIME], it should also subsequently write
                                                         the corresponding SLI_PKT()_CNTS register (with a value of zero if desired)
                                                         to ensure that the hardware correspondingly updates SLI_PKT()_CNTS[PO_INT]. */
	uint64_t cnt                          : 32; /**< Output ring counter interrupt threshold. SLI sets SLI_PKT()_CNTS[PO_INT]
                                                         (and SLI_PKT_CNT_INT<i> and SLI_PKT_INT<i>), and may cause an MSI-X, MSI, or
                                                         INTA/B/C/D interrupt, whenever SLI_PKT()_CNTS[CNT] > [CNT].
                                                         Whenever software changes the value of [TIME], it should also subsequently write
                                                         the corresponding SLI_PKT()_CNTS register (with a value of zero if desired)
                                                         to ensure that the hardware correspondingly updates SLI_PKT()_CNTS[PO_INT]. */
#else
	uint64_t cnt                          : 32;
	uint64_t time                         : 22;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_sli_pktx_int_levels_s     cn73xx;
	struct cvmx_sli_pktx_int_levels_s     cn78xx;
	struct cvmx_sli_pktx_int_levels_s     cn78xxp1;
	struct cvmx_sli_pktx_int_levels_s     cnf75xx;
};
typedef union cvmx_sli_pktx_int_levels cvmx_sli_pktx_int_levels_t;

/**
 * cvmx_sli_pkt#_mbox_int
 *
 * This register contains information to service mbox interrupts to the VF
 * when the PF writes SLI_PKT()_PF_VF_MBOX_SIG(0).
 */
union cvmx_sli_pktx_mbox_int {
	uint64_t u64;
	struct cvmx_sli_pktx_mbox_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t po_int                       : 1;  /**< "Returns a 1 when either the corresponding bit in SLI_PKT_TIME_INT[RING<\#>] or
                                                         SLI_PKT_CNT_INT[RING<\#>] is set. This interrupt can be cleared by writing
                                                         SLI_PKT()_CNTS[CNT]." */
	uint64_t pi_int                       : 1;  /**< "Reads corresponding bit of SLI_PKT_IN_INT[RING<\#>]. This interrupt
                                                         can be cleared by writing SLI_PKT_IN_DONE()_CNTS." */
	uint64_t mbox_int                     : 1;  /**< Set to one when a PF writes the corresponding ring SLI_PKT()_PF_VF_MBOX_SIG(0)
                                                         register. Writes will clear this interrupt.  This bit can only be written by the VF side.
                                                         It cannot be cleared by the PF. [MBOX_INT] can cause an MSI-X interrupt for the ring,
                                                         but will never cause an INTA/B/C/D nor MSI interrupt nor set any
                                                         SLI_MAC()_PF()_INT_SUM bit. */
	uint64_t resend                       : 1;  /**< A write of 1 will resend an MSI-X interrupt message if there is a pending interrupt in
                                                         P0_INT, PI_INT or MBOX_INT for this ring after the clear of [MBOX_INT] occurs.
                                                         [RESEND] and [MBOX_INT] must be written together with the assumption that the write of
                                                         [CNT] will clear the [MBOX_INT] interrupt bit.
                                                         The [RESEND] bit will never effect INTA/B/C/D or MSI interrupt */
	uint64_t reserved_1_59                : 59;
	uint64_t mbox_en                      : 1;  /**< Enables interrupt to the MSIX vector associated with this VF when the PF writes the
                                                         corresponding ring in SLI_PKT()_VF_MBOX_SIG(0). */
#else
	uint64_t mbox_en                      : 1;
	uint64_t reserved_1_59                : 59;
	uint64_t resend                       : 1;
	uint64_t mbox_int                     : 1;
	uint64_t pi_int                       : 1;
	uint64_t po_int                       : 1;
#endif
	} s;
	struct cvmx_sli_pktx_mbox_int_s       cn73xx;
	struct cvmx_sli_pktx_mbox_int_s       cn78xx;
	struct cvmx_sli_pktx_mbox_int_s       cnf75xx;
};
typedef union cvmx_sli_pktx_mbox_int cvmx_sli_pktx_mbox_int_t;

/**
 * cvmx_sli_pkt#_out_size
 *
 * This register contains the BSIZE and ISIZE for output packet rings.
 *
 */
union cvmx_sli_pktx_out_size {
	uint64_t u64;
	struct cvmx_sli_pktx_out_size_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t isize                        : 7;  /**< Info bytes size (bytes) for this ring. Legal sizes are 0 to 59. Not used in buffer-
                                                         pointer-only mode.
                                                         PKO should be configured to pad all DPI packet output packets to 60 bytes
                                                         or more, so all packets will be more than [ISIZE] bytes. See
                                                         PKO_MAC()_CFG[MIN_PAD_ENA] and PKO_PDM_CFG[PKO_PAD_MINLEN]. */
	uint64_t bsize                        : 16; /**< Buffer size (bytes) for this ring. Legal values have to be greater then 128.
                                                         Writes of [BSIZE] less than 128 will set [BSIZE] to 128. */
#else
	uint64_t bsize                        : 16;
	uint64_t isize                        : 7;
	uint64_t reserved_23_63               : 41;
#endif
	} s;
	struct cvmx_sli_pktx_out_size_s       cn61xx;
	struct cvmx_sli_pktx_out_size_s       cn63xx;
	struct cvmx_sli_pktx_out_size_s       cn63xxp1;
	struct cvmx_sli_pktx_out_size_s       cn66xx;
	struct cvmx_sli_pktx_out_size_s       cn68xx;
	struct cvmx_sli_pktx_out_size_s       cn68xxp1;
	struct cvmx_sli_pktx_out_size_s       cn70xx;
	struct cvmx_sli_pktx_out_size_s       cn70xxp1;
	struct cvmx_sli_pktx_out_size_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_22_63               : 42;
	uint64_t isize                        : 6;  /**< Info bytes size (bytes) for this ring. Legal sizes are 0 to 59. Not used in buffer-
                                                         pointer-only mode.
                                                         PKO should be configured to pad all DPI packet output packets to 60 bytes
                                                         or more, so all packets will be more than [ISIZE] bytes. See
                                                         PKO_MAC()_CFG[MIN_PAD_ENA] and PKO_PDM_CFG[PKO_PAD_MINLEN]. */
	uint64_t bsize                        : 16; /**< Buffer size (bytes) for this ring. Legal values have to be greater then 128.
                                                         Writes of [BSIZE] less than 128 will set [BSIZE] to 128. */
#else
	uint64_t bsize                        : 16;
	uint64_t isize                        : 6;
	uint64_t reserved_22_63               : 42;
#endif
	} cn73xx;
	struct cvmx_sli_pktx_out_size_cn73xx  cn78xx;
	struct cvmx_sli_pktx_out_size_s       cn78xxp1;
	struct cvmx_sli_pktx_out_size_s       cnf71xx;
	struct cvmx_sli_pktx_out_size_cn73xx  cnf75xx;
};
typedef union cvmx_sli_pktx_out_size cvmx_sli_pktx_out_size_t;

/**
 * cvmx_sli_pkt#_output_control
 *
 * This register is the control for read operations for gather list and instructions.
 *
 */
union cvmx_sli_pktx_output_control {
	uint64_t u64;
	struct cvmx_sli_pktx_output_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t tenb                         : 1;  /**< SLI_MAC()_PF()_INT_SUM[PTIME] interrupt enable for this ring i. When [TENB] is set
                                                         and (SLI_PKT(i)_CNTS[TIMER] > SLI_PKT(i)_INT_LEVELS[TIME]),
                                                         SLI_MAC()_PF()_INT_SUM[PTIME] will be set, and SLI_MAC()_PF()_INT_SUM interrupts
                                                         can occur if corresponding SLI_MAC()_PF()_INT_SUM[PTIME] is set.
                                                         When [TENB] is clear, SLI_MAC()_PF()_INT_SUM[PTIME] will never assert due to
                                                         ring i.
                                                         [TENB] is RO when accessed via BAR0 of a virtual function, and R/W otherwise.
                                                         [TENB] has no effect on SLI_PKT_TIME_INT or SLI_PKT_INT, and
                                                         has no effect on any non-SLI_MAC()_PF()_INT_SUM interrupt. */
	uint64_t cenb                         : 1;  /**< SLI_MAC()_PF()_INT_SUM[PCNT] interrupt enable for this ring i. When [CENB] is set
                                                         and (SLI_PKT(i)_CNTS[CNT] > SLI_PKT(i)_INT_LEVELS[CNT]),
                                                         SLI_MAC()_PF()_INT_SUM[PCNT] will be set, and SLI_INT_SUM interrupts
                                                         can occur if corresponding SLI_MAC()_PF()_INT_SUM[PCNT] is set.
                                                         When [CENB] is clear, SLI_MAC()_PF()_INT_SUM[PCNT] will never assert due to
                                                         ring i.
                                                         [CENB] is RO when accessed via BAR0 of a virtual function, and R/W otherwise.
                                                         [CENB] has no effect on SLI_PKT_CNT_INT or SLI_PKT_INT, and
                                                         has no effect on any non-SLI_MAC()_PF()_INT_SUM interrupt. */
	uint64_t iptr                         : 1;  /**< When IPTR=1, packet output ring is in info-pointer mode; otherwise the packet output ring
                                                         is in buffer-pointer-only mode. */
	uint64_t es                           : 2;  /**< If [DPTR]=1 (DPTR Format 0), [ES] is ES<1:0> for buffer/info write operations to
                                                         buffer/info pair MAC memory space addresses fetched from packet output
                                                         ring. ES<1:0> is the endian-swap attribute for these MAC memory space writes.
                                                         If [DPTR]=0 (DPTR Format 1), [ES] is MACADD<63:62> for buffer/info write
                                                         operations to buffer/info pair MAC memory space addresses fetched from packet
                                                         output ring. (<63:62> of the buffer or info pointer is ES<1:0> for the writes in
                                                         this case when [DPTR]=0.) */
	uint64_t nsr                          : 1;  /**< If [DPTR]=1 (DPTR Format 0), [NSR] is ADDRTYPE<1> for buffer/info write
                                                         operations to buffer/info pair MAC memory space addresses fetched from packet
                                                         output ring. ADDRTYPE<1> is the no-snoop attribute for PCIe.
                                                         If [DPTR]=0 (DPTR Format 1), [NSR] is MACADD<61> for buffer/info write
                                                         operations to buffer/info pair MAC memory space addresses fetched from packet
                                                         output ring. (<61> of the buffer or info pointer is ADDRTYPE<1> for the writes
                                                         in this case when [DPTR]=0.) */
	uint64_t ror                          : 1;  /**< If [DPTR]=1 (DPTR Format 0), [ROR] is ADDRTYPE<0> for buffer/info write
                                                         operations to buffer/info pair MAC memory space addresses fetched from packet
                                                         output ring. ADDRTYPE<0> is the relaxed-order attribute for PCIe.
                                                         If [DPTR]=0 (DPTR Format 1), [ROR] is MACADD<60> for buffer/info write
                                                         operations to buffer/info pair MAC memory space addresses fetched from packet
                                                         output ring. (<60> of the buffer or info pointer is ADDRTYPE<0> for the writes
                                                         in this case when [DPTR]=0.) */
	uint64_t dptr                         : 1;  /**< Determines [ES,NSR,ROR] usage and the format of buffer/info pointers. When set,
                                                         buffer/info pointers are DPTR format 0. When clear, buffer/info pointers
                                                         are DPTR format 1. */
	uint64_t bmode                        : 1;  /**< Determines whether SLI_PKT()_CNTS[CNT] is a byte or packet counter. When BMODE=1,
                                                         SLI_PKT()_CNTS[CNT] is a byte counter, else SLI_PKT()_CNTS[CNT] is a packet counter. */
	uint64_t es_p                         : 2;  /**< [ES_P] is ES<1:0> for the packet output ring reads that fetch buffer/info pointer pairs
                                                         (from SLI_PKT()_SLIST_BADDR[ADDR]+). ES<1:0> is the endian-swap attribute for these
                                                         MAC memory space reads. */
	uint64_t nsr_p                        : 1;  /**< [NSR_P] is ADDRTYPE<1> for the packet output ring reads that fetch buffer/info pointer
                                                         pairs (from SLI_PKT()_SLIST_BADDR[ADDR]+). ADDRTYPE<1> is the no-snoop attribute for PCIe. */
	uint64_t ror_p                        : 1;  /**< [ROR_P] is ADDRTYPE<0> for the packet output ring reads that fetch buffer/info pointer
                                                         pairs (from SLI_PKT()_SLIST_BADDR[ADDR]+). ADDRTYPE<0> is the relaxed-order attribute
                                                         for PCIe. */
	uint64_t enb                          : 1;  /**< Enable for the output ring. Whenever SLI_PKT()_INPUT_CONTROL[RST] is set, hardware
                                                         forces [ENB] clear.  Software can only write [ENB] to 1.  [ENB] can only be cleared
                                                         only by writing SLI_PKT()_INPUT_CONTROL[RST].  Once [ENB] is cleared software can
                                                         only write [ENB] to a 1 once SLI_PKT()_INPUT_CONTROL[QUIET] is 1. */
#else
	uint64_t enb                          : 1;
	uint64_t ror_p                        : 1;
	uint64_t nsr_p                        : 1;
	uint64_t es_p                         : 2;
	uint64_t bmode                        : 1;
	uint64_t dptr                         : 1;
	uint64_t ror                          : 1;
	uint64_t nsr                          : 1;
	uint64_t es                           : 2;
	uint64_t iptr                         : 1;
	uint64_t cenb                         : 1;
	uint64_t tenb                         : 1;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_sli_pktx_output_control_s cn73xx;
	struct cvmx_sli_pktx_output_control_s cn78xx;
	struct cvmx_sli_pktx_output_control_s cn78xxp1;
	struct cvmx_sli_pktx_output_control_s cnf75xx;
};
typedef union cvmx_sli_pktx_output_control cvmx_sli_pktx_output_control_t;

/**
 * cvmx_sli_pkt#_pf_vf_mbox_sig#
 *
 * These registers are used for communication of data from the PF to the VF and vice versa.
 *
 * There are two registers per ring, SIG(0) and SIG(1). The PF and VF, both, have read and
 * write access to these registers.
 *
 * For PF-to-VF ring interrupts, SLI_PKT(0..63)_MBOX_INT[MBOX_EN] must be set.
 * When [MBOX_EN] is set, writes from the PF to byte 0 of the SIG(0) register will cause
 * an interrupt by setting [MBOX_INT] in the corresponding ring address of
 * SLI_PKT()_MBOX_INT[MBOX_INT],
 * SLI_PKT_IN_DONE()_CNTS[MBOX_INT], and SLI_PKT()_CNTS[MBOX_INT].
 *
 * For VF-to-PF ring interrupt, SLI_MAC()_PF()_INT_ENB[VF_MBOX] must be set.
 * When [VF_MBOX] is set, write from the VF to byte 0 of the SIG(1) register will cause an
 * interrupt by setting ring address VF_INT field in corresponding SLI_MAC()_PF()_MBOX_INT
 * register,
 * which may cause an interrupt to occur through PF.
 *
 * Each PF and VF can only access the rings that it owns as programmed by
 * SLI_PKT_MAC()_PF()_RINFO.
 * The signaling is ring-based. If a VF owns more than one ring, it can ignore the other
 * rings' registers if not needed.
 */
union cvmx_sli_pktx_pf_vf_mbox_sigx {
	uint64_t u64;
	struct cvmx_sli_pktx_pf_vf_mbox_sigx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Communication data from PF to VF. Writes to SLI_PKT()_PF_VF_MBOX_SIG(0)
                                                         in the corresponding VF. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_pktx_pf_vf_mbox_sigx_s cn73xx;
	struct cvmx_sli_pktx_pf_vf_mbox_sigx_s cn78xx;
	struct cvmx_sli_pktx_pf_vf_mbox_sigx_s cnf75xx;
};
typedef union cvmx_sli_pktx_pf_vf_mbox_sigx cvmx_sli_pktx_pf_vf_mbox_sigx_t;

/**
 * cvmx_sli_pkt#_slist_baddr
 *
 * This register contains the start of scatter list for output-packet pointers. This address must
 * be 16-byte aligned.
 */
union cvmx_sli_pktx_slist_baddr {
	uint64_t u64;
	struct cvmx_sli_pktx_slist_baddr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t addr                         : 60; /**< Base address for the packet output ring, containing buffer/info pointer pairs. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t addr                         : 60;
#endif
	} s;
	struct cvmx_sli_pktx_slist_baddr_s    cn61xx;
	struct cvmx_sli_pktx_slist_baddr_s    cn63xx;
	struct cvmx_sli_pktx_slist_baddr_s    cn63xxp1;
	struct cvmx_sli_pktx_slist_baddr_s    cn66xx;
	struct cvmx_sli_pktx_slist_baddr_s    cn68xx;
	struct cvmx_sli_pktx_slist_baddr_s    cn68xxp1;
	struct cvmx_sli_pktx_slist_baddr_s    cn70xx;
	struct cvmx_sli_pktx_slist_baddr_s    cn70xxp1;
	struct cvmx_sli_pktx_slist_baddr_s    cn73xx;
	struct cvmx_sli_pktx_slist_baddr_s    cn78xx;
	struct cvmx_sli_pktx_slist_baddr_s    cn78xxp1;
	struct cvmx_sli_pktx_slist_baddr_s    cnf71xx;
	struct cvmx_sli_pktx_slist_baddr_s    cnf75xx;
};
typedef union cvmx_sli_pktx_slist_baddr cvmx_sli_pktx_slist_baddr_t;

/**
 * cvmx_sli_pkt#_slist_baoff_dbell
 *
 * This register contains the doorbell and base-address offset for next read operation.
 *
 */
union cvmx_sli_pktx_slist_baoff_dbell {
	uint64_t u64;
	struct cvmx_sli_pktx_slist_baoff_dbell_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t aoff                         : 32; /**< Address offset. The offset from the SLI_PKT()_SLIST_BADDR where the next buffer/info
                                                         pointer
                                                         pair will be read.
                                                         A write of 0xFFFFFFFF to [DBELL] clears both [DBELL] and [AOFF]. */
	uint64_t dbell                        : 32; /**< Buffer/info pointer pair doorbell count. Software writes this register increment
                                                         [DBELL] by the amount written to [DBELL]. Hardware decrements [DBELL] as it
                                                         issues read operations for buffer/info pointer pairs, simultaneously also
                                                         "incrementing" [AOFF].
                                                         A write of 0xFFFFFFFF to [DBELL] clears both [DBELL] and [AOFF]. */
#else
	uint64_t dbell                        : 32;
	uint64_t aoff                         : 32;
#endif
	} s;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn61xx;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn63xx;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn63xxp1;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn66xx;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn68xx;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn68xxp1;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn70xx;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn70xxp1;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn73xx;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn78xx;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn78xxp1;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cnf71xx;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cnf75xx;
};
typedef union cvmx_sli_pktx_slist_baoff_dbell cvmx_sli_pktx_slist_baoff_dbell_t;

/**
 * cvmx_sli_pkt#_slist_fifo_rsize
 *
 * This register contains the number of scatter pointer pairs in the scatter list.
 *
 */
union cvmx_sli_pktx_slist_fifo_rsize {
	uint64_t u64;
	struct cvmx_sli_pktx_slist_fifo_rsize_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t rsize                        : 32; /**< The number of buffer/info pointer pairs in the ring.
                                                         Legal values have to be greater then 128.
                                                         Writes to [RSIZE] of less than 128 will set [RSIZE] to 128. */
#else
	uint64_t rsize                        : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cn61xx;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cn63xx;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cn63xxp1;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cn66xx;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cn68xx;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cn68xxp1;
	struct cvmx_sli_pktx_slist_fifo_rsize_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_63_32               : 32;
	uint64_t rsize                        : 32; /**< The number of scatter pointer pairs contained in
                                                         the scatter list ring. */
#else
	uint64_t rsize                        : 32;
	uint64_t reserved_63_32               : 32;
#endif
	} cn70xx;
	struct cvmx_sli_pktx_slist_fifo_rsize_cn70xx cn70xxp1;
	struct cvmx_sli_pktx_slist_fifo_rsize_cn70xx cn73xx;
	struct cvmx_sli_pktx_slist_fifo_rsize_cn70xx cn78xx;
	struct cvmx_sli_pktx_slist_fifo_rsize_cn70xx cn78xxp1;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cnf71xx;
	struct cvmx_sli_pktx_slist_fifo_rsize_cn70xx cnf75xx;
};
typedef union cvmx_sli_pktx_slist_fifo_rsize cvmx_sli_pktx_slist_fifo_rsize_t;

/**
 * cvmx_sli_pkt#_vf_int_sum
 *
 * This register contains summary interrupts bits for a VF. A VF read of this register
 * for any of its 8 rings will return the same 8-bit summary for packet input, packet
 * output and mailbox interrupts. If a PF reads this register it will return 0x0.
 */
union cvmx_sli_pktx_vf_int_sum {
	uint64_t u64;
	struct cvmx_sli_pktx_vf_int_sum_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t mbox                         : 8;  /**< Summary read-only bits of SLI_PKT()_PF_VF_MBOX_SIG()[MBOX_INT] for rings owned by this VF. */
	uint64_t reserved_24_31               : 8;
	uint64_t pkt_out                      : 8;  /**< Summary read-only bits of SLI_PKT()_CNTS[PO_INT] for rings owned by this VF. */
	uint64_t reserved_8_15                : 8;
	uint64_t pkt_in                       : 8;  /**< Summary read-only bits of SLI_PKT_IN_DONE()_CNTS[PI_INT] for rings owned by this VF. */
#else
	uint64_t pkt_in                       : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t pkt_out                      : 8;
	uint64_t reserved_24_31               : 8;
	uint64_t mbox                         : 8;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_sli_pktx_vf_int_sum_s     cn73xx;
	struct cvmx_sli_pktx_vf_int_sum_s     cn78xx;
	struct cvmx_sli_pktx_vf_int_sum_s     cnf75xx;
};
typedef union cvmx_sli_pktx_vf_int_sum cvmx_sli_pktx_vf_int_sum_t;

/**
 * cvmx_sli_pkt#_vf_sig
 *
 * This register is used to signal between PF/VF. These 64 registers are index by VF number.
 *
 */
union cvmx_sli_pktx_vf_sig {
	uint64_t u64;
	struct cvmx_sli_pktx_vf_sig_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Field can be Read or written to by PF and owning VF. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_pktx_vf_sig_s         cn78xxp1;
};
typedef union cvmx_sli_pktx_vf_sig cvmx_sli_pktx_vf_sig_t;

/**
 * cvmx_sli_pkt_bist_status
 *
 * This is the built-in self-test (BIST) status register. Each bit is the BIST result of an
 * individual memory (per bit, 0 = pass and 1 = fail).
 */
union cvmx_sli_pkt_bist_status {
	uint64_t u64;
	struct cvmx_sli_pkt_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_22_63               : 42;
	uint64_t bist                         : 22; /**< BIST results. Hardware sets a bit in BIST for memory that fails. */
#else
	uint64_t bist                         : 22;
	uint64_t reserved_22_63               : 42;
#endif
	} s;
	struct cvmx_sli_pkt_bist_status_s     cn73xx;
	struct cvmx_sli_pkt_bist_status_s     cn78xx;
	struct cvmx_sli_pkt_bist_status_s     cnf75xx;
};
typedef union cvmx_sli_pkt_bist_status cvmx_sli_pkt_bist_status_t;

/**
 * cvmx_sli_pkt_cnt_int
 *
 * This register specifies which output packet rings are interrupting because of packet counters.
 * A bit set in this interrupt register will set a corresponding bit in SLI_PKT_INT and can
 * also cause SLI_MAC()_PF()_INT_SUM[PCNT] to be set if SLI_PKT()_OUTPUT_CONTROL[CENB] is set.
 * When read by a function, this register informs which rings owned by the function (0 to N,
 * N as large as 63) have this interrupt pending.
 */
union cvmx_sli_pkt_cnt_int {
	uint64_t u64;
	struct cvmx_sli_pkt_cnt_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_sli_pkt_cnt_int_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t port                         : 32; /**< Output ring packet counter interrupt bits
                                                         SLI sets PORT<i> whenever
                                                         SLI_PKTi_CNTS[CNT] > SLI_PKT_INT_LEVELS[CNT].
                                                         SLI_PKT_CNT_INT_ENB[PORT<i>] is the corresponding
                                                         enable. */
#else
	uint64_t port                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn61xx;
	struct cvmx_sli_pkt_cnt_int_cn61xx    cn63xx;
	struct cvmx_sli_pkt_cnt_int_cn61xx    cn63xxp1;
	struct cvmx_sli_pkt_cnt_int_cn61xx    cn66xx;
	struct cvmx_sli_pkt_cnt_int_cn61xx    cn68xx;
	struct cvmx_sli_pkt_cnt_int_cn61xx    cn68xxp1;
	struct cvmx_sli_pkt_cnt_int_cn61xx    cn70xx;
	struct cvmx_sli_pkt_cnt_int_cn61xx    cn70xxp1;
	struct cvmx_sli_pkt_cnt_int_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ring                         : 64; /**< Multi-ring output ring packet counter interrupt bits. RING<i> is one
                                                         whenever SLI_PKT(i)_CNTS[CNT] > SLI_PKT(i)_INT_LEVELS[CNT].
                                                         RING<i> is the CNT component of SLI_PKT(i)_CNTS[PO_INT]
                                                         (and SLI_PKT_IN_DONE(i)_CNTS[PO_INT]), and one of the components
                                                         of SLI_PKT_INT[RING<i>]. Hardware may not update RING<i> when
                                                         software modifies SLI_PKT(i)_INT_LEVELS[CNT] - refer to the
                                                         description of SLI_PKT()_INT_LEVELS[CNT].
                                                         SLI_PKT(i)_OUTPUT_CONTROL[CENB] does not affect RING<i>. */
#else
	uint64_t ring                         : 64;
#endif
	} cn73xx;
	struct cvmx_sli_pkt_cnt_int_cn73xx    cn78xx;
	struct cvmx_sli_pkt_cnt_int_cn73xx    cn78xxp1;
	struct cvmx_sli_pkt_cnt_int_cn61xx    cnf71xx;
	struct cvmx_sli_pkt_cnt_int_cn73xx    cnf75xx;
};
typedef union cvmx_sli_pkt_cnt_int cvmx_sli_pkt_cnt_int_t;

/**
 * cvmx_sli_pkt_cnt_int_enb
 *
 * Enable for the packets rings that are interrupting because of Packet Counters.
 *
 */
union cvmx_sli_pkt_cnt_int_enb {
	uint64_t u64;
	struct cvmx_sli_pkt_cnt_int_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t port                         : 32; /**< Output ring packet counter interrupt enables
                                                         When both PORT<i> and corresponding
                                                         SLI_PKT_CNT_INT[PORT<i>] are set, for any i,
                                                         then SLI_INT_SUM[PCNT] is set, which can cause
                                                         an interrupt. */
#else
	uint64_t port                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_cnt_int_enb_s     cn61xx;
	struct cvmx_sli_pkt_cnt_int_enb_s     cn63xx;
	struct cvmx_sli_pkt_cnt_int_enb_s     cn63xxp1;
	struct cvmx_sli_pkt_cnt_int_enb_s     cn66xx;
	struct cvmx_sli_pkt_cnt_int_enb_s     cn68xx;
	struct cvmx_sli_pkt_cnt_int_enb_s     cn68xxp1;
	struct cvmx_sli_pkt_cnt_int_enb_s     cn70xx;
	struct cvmx_sli_pkt_cnt_int_enb_s     cn70xxp1;
	struct cvmx_sli_pkt_cnt_int_enb_s     cnf71xx;
};
typedef union cvmx_sli_pkt_cnt_int_enb cvmx_sli_pkt_cnt_int_enb_t;

/**
 * cvmx_sli_pkt_ctl
 *
 * Control for packets.
 *
 */
union cvmx_sli_pkt_ctl {
	uint64_t u64;
	struct cvmx_sli_pkt_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t ring_en                      : 1;  /**< When '0' forces "relative Q position" received
                                                         from PKO to be zero, and replicates the back-
                                                         pressure indication for the first ring attached
                                                         to a PKO port across all the rings attached to a
                                                         PKO port. When '1' backpressure is on a per
                                                         port/ring. */
	uint64_t pkt_bp                       : 4;  /**< When set '1' enable the port level backpressure for
                                                         PKO ports associated with the bit. */
#else
	uint64_t pkt_bp                       : 4;
	uint64_t ring_en                      : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_sli_pkt_ctl_s             cn61xx;
	struct cvmx_sli_pkt_ctl_s             cn63xx;
	struct cvmx_sli_pkt_ctl_s             cn63xxp1;
	struct cvmx_sli_pkt_ctl_s             cn66xx;
	struct cvmx_sli_pkt_ctl_s             cn68xx;
	struct cvmx_sli_pkt_ctl_s             cn68xxp1;
	struct cvmx_sli_pkt_ctl_s             cn70xx;
	struct cvmx_sli_pkt_ctl_s             cn70xxp1;
	struct cvmx_sli_pkt_ctl_s             cnf71xx;
};
typedef union cvmx_sli_pkt_ctl cvmx_sli_pkt_ctl_t;

/**
 * cvmx_sli_pkt_data_out_es
 *
 * The Endian Swap for writing Data Out.
 *
 */
union cvmx_sli_pkt_data_out_es {
	uint64_t u64;
	struct cvmx_sli_pkt_data_out_es_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t es                           : 64; /**< ES<1:0> or MACADD<63:62> for buffer/info writes.
                                                         ES<2i+1:2i> becomes either ES<1:0> or
                                                         MACADD<63:62> for writes to buffer/info pair
                                                         MAC memory space addresses fetched from packet
                                                         output ring i. ES<1:0> if SLI_PKT_DPADDR[DPTR<i>]=1
                                                         , else MACADD<63:62>.
                                                         In the latter case, ES<1:0> comes from DPTR<63:62>.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space writes. */
#else
	uint64_t es                           : 64;
#endif
	} s;
	struct cvmx_sli_pkt_data_out_es_s     cn61xx;
	struct cvmx_sli_pkt_data_out_es_s     cn63xx;
	struct cvmx_sli_pkt_data_out_es_s     cn63xxp1;
	struct cvmx_sli_pkt_data_out_es_s     cn66xx;
	struct cvmx_sli_pkt_data_out_es_s     cn68xx;
	struct cvmx_sli_pkt_data_out_es_s     cn68xxp1;
	struct cvmx_sli_pkt_data_out_es_s     cn70xx;
	struct cvmx_sli_pkt_data_out_es_s     cn70xxp1;
	struct cvmx_sli_pkt_data_out_es_s     cnf71xx;
};
typedef union cvmx_sli_pkt_data_out_es cvmx_sli_pkt_data_out_es_t;

/**
 * cvmx_sli_pkt_data_out_ns
 *
 * The NS field for the TLP when writing packet data.
 *
 */
union cvmx_sli_pkt_data_out_ns {
	uint64_t u64;
	struct cvmx_sli_pkt_data_out_ns_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t nsr                          : 32; /**< ADDRTYPE<1> or MACADD<61> for buffer/info writes.
                                                         NSR<i> becomes either ADDRTYPE<1> or MACADD<61>
                                                         for writes to buffer/info pair MAC memory space
                                                         addresses fetched from packet output ring i.
                                                         ADDRTYPE<1> if SLI_PKT_DPADDR[DPTR<i>]=1, else
                                                         MACADD<61>.
                                                         In the latter case,ADDRTYPE<1> comes from DPTR<61>.
                                                         ADDRTYPE<1> is the no-snoop attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
#else
	uint64_t nsr                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_data_out_ns_s     cn61xx;
	struct cvmx_sli_pkt_data_out_ns_s     cn63xx;
	struct cvmx_sli_pkt_data_out_ns_s     cn63xxp1;
	struct cvmx_sli_pkt_data_out_ns_s     cn66xx;
	struct cvmx_sli_pkt_data_out_ns_s     cn68xx;
	struct cvmx_sli_pkt_data_out_ns_s     cn68xxp1;
	struct cvmx_sli_pkt_data_out_ns_s     cn70xx;
	struct cvmx_sli_pkt_data_out_ns_s     cn70xxp1;
	struct cvmx_sli_pkt_data_out_ns_s     cnf71xx;
};
typedef union cvmx_sli_pkt_data_out_ns cvmx_sli_pkt_data_out_ns_t;

/**
 * cvmx_sli_pkt_data_out_ror
 *
 * The ROR field for the TLP when writing Packet Data.
 *
 */
union cvmx_sli_pkt_data_out_ror {
	uint64_t u64;
	struct cvmx_sli_pkt_data_out_ror_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t ror                          : 32; /**< ADDRTYPE<0> or MACADD<60> for buffer/info writes.
                                                         ROR<i> becomes either ADDRTYPE<0> or MACADD<60>
                                                         for writes to buffer/info pair MAC memory space
                                                         addresses fetched from packet output ring i.
                                                         ADDRTYPE<0> if SLI_PKT_DPADDR[DPTR<i>]=1, else
                                                         MACADD<60>.
                                                         In the latter case,ADDRTYPE<0> comes from DPTR<60>.
                                                         ADDRTYPE<0> is the relaxed-order attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
#else
	uint64_t ror                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_data_out_ror_s    cn61xx;
	struct cvmx_sli_pkt_data_out_ror_s    cn63xx;
	struct cvmx_sli_pkt_data_out_ror_s    cn63xxp1;
	struct cvmx_sli_pkt_data_out_ror_s    cn66xx;
	struct cvmx_sli_pkt_data_out_ror_s    cn68xx;
	struct cvmx_sli_pkt_data_out_ror_s    cn68xxp1;
	struct cvmx_sli_pkt_data_out_ror_s    cn70xx;
	struct cvmx_sli_pkt_data_out_ror_s    cn70xxp1;
	struct cvmx_sli_pkt_data_out_ror_s    cnf71xx;
};
typedef union cvmx_sli_pkt_data_out_ror cvmx_sli_pkt_data_out_ror_t;

/**
 * cvmx_sli_pkt_dpaddr
 *
 * Used to detemine address and attributes for packet data writes.
 *
 */
union cvmx_sli_pkt_dpaddr {
	uint64_t u64;
	struct cvmx_sli_pkt_dpaddr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t dptr                         : 32; /**< Determines whether buffer/info pointers are
                                                         DPTR format 0 or DPTR format 1.
                                                         When DPTR<i>=1, the buffer/info pointers fetched
                                                         from packet output ring i are DPTR format 0.
                                                         When DPTR<i>=0, the buffer/info pointers fetched
                                                         from packet output ring i are DPTR format 1.
                                                         (Replace SLI_PKT_INPUT_CONTROL[D_ESR,D_NSR,D_ROR]
                                                         in the HRM descriptions of DPTR format 0/1 with
                                                         SLI_PKT_DATA_OUT_ES[ES<2i+1:2i>],
                                                         SLI_PKT_DATA_OUT_NS[NSR<i>], and
                                                         SLI_PKT_DATA_OUT_ROR[ROR<i>], respectively,
                                                         though.) */
#else
	uint64_t dptr                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_dpaddr_s          cn61xx;
	struct cvmx_sli_pkt_dpaddr_s          cn63xx;
	struct cvmx_sli_pkt_dpaddr_s          cn63xxp1;
	struct cvmx_sli_pkt_dpaddr_s          cn66xx;
	struct cvmx_sli_pkt_dpaddr_s          cn68xx;
	struct cvmx_sli_pkt_dpaddr_s          cn68xxp1;
	struct cvmx_sli_pkt_dpaddr_s          cn70xx;
	struct cvmx_sli_pkt_dpaddr_s          cn70xxp1;
	struct cvmx_sli_pkt_dpaddr_s          cnf71xx;
};
typedef union cvmx_sli_pkt_dpaddr cvmx_sli_pkt_dpaddr_t;

/**
 * cvmx_sli_pkt_gbl_control
 *
 * This register contains control bits that affect all packet rings.
 *
 */
union cvmx_sli_pkt_gbl_control {
	uint64_t u64;
	struct cvmx_sli_pkt_gbl_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t qtime                        : 16; /**< After a packet ring is disabled on the assertion of SLI_PKT()_INPUT_CONTROL[RST],
                                                         the hardware will set SLI_PKT()_INPUT_CONTROL[QUIET]
                                                         after at least [QTIME] * 1024 cycles. */
	uint64_t reserved_14_15               : 2;
	uint64_t bpkind                       : 6;  /**< PKIND sent to PKI when DPI_PKT_INST_HDR_S[PKIND] corresponding bit in
                                                         SLI_PKT_PKIND_VALID[ENB] is cleared to a 0. */
	uint64_t reserved_4_7                 : 4;
	uint64_t pkpfval                      : 1;  /**< When 0, only VFs are subject to SLI_PKT_PKIND_VALID constraints, and PF instructions
                                                         can select any PKI PKIND.
                                                         When 1, both PFs and VFs are subject to SLI_PKT_PKIND_VALID constraints. */
	uint64_t bpflr_d                      : 1;  /**< Disables clearing SLI_PKT_OUT_BP_EN bit on an FLR. */
	uint64_t noptr_d                      : 1;  /**< Disables putting a ring into reset when a packet is received from PKO and
                                                         the associated ring has no doorbells to send the packet out. */
	uint64_t picnt_d                      : 1;  /**< Disables the subtraction of SLI_PKT_IN_DONE()_CNTS[CNT] from
                                                         SLI_PKT_IN_DONE()_CNTS[CNT] when written. */
#else
	uint64_t picnt_d                      : 1;
	uint64_t noptr_d                      : 1;
	uint64_t bpflr_d                      : 1;
	uint64_t pkpfval                      : 1;
	uint64_t reserved_4_7                 : 4;
	uint64_t bpkind                       : 6;
	uint64_t reserved_14_15               : 2;
	uint64_t qtime                        : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_gbl_control_s     cn73xx;
	struct cvmx_sli_pkt_gbl_control_s     cn78xx;
	struct cvmx_sli_pkt_gbl_control_s     cnf75xx;
};
typedef union cvmx_sli_pkt_gbl_control cvmx_sli_pkt_gbl_control_t;

/**
 * cvmx_sli_pkt_in_bp
 *
 * Which input rings have backpressure applied.
 *
 */
union cvmx_sli_pkt_in_bp {
	uint64_t u64;
	struct cvmx_sli_pkt_in_bp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t bp                           : 32; /**< A packet input  ring that has its count greater
                                                         than its WMARK will have backpressure applied.
                                                         Each of the 32 bits coorespond to an input ring.
                                                         When '1' that ring has backpressure applied an
                                                         will fetch no more instructions, but will process
                                                         any previously fetched instructions. */
#else
	uint64_t bp                           : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_in_bp_s           cn61xx;
	struct cvmx_sli_pkt_in_bp_s           cn63xx;
	struct cvmx_sli_pkt_in_bp_s           cn63xxp1;
	struct cvmx_sli_pkt_in_bp_s           cn66xx;
	struct cvmx_sli_pkt_in_bp_s           cn70xx;
	struct cvmx_sli_pkt_in_bp_s           cn70xxp1;
	struct cvmx_sli_pkt_in_bp_s           cnf71xx;
};
typedef union cvmx_sli_pkt_in_bp cvmx_sli_pkt_in_bp_t;

/**
 * cvmx_sli_pkt_in_done#_cnts
 *
 * This register contains counters for instructions completed on input rings.
 *
 */
union cvmx_sli_pkt_in_donex_cnts {
	uint64_t u64;
	struct cvmx_sli_pkt_in_donex_cnts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t po_int                       : 1;  /**< "Returns a 1 when either the corresponding bit in SLI_PKT_TIME_INT[RING[\#]] or
                                                         SLI_PKT_CNT_INT[RING[\#]] is set." */
	uint64_t pi_int                       : 1;  /**< Packet input interrupt bit for the ring. The hardware sets [PI_INT] whenever it updates
                                                         [CNT<31:0>] and is greater then [WMARK][15:0] and [CINT_ENB] is set.
                                                         The hardware will clear [PI_INT] when [CNT] is less then or equal to [WMARK][15:0]
                                                         [PI_INT] can cause an MSI-X interrupt for the ring, but will never cause an INTA/B/C/D
                                                         nor MSI interrupt nor set any SLI_MAC()_PF()_INT_SUM bit. SLI_PKT_IN_INT is a
                                                         multi-ring version of [PI_INT], and [PI_INT] is one component of SLI_PKT_INT. See also
                                                         SLI_PKT()_CNTS[PI_INT]. */
	uint64_t mbox_int                     : 1;  /**< Reads corresponding bit in SLI_PKT()_MBOX_INT. */
	uint64_t resend                       : 1;  /**< A write of 1 will resend an MSI-X interrupt message if there is a pending interrupt in
                                                         [P0_INT], [PI_INT] or [MBOX_INT] for this ring after the write of [CNT] occurs.
                                                         [RESEND] and [CNT] must be written together with the assumption that the write of
                                                         [CNT] will clear the [PI_INT] interrupt bit. If the write of [CNT] does not cause
                                                         the [CNT] to drop below the thresholds another MSI-X message is sent.
                                                         The [RESEND] bit never affects INTA/B/C/D or MSI interrupts. */
	uint64_t reserved_49_59               : 11;
	uint64_t cint_enb                     : 1;  /**< Packet input interrupt enable bit for the ring. When [CINT_ENB] is set,
                                                         the hardware sets [PI_INT] whenever it updates [CNT] and it is greater
                                                         than [WMARK].
                                                         When [CINT_ENB] is clear, the hardware will never set [PI_INT]. */
	uint64_t wmark                        : 16; /**< Packet input interrupt watermark for the ring. If [CINT_ENB] is set
                                                         and WMARK does not equal 0xFFFF, the hardware sets [PI_INT] whenever
                                                         it updates [CNT][31:0] and it is greater than [16'b0,[WMARK][15:0]]. */
	uint64_t cnt                          : 32; /**< Packet input done count for the ring. The hardware increments [CNT] by one
                                                         after it finishes reading (from the remote host) an instruction from the ring
                                                         and all of its associated packet data.
                                                         If SLI_PKT_GBL_CONTROL[PICNT_D] is not set, when [CNT] is written, it will subtract
                                                         the value from [CNT]. */
#else
	uint64_t cnt                          : 32;
	uint64_t wmark                        : 16;
	uint64_t cint_enb                     : 1;
	uint64_t reserved_49_59               : 11;
	uint64_t resend                       : 1;
	uint64_t mbox_int                     : 1;
	uint64_t pi_int                       : 1;
	uint64_t po_int                       : 1;
#endif
	} s;
	struct cvmx_sli_pkt_in_donex_cnts_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t cnt                          : 32; /**< This field is incrmented by '1' when an instruction
                                                         is completed. This field is incremented as the
                                                         last of the data is read from the MAC. */
#else
	uint64_t cnt                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn61xx;
	struct cvmx_sli_pkt_in_donex_cnts_cn61xx cn63xx;
	struct cvmx_sli_pkt_in_donex_cnts_cn61xx cn63xxp1;
	struct cvmx_sli_pkt_in_donex_cnts_cn61xx cn66xx;
	struct cvmx_sli_pkt_in_donex_cnts_cn61xx cn68xx;
	struct cvmx_sli_pkt_in_donex_cnts_cn61xx cn68xxp1;
	struct cvmx_sli_pkt_in_donex_cnts_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_63_32               : 32;
	uint64_t cnt                          : 32; /**< This field is incrmented by '1' when an instruction
                                                         is completed. This field is incremented as the
                                                         last of the data is read from the MAC. */
#else
	uint64_t cnt                          : 32;
	uint64_t reserved_63_32               : 32;
#endif
	} cn70xx;
	struct cvmx_sli_pkt_in_donex_cnts_cn70xx cn70xxp1;
	struct cvmx_sli_pkt_in_donex_cnts_s   cn73xx;
	struct cvmx_sli_pkt_in_donex_cnts_s   cn78xx;
	struct cvmx_sli_pkt_in_donex_cnts_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t po_int                       : 1;  /**< Packet output interrupt bit for the ring. A copy of SLI_PKT(i)_CNTS[PO_INT]. */
	uint64_t pi_int                       : 1;  /**< Packet input interrupt bit for the ring. The hardware sets [PI_INT] whenever it updates
                                                         [CNT<15:0>] to equal [WMARK] when CINT_ENB is set. Writing a 1 clears [PI_INT].
                                                         [PI_INT] can cause an MSI-X interrupt for the ring, but will never cause an INTA/B/C/D
                                                         nor MSI interrupt nor set any SLI_INT_SUM bit. SLI_PKT_IN_INT is a multi-ring version of
                                                         [PI_INT], and [PI_INT] is one component of SLI_PKT_INT. See also
                                                         SLI_PKT(0..63)_CNTS[PI_INT]. */
	uint64_t reserved_61_49               : 13;
	uint64_t cint_enb                     : 1;  /**< Packet input interrupt enable bit for the ring. When [CINT_ENB] is set, the hardware will
                                                         set [PI_INT] whenever it updates [CNT<15:0>] to equal [WMARK]. When [CINT_ENB]
                                                         is clear, the hardware will never set [PI_INT]. */
	uint64_t wmark                        : 16; /**< Packet input interrupt watermark for the ring. If [CINT_ENB] is set, the hardware
                                                         sets [PI_INT] whenever it updates [CNT<15:0>] to equal [WMARK]. */
	uint64_t cnt                          : 32; /**< Packet input done count for the ring. The hardware increments [CNT] by one
                                                         after it finishes reading (from the remote host) an instruction from the ring
                                                         and all of its associated packet data. */
#else
	uint64_t cnt                          : 32;
	uint64_t wmark                        : 16;
	uint64_t cint_enb                     : 1;
	uint64_t reserved_61_49               : 13;
	uint64_t pi_int                       : 1;
	uint64_t po_int                       : 1;
#endif
	} cn78xxp1;
	struct cvmx_sli_pkt_in_donex_cnts_cn61xx cnf71xx;
	struct cvmx_sli_pkt_in_donex_cnts_s   cnf75xx;
};
typedef union cvmx_sli_pkt_in_donex_cnts cvmx_sli_pkt_in_donex_cnts_t;

/**
 * cvmx_sli_pkt_in_instr_counts
 *
 * This register contains keeps track of the number of instructions read into the FIFO and
 * packets sent to PKI. This register is PF-only.
 */
union cvmx_sli_pkt_in_instr_counts {
	uint64_t u64;
	struct cvmx_sli_pkt_in_instr_counts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wr_cnt                       : 32; /**< Write count. This field shows the number of packets sent to PKI. */
	uint64_t rd_cnt                       : 32; /**< Read count. This field shows the value of instructions that have had read operations
                                                         issued for them. */
#else
	uint64_t rd_cnt                       : 32;
	uint64_t wr_cnt                       : 32;
#endif
	} s;
	struct cvmx_sli_pkt_in_instr_counts_s cn61xx;
	struct cvmx_sli_pkt_in_instr_counts_s cn63xx;
	struct cvmx_sli_pkt_in_instr_counts_s cn63xxp1;
	struct cvmx_sli_pkt_in_instr_counts_s cn66xx;
	struct cvmx_sli_pkt_in_instr_counts_s cn68xx;
	struct cvmx_sli_pkt_in_instr_counts_s cn68xxp1;
	struct cvmx_sli_pkt_in_instr_counts_s cn70xx;
	struct cvmx_sli_pkt_in_instr_counts_s cn70xxp1;
	struct cvmx_sli_pkt_in_instr_counts_s cn73xx;
	struct cvmx_sli_pkt_in_instr_counts_s cn78xx;
	struct cvmx_sli_pkt_in_instr_counts_s cn78xxp1;
	struct cvmx_sli_pkt_in_instr_counts_s cnf71xx;
	struct cvmx_sli_pkt_in_instr_counts_s cnf75xx;
};
typedef union cvmx_sli_pkt_in_instr_counts cvmx_sli_pkt_in_instr_counts_t;

/**
 * cvmx_sli_pkt_in_int
 *
 * This register specifies which input packets rings are interrupting because of done counts.
 * A bit set in this interrupt register will set a corresponding bit in SLI_PKT_INT which
 * can cause a MSI-X interrupt.  When read by a function, this register informs which rings
 * owned by the function (0 to N, N as large as 63) have this interrupt pending.
 * SLI_PKT_IN_INT conditions can cause MSI-X interrupts, but do not cause any
 * SLI_MAC()_PF()_INT_SUM
 * bit to set, and cannot cause INTA/B/C/D nor MSI interrupts.
 */
union cvmx_sli_pkt_in_int {
	uint64_t u64;
	struct cvmx_sli_pkt_in_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ring                         : 64; /**< Multi-ring packet input interrupt register. Each RING<i> is a read-only copy of
                                                         SLI_PKT_IN_DONE(i)_CNTS[PI_INT]. */
#else
	uint64_t ring                         : 64;
#endif
	} s;
	struct cvmx_sli_pkt_in_int_s          cn73xx;
	struct cvmx_sli_pkt_in_int_s          cn78xx;
	struct cvmx_sli_pkt_in_int_s          cn78xxp1;
	struct cvmx_sli_pkt_in_int_s          cnf75xx;
};
typedef union cvmx_sli_pkt_in_int cvmx_sli_pkt_in_int_t;

/**
 * cvmx_sli_pkt_in_jabber
 *
 * Register to set limit on SLI packet input packet sizes.
 *
 */
union cvmx_sli_pkt_in_jabber {
	uint64_t u64;
	struct cvmx_sli_pkt_in_jabber_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t size                         : 32; /**< Byte count for limiting sizes of packet sizes that are allowed for SLI packet inbound
                                                         packets. This byte limit does not include FSZ bytes of a packet. */
#else
	uint64_t size                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_in_jabber_s       cn73xx;
	struct cvmx_sli_pkt_in_jabber_s       cn78xx;
	struct cvmx_sli_pkt_in_jabber_s       cnf75xx;
};
typedef union cvmx_sli_pkt_in_jabber cvmx_sli_pkt_in_jabber_t;

/**
 * cvmx_sli_pkt_in_pcie_port
 *
 * Assigns Packet Input rings to MAC ports.
 *
 */
union cvmx_sli_pkt_in_pcie_port {
	uint64_t u64;
	struct cvmx_sli_pkt_in_pcie_port_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pp                           : 64; /**< The MAC port that the Packet ring number is
                                                         assigned. Two bits are used per ring (i.e. ring 0
                                                         [1:0], ring 1 [3:2], ....). A value of '0 means
                                                         that the Packetring is assign to MAC Port 0, a '1'
                                                         MAC Port 1, a '2' MAC Port 2, and a '3' MAC Port 3. */
#else
	uint64_t pp                           : 64;
#endif
	} s;
	struct cvmx_sli_pkt_in_pcie_port_s    cn61xx;
	struct cvmx_sli_pkt_in_pcie_port_s    cn63xx;
	struct cvmx_sli_pkt_in_pcie_port_s    cn63xxp1;
	struct cvmx_sli_pkt_in_pcie_port_s    cn66xx;
	struct cvmx_sli_pkt_in_pcie_port_s    cn68xx;
	struct cvmx_sli_pkt_in_pcie_port_s    cn68xxp1;
	struct cvmx_sli_pkt_in_pcie_port_s    cn70xx;
	struct cvmx_sli_pkt_in_pcie_port_s    cn70xxp1;
	struct cvmx_sli_pkt_in_pcie_port_s    cnf71xx;
};
typedef union cvmx_sli_pkt_in_pcie_port cvmx_sli_pkt_in_pcie_port_t;

/**
 * cvmx_sli_pkt_input_control
 *
 * Control for reads for gather list and instructions.
 *
 */
union cvmx_sli_pkt_input_control {
	uint64_t u64;
	struct cvmx_sli_pkt_input_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t prd_erst                     : 1;  /**< PRD Error Reset */
	uint64_t prd_rds                      : 7;  /**< PRD Reads Out */
	uint64_t gii_erst                     : 1;  /**< GII Error Reset */
	uint64_t gii_rds                      : 7;  /**< GII Reads Out */
	uint64_t reserved_41_47               : 7;
	uint64_t prc_idle                     : 1;  /**< PRC In IDLE */
	uint64_t reserved_24_39               : 16;
	uint64_t pin_rst                      : 1;  /**< Packet In Reset. When a gather-list read receives
                                                         an error this bit (along with SLI_INT_SUM[PGL_ERR])
                                                         is set. When receiveing a PGL_ERR interrupt the SW
                                                         should:
                                                         1. Wait 2ms to allow any outstanding reads to return
                                                            or be timed out.
                                                         2. Write a '0' to this bit.
                                                         3. Startup the packet input again (all previous
                                                            CSR setting of the packet-input will be lost). */
	uint64_t pkt_rr                       : 1;  /**< When set '1' the input packet selection will be
                                                         made with a Round Robin arbitration. When '0'
                                                         the input packet ring is fixed in priority,
                                                         where the lower ring number has higher priority. */
	uint64_t pbp_dhi                      : 13; /**< PBP_DHI replaces address bits that are used
                                                         for parse mode and skip-length when
                                                         SLI_PKTi_INSTR_HEADER[PBP]=1.
                                                         PBP_DHI becomes either MACADD<63:55> or MACADD<59:51>
                                                         for the instruction DPTR reads in this case.
                                                         The instruction DPTR reads are called
                                                         "First Direct" or "First Indirect" in the HRM.
                                                         When PBP=1, if "First Direct" and USE_CSR=0, PBP_DHI
                                                         becomes MACADD<59:51>, else MACADD<63:55>. */
	uint64_t d_nsr                        : 1;  /**< ADDRTYPE<1> or MACADD<61> for packet input data
                                                         reads.
                                                         D_NSR becomes either ADDRTYPE<1> or MACADD<61>
                                                         for MAC memory space reads of packet input data
                                                         fetched for any packet input ring.
                                                         ADDRTYPE<1> if USE_CSR=1, else MACADD<61>.
                                                         In the latter case, ADDRTYPE<1> comes from DPTR<61>.
                                                         ADDRTYPE<1> is the no-snoop attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
	uint64_t d_esr                        : 2;  /**< ES<1:0> or MACADD<63:62> for packet input data
                                                         reads.
                                                         D_ESR becomes either ES<1:0> or MACADD<63:62>
                                                         for MAC memory space reads of packet input data
                                                         fetched for any packet input ring.
                                                         ES<1:0> if USE_CSR=1, else MACADD<63:62>.
                                                         In the latter case, ES<1:0> comes from DPTR<63:62>.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space reads. */
	uint64_t d_ror                        : 1;  /**< ADDRTYPE<0> or MACADD<60> for packet input data
                                                         reads.
                                                         D_ROR becomes either ADDRTYPE<0> or MACADD<60>
                                                         for MAC memory space reads of packet input data
                                                         fetched for any packet input ring.
                                                         ADDRTYPE<0> if USE_CSR=1, else MACADD<60>.
                                                         In the latter case, ADDRTYPE<0> comes from DPTR<60>.
                                                         ADDRTYPE<0> is the relaxed-order attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
	uint64_t use_csr                      : 1;  /**< When set '1' the csr value will be used for
                                                         ROR, ESR, and NSR. When clear '0' the value in
                                                         DPTR will be used. In turn the bits not used for
                                                         ROR, ESR, and NSR, will be used for bits [63:60]
                                                         of the address used to fetch packet data. */
	uint64_t nsr                          : 1;  /**< ADDRTYPE<1> for packet input instruction reads and
                                                         gather list (i.e. DPI component) reads from MAC
                                                         memory space.
                                                         ADDRTYPE<1> is the no-snoop attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
	uint64_t esr                          : 2;  /**< ES<1:0> for packet input instruction reads and
                                                         gather list (i.e. DPI component) reads from MAC
                                                         memory space.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space reads. */
	uint64_t ror                          : 1;  /**< ADDRTYPE<0> for packet input instruction reads and
                                                         gather list (i.e. DPI component) reads from MAC
                                                         memory space.
                                                         ADDRTYPE<0> is the relaxed-order attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
#else
	uint64_t ror                          : 1;
	uint64_t esr                          : 2;
	uint64_t nsr                          : 1;
	uint64_t use_csr                      : 1;
	uint64_t d_ror                        : 1;
	uint64_t d_esr                        : 2;
	uint64_t d_nsr                        : 1;
	uint64_t pbp_dhi                      : 13;
	uint64_t pkt_rr                       : 1;
	uint64_t pin_rst                      : 1;
	uint64_t reserved_24_39               : 16;
	uint64_t prc_idle                     : 1;
	uint64_t reserved_41_47               : 7;
	uint64_t gii_rds                      : 7;
	uint64_t gii_erst                     : 1;
	uint64_t prd_rds                      : 7;
	uint64_t prd_erst                     : 1;
#endif
	} s;
	struct cvmx_sli_pkt_input_control_s   cn61xx;
	struct cvmx_sli_pkt_input_control_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t pkt_rr                       : 1;  /**< When set '1' the input packet selection will be
                                                         made with a Round Robin arbitration. When '0'
                                                         the input packet ring is fixed in priority,
                                                         where the lower ring number has higher priority. */
	uint64_t pbp_dhi                      : 13; /**< PBP_DHI replaces address bits that are used
                                                         for parse mode and skip-length when
                                                         SLI_PKTi_INSTR_HEADER[PBP]=1.
                                                         PBP_DHI becomes either MACADD<63:55> or MACADD<59:51>
                                                         for the instruction DPTR reads in this case.
                                                         The instruction DPTR reads are called
                                                         "First Direct" or "First Indirect" in the HRM.
                                                         When PBP=1, if "First Direct" and USE_CSR=0, PBP_DHI
                                                         becomes MACADD<59:51>, else MACADD<63:55>. */
	uint64_t d_nsr                        : 1;  /**< ADDRTYPE<1> or MACADD<61> for packet input data
                                                         reads.
                                                         D_NSR becomes either ADDRTYPE<1> or MACADD<61>
                                                         for MAC memory space reads of packet input data
                                                         fetched for any packet input ring.
                                                         ADDRTYPE<1> if USE_CSR=1, else MACADD<61>.
                                                         In the latter case, ADDRTYPE<1> comes from DPTR<61>.
                                                         ADDRTYPE<1> is the no-snoop attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
	uint64_t d_esr                        : 2;  /**< ES<1:0> or MACADD<63:62> for packet input data
                                                         reads.
                                                         D_ESR becomes either ES<1:0> or MACADD<63:62>
                                                         for MAC memory space reads of packet input data
                                                         fetched for any packet input ring.
                                                         ES<1:0> if USE_CSR=1, else MACADD<63:62>.
                                                         In the latter case, ES<1:0> comes from DPTR<63:62>.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space reads. */
	uint64_t d_ror                        : 1;  /**< ADDRTYPE<0> or MACADD<60> for packet input data
                                                         reads.
                                                         D_ROR becomes either ADDRTYPE<0> or MACADD<60>
                                                         for MAC memory space reads of packet input data
                                                         fetched for any packet input ring.
                                                         ADDRTYPE<0> if USE_CSR=1, else MACADD<60>.
                                                         In the latter case, ADDRTYPE<0> comes from DPTR<60>.
                                                         ADDRTYPE<0> is the relaxed-order attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
	uint64_t use_csr                      : 1;  /**< When set '1' the csr value will be used for
                                                         ROR, ESR, and NSR. When clear '0' the value in
                                                         DPTR will be used. In turn the bits not used for
                                                         ROR, ESR, and NSR, will be used for bits [63:60]
                                                         of the address used to fetch packet data. */
	uint64_t nsr                          : 1;  /**< ADDRTYPE<1> for packet input instruction reads and
                                                         gather list (i.e. DPI component) reads from MAC
                                                         memory space.
                                                         ADDRTYPE<1> is the no-snoop attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
	uint64_t esr                          : 2;  /**< ES<1:0> for packet input instruction reads and
                                                         gather list (i.e. DPI component) reads from MAC
                                                         memory space.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space reads. */
	uint64_t ror                          : 1;  /**< ADDRTYPE<0> for packet input instruction reads and
                                                         gather list (i.e. DPI component) reads from MAC
                                                         memory space.
                                                         ADDRTYPE<0> is the relaxed-order attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
#else
	uint64_t ror                          : 1;
	uint64_t esr                          : 2;
	uint64_t nsr                          : 1;
	uint64_t use_csr                      : 1;
	uint64_t d_ror                        : 1;
	uint64_t d_esr                        : 2;
	uint64_t d_nsr                        : 1;
	uint64_t pbp_dhi                      : 13;
	uint64_t pkt_rr                       : 1;
	uint64_t reserved_23_63               : 41;
#endif
	} cn63xx;
	struct cvmx_sli_pkt_input_control_cn63xx cn63xxp1;
	struct cvmx_sli_pkt_input_control_s   cn66xx;
	struct cvmx_sli_pkt_input_control_s   cn68xx;
	struct cvmx_sli_pkt_input_control_s   cn68xxp1;
	struct cvmx_sli_pkt_input_control_s   cn70xx;
	struct cvmx_sli_pkt_input_control_s   cn70xxp1;
	struct cvmx_sli_pkt_input_control_s   cnf71xx;
};
typedef union cvmx_sli_pkt_input_control cvmx_sli_pkt_input_control_t;

/**
 * cvmx_sli_pkt_instr_enb
 *
 * Multi-ring instruction input enable register. This register is PF-only.
 *
 */
union cvmx_sli_pkt_instr_enb {
	uint64_t u64;
	struct cvmx_sli_pkt_instr_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t enb                          : 64; /**< When ENB<i>=1, instruction input ring i is enabled. */
#else
	uint64_t enb                          : 64;
#endif
	} s;
	struct cvmx_sli_pkt_instr_enb_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t enb                          : 32; /**< When ENB<i>=1, instruction input ring i is enabled. */
#else
	uint64_t enb                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn61xx;
	struct cvmx_sli_pkt_instr_enb_cn61xx  cn63xx;
	struct cvmx_sli_pkt_instr_enb_cn61xx  cn63xxp1;
	struct cvmx_sli_pkt_instr_enb_cn61xx  cn66xx;
	struct cvmx_sli_pkt_instr_enb_cn61xx  cn68xx;
	struct cvmx_sli_pkt_instr_enb_cn61xx  cn68xxp1;
	struct cvmx_sli_pkt_instr_enb_cn61xx  cn70xx;
	struct cvmx_sli_pkt_instr_enb_cn61xx  cn70xxp1;
	struct cvmx_sli_pkt_instr_enb_s       cn78xxp1;
	struct cvmx_sli_pkt_instr_enb_cn61xx  cnf71xx;
};
typedef union cvmx_sli_pkt_instr_enb cvmx_sli_pkt_instr_enb_t;

/**
 * cvmx_sli_pkt_instr_rd_size
 *
 * The number of instruction allowed to be read at one time.
 *
 */
union cvmx_sli_pkt_instr_rd_size {
	uint64_t u64;
	struct cvmx_sli_pkt_instr_rd_size_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rdsize                       : 64; /**< Number of instructions to be read in one MAC read
                                                         request for the 4 ports - 8 rings. Every two bits
                                                         (i.e. 1:0, 3:2, 5:4..) are assign to the port/ring
                                                         combinations.
                                                         - 15:0  PKIPort0,Ring 7..0  31:16 PKIPort1,Ring 7..0
                                                         - 47:32 PKIPort2,Ring 7..0  63:48 PKIPort3,Ring 7..0
                                                         Two bit value are:
                                                         0 - 1 Instruction
                                                         1 - 2 Instructions
                                                         2 - 3 Instructions
                                                         3 - 4 Instructions */
#else
	uint64_t rdsize                       : 64;
#endif
	} s;
	struct cvmx_sli_pkt_instr_rd_size_s   cn61xx;
	struct cvmx_sli_pkt_instr_rd_size_s   cn63xx;
	struct cvmx_sli_pkt_instr_rd_size_s   cn63xxp1;
	struct cvmx_sli_pkt_instr_rd_size_s   cn66xx;
	struct cvmx_sli_pkt_instr_rd_size_s   cn68xx;
	struct cvmx_sli_pkt_instr_rd_size_s   cn68xxp1;
	struct cvmx_sli_pkt_instr_rd_size_s   cn70xx;
	struct cvmx_sli_pkt_instr_rd_size_s   cn70xxp1;
	struct cvmx_sli_pkt_instr_rd_size_s   cnf71xx;
};
typedef union cvmx_sli_pkt_instr_rd_size cvmx_sli_pkt_instr_rd_size_t;

/**
 * cvmx_sli_pkt_instr_size
 *
 * Determines if instructions are 64 or 32 byte in size for a Packet-ring.
 *
 */
union cvmx_sli_pkt_instr_size {
	uint64_t u64;
	struct cvmx_sli_pkt_instr_size_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t is_64b                       : 32; /**< When IS_64B<i>=1, instruction input ring i uses 64B
                                                         instructions, else 32B instructions. */
#else
	uint64_t is_64b                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_instr_size_s      cn61xx;
	struct cvmx_sli_pkt_instr_size_s      cn63xx;
	struct cvmx_sli_pkt_instr_size_s      cn63xxp1;
	struct cvmx_sli_pkt_instr_size_s      cn66xx;
	struct cvmx_sli_pkt_instr_size_s      cn68xx;
	struct cvmx_sli_pkt_instr_size_s      cn68xxp1;
	struct cvmx_sli_pkt_instr_size_s      cn70xx;
	struct cvmx_sli_pkt_instr_size_s      cn70xxp1;
	struct cvmx_sli_pkt_instr_size_s      cnf71xx;
};
typedef union cvmx_sli_pkt_instr_size cvmx_sli_pkt_instr_size_t;

/**
 * cvmx_sli_pkt_int
 *
 * This register combines the SLI_PKT_CNT_INT, SLI_PKT_TIME_INT or SLI_PKT_IN_INT interrupt
 * registers. When read by a function, this register informs which rings owned by the function
 * (0 to N, N as large as 63) have an interrupt pending.
 */
union cvmx_sli_pkt_int {
	uint64_t u64;
	struct cvmx_sli_pkt_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ring                         : 64; /**< Multi-ring packet interrupt register. Each RING<i> is set whenever any
                                                         of these three conditions are true:
                                                          * SLI_PKT(i)_CNTS[CNT] > SLI_PKT(i)_INT_LEVELS[CNT] (i.e. SLI_PKT_CNT_INT<i>
                                                            is set),
                                                          * Or, SLI_PKT(i)_CNTS[TIMER] > SLI_PKT(i)_INT_LEVELS[TIME] (i.e. SLI_PKT_TIME_INT<i>
                                                            is set).
                                                          * Or, SLI_PKT_IN_DONE(i)_CNTS[PI_INT] (i.e. SLI_PKT_IN_INT<i>) is set.
                                                         Any of these three conditions can cause an MSI-X interrupt, but only
                                                         the first two (i.e. SLI_PKT_CNT_INT and SLI_PKT_TIME_INT) can cause
                                                         INTA/B/C/D and MSI interrupts.
                                                         SLI_PKT(i)_OUTPUT_CONTROL[CENB,TENB] have no effect on RING<i>. */
#else
	uint64_t ring                         : 64;
#endif
	} s;
	struct cvmx_sli_pkt_int_s             cn73xx;
	struct cvmx_sli_pkt_int_s             cn78xx;
	struct cvmx_sli_pkt_int_s             cn78xxp1;
	struct cvmx_sli_pkt_int_s             cnf75xx;
};
typedef union cvmx_sli_pkt_int cvmx_sli_pkt_int_t;

/**
 * cvmx_sli_pkt_int_levels
 *
 * SLI_PKT_INT_LEVELS = SLI's Packet Interrupt Levels
 * Output packet interrupt levels.
 */
union cvmx_sli_pkt_int_levels {
	uint64_t u64;
	struct cvmx_sli_pkt_int_levels_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t time                         : 22; /**< Output ring counter time interrupt threshold
                                                         SLI sets SLI_PKT_TIME_INT[PORT<i>] whenever
                                                         SLI_PKTi_CNTS[TIMER] > TIME */
	uint64_t cnt                          : 32; /**< Output ring counter interrupt threshold
                                                         SLI sets SLI_PKT_CNT_INT[PORT<i>] whenever
                                                         SLI_PKTi_CNTS[CNT] > CNT */
#else
	uint64_t cnt                          : 32;
	uint64_t time                         : 22;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_sli_pkt_int_levels_s      cn61xx;
	struct cvmx_sli_pkt_int_levels_s      cn63xx;
	struct cvmx_sli_pkt_int_levels_s      cn63xxp1;
	struct cvmx_sli_pkt_int_levels_s      cn66xx;
	struct cvmx_sli_pkt_int_levels_s      cn68xx;
	struct cvmx_sli_pkt_int_levels_s      cn68xxp1;
	struct cvmx_sli_pkt_int_levels_s      cn70xx;
	struct cvmx_sli_pkt_int_levels_s      cn70xxp1;
	struct cvmx_sli_pkt_int_levels_s      cnf71xx;
};
typedef union cvmx_sli_pkt_int_levels cvmx_sli_pkt_int_levels_t;

/**
 * cvmx_sli_pkt_iptr
 *
 * Controls using the Info-Pointer to store length and data.
 *
 */
union cvmx_sli_pkt_iptr {
	uint64_t u64;
	struct cvmx_sli_pkt_iptr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t iptr                         : 32; /**< When IPTR<i>=1, packet output ring i is in info-
                                                         pointer mode, else buffer-pointer-only mode. */
#else
	uint64_t iptr                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_iptr_s            cn61xx;
	struct cvmx_sli_pkt_iptr_s            cn63xx;
	struct cvmx_sli_pkt_iptr_s            cn63xxp1;
	struct cvmx_sli_pkt_iptr_s            cn66xx;
	struct cvmx_sli_pkt_iptr_s            cn68xx;
	struct cvmx_sli_pkt_iptr_s            cn68xxp1;
	struct cvmx_sli_pkt_iptr_s            cn70xx;
	struct cvmx_sli_pkt_iptr_s            cn70xxp1;
	struct cvmx_sli_pkt_iptr_s            cnf71xx;
};
typedef union cvmx_sli_pkt_iptr cvmx_sli_pkt_iptr_t;

/**
 * cvmx_sli_pkt_mac#_pf#_rinfo
 *
 * This register sets the total number and starting number of rings for a given MAC and PF
 * combination. Indexed by (MAC index) SLI_PORT_E. In SR-IOV mode, SLI_PKT_MAC()_PF()_RINFO[RPVF]
 * and SLI_PKT_MAC()_PF()_RINFO[NVFS] must be non zero and determine which rings the PFs and
 * VFs own.
 *
 * An individual VF will own SLI_PKT_MAC()_PF()_RINFO[RPVF] number of rings.
 *
 * A PF will own the rings starting from ((SLI_PKT_MAC()_PF()_RINFO[SRN] +
 * (SLI_PKT_MAC()_PF()_RINFO[RPVF] * SLI_PKT_MAC()_PF()_RINFO[NVFS]))
 * to (SLI_PKT_MAC()_PF()_RINFO[SRN] + (SLI_PKT_MAC()_PF()_RINFO[TRS] -
 * 1)). SLI_PKT()_INPUT_CONTROL[PVF_NUM] must be written to values that
 * correlate with the fields in this register.
 *
 * e.g. Given:
 * _ SLI_PKT_MAC0_PF0_RINFO[SRN] = 32,
 * _ SLI_PKT_MAC0_PF0_RINFO[TRS] = 32,
 * _ SLI_PKT_MAC0_PF0_RINFO[RPVF] = 4,
 * _ SLI_PKT_MAC0_PF0_RINFO[NVFS] = 7:
 * _ rings owned by VF1: 32,33,34,35
 * _ rings owned by VF2: 36,37,38,39
 * _ rings owned by VF3: 40,41,42,43
 * _ rings owned by VF4: 44,45,46,47
 * _ rings owned by VF5: 48,49,50,51
 * _ rings owned by VF6: 52,53,54,55
 * _ rings owned by VF7: 56,57,58,59
 * _ rings owned by PF:  60,61,62,63
 */
union cvmx_sli_pkt_macx_pfx_rinfo {
	uint64_t u64;
	struct cvmx_sli_pkt_macx_pfx_rinfo_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_55_63               : 9;
	uint64_t nvfs                         : 7;  /**< The number of VFs for this PF. This field must not be zero whenever RPVF != 0.
                                                         Legal values are 0 to 64, with the requirement of (NVFS * RPVF) <= TRS. */
	uint64_t reserved_40_47               : 8;
	uint64_t rpvf                         : 8;  /**< The number of rings assigned to a VF for this PF. Legal values are 0 to 8
                                                         with the requirement of (NVFS * RPVF) <= TRS. */
	uint64_t reserved_24_31               : 8;
	uint64_t trs                          : 8;  /**< The number of rings assigned to the PF. Legal value are 0 to 64. */
	uint64_t reserved_7_15                : 9;
	uint64_t srn                          : 7;  /**< The starting ring number used by the PF. Legal value are 0 to 63. */
#else
	uint64_t srn                          : 7;
	uint64_t reserved_7_15                : 9;
	uint64_t trs                          : 8;
	uint64_t reserved_24_31               : 8;
	uint64_t rpvf                         : 8;
	uint64_t reserved_40_47               : 8;
	uint64_t nvfs                         : 7;
	uint64_t reserved_55_63               : 9;
#endif
	} s;
	struct cvmx_sli_pkt_macx_pfx_rinfo_s  cn73xx;
	struct cvmx_sli_pkt_macx_pfx_rinfo_s  cn78xx;
	struct cvmx_sli_pkt_macx_pfx_rinfo_s  cnf75xx;
};
typedef union cvmx_sli_pkt_macx_pfx_rinfo cvmx_sli_pkt_macx_pfx_rinfo_t;

/**
 * cvmx_sli_pkt_mac#_rinfo
 *
 * This register sets the total number and starting number of rings used by the MAC.
 * This register is PF-only.
 */
union cvmx_sli_pkt_macx_rinfo {
	uint64_t u64;
	struct cvmx_sli_pkt_macx_rinfo_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t rpvf                         : 8;  /**< The number of rings assigned to a VF for this MAC. Legal values are 0 to 64. */
	uint64_t reserved_24_31               : 8;
	uint64_t trs                          : 8;  /**< The number of rings assigned to the MAC. Legal value are 0 to 64. */
	uint64_t reserved_7_15                : 9;
	uint64_t srn                          : 7;  /**< The starting ring number used by the MAC. Legal value are 0 to 63. */
#else
	uint64_t srn                          : 7;
	uint64_t reserved_7_15                : 9;
	uint64_t trs                          : 8;
	uint64_t reserved_24_31               : 8;
	uint64_t rpvf                         : 8;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_sli_pkt_macx_rinfo_s      cn78xxp1;
};
typedef union cvmx_sli_pkt_macx_rinfo cvmx_sli_pkt_macx_rinfo_t;

/**
 * cvmx_sli_pkt_mac0_sig0
 *
 * This register is used to signal between PF/VF. This register can be R/W by the PF from MAC0
 * and any VF.
 */
union cvmx_sli_pkt_mac0_sig0 {
	uint64_t u64;
	struct cvmx_sli_pkt_mac0_sig0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Field can be read or written to by PF and any VF. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_pkt_mac0_sig0_s       cn78xxp1;
};
typedef union cvmx_sli_pkt_mac0_sig0 cvmx_sli_pkt_mac0_sig0_t;

/**
 * cvmx_sli_pkt_mac0_sig1
 *
 * This register is used to signal between PF/VF. This register can be R/W by the PF from MAC0
 * and any VF.
 */
union cvmx_sli_pkt_mac0_sig1 {
	uint64_t u64;
	struct cvmx_sli_pkt_mac0_sig1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Field can be read or written to by PF and any VF. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_pkt_mac0_sig1_s       cn78xxp1;
};
typedef union cvmx_sli_pkt_mac0_sig1 cvmx_sli_pkt_mac0_sig1_t;

/**
 * cvmx_sli_pkt_mac1_sig0
 *
 * This register is used to signal between PF/VF. This register can be R/W by the PF from MAC1
 * and any VF.
 */
union cvmx_sli_pkt_mac1_sig0 {
	uint64_t u64;
	struct cvmx_sli_pkt_mac1_sig0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Field can be read or written to by PF and any VF. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_pkt_mac1_sig0_s       cn78xxp1;
};
typedef union cvmx_sli_pkt_mac1_sig0 cvmx_sli_pkt_mac1_sig0_t;

/**
 * cvmx_sli_pkt_mac1_sig1
 *
 * This register is used to signal between PF/VF. This register can be R/W by the PF from MAC1
 * and any VF.
 */
union cvmx_sli_pkt_mac1_sig1 {
	uint64_t u64;
	struct cvmx_sli_pkt_mac1_sig1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Field can be read or written to by PF and any VF. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_pkt_mac1_sig1_s       cn78xxp1;
};
typedef union cvmx_sli_pkt_mac1_sig1 cvmx_sli_pkt_mac1_sig1_t;

/**
 * cvmx_sli_pkt_mem_ctl
 *
 * This register controls the ECC of the SLI packet memories.
 *
 */
union cvmx_sli_pkt_mem_ctl {
	uint64_t u64;
	struct cvmx_sli_pkt_mem_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t msix_mbox_fs                 : 2;  /**< Used to flip the synd. for pcsr_ncsr_msix_mailbox_flip_synd. */
	uint64_t msix_mbox_ecc                : 1;  /**< When set pcsr_ncsr_msix_mailbox_ecc_ena will have an ECC not generated and checked. */
	uint64_t reserved_36_44               : 9;
	uint64_t pos_fs                       : 2;  /**< Used to flip the synd. for pcsr_pout_size_csr_flip_synd. */
	uint64_t pos_ecc                      : 1;  /**< When set will have an ECC not generated and checked. */
	uint64_t pinm_fs                      : 2;  /**< Used to flip the synd. for pcsr_instr_mem_csr_flip_synd. */
	uint64_t pinm_ecc                     : 1;  /**< When set pcsr_instr_mem_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t pind_fs                      : 2;  /**< Used to flip the synd. for pcsr_in_done_csr_flip_synd. */
	uint64_t pind_ecc                     : 1;  /**< When set pcsr_in_done_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t point_fs                     : 2;  /**< Used to flip the synd. for pout_int_csr_flip_synd. */
	uint64_t point_ecc                    : 1;  /**< When set pout_int_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t slist_fs                     : 2;  /**< Used to flip the synd. for pcsr_slist_csr_flip_synd. */
	uint64_t slist_ecc                    : 1;  /**< When set pcsr_slist_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t pop1_fs                      : 2;  /**< Used to flip the synd for packet-out-pointer memory1. */
	uint64_t pop1_ecc                     : 1;  /**< When set Packet Out Pointer memory1 will have an ECC not generated and checked. */
	uint64_t pop0_fs                      : 2;  /**< Used to flip the synd for packet-out-pointer memory0. */
	uint64_t pop0_ecc                     : 1;  /**< When set packet-out-pointer memory0 will have an ECC not generated and checked. */
	uint64_t pfp_fs                       : 2;  /**< Reserved. */
	uint64_t pfp_ecc                      : 1;  /**< Reserved. */
	uint64_t pbn_fs                       : 2;  /**< Used to flip the synd for pointer-base-number memory. */
	uint64_t pbn_ecc                      : 1;  /**< When set pointer-base-number memory will have an ECC not generated and checked. */
	uint64_t pdf_fs                       : 2;  /**< Used to flip the synd for packet-data-info memory. */
	uint64_t pdf_ecc                      : 1;  /**< When set packet data memory will have an ECC not generated and checked. */
	uint64_t psf_fs                       : 2;  /**< Used to flip the synd for PSF memory. */
	uint64_t psf_ecc                      : 1;  /**< When set PSF memory will have an ECC not generated and checked. */
	uint64_t poi_fs                       : 2;  /**< Used to flip the synd for packet-out-info memory. */
	uint64_t poi_ecc                      : 1;  /**< When set packet-out-info memory will have an ECC not generated and checked. */
#else
	uint64_t poi_ecc                      : 1;
	uint64_t poi_fs                       : 2;
	uint64_t psf_ecc                      : 1;
	uint64_t psf_fs                       : 2;
	uint64_t pdf_ecc                      : 1;
	uint64_t pdf_fs                       : 2;
	uint64_t pbn_ecc                      : 1;
	uint64_t pbn_fs                       : 2;
	uint64_t pfp_ecc                      : 1;
	uint64_t pfp_fs                       : 2;
	uint64_t pop0_ecc                     : 1;
	uint64_t pop0_fs                      : 2;
	uint64_t pop1_ecc                     : 1;
	uint64_t pop1_fs                      : 2;
	uint64_t slist_ecc                    : 1;
	uint64_t slist_fs                     : 2;
	uint64_t point_ecc                    : 1;
	uint64_t point_fs                     : 2;
	uint64_t pind_ecc                     : 1;
	uint64_t pind_fs                      : 2;
	uint64_t pinm_ecc                     : 1;
	uint64_t pinm_fs                      : 2;
	uint64_t pos_ecc                      : 1;
	uint64_t pos_fs                       : 2;
	uint64_t reserved_36_44               : 9;
	uint64_t msix_mbox_ecc                : 1;
	uint64_t msix_mbox_fs                 : 2;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_sli_pkt_mem_ctl_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t msix_mbox_fs                 : 2;  /**< Used to flip the synd. for pcsr_ncsr_msix_mailbox_flip_synd. */
	uint64_t msix_mbox_ecc                : 1;  /**< When set pcsr_ncsr_msix_mailbox_ecc_ena will have an ECC not generated and checked. */
	uint64_t msix_data_fs                 : 2;  /**< Used to flip the synd. for pcsr_ncsr_msix_data_flip_synd. */
	uint64_t msix_data_ecc                : 1;  /**< When set pcsr_ncsr_msix_data_ecc_ena will have an ECC not generated and checked. */
	uint64_t msix_addr_fs                 : 2;  /**< Used to flip the synd. for pcsr_ncsr_msix_addr_flip_synd. */
	uint64_t msix_addr_ecc                : 1;  /**< When set pcsr_ncsr_msix_addr_ecc_ena will have an ECC not generated and checked. */
	uint64_t pof_fs                       : 2;  /**< Used to flip the synd for packet-out-FIFO memory. */
	uint64_t pof_ecc                      : 1;  /**< When set packet-out-FIFO memory will have an ECC not generated and checked. */
	uint64_t pos_fs                       : 2;  /**< Used to flip the synd. for pcsr_pout_size_csr_flip_synd. */
	uint64_t pos_ecc                      : 1;  /**< When set will have an ECC not generated and checked. */
	uint64_t pinm_fs                      : 2;  /**< Used to flip the synd. for pcsr_instr_mem_csr_flip_synd. */
	uint64_t pinm_ecc                     : 1;  /**< When set pcsr_instr_mem_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t pind_fs                      : 2;  /**< Used to flip the synd. for pcsr_in_done_csr_flip_synd. */
	uint64_t pind_ecc                     : 1;  /**< When set pcsr_in_done_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t point_fs                     : 2;  /**< Used to flip the synd. for pout_int_csr_flip_synd. */
	uint64_t point_ecc                    : 1;  /**< When set pout_int_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t slist_fs                     : 2;  /**< Used to flip the synd. for pcsr_slist_csr_flip_synd. */
	uint64_t slist_ecc                    : 1;  /**< When set pcsr_slist_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t pop1_fs                      : 2;  /**< Used to flip the synd for packet-out-pointer memory1. */
	uint64_t pop1_ecc                     : 1;  /**< When set Packet Out Pointer memory1 will have an ECC not generated and checked. */
	uint64_t pop0_fs                      : 2;  /**< Used to flip the synd for packet-out-pointer memory0. */
	uint64_t pop0_ecc                     : 1;  /**< When set packet-out-pointer memory0 will have an ECC not generated and checked. */
	uint64_t pfp_fs                       : 2;  /**< Reserved. */
	uint64_t pfp_ecc                      : 1;  /**< Reserved. */
	uint64_t pbn_fs                       : 2;  /**< Used to flip the synd for pointer-base-number memory. */
	uint64_t pbn_ecc                      : 1;  /**< When set pointer-base-number memory will have an ECC not generated and checked. */
	uint64_t pdf_fs                       : 2;  /**< Used to flip the synd for packet-data-info memory. */
	uint64_t pdf_ecc                      : 1;  /**< When set packet data memory will have an ECC not generated and checked. */
	uint64_t psf_fs                       : 2;  /**< Used to flip the synd for PSF memory. */
	uint64_t psf_ecc                      : 1;  /**< When set PSF memory will have an ECC not generated and checked. */
	uint64_t poi_fs                       : 2;  /**< Used to flip the synd for packet-out-info memory. */
	uint64_t poi_ecc                      : 1;  /**< When set packet-out-info memory will have an ECC not generated and checked. */
#else
	uint64_t poi_ecc                      : 1;
	uint64_t poi_fs                       : 2;
	uint64_t psf_ecc                      : 1;
	uint64_t psf_fs                       : 2;
	uint64_t pdf_ecc                      : 1;
	uint64_t pdf_fs                       : 2;
	uint64_t pbn_ecc                      : 1;
	uint64_t pbn_fs                       : 2;
	uint64_t pfp_ecc                      : 1;
	uint64_t pfp_fs                       : 2;
	uint64_t pop0_ecc                     : 1;
	uint64_t pop0_fs                      : 2;
	uint64_t pop1_ecc                     : 1;
	uint64_t pop1_fs                      : 2;
	uint64_t slist_ecc                    : 1;
	uint64_t slist_fs                     : 2;
	uint64_t point_ecc                    : 1;
	uint64_t point_fs                     : 2;
	uint64_t pind_ecc                     : 1;
	uint64_t pind_fs                      : 2;
	uint64_t pinm_ecc                     : 1;
	uint64_t pinm_fs                      : 2;
	uint64_t pos_ecc                      : 1;
	uint64_t pos_fs                       : 2;
	uint64_t pof_ecc                      : 1;
	uint64_t pof_fs                       : 2;
	uint64_t msix_addr_ecc                : 1;
	uint64_t msix_addr_fs                 : 2;
	uint64_t msix_data_ecc                : 1;
	uint64_t msix_data_fs                 : 2;
	uint64_t msix_mbox_ecc                : 1;
	uint64_t msix_mbox_fs                 : 2;
	uint64_t reserved_48_63               : 16;
#endif
	} cn73xx;
	struct cvmx_sli_pkt_mem_ctl_cn73xx    cn78xx;
	struct cvmx_sli_pkt_mem_ctl_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t msid_fs                      : 2;  /**< Used to flip the synd. for pcsr_ncsr_msix_data_flip_synd. */
	uint64_t msia_fs                      : 2;  /**< Used to flip the synd. for pcsr_ncsr_msix_addr_flip_synd. */
	uint64_t msi_ecc                      : 1;  /**< When set pcsr_ncsr_msix_ecc_enawill have an ECC not generated and checked. */
	uint64_t posi_fs                      : 2;  /**< Used to flip the synd. for pout_signal_csr_flip_synd. */
	uint64_t posi_ecc                     : 1;  /**< When set pout_signal_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t pos_fs                       : 2;  /**< Used to flip the synd. for pcsr_pout_size_csr_flip_synd. */
	uint64_t pos_ecc                      : 1;  /**< When set will have an ECC not generated and checked. */
	uint64_t pinm_fs                      : 2;  /**< Used to flip the synd. for pcsr_instr_mem_csr_flip_synd. */
	uint64_t pinm_ecc                     : 1;  /**< When set pcsr_instr_mem_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t pind_fs                      : 2;  /**< Used to flip the synd. for pcsr_in_done_csr_flip_synd. */
	uint64_t pind_ecc                     : 1;  /**< When set pcsr_in_done_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t point_fs                     : 2;  /**< Used to flip the synd. for pout_int_csr_flip_synd. */
	uint64_t point_ecc                    : 1;  /**< When set pout_int_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t slist_fs                     : 2;  /**< Used to flip the synd. for pcsr_slist_csr_flip_synd. */
	uint64_t slist_ecc                    : 1;  /**< When set pcsr_slist_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t pop1_fs                      : 2;  /**< Used to flip the synd for packet-out-pointer memory1. */
	uint64_t pop1_ecc                     : 1;  /**< When set Packet Out Pointer memory1 will have an ECC not generated and checked. */
	uint64_t pop0_fs                      : 2;  /**< Used to flip the synd for packet-out-pointer memory0. */
	uint64_t pop0_ecc                     : 1;  /**< When set packet-out-pointer memory0 will have an ECC not generated and checked. */
	uint64_t pfp_fs                       : 2;  /**< Reserved. */
	uint64_t pfp_ecc                      : 1;  /**< Reserved. */
	uint64_t pbn_fs                       : 2;  /**< Used to flip the synd for pointer-base-number memory. */
	uint64_t pbn_ecc                      : 1;  /**< When set pointer-base-number memory will have an ECC not generated and checked. */
	uint64_t pdf_fs                       : 2;  /**< Used to flip the synd for packet-data-info memory. */
	uint64_t pdf_ecc                      : 1;  /**< When set packet data memory will have an ECC not generated and checked. */
	uint64_t psf_fs                       : 2;  /**< Used to flip the synd for PSF memory. */
	uint64_t psf_ecc                      : 1;  /**< When set PSF memory will have an ECC not generated and checked. */
	uint64_t poi_fs                       : 2;  /**< Used to flip the synd for packet-out-info memory. */
	uint64_t poi_ecc                      : 1;  /**< When set packet-out-info memory will have an ECC not generated and checked. */
#else
	uint64_t poi_ecc                      : 1;
	uint64_t poi_fs                       : 2;
	uint64_t psf_ecc                      : 1;
	uint64_t psf_fs                       : 2;
	uint64_t pdf_ecc                      : 1;
	uint64_t pdf_fs                       : 2;
	uint64_t pbn_ecc                      : 1;
	uint64_t pbn_fs                       : 2;
	uint64_t pfp_ecc                      : 1;
	uint64_t pfp_fs                       : 2;
	uint64_t pop0_ecc                     : 1;
	uint64_t pop0_fs                      : 2;
	uint64_t pop1_ecc                     : 1;
	uint64_t pop1_fs                      : 2;
	uint64_t slist_ecc                    : 1;
	uint64_t slist_fs                     : 2;
	uint64_t point_ecc                    : 1;
	uint64_t point_fs                     : 2;
	uint64_t pind_ecc                     : 1;
	uint64_t pind_fs                      : 2;
	uint64_t pinm_ecc                     : 1;
	uint64_t pinm_fs                      : 2;
	uint64_t pos_ecc                      : 1;
	uint64_t pos_fs                       : 2;
	uint64_t posi_ecc                     : 1;
	uint64_t posi_fs                      : 2;
	uint64_t msi_ecc                      : 1;
	uint64_t msia_fs                      : 2;
	uint64_t msid_fs                      : 2;
	uint64_t reserved_44_63               : 20;
#endif
	} cn78xxp1;
	struct cvmx_sli_pkt_mem_ctl_cn73xx    cnf75xx;
};
typedef union cvmx_sli_pkt_mem_ctl cvmx_sli_pkt_mem_ctl_t;

/**
 * cvmx_sli_pkt_out_bmode
 *
 * Control the updating of the SLI_PKT#_CNT register.
 *
 */
union cvmx_sli_pkt_out_bmode {
	uint64_t u64;
	struct cvmx_sli_pkt_out_bmode_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t bmode                        : 32; /**< Determines whether SLI_PKTi_CNTS[CNT] is a byte or
                                                         packet counter.
                                                         When BMODE<i>=1, SLI_PKTi_CNTS[CNT] is a byte
                                                         counter, else SLI_PKTi_CNTS[CNT] is a packet
                                                         counter. */
#else
	uint64_t bmode                        : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_out_bmode_s       cn61xx;
	struct cvmx_sli_pkt_out_bmode_s       cn63xx;
	struct cvmx_sli_pkt_out_bmode_s       cn63xxp1;
	struct cvmx_sli_pkt_out_bmode_s       cn66xx;
	struct cvmx_sli_pkt_out_bmode_s       cn68xx;
	struct cvmx_sli_pkt_out_bmode_s       cn68xxp1;
	struct cvmx_sli_pkt_out_bmode_s       cn70xx;
	struct cvmx_sli_pkt_out_bmode_s       cn70xxp1;
	struct cvmx_sli_pkt_out_bmode_s       cnf71xx;
};
typedef union cvmx_sli_pkt_out_bmode cvmx_sli_pkt_out_bmode_t;

/**
 * cvmx_sli_pkt_out_bp_en
 *
 * This register enables sending backpressure to PKO.
 *
 */
union cvmx_sli_pkt_out_bp_en {
	uint64_t u64;
	struct cvmx_sli_pkt_out_bp_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bp_en                        : 64; /**< When set, enable the channel-level backpressure to be sent to PKO. Backpressure is sent to
                                                         the PKO on the channels 0x100-0x13F. See SLI_PKT_OUTPUT_WMARK[WMARK]. */
#else
	uint64_t bp_en                        : 64;
#endif
	} s;
	struct cvmx_sli_pkt_out_bp_en_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t bp_en                        : 32; /**< When set '1' enable the ring level backpressure
                                                         to be sent to PKO. Backpressure is sent to the
                                                         PKO on the PIPE number associated with the ring.
                                                         (See SLI_TX_PIPE for ring to pipe associations). */
#else
	uint64_t bp_en                        : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn68xx;
	struct cvmx_sli_pkt_out_bp_en_cn68xx  cn68xxp1;
	struct cvmx_sli_pkt_out_bp_en_s       cn78xxp1;
};
typedef union cvmx_sli_pkt_out_bp_en cvmx_sli_pkt_out_bp_en_t;

/**
 * cvmx_sli_pkt_out_bp_en2_w1c
 *
 * This register disables sending backpressure to PKO.
 *
 */
union cvmx_sli_pkt_out_bp_en2_w1c {
	uint64_t u64;
	struct cvmx_sli_pkt_out_bp_en2_w1c_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t w1c                          : 64; /**< When set, disables the channel-level backpressure to be sent to PKO. Backpressure is sent
                                                         to the PKO on the channels 0x140-0x17F. See SLI_PKT_OUTPUT_WMARK[WMARK].
                                                         A read of this register will return the current value of the enables for those channels. */
#else
	uint64_t w1c                          : 64;
#endif
	} s;
	struct cvmx_sli_pkt_out_bp_en2_w1c_s  cn73xx;
};
typedef union cvmx_sli_pkt_out_bp_en2_w1c cvmx_sli_pkt_out_bp_en2_w1c_t;

/**
 * cvmx_sli_pkt_out_bp_en2_w1s
 *
 * This register enables sending backpressure to PKO.
 *
 */
union cvmx_sli_pkt_out_bp_en2_w1s {
	uint64_t u64;
	struct cvmx_sli_pkt_out_bp_en2_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t w1s                          : 64; /**< When set, enables the channel-level backpressure to be sent to PKO. Backpressure is sent
                                                         to the PKO on the channels 0x140-0x17F. See SLI_PKT_OUTPUT_WMARK[WMARK].
                                                         A read of this register will return the current value of the enables for those channels. */
#else
	uint64_t w1s                          : 64;
#endif
	} s;
	struct cvmx_sli_pkt_out_bp_en2_w1s_s  cn73xx;
};
typedef union cvmx_sli_pkt_out_bp_en2_w1s cvmx_sli_pkt_out_bp_en2_w1s_t;

/**
 * cvmx_sli_pkt_out_bp_en_w1c
 *
 * This register disables sending backpressure to PKO.
 *
 */
union cvmx_sli_pkt_out_bp_en_w1c {
	uint64_t u64;
	struct cvmx_sli_pkt_out_bp_en_w1c_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t w1c                          : 64; /**< When set, disables the channel-level backpressure to be sent to PKO. Backpressure is sent
                                                         to the PKO on the channels 0x100-0x13F. See SLI_PKT_OUTPUT_WMARK[WMARK].
                                                         A read of this register will return the current value of the enables for those channels. */
#else
	uint64_t w1c                          : 64;
#endif
	} s;
	struct cvmx_sli_pkt_out_bp_en_w1c_s   cn73xx;
	struct cvmx_sli_pkt_out_bp_en_w1c_s   cn78xx;
	struct cvmx_sli_pkt_out_bp_en_w1c_s   cnf75xx;
};
typedef union cvmx_sli_pkt_out_bp_en_w1c cvmx_sli_pkt_out_bp_en_w1c_t;

/**
 * cvmx_sli_pkt_out_bp_en_w1s
 *
 * This register enables sending backpressure to PKO.
 *
 */
union cvmx_sli_pkt_out_bp_en_w1s {
	uint64_t u64;
	struct cvmx_sli_pkt_out_bp_en_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t w1s                          : 64; /**< When set, enables the channel-level backpressure to be sent to PKO. Backpressure is sent
                                                         to the PKO on the channels 0x100-0x13F. See SLI_PKT_OUTPUT_WMARK[WMARK].
                                                         A read of this register will return the current value of the enables for those channels. */
#else
	uint64_t w1s                          : 64;
#endif
	} s;
	struct cvmx_sli_pkt_out_bp_en_w1s_s   cn73xx;
	struct cvmx_sli_pkt_out_bp_en_w1s_s   cn78xx;
	struct cvmx_sli_pkt_out_bp_en_w1s_s   cnf75xx;
};
typedef union cvmx_sli_pkt_out_bp_en_w1s cvmx_sli_pkt_out_bp_en_w1s_t;

/**
 * cvmx_sli_pkt_out_enb
 *
 * Multi-ring packet output enable register. This register is PF-only.
 *
 */
union cvmx_sli_pkt_out_enb {
	uint64_t u64;
	struct cvmx_sli_pkt_out_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t enb                          : 64; /**< When ENB<i>=1, packet output ring i is enabled.
                                                         If an error occurs on reading pointers for an
                                                         output ring, the ring will be disabled by clearing
                                                         the bit associated with the ring to '0'. */
#else
	uint64_t enb                          : 64;
#endif
	} s;
	struct cvmx_sli_pkt_out_enb_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t enb                          : 32; /**< When ENB<i>=1, packet output ring i is enabled.
                                                         If an error occurs on reading pointers for an
                                                         output ring, the ring will be disabled by clearing
                                                         the bit associated with the ring to '0'. */
#else
	uint64_t enb                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn61xx;
	struct cvmx_sli_pkt_out_enb_cn61xx    cn63xx;
	struct cvmx_sli_pkt_out_enb_cn61xx    cn63xxp1;
	struct cvmx_sli_pkt_out_enb_cn61xx    cn66xx;
	struct cvmx_sli_pkt_out_enb_cn61xx    cn68xx;
	struct cvmx_sli_pkt_out_enb_cn61xx    cn68xxp1;
	struct cvmx_sli_pkt_out_enb_cn61xx    cn70xx;
	struct cvmx_sli_pkt_out_enb_cn61xx    cn70xxp1;
	struct cvmx_sli_pkt_out_enb_s         cn78xxp1;
	struct cvmx_sli_pkt_out_enb_cn61xx    cnf71xx;
};
typedef union cvmx_sli_pkt_out_enb cvmx_sli_pkt_out_enb_t;

/**
 * cvmx_sli_pkt_output_wmark
 *
 * This register sets the value that determines when backpressure is applied to the PKO. When
 * SLI_PKT()_SLIST_BAOFF_DBELL[DBELL] is less than [WMARK], backpressure is sent to PKO for
 * the associated channel. This register is PF-only.
 */
union cvmx_sli_pkt_output_wmark {
	uint64_t u64;
	struct cvmx_sli_pkt_output_wmark_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t wmark                        : 32; /**< Packet output backpressure watermark. When SLI_PKT_OUT_BP_EN<i> is set and
                                                         SLI_PKT(i)_SLIST_BAOFF_DBELL[DBELL] drops below this value, PKO receives backpressure
                                                         for channel/ring i. */
#else
	uint64_t wmark                        : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_output_wmark_s    cn61xx;
	struct cvmx_sli_pkt_output_wmark_s    cn63xx;
	struct cvmx_sli_pkt_output_wmark_s    cn63xxp1;
	struct cvmx_sli_pkt_output_wmark_s    cn66xx;
	struct cvmx_sli_pkt_output_wmark_s    cn68xx;
	struct cvmx_sli_pkt_output_wmark_s    cn68xxp1;
	struct cvmx_sli_pkt_output_wmark_s    cn70xx;
	struct cvmx_sli_pkt_output_wmark_s    cn70xxp1;
	struct cvmx_sli_pkt_output_wmark_s    cn73xx;
	struct cvmx_sli_pkt_output_wmark_s    cn78xx;
	struct cvmx_sli_pkt_output_wmark_s    cn78xxp1;
	struct cvmx_sli_pkt_output_wmark_s    cnf71xx;
	struct cvmx_sli_pkt_output_wmark_s    cnf75xx;
};
typedef union cvmx_sli_pkt_output_wmark cvmx_sli_pkt_output_wmark_t;

/**
 * cvmx_sli_pkt_pcie_port
 *
 * Assigns Packet Ports to MAC ports.
 *
 */
union cvmx_sli_pkt_pcie_port {
	uint64_t u64;
	struct cvmx_sli_pkt_pcie_port_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pp                           : 64; /**< The physical MAC  port that the output ring uses.
                                                         Two bits are used per ring (i.e. ring 0 [1:0],
                                                         ring 1 [3:2], ....). A value of '0 means
                                                         that the Packetring is assign to MAC Port 0, a '1'
                                                         MAC Port 1, '2' and '3' are reserved. */
#else
	uint64_t pp                           : 64;
#endif
	} s;
	struct cvmx_sli_pkt_pcie_port_s       cn61xx;
	struct cvmx_sli_pkt_pcie_port_s       cn63xx;
	struct cvmx_sli_pkt_pcie_port_s       cn63xxp1;
	struct cvmx_sli_pkt_pcie_port_s       cn66xx;
	struct cvmx_sli_pkt_pcie_port_s       cn68xx;
	struct cvmx_sli_pkt_pcie_port_s       cn68xxp1;
	struct cvmx_sli_pkt_pcie_port_s       cn70xx;
	struct cvmx_sli_pkt_pcie_port_s       cn70xxp1;
	struct cvmx_sli_pkt_pcie_port_s       cnf71xx;
};
typedef union cvmx_sli_pkt_pcie_port cvmx_sli_pkt_pcie_port_t;

/**
 * cvmx_sli_pkt_pkind_valid
 *
 * Enables bits per PKIND that are allowed to be sent to PKI specified in the
 * DPI_PKT_INST_HDR_S[PKIND] DPI packet instruction field.
 */
union cvmx_sli_pkt_pkind_valid {
	uint64_t u64;
	struct cvmx_sli_pkt_pkind_valid_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t enb                          : 64; /**< Enables bits for 64 possible pkinds. If set to a 1, the corresponding
                                                         DPI_PKT_INST_HDR_S[PKIND] is allowed to be passed to PKI. If cleared to a 0,
                                                         the DPI_PKT_INST_HDR_S[PKIND] will be changed to the pkind set in
                                                         SLI_PKT_GBL_CONTROL[BPKIND] when sent to PKI. */
#else
	uint64_t enb                          : 64;
#endif
	} s;
	struct cvmx_sli_pkt_pkind_valid_s     cn73xx;
	struct cvmx_sli_pkt_pkind_valid_s     cn78xx;
	struct cvmx_sli_pkt_pkind_valid_s     cnf75xx;
};
typedef union cvmx_sli_pkt_pkind_valid cvmx_sli_pkt_pkind_valid_t;

/**
 * cvmx_sli_pkt_port_in_rst
 *
 * SLI_PKT_PORT_IN_RST = SLI Packet Port In Reset
 * Vector bits related to ring-port for ones that are reset.
 */
union cvmx_sli_pkt_port_in_rst {
	uint64_t u64;
	struct cvmx_sli_pkt_port_in_rst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t in_rst                       : 32; /**< When asserted '1' the vector bit cooresponding
                                                         to the inbound Packet-ring is in reset. */
	uint64_t out_rst                      : 32; /**< When asserted '1' the vector bit cooresponding
                                                         to the outbound Packet-ring is in reset. */
#else
	uint64_t out_rst                      : 32;
	uint64_t in_rst                       : 32;
#endif
	} s;
	struct cvmx_sli_pkt_port_in_rst_s     cn61xx;
	struct cvmx_sli_pkt_port_in_rst_s     cn63xx;
	struct cvmx_sli_pkt_port_in_rst_s     cn63xxp1;
	struct cvmx_sli_pkt_port_in_rst_s     cn66xx;
	struct cvmx_sli_pkt_port_in_rst_s     cn68xx;
	struct cvmx_sli_pkt_port_in_rst_s     cn68xxp1;
	struct cvmx_sli_pkt_port_in_rst_s     cn70xx;
	struct cvmx_sli_pkt_port_in_rst_s     cn70xxp1;
	struct cvmx_sli_pkt_port_in_rst_s     cnf71xx;
};
typedef union cvmx_sli_pkt_port_in_rst cvmx_sli_pkt_port_in_rst_t;

/**
 * cvmx_sli_pkt_ring_rst
 *
 * When read by a PF, this register informs which rings owned by the function (0 to N, N as large
 * as 63) are in reset. See also SLI_PKT()_INPUT_CONTROL[RST].
 */
union cvmx_sli_pkt_ring_rst {
	uint64_t u64;
	struct cvmx_sli_pkt_ring_rst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 64; /**< RST<i> is a read-only copy of SLI_PKT(i)_INPUT_CONTROL[RST]. */
#else
	uint64_t rst                          : 64;
#endif
	} s;
	struct cvmx_sli_pkt_ring_rst_s        cn73xx;
	struct cvmx_sli_pkt_ring_rst_s        cn78xx;
	struct cvmx_sli_pkt_ring_rst_s        cn78xxp1;
	struct cvmx_sli_pkt_ring_rst_s        cnf75xx;
};
typedef union cvmx_sli_pkt_ring_rst cvmx_sli_pkt_ring_rst_t;

/**
 * cvmx_sli_pkt_slist_es
 *
 * The Endian Swap for Scatter List Read.
 *
 */
union cvmx_sli_pkt_slist_es {
	uint64_t u64;
	struct cvmx_sli_pkt_slist_es_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t es                           : 64; /**< ES<1:0> for the packet output ring reads that
                                                         fetch buffer/info pointer pairs.
                                                         ES<2i+1:2i> becomes ES<1:0> in DPI/SLI reads that
                                                         fetch buffer/info pairs from packet output ring i
                                                         (from address SLI_PKTi_SLIST_BADDR+ in MAC memory
                                                         space.)
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space reads. */
#else
	uint64_t es                           : 64;
#endif
	} s;
	struct cvmx_sli_pkt_slist_es_s        cn61xx;
	struct cvmx_sli_pkt_slist_es_s        cn63xx;
	struct cvmx_sli_pkt_slist_es_s        cn63xxp1;
	struct cvmx_sli_pkt_slist_es_s        cn66xx;
	struct cvmx_sli_pkt_slist_es_s        cn68xx;
	struct cvmx_sli_pkt_slist_es_s        cn68xxp1;
	struct cvmx_sli_pkt_slist_es_s        cn70xx;
	struct cvmx_sli_pkt_slist_es_s        cn70xxp1;
	struct cvmx_sli_pkt_slist_es_s        cnf71xx;
};
typedef union cvmx_sli_pkt_slist_es cvmx_sli_pkt_slist_es_t;

/**
 * cvmx_sli_pkt_slist_ns
 *
 * The NS field for the TLP when fetching Scatter List.
 *
 */
union cvmx_sli_pkt_slist_ns {
	uint64_t u64;
	struct cvmx_sli_pkt_slist_ns_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t nsr                          : 32; /**< ADDRTYPE<1> for the packet output ring reads that
                                                         fetch buffer/info pointer pairs.
                                                         NSR<i> becomes ADDRTYPE<1> in DPI/SLI reads that
                                                         fetch buffer/info pairs from packet output ring i
                                                         (from address SLI_PKTi_SLIST_BADDR+ in MAC memory
                                                         space.)
                                                         ADDRTYPE<1> is the relaxed-order attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
#else
	uint64_t nsr                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_slist_ns_s        cn61xx;
	struct cvmx_sli_pkt_slist_ns_s        cn63xx;
	struct cvmx_sli_pkt_slist_ns_s        cn63xxp1;
	struct cvmx_sli_pkt_slist_ns_s        cn66xx;
	struct cvmx_sli_pkt_slist_ns_s        cn68xx;
	struct cvmx_sli_pkt_slist_ns_s        cn68xxp1;
	struct cvmx_sli_pkt_slist_ns_s        cn70xx;
	struct cvmx_sli_pkt_slist_ns_s        cn70xxp1;
	struct cvmx_sli_pkt_slist_ns_s        cnf71xx;
};
typedef union cvmx_sli_pkt_slist_ns cvmx_sli_pkt_slist_ns_t;

/**
 * cvmx_sli_pkt_slist_ror
 *
 * The ROR field for the TLP when fetching Scatter List.
 *
 */
union cvmx_sli_pkt_slist_ror {
	uint64_t u64;
	struct cvmx_sli_pkt_slist_ror_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t ror                          : 32; /**< ADDRTYPE<0> for the packet output ring reads that
                                                         fetch buffer/info pointer pairs.
                                                         ROR<i> becomes ADDRTYPE<0> in DPI/SLI reads that
                                                         fetch buffer/info pairs from packet output ring i
                                                         (from address SLI_PKTi_SLIST_BADDR+ in MAC memory
                                                         space.)
                                                         ADDRTYPE<0> is the relaxed-order attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
#else
	uint64_t ror                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_slist_ror_s       cn61xx;
	struct cvmx_sli_pkt_slist_ror_s       cn63xx;
	struct cvmx_sli_pkt_slist_ror_s       cn63xxp1;
	struct cvmx_sli_pkt_slist_ror_s       cn66xx;
	struct cvmx_sli_pkt_slist_ror_s       cn68xx;
	struct cvmx_sli_pkt_slist_ror_s       cn68xxp1;
	struct cvmx_sli_pkt_slist_ror_s       cn70xx;
	struct cvmx_sli_pkt_slist_ror_s       cn70xxp1;
	struct cvmx_sli_pkt_slist_ror_s       cnf71xx;
};
typedef union cvmx_sli_pkt_slist_ror cvmx_sli_pkt_slist_ror_t;

/**
 * cvmx_sli_pkt_time_int
 *
 * This register specifies which output packets rings are interrupting because of packet timers.
 * A bit set in this interrupt register will set a corresponding bit in SLI_PKT_INT and can
 * also cause SLI_MAC()_PF()_INT_SUM[PTIME] to be set if
 * SLI_PKT()_OUTPUT_CONTROL[TENB]
 * is set. When read by a function, this register informs which rings owned by the function (0 to
 * N,
 * N as large as 63) have this interrupt pending.
 */
union cvmx_sli_pkt_time_int {
	uint64_t u64;
	struct cvmx_sli_pkt_time_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_sli_pkt_time_int_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t port                         : 32; /**< Output ring packet timer interrupt bits
                                                         SLI sets PORT<i> whenever
                                                         SLI_PKTi_CNTS[TIMER] > SLI_PKT_INT_LEVELS[TIME].
                                                         SLI_PKT_TIME_INT_ENB[PORT<i>] is the corresponding
                                                         enable. */
#else
	uint64_t port                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn61xx;
	struct cvmx_sli_pkt_time_int_cn61xx   cn63xx;
	struct cvmx_sli_pkt_time_int_cn61xx   cn63xxp1;
	struct cvmx_sli_pkt_time_int_cn61xx   cn66xx;
	struct cvmx_sli_pkt_time_int_cn61xx   cn68xx;
	struct cvmx_sli_pkt_time_int_cn61xx   cn68xxp1;
	struct cvmx_sli_pkt_time_int_cn61xx   cn70xx;
	struct cvmx_sli_pkt_time_int_cn61xx   cn70xxp1;
	struct cvmx_sli_pkt_time_int_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ring                         : 64; /**< Multi-ring output ring packet time interrupt bits. RING<i> reads as one
                                                         whenever SLI_PKT(i)_CNTS[TIMER] > SLI_PKT(i)_INT_LEVELS[TIME].
                                                         RING<i> is the TIMER component of SLI_PKT(i)_CNTS[PO_INT]
                                                         (and SLI_PKT_IN_DONE(i)_CNTS[PO_INT]), and one of the components
                                                         of SLI_PKT_INT[RING<i>]. Hardware may not update RING<i> when
                                                         software modifies SLI_PKT(i)_INT_LEVELS[TIME] - refer to the
                                                         description of SLI_PKT()_INT_LEVELS[TIME].
                                                         SLI_PKT(i)_OUTPUT_CONTROL[TENB] does not affect RING<i>. */
#else
	uint64_t ring                         : 64;
#endif
	} cn73xx;
	struct cvmx_sli_pkt_time_int_cn73xx   cn78xx;
	struct cvmx_sli_pkt_time_int_cn73xx   cn78xxp1;
	struct cvmx_sli_pkt_time_int_cn61xx   cnf71xx;
	struct cvmx_sli_pkt_time_int_cn73xx   cnf75xx;
};
typedef union cvmx_sli_pkt_time_int cvmx_sli_pkt_time_int_t;

/**
 * cvmx_sli_pkt_time_int_enb
 *
 * The packets rings that are interrupting because of Packet Timers.
 *
 */
union cvmx_sli_pkt_time_int_enb {
	uint64_t u64;
	struct cvmx_sli_pkt_time_int_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t port                         : 32; /**< Output ring packet timer interrupt enables
                                                         When both PORT<i> and corresponding
                                                         SLI_PKT_TIME_INT[PORT<i>] are set, for any i,
                                                         then SLI_INT_SUM[PTIME] is set, which can cause
                                                         an interrupt. */
#else
	uint64_t port                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_time_int_enb_s    cn61xx;
	struct cvmx_sli_pkt_time_int_enb_s    cn63xx;
	struct cvmx_sli_pkt_time_int_enb_s    cn63xxp1;
	struct cvmx_sli_pkt_time_int_enb_s    cn66xx;
	struct cvmx_sli_pkt_time_int_enb_s    cn68xx;
	struct cvmx_sli_pkt_time_int_enb_s    cn68xxp1;
	struct cvmx_sli_pkt_time_int_enb_s    cn70xx;
	struct cvmx_sli_pkt_time_int_enb_s    cn70xxp1;
	struct cvmx_sli_pkt_time_int_enb_s    cnf71xx;
};
typedef union cvmx_sli_pkt_time_int_enb cvmx_sli_pkt_time_int_enb_t;

/**
 * cvmx_sli_port#_pkind
 *
 * SLI_PORT[0..31]_PKIND = SLI Port Pkind
 *
 * The SLI/DPI supports 32 input rings for fetching input packets. This register maps the input-rings (0-31) to a PKIND.
 */
union cvmx_sli_portx_pkind {
	uint64_t u64;
	struct cvmx_sli_portx_pkind_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t rpk_enb                      : 1;  /**< Alternate PKT_INST_HDR PKind Enable for this ring.
                                                         When RPK_ENB==1 and DPI prepends
                                                         a PKT_INST_HDR to a packet, the pkind for the
                                                         packet is PKINDR (rather than PKIND), and any
                                                         special PIP/IPD processing of the DPI packet is
                                                         disabled (see PIP_PRT_CFG*[INST_HDR,HIGIG_EN]).
                                                         (DPI prepends a PKT_INST_HDR when either
                                                         DPI_INST_HDR[R]==1 for the packet or
                                                         SLI_PKT*_INSTR_HEADER[USE_IHDR]==1 for the ring.)
                                                         When RPK_ENB==0, PKIND is the pkind for all
                                                         packets through the input ring, and
                                                         PIP/IPD will process a DPI packet that has a
                                                         PKT_INST_HDR specially. */
	uint64_t reserved_22_23               : 2;
	uint64_t pkindr                       : 6;  /**< Port Kind For this Ring used with packets
                                                         that include a DPI-prepended PKT_INST_HDR
                                                         when RPK_ENB is set. */
	uint64_t reserved_14_15               : 2;
	uint64_t bpkind                       : 6;  /**< Back-pressure pkind for this Ring. */
	uint64_t reserved_6_7                 : 2;
	uint64_t pkind                        : 6;  /**< Port Kind For this Ring. */
#else
	uint64_t pkind                        : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t bpkind                       : 6;
	uint64_t reserved_14_15               : 2;
	uint64_t pkindr                       : 6;
	uint64_t reserved_22_23               : 2;
	uint64_t rpk_enb                      : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_sli_portx_pkind_s         cn68xx;
	struct cvmx_sli_portx_pkind_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t bpkind                       : 6;  /**< Back-pressure pkind for this Ring. */
	uint64_t reserved_6_7                 : 2;
	uint64_t pkind                        : 6;  /**< Port Kind For this Ring. */
#else
	uint64_t pkind                        : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t bpkind                       : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} cn68xxp1;
};
typedef union cvmx_sli_portx_pkind cvmx_sli_portx_pkind_t;

/**
 * cvmx_sli_pp_pkt_csr_control
 *
 * This register provides access to SLI packet register space from the cores.
 * These SLI packet registers include the following:
 *  SLI_MSIXX_TABLE_ADDR,
 *  SLI_MSIXX_TABLE_DATA,
 *  SLI_MSIX_PBA0,
 *  SLI_MSIX_PBA1,
 *  SLI_PKTX_INPUT_CONTROL,
 *  SLI_PKTX_INSTR_BADDR,
 *  SLI_PKTX_INSTR_BAOFF_DBELL,
 *  SLI_PKTX_INSTR_FIFO_RSIZE,
 *  SLI_PKT_IN_DONEX_CNTS,
 *  SLI_PKTX_OUTPUT_CONTROL,
 *  SLI_PKTX_OUT_SIZE,
 *  SLI_PKTX_SLIST_BADDR,
 *  SLI_PKTX_SLIST_BAOFF_DBELL,
 *  SLI_PKTX_SLIST_FIFO_RSIZE,
 *  SLI_PKTX_INT_LEVELS,
 *  SLI_PKTX_CNTS,
 *  SLI_PKTX_ERROR_INFO,
 *  SLI_PKTX_VF_INT_SUM,
 *  SLI_PKTX_PF_VF_MBOX_SIG,
 *  SLI_PKTX_MBOX_INT.
 */
union cvmx_sli_pp_pkt_csr_control {
	uint64_t u64;
	struct cvmx_sli_pp_pkt_csr_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t mac                          : 2;  /**< MAC number to use on a PP register accesses to SLI Packet CSRs. Enumerated by SLI_PORT_E. */
	uint64_t pvf                          : 16; /**< Function number to use on a PP register accesses to SLI Packet CSRs,
                                                         where <15:13> selects the PF the
                                                         VF belongs to, and <12:0> selects the VF within that PF (or 0x0 for the PF
                                                         itself). */
#else
	uint64_t pvf                          : 16;
	uint64_t mac                          : 2;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_sli_pp_pkt_csr_control_s  cn73xx;
	struct cvmx_sli_pp_pkt_csr_control_s  cn78xx;
	struct cvmx_sli_pp_pkt_csr_control_s  cnf75xx;
};
typedef union cvmx_sli_pp_pkt_csr_control cvmx_sli_pp_pkt_csr_control_t;

/**
 * cvmx_sli_s2c_end_merge
 *
 * Writing this register will cause a merge to end.
 *
 */
union cvmx_sli_s2c_end_merge {
	uint64_t u64;
	struct cvmx_sli_s2c_end_merge_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_sli_s2c_end_merge_s       cn73xx;
	struct cvmx_sli_s2c_end_merge_s       cn78xx;
	struct cvmx_sli_s2c_end_merge_s       cn78xxp1;
	struct cvmx_sli_s2c_end_merge_s       cnf75xx;
};
typedef union cvmx_sli_s2c_end_merge cvmx_sli_s2c_end_merge_t;

/**
 * cvmx_sli_s2m_port#_ctl
 *
 * These registers contain control for access from SLI to a MAC port. Indexed by SLI_PORT_E.
 * Write operations to these registers are not ordered with write/read operations to the MAC
 * memory space. To ensure that a write operation has completed, read the register before
 * making an access (i.e. MAC memory space) that requires the value of this register to be
 * updated.
 */
union cvmx_sli_s2m_portx_ctl {
	uint64_t u64;
	struct cvmx_sli_s2m_portx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t dvferr                       : 1;  /**< Disables setting SLI_MAC()_PF()_INT_SUM[PKTVF_ERR] for vf illegal memory access */
	uint64_t lcl_node                     : 1;  /**< Must be zero. */
	uint64_t wind_d                       : 1;  /**< Window disable. When set to 1, disables access to the window registers from the MAC port. */
	uint64_t bar0_d                       : 1;  /**< BAR0 disable. When set, disables access from the MAC to BAR0 for the following
                                                         address offsets:
                                                           SLI_WIN_WR_ADDR,
                                                           SLI_WIN_RD_ADDR,
                                                           SLI_WIN_WR_DATA,
                                                           SLI_WIN_WR_MASK,
                                                           SLI_WIN_RD_DATA,
                                                           SLI_MAC_CREDIT_CNT,
                                                           SLI_S2M_PORT()_CTL,
                                                           SLI_MAC_CREDIT_CNT2,
                                                           SLI_S2C_END_MERGE,
                                                           SLI_CIU_INT_SUM,
                                                           SLI_CIU_INT_ENB,
                                                           SLI_MAC()_PF()_FLR_VF_INT,
                                                           SLI_MEM_ACCESS_SUBID(),
                                                           SLI_PP_PKT_CSR_CONTROL,
                                                           SLI_WINDOW_CTL,
                                                           SLI_MEM_ACCESS_CTL,
                                                           SLI_CTL_STATUS,
                                                           SLI_BIST_STATUS,
                                                           SLI_MEM_INT_SUM,
                                                           SLI_MEM_CTL,
                                                           SLI_CTL_PORT(),
                                                           SLI_PKT_MEM_CTL.
                                                         If [BAR0_D] is set, [WIND_D] should also be set. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t bar0_d                       : 1;
	uint64_t wind_d                       : 1;
	uint64_t lcl_node                     : 1;
	uint64_t dvferr                       : 1;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_sli_s2m_portx_ctl_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t wind_d                       : 1;  /**< When set '1' disables access to the Window
                                                         Registers from the MAC-Port.
                                                         When Authenticate-Mode is set the reset value of
                                                         this field is "1" else "0'. */
	uint64_t bar0_d                       : 1;  /**< When set '1' disables access from MAC to
                                                         BAR-0 address offsets: Less Than 0x330,
                                                         0x3CD0, and greater than 0x3D70 excluding
                                                         0x3e00.
                                                         When Authenticate-Mode is set the reset value of
                                                         this field is "1" else "0'. */
	uint64_t mrrs                         : 3;  /**< Max Read Request Size
                                                                 0 = 128B
                                                                 1 = 256B
                                                                 2 = 512B
                                                                 3 = 1024B
                                                                 4 = 2048B
                                                                 5-7 = Reserved
                                                         This field should not exceed the desired
                                                               max read request size. This field is used to
                                                               determine if an IOBDMA is too large.
                                                         For a PCIe MAC, this field should not exceed
                                                               PCIE*_CFG030[MRRS].
                                                         For a sRIO MAC, this field should indicate a size
                                                               of 256B or smaller. */
#else
	uint64_t mrrs                         : 3;
	uint64_t bar0_d                       : 1;
	uint64_t wind_d                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} cn61xx;
	struct cvmx_sli_s2m_portx_ctl_cn61xx  cn63xx;
	struct cvmx_sli_s2m_portx_ctl_cn61xx  cn63xxp1;
	struct cvmx_sli_s2m_portx_ctl_cn61xx  cn66xx;
	struct cvmx_sli_s2m_portx_ctl_cn61xx  cn68xx;
	struct cvmx_sli_s2m_portx_ctl_cn61xx  cn68xxp1;
	struct cvmx_sli_s2m_portx_ctl_cn61xx  cn70xx;
	struct cvmx_sli_s2m_portx_ctl_cn61xx  cn70xxp1;
	struct cvmx_sli_s2m_portx_ctl_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t dvferr                       : 1;  /**< Disables setting SLI_MAC()_PF()_INT_SUM[PKTVF_ERR] for vf illegal memory access */
	uint64_t lcl_node                     : 1;  /**< Must be zero. */
	uint64_t wind_d                       : 1;  /**< Window disable. When set to 1, disables access to the window registers from the MAC port. */
	uint64_t bar0_d                       : 1;  /**< BAR0 disable. When set, disables access from the MAC to BAR0 for the following
                                                         address offsets:
                                                           SLI_WIN_WR_ADDR,
                                                           SLI_WIN_RD_ADDR,
                                                           SLI_WIN_WR_DATA,
                                                           SLI_WIN_WR_MASK,
                                                           SLI_WIN_RD_DATA,
                                                           SLI_MAC_CREDIT_CNT,
                                                           SLI_S2M_PORT()_CTL,
                                                           SLI_MAC_CREDIT_CNT2,
                                                           SLI_S2C_END_MERGE,
                                                           SLI_CIU_INT_SUM,
                                                           SLI_CIU_INT_ENB,
                                                           SLI_MAC()_PF()_FLR_VF_INT,
                                                           SLI_MEM_ACCESS_SUBID(),
                                                           SLI_PP_PKT_CSR_CONTROL,
                                                           SLI_WINDOW_CTL,
                                                           SLI_MEM_ACCESS_CTL,
                                                           SLI_CTL_STATUS,
                                                           SLI_BIST_STATUS,
                                                           SLI_MEM_INT_SUM,
                                                           SLI_MEM_CTL,
                                                           SLI_CTL_PORT(),
                                                           SLI_PKT_MEM_CTL.
                                                         If [BAR0_D] is set, [WIND_D] should also be set. */
	uint64_t ld_cmd                       : 2;  /**< When SLI issues a load command to the L2C that is to be cached, this field selects the
                                                         type of load command to use:
                                                         0x0 = LDD.
                                                         0x1 = LDI.
                                                         0x2 = LDE.
                                                         0x3 = LDY. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t ld_cmd                       : 2;
	uint64_t bar0_d                       : 1;
	uint64_t wind_d                       : 1;
	uint64_t lcl_node                     : 1;
	uint64_t dvferr                       : 1;
	uint64_t reserved_7_63                : 57;
#endif
	} cn73xx;
	struct cvmx_sli_s2m_portx_ctl_cn73xx  cn78xx;
	struct cvmx_sli_s2m_portx_ctl_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t lcl_node                     : 1;  /**< Local CCPI node. When set to 1, all window access is treated as local CCPI
                                                         access. Normally, if address bits [37:36] of the window address CSRs are not
                                                         equal to the chip's CCPI value, the window operation is sent to the CCPI for
                                                         remote chip access. This field, when set, disables this and treats all access to
                                                         be for the local CCPI. */
	uint64_t wind_d                       : 1;  /**< Window disable. When set to 1, disables access to the window registers from the MAC port. */
	uint64_t bar0_d                       : 1;  /**< BAR0 disable. When set, disables access from the MAC to BAR0 for the following
                                                         address offsets:
                                                         * 0x0-0x32F.
                                                         * 0x3CD0.
                                                         * greater than 0x3D70, excluding 0x3E00. */
	uint64_t ld_cmd                       : 2;  /**< When SLI issues a load command to the L2C that is to be cached, this field selects the
                                                         type of load command to use:
                                                         0x0 = LDD.
                                                         0x1 = LDI.
                                                         0x2 = LDE.
                                                         0x3 = LDY. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t ld_cmd                       : 2;
	uint64_t bar0_d                       : 1;
	uint64_t wind_d                       : 1;
	uint64_t lcl_node                     : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} cn78xxp1;
	struct cvmx_sli_s2m_portx_ctl_cn61xx  cnf71xx;
	struct cvmx_sli_s2m_portx_ctl_cn73xx  cnf75xx;
};
typedef union cvmx_sli_s2m_portx_ctl cvmx_sli_s2m_portx_ctl_t;

/**
 * cvmx_sli_scratch_1
 *
 * This registers is a general purpose 64-bit scratch register for software use.
 *
 */
union cvmx_sli_scratch_1 {
	uint64_t u64;
	struct cvmx_sli_scratch_1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< The value in this register is totally software dependent. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_scratch_1_s           cn61xx;
	struct cvmx_sli_scratch_1_s           cn63xx;
	struct cvmx_sli_scratch_1_s           cn63xxp1;
	struct cvmx_sli_scratch_1_s           cn66xx;
	struct cvmx_sli_scratch_1_s           cn68xx;
	struct cvmx_sli_scratch_1_s           cn68xxp1;
	struct cvmx_sli_scratch_1_s           cn70xx;
	struct cvmx_sli_scratch_1_s           cn70xxp1;
	struct cvmx_sli_scratch_1_s           cn73xx;
	struct cvmx_sli_scratch_1_s           cn78xx;
	struct cvmx_sli_scratch_1_s           cn78xxp1;
	struct cvmx_sli_scratch_1_s           cnf71xx;
	struct cvmx_sli_scratch_1_s           cnf75xx;
};
typedef union cvmx_sli_scratch_1 cvmx_sli_scratch_1_t;

/**
 * cvmx_sli_scratch_2
 *
 * This registers is a general purpose 64-bit scratch register for software use.
 *
 */
union cvmx_sli_scratch_2 {
	uint64_t u64;
	struct cvmx_sli_scratch_2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< The value in this register is totally software dependent. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_scratch_2_s           cn61xx;
	struct cvmx_sli_scratch_2_s           cn63xx;
	struct cvmx_sli_scratch_2_s           cn63xxp1;
	struct cvmx_sli_scratch_2_s           cn66xx;
	struct cvmx_sli_scratch_2_s           cn68xx;
	struct cvmx_sli_scratch_2_s           cn68xxp1;
	struct cvmx_sli_scratch_2_s           cn70xx;
	struct cvmx_sli_scratch_2_s           cn70xxp1;
	struct cvmx_sli_scratch_2_s           cn73xx;
	struct cvmx_sli_scratch_2_s           cn78xx;
	struct cvmx_sli_scratch_2_s           cn78xxp1;
	struct cvmx_sli_scratch_2_s           cnf71xx;
	struct cvmx_sli_scratch_2_s           cnf75xx;
};
typedef union cvmx_sli_scratch_2 cvmx_sli_scratch_2_t;

/**
 * cvmx_sli_state1
 *
 * This register contains state machines in SLI and is for debug.
 *
 */
union cvmx_sli_state1 {
	uint64_t u64;
	struct cvmx_sli_state1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cpl1                         : 12; /**< CPL1 state. */
	uint64_t cpl0                         : 12; /**< CPL0 state. */
	uint64_t arb                          : 1;  /**< ARB state. */
	uint64_t csr                          : 39; /**< CSR state. */
#else
	uint64_t csr                          : 39;
	uint64_t arb                          : 1;
	uint64_t cpl0                         : 12;
	uint64_t cpl1                         : 12;
#endif
	} s;
	struct cvmx_sli_state1_s              cn61xx;
	struct cvmx_sli_state1_s              cn63xx;
	struct cvmx_sli_state1_s              cn63xxp1;
	struct cvmx_sli_state1_s              cn66xx;
	struct cvmx_sli_state1_s              cn68xx;
	struct cvmx_sli_state1_s              cn68xxp1;
	struct cvmx_sli_state1_s              cn70xx;
	struct cvmx_sli_state1_s              cn70xxp1;
	struct cvmx_sli_state1_s              cn73xx;
	struct cvmx_sli_state1_s              cn78xx;
	struct cvmx_sli_state1_s              cn78xxp1;
	struct cvmx_sli_state1_s              cnf71xx;
	struct cvmx_sli_state1_s              cnf75xx;
};
typedef union cvmx_sli_state1 cvmx_sli_state1_t;

/**
 * cvmx_sli_state2
 *
 * This register contains state machines in SLI and is for debug.
 *
 */
union cvmx_sli_state2 {
	uint64_t u64;
	struct cvmx_sli_state2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_sli_state2_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t nnp1                         : 8;  /**< NNP1 State */
	uint64_t reserved_47_47               : 1;
	uint64_t rac                          : 1;  /**< RAC State */
	uint64_t csm1                         : 15; /**< CSM1 State */
	uint64_t csm0                         : 15; /**< CSM0 State */
	uint64_t nnp0                         : 8;  /**< NNP0 State */
	uint64_t nnd                          : 8;  /**< NND State */
#else
	uint64_t nnd                          : 8;
	uint64_t nnp0                         : 8;
	uint64_t csm0                         : 15;
	uint64_t csm1                         : 15;
	uint64_t rac                          : 1;
	uint64_t reserved_47_47               : 1;
	uint64_t nnp1                         : 8;
	uint64_t reserved_56_63               : 8;
#endif
	} cn61xx;
	struct cvmx_sli_state2_cn61xx         cn63xx;
	struct cvmx_sli_state2_cn61xx         cn63xxp1;
	struct cvmx_sli_state2_cn61xx         cn66xx;
	struct cvmx_sli_state2_cn61xx         cn68xx;
	struct cvmx_sli_state2_cn61xx         cn68xxp1;
	struct cvmx_sli_state2_cn61xx         cn70xx;
	struct cvmx_sli_state2_cn61xx         cn70xxp1;
	struct cvmx_sli_state2_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_57_63               : 7;
	uint64_t nnp1                         : 8;  /**< NNP1 state. */
	uint64_t reserved_48_48               : 1;
	uint64_t rac                          : 1;  /**< RAC state. */
	uint64_t csm1                         : 15; /**< CSM1 state. */
	uint64_t csm0                         : 15; /**< CSM0 state. */
	uint64_t nnp0                         : 8;  /**< NNP0 state. */
	uint64_t nnd                          : 9;  /**< NND state. */
#else
	uint64_t nnd                          : 9;
	uint64_t nnp0                         : 8;
	uint64_t csm0                         : 15;
	uint64_t csm1                         : 15;
	uint64_t rac                          : 1;
	uint64_t reserved_48_48               : 1;
	uint64_t nnp1                         : 8;
	uint64_t reserved_57_63               : 7;
#endif
	} cn73xx;
	struct cvmx_sli_state2_cn73xx         cn78xx;
	struct cvmx_sli_state2_cn73xx         cn78xxp1;
	struct cvmx_sli_state2_cn61xx         cnf71xx;
	struct cvmx_sli_state2_cn73xx         cnf75xx;
};
typedef union cvmx_sli_state2 cvmx_sli_state2_t;

/**
 * cvmx_sli_state3
 *
 * This register contains state machines in SLI and is for debug.
 *
 */
union cvmx_sli_state3 {
	uint64_t u64;
	struct cvmx_sli_state3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_sli_state3_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t psm1                         : 15; /**< PSM1 State */
	uint64_t psm0                         : 15; /**< PSM0 State */
	uint64_t nsm1                         : 13; /**< NSM1 State */
	uint64_t nsm0                         : 13; /**< NSM0 State */
#else
	uint64_t nsm0                         : 13;
	uint64_t nsm1                         : 13;
	uint64_t psm0                         : 15;
	uint64_t psm1                         : 15;
	uint64_t reserved_56_63               : 8;
#endif
	} cn61xx;
	struct cvmx_sli_state3_cn61xx         cn63xx;
	struct cvmx_sli_state3_cn61xx         cn63xxp1;
	struct cvmx_sli_state3_cn61xx         cn66xx;
	struct cvmx_sli_state3_cn61xx         cn68xx;
	struct cvmx_sli_state3_cn61xx         cn68xxp1;
	struct cvmx_sli_state3_cn61xx         cn70xx;
	struct cvmx_sli_state3_cn61xx         cn70xxp1;
	struct cvmx_sli_state3_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t psm1                         : 15; /**< PSM1 state. */
	uint64_t psm0                         : 15; /**< PSM0 state. */
	uint64_t nsm1                         : 15; /**< NSM1 state. */
	uint64_t nsm0                         : 15; /**< NSM0 state. */
#else
	uint64_t nsm0                         : 15;
	uint64_t nsm1                         : 15;
	uint64_t psm0                         : 15;
	uint64_t psm1                         : 15;
	uint64_t reserved_60_63               : 4;
#endif
	} cn73xx;
	struct cvmx_sli_state3_cn73xx         cn78xx;
	struct cvmx_sli_state3_cn73xx         cn78xxp1;
	struct cvmx_sli_state3_cn61xx         cnf71xx;
	struct cvmx_sli_state3_cn73xx         cnf75xx;
};
typedef union cvmx_sli_state3 cvmx_sli_state3_t;

/**
 * cvmx_sli_tx_pipe
 *
 * SLI_TX_PIPE = SLI Packet TX Pipe
 *
 * Contains the starting pipe number and number of pipes used by the SLI packet Output.
 * If a packet is recevied from PKO with an out of range PIPE number, the following occurs:
 * - SLI_INT_SUM[PIPE_ERR] is set.
 * - the out of range pipe value is used for returning credits to the PKO.
 * - the PCIe packet engine will treat the PIPE value to be equal to [BASE].
 */
union cvmx_sli_tx_pipe {
	uint64_t u64;
	struct cvmx_sli_tx_pipe_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t nump                         : 8;  /**< Number of pipes the the SLI/DPI supports.
                                                         When this value is 4 or less there is a performance
                                                         advantage for output packets.
                                                         The SLI/DPI can support up to 32 pipes assigned to
                                                         packet-rings 0 - 31. */
	uint64_t reserved_7_15                : 9;
	uint64_t base                         : 7;  /**< When NUMP is non-zero, indicates the base pipe
                                                         number the SLI/DPI will accept.
                                                         The SLI/DPI will accept pko packets from pipes in
                                                         the range of:
                                                           BASE .. (BASE+(NUMP-1))
                                                         BASE and NUMP must be constrained such that
                                                           1) BASE+(NUMP-1) < 127
                                                           2) Each used PKO pipe must map to exactly
                                                              one ring. Where BASE == ring 0, BASE+1 == to
                                                              ring 1, etc
                                                           3) The pipe ranges must be consistent with
                                                              the PKO configuration. */
#else
	uint64_t base                         : 7;
	uint64_t reserved_7_15                : 9;
	uint64_t nump                         : 8;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_sli_tx_pipe_s             cn68xx;
	struct cvmx_sli_tx_pipe_s             cn68xxp1;
};
typedef union cvmx_sli_tx_pipe cvmx_sli_tx_pipe_t;

/**
 * cvmx_sli_win_rd_addr
 *
 * When the LSB of this register is written, the address in this register will be read. The data
 * returned from this read operation is placed in the WIN_RD_DATA register. This register should
 * NOT
 * be used to read SLI_* registers.
 */
union cvmx_sli_win_rd_addr {
	uint64_t u64;
	struct cvmx_sli_win_rd_addr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_51_63               : 13;
	uint64_t ld_cmd                       : 2;  /**< The load command sent with the read.
                                                         0x0 = Load 1 bytes.
                                                         0x1 = Load 2 bytes.
                                                         0x2 = Load 4 bytes.
                                                         0x3 = Load 8 bytes. */
	uint64_t iobit                        : 1;  /**< A 1 or 0 can be written here, but will not be used in address generation. */
	uint64_t rd_addr                      : 48; /**< The address to be read.
                                                         [47:40] = NCB_ID.
                                                         [39:38] = 0x0, Not used.
                                                         [37:36] = Must be zero.
                                                         [35:0]  = Address.
                                                         When [47:43] specifies SLI and [42:40] = 0x0, bits [39:0] are defined as follows:
                                                         [39:38] = Not used.
                                                         [37:36] = Must be zero.
                                                         [35:32] = 0x0, Not used.
                                                         [31:24] = RSL_ID.
                                                         [23:0]  = RSL register offset. */
#else
	uint64_t rd_addr                      : 48;
	uint64_t iobit                        : 1;
	uint64_t ld_cmd                       : 2;
	uint64_t reserved_51_63               : 13;
#endif
	} s;
	struct cvmx_sli_win_rd_addr_s         cn61xx;
	struct cvmx_sli_win_rd_addr_s         cn63xx;
	struct cvmx_sli_win_rd_addr_s         cn63xxp1;
	struct cvmx_sli_win_rd_addr_s         cn66xx;
	struct cvmx_sli_win_rd_addr_s         cn68xx;
	struct cvmx_sli_win_rd_addr_s         cn68xxp1;
	struct cvmx_sli_win_rd_addr_s         cn70xx;
	struct cvmx_sli_win_rd_addr_s         cn70xxp1;
	struct cvmx_sli_win_rd_addr_s         cn73xx;
	struct cvmx_sli_win_rd_addr_s         cn78xx;
	struct cvmx_sli_win_rd_addr_s         cn78xxp1;
	struct cvmx_sli_win_rd_addr_s         cnf71xx;
	struct cvmx_sli_win_rd_addr_s         cnf75xx;
};
typedef union cvmx_sli_win_rd_addr cvmx_sli_win_rd_addr_t;

/**
 * cvmx_sli_win_rd_data
 *
 * This register holds the data returned when a read operation is started by the writing of the
 * SLI_WIN_RD_ADDR register.
 */
union cvmx_sli_win_rd_data {
	uint64_t u64;
	struct cvmx_sli_win_rd_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rd_data                      : 64; /**< The read data. */
#else
	uint64_t rd_data                      : 64;
#endif
	} s;
	struct cvmx_sli_win_rd_data_s         cn61xx;
	struct cvmx_sli_win_rd_data_s         cn63xx;
	struct cvmx_sli_win_rd_data_s         cn63xxp1;
	struct cvmx_sli_win_rd_data_s         cn66xx;
	struct cvmx_sli_win_rd_data_s         cn68xx;
	struct cvmx_sli_win_rd_data_s         cn68xxp1;
	struct cvmx_sli_win_rd_data_s         cn70xx;
	struct cvmx_sli_win_rd_data_s         cn70xxp1;
	struct cvmx_sli_win_rd_data_s         cn73xx;
	struct cvmx_sli_win_rd_data_s         cn78xx;
	struct cvmx_sli_win_rd_data_s         cn78xxp1;
	struct cvmx_sli_win_rd_data_s         cnf71xx;
	struct cvmx_sli_win_rd_data_s         cnf75xx;
};
typedef union cvmx_sli_win_rd_data cvmx_sli_win_rd_data_t;

/**
 * cvmx_sli_win_wr_addr
 *
 * This register contains the address to be written to when a write operation is started by
 * writing the SLI_WIN_WR_DATA register.
 *
 * This register should NOT be used to write SLI_* registers.
 */
union cvmx_sli_win_wr_addr {
	uint64_t u64;
	struct cvmx_sli_win_wr_addr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_49_63               : 15;
	uint64_t iobit                        : 1;  /**< A 1 or 0 can be written here, but this will always read as 0. */
	uint64_t wr_addr                      : 45; /**< The address that is written to when the SLI_WIN_WR_DATA register is written.
                                                         [47:40] = NCB_ID.
                                                         [39:38] = 0x0, Not used.
                                                         [37:36] = Must be zero.
                                                         [35:0]  = Address.
                                                         When [47:43] specifies SLI and [42:40] = 0x0, bits [39:0] are defined as follows:
                                                         [39:38] = Not used.
                                                         [37:36] = Must be zero.
                                                         [35:32] = 0x0, Not used.
                                                         [31:24] = RSL_ID.
                                                         [23:0]  = RSL register offset. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t wr_addr                      : 45;
	uint64_t iobit                        : 1;
	uint64_t reserved_49_63               : 15;
#endif
	} s;
	struct cvmx_sli_win_wr_addr_s         cn61xx;
	struct cvmx_sli_win_wr_addr_s         cn63xx;
	struct cvmx_sli_win_wr_addr_s         cn63xxp1;
	struct cvmx_sli_win_wr_addr_s         cn66xx;
	struct cvmx_sli_win_wr_addr_s         cn68xx;
	struct cvmx_sli_win_wr_addr_s         cn68xxp1;
	struct cvmx_sli_win_wr_addr_s         cn70xx;
	struct cvmx_sli_win_wr_addr_s         cn70xxp1;
	struct cvmx_sli_win_wr_addr_s         cn73xx;
	struct cvmx_sli_win_wr_addr_s         cn78xx;
	struct cvmx_sli_win_wr_addr_s         cn78xxp1;
	struct cvmx_sli_win_wr_addr_s         cnf71xx;
	struct cvmx_sli_win_wr_addr_s         cnf75xx;
};
typedef union cvmx_sli_win_wr_addr cvmx_sli_win_wr_addr_t;

/**
 * cvmx_sli_win_wr_data
 *
 * This register contains the data to write to the address located in the SLI_WIN_WR_ADDR
 * register. Writing the least-significant byte of this register causes a write operation to take
 * place.
 */
union cvmx_sli_win_wr_data {
	uint64_t u64;
	struct cvmx_sli_win_wr_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wr_data                      : 64; /**< The data to be written. Whenever the LSB of this register is written, the window write
                                                         operation takes place. */
#else
	uint64_t wr_data                      : 64;
#endif
	} s;
	struct cvmx_sli_win_wr_data_s         cn61xx;
	struct cvmx_sli_win_wr_data_s         cn63xx;
	struct cvmx_sli_win_wr_data_s         cn63xxp1;
	struct cvmx_sli_win_wr_data_s         cn66xx;
	struct cvmx_sli_win_wr_data_s         cn68xx;
	struct cvmx_sli_win_wr_data_s         cn68xxp1;
	struct cvmx_sli_win_wr_data_s         cn70xx;
	struct cvmx_sli_win_wr_data_s         cn70xxp1;
	struct cvmx_sli_win_wr_data_s         cn73xx;
	struct cvmx_sli_win_wr_data_s         cn78xx;
	struct cvmx_sli_win_wr_data_s         cn78xxp1;
	struct cvmx_sli_win_wr_data_s         cnf71xx;
	struct cvmx_sli_win_wr_data_s         cnf75xx;
};
typedef union cvmx_sli_win_wr_data cvmx_sli_win_wr_data_t;

/**
 * cvmx_sli_win_wr_mask
 *
 * This register contains the mask for the data in the SLI_WIN_WR_DATA register.
 *
 */
union cvmx_sli_win_wr_mask {
	uint64_t u64;
	struct cvmx_sli_win_wr_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t wr_mask                      : 8;  /**< The byte mask for the data to be written. When a bit is 1, the corresponding byte will be
                                                         written. The values of this field must be contiguous and for one-, two-, four-, or eight-
                                                         byte operations and aligned to operation size. A value of 0x0 produces unpredictable
                                                         results. */
#else
	uint64_t wr_mask                      : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_sli_win_wr_mask_s         cn61xx;
	struct cvmx_sli_win_wr_mask_s         cn63xx;
	struct cvmx_sli_win_wr_mask_s         cn63xxp1;
	struct cvmx_sli_win_wr_mask_s         cn66xx;
	struct cvmx_sli_win_wr_mask_s         cn68xx;
	struct cvmx_sli_win_wr_mask_s         cn68xxp1;
	struct cvmx_sli_win_wr_mask_s         cn70xx;
	struct cvmx_sli_win_wr_mask_s         cn70xxp1;
	struct cvmx_sli_win_wr_mask_s         cn73xx;
	struct cvmx_sli_win_wr_mask_s         cn78xx;
	struct cvmx_sli_win_wr_mask_s         cn78xxp1;
	struct cvmx_sli_win_wr_mask_s         cnf71xx;
	struct cvmx_sli_win_wr_mask_s         cnf75xx;
};
typedef union cvmx_sli_win_wr_mask cvmx_sli_win_wr_mask_t;

/**
 * cvmx_sli_window_ctl
 *
 * Access to register space on the IOI (caused by window read/write operations) waits for a
 * period of time specified by this register before timing out.
 */
union cvmx_sli_window_ctl {
	uint64_t u64;
	struct cvmx_sli_window_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ocx_time                     : 32; /**< This field is unused. */
	uint64_t time                         : 32; /**< Time to wait. The number of coprocessor-clock cycles to wait for a window access before timing out. */
#else
	uint64_t time                         : 32;
	uint64_t ocx_time                     : 32;
#endif
	} s;
	struct cvmx_sli_window_ctl_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t time                         : 32; /**< Time to wait in core clocks for a
                                                         BAR0 access to completeon the NCB
                                                         before timing out. A value of 0 will cause no
                                                         timeouts. A minimum value of 0x200000 should be
                                                         used when this register is not set to 0x0. */
#else
	uint64_t time                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn61xx;
	struct cvmx_sli_window_ctl_cn61xx     cn63xx;
	struct cvmx_sli_window_ctl_cn61xx     cn63xxp1;
	struct cvmx_sli_window_ctl_cn61xx     cn66xx;
	struct cvmx_sli_window_ctl_cn61xx     cn68xx;
	struct cvmx_sli_window_ctl_cn61xx     cn68xxp1;
	struct cvmx_sli_window_ctl_cn61xx     cn70xx;
	struct cvmx_sli_window_ctl_cn61xx     cn70xxp1;
	struct cvmx_sli_window_ctl_s          cn73xx;
	struct cvmx_sli_window_ctl_s          cn78xx;
	struct cvmx_sli_window_ctl_s          cn78xxp1;
	struct cvmx_sli_window_ctl_cn61xx     cnf71xx;
	struct cvmx_sli_window_ctl_s          cnf75xx;
};
typedef union cvmx_sli_window_ctl cvmx_sli_window_ctl_t;

#endif
