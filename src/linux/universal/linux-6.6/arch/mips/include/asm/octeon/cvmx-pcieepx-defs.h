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
 * cvmx-pcieepx-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon pcieepx.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_PCIEEPX_DEFS_H__
#define __CVMX_PCIEEPX_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG000(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000000ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000000ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000000ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000000ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000000ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000000ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG000 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000000ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG000(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000000ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000000ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000000ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000000ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000000ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000000ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000000ull;
	}
	return 0x0000030000000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG001(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000004ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000004ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000004ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000004ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000004ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000004ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000004ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG001 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000004ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG001(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000004ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000004ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000004ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000004ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000004ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000004ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000004ull;
	}
	return 0x0000030000000004ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG002(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000008ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000008ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000008ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000008ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000008ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000008ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000008ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG002 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000008ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG002(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000008ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000008ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000008ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000008ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000008ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000008ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000008ull;
	}
	return 0x0000030000000008ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG003(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000000Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000000Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000000Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000000Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000000Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000000Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000000000000000Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG003 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000000Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG003(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000000Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000000Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000000Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000000Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000000Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000000Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000000Cull;
	}
	return 0x000003000000000Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG004(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000010ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000010ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000010ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000010ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000010ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000010ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000010ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG004 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000010ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG004(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000010ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000010ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000010ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000010ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000010ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000010ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000010ull;
	}
	return 0x0000030000000010ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG004_MASK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030080000010ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030080000010ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030080000010ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030080000010ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030080000010ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000080000010ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000080000010ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG004_MASK (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030080000010ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG004_MASK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000010ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000010ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030080000010ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030080000010ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000010ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000010ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000010ull;
	}
	return 0x0000030080000010ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG005(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000014ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000014ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000014ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000014ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000014ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000014ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000014ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG005 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000014ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG005(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000014ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000014ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000014ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000014ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000014ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000014ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000014ull;
	}
	return 0x0000030000000014ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG005_MASK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030080000014ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030080000014ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030080000014ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030080000014ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030080000014ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000080000014ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000080000014ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG005_MASK (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030080000014ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG005_MASK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000014ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000014ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030080000014ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030080000014ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000014ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000014ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000014ull;
	}
	return 0x0000030080000014ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG006(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000018ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000018ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000018ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000018ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000018ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000018ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000018ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG006 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000018ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG006(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000018ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000018ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000018ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000018ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000018ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000018ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000018ull;
	}
	return 0x0000030000000018ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG006_MASK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030080000018ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030080000018ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030080000018ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030080000018ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030080000018ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000080000018ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000080000018ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG006_MASK (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030080000018ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG006_MASK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000018ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000018ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030080000018ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030080000018ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000018ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000018ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000018ull;
	}
	return 0x0000030080000018ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG007(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000001Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000001Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000001Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000001Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000001Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000001Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000000000000001Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG007 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000001Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG007(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000001Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000001Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000001Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000001Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000001Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000001Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000001Cull;
	}
	return 0x000003000000001Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG007_MASK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003008000001Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003008000001Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003008000001Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003008000001Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003008000001Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000008000001Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000000008000001Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG007_MASK (offset = %lu) not supported on this chip\n", offset);
	return 0x000003008000001Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG007_MASK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003008000001Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003008000001Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003008000001Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003008000001Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003008000001Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000008000001Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000008000001Cull;
	}
	return 0x000003008000001Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG008(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000020ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000020ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000020ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000020ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000020ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000020ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000020ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG008 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000020ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG008(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000020ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000020ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000020ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000020ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000020ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000020ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000020ull;
	}
	return 0x0000030000000020ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG008_MASK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030080000020ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030080000020ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030080000020ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030080000020ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030080000020ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000080000020ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000080000020ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG008_MASK (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030080000020ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG008_MASK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000020ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000020ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030080000020ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030080000020ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000020ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000020ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000020ull;
	}
	return 0x0000030080000020ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG009(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000024ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000024ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000024ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000024ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000024ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000024ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000024ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG009 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000024ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG009(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000024ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000024ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000024ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000024ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000024ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000024ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000024ull;
	}
	return 0x0000030000000024ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG009_MASK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030080000024ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030080000024ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030080000024ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030080000024ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030080000024ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000080000024ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000080000024ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG009_MASK (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030080000024ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG009_MASK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000024ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000024ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030080000024ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030080000024ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000024ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000024ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000024ull;
	}
	return 0x0000030080000024ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG010(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000028ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000028ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000028ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000028ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000028ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000028ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000028ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG010 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000028ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG010(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000028ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000028ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000028ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000028ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000028ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000028ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000028ull;
	}
	return 0x0000030000000028ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG011(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000002Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000002Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000002Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000002Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000002Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000002Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000000000000002Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG011 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000002Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG011(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000002Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000002Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000002Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000002Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000002Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000002Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000002Cull;
	}
	return 0x000003000000002Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG012(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000030ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000030ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000030ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000030ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000030ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000030ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000030ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG012 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000030ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG012(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000030ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000030ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000030ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000030ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000030ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000030ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000030ull;
	}
	return 0x0000030000000030ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG012_MASK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030080000030ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030080000030ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030080000030ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030080000030ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030080000030ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000080000030ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000080000030ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG012_MASK (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030080000030ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG012_MASK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000030ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000030ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030080000030ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030080000030ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000030ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000030ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000030ull;
	}
	return 0x0000030080000030ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG013(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000034ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000034ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000034ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000034ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000034ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000034ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000034ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG013 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000034ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG013(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000034ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000034ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000034ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000034ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000034ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000034ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000034ull;
	}
	return 0x0000030000000034ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG015(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000003Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000003Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000003Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000003Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000003Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000003Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000000000000003Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG015 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000003Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG015(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000003Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000003Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000003Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000003Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000003Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000003Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000003Cull;
	}
	return 0x000003000000003Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG016(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000040ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000040ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000040ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000040ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000040ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000040ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000040ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG016 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000040ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG016(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000040ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000040ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000040ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000040ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000040ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000040ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000040ull;
	}
	return 0x0000030000000040ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG017(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000044ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000044ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000044ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000044ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000044ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000044ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000044ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG017 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000044ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG017(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000044ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000044ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000044ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000044ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000044ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000044ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000044ull;
	}
	return 0x0000030000000044ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG020(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000050ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000050ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000050ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000050ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000050ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000050ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000050ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG020 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000050ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG020(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000050ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000050ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000050ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000050ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000050ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000050ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000050ull;
	}
	return 0x0000030000000050ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG021(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000054ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000054ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000054ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000054ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000054ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000054ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000054ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG021 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000054ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG021(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000054ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000054ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000054ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000054ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000054ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000054ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000054ull;
	}
	return 0x0000030000000054ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG022(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000058ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000058ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000058ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000058ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000058ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000058ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000058ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG022 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000058ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG022(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000058ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000058ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000058ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000058ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000058ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000058ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000058ull;
	}
	return 0x0000030000000058ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG023(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000005Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000005Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000005Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000005Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000005Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000005Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000000000000005Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG023 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000005Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG023(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000005Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000005Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000005Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000005Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000005Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000005Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000005Cull;
	}
	return 0x000003000000005Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG024(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000060ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000060ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000060ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000060ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000060ull + ((offset) & 7) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG024 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000060ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG024(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000060ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000060ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000060ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000060ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000060ull + (offset) * 0x100000000ull;
	}
	return 0x0000030000000060ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG025(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000064ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000064ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000064ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000064ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000064ull + ((offset) & 7) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG025 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000064ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG025(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000064ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000064ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000064ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000064ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000064ull + (offset) * 0x100000000ull;
	}
	return 0x0000030000000064ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG028(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000070ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000070ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000070ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000070ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000070ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000070ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000070ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG028 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000070ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG028(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000070ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000070ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000070ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000070ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000070ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000070ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000070ull;
	}
	return 0x0000030000000070ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG029(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000074ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000074ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000074ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000074ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000074ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000074ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000074ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG029 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000074ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG029(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000074ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000074ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000074ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000074ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000074ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000074ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000074ull;
	}
	return 0x0000030000000074ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG030(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000078ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000078ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000078ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000078ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000078ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000078ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000078ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG030 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000078ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG030(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000078ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000078ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000078ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000078ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000078ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000078ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000078ull;
	}
	return 0x0000030000000078ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG031(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000007Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000007Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000007Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000007Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000007Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000007Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000000000000007Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG031 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000007Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG031(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000007Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000007Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000007Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000007Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000007Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000007Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000007Cull;
	}
	return 0x000003000000007Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG032(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000080ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000080ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000080ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000080ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000080ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000080ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000080ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG032 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000080ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG032(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000080ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000080ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000080ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000080ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000080ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000080ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000080ull;
	}
	return 0x0000030000000080ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG033(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPX_CFG033(%lu) is invalid on this chip\n", offset);
	return 0x0000000000000084ull;
}
#else
#define CVMX_PCIEEPX_CFG033(offset) (0x0000000000000084ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG034(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPX_CFG034(%lu) is invalid on this chip\n", offset);
	return 0x0000000000000088ull;
}
#else
#define CVMX_PCIEEPX_CFG034(offset) (0x0000000000000088ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG037(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000094ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000094ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000094ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000094ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000094ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000094ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000094ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG037 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000094ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG037(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000094ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000094ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000094ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000094ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000094ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000094ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000094ull;
	}
	return 0x0000030000000094ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG038(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000098ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000098ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000098ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000098ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000098ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000098ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000098ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG038 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000098ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG038(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000098ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000098ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000098ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000098ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000098ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000098ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000098ull;
	}
	return 0x0000030000000098ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG039(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000009Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000009Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000009Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000009Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000009Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000009Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000000000000009Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG039 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000009Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG039(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000009Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000009Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000009Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000009Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000009Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000009Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000009Cull;
	}
	return 0x000003000000009Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG040(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000000A0ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x00000300000000A0ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000000A0ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000000A0ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000000A0ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x00000000000000A0ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000000000000A0ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG040 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000000A0ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG040(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000000A0ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x00000300000000A0ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000000A0ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000000A0ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000000A0ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000000A0ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x00000000000000A0ull;
	}
	return 0x00000300000000A0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG041(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPX_CFG041(%lu) is invalid on this chip\n", offset);
	return 0x00000000000000A4ull;
}
#else
#define CVMX_PCIEEPX_CFG041(offset) (0x00000000000000A4ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG042(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PCIEEPX_CFG042(%lu) is invalid on this chip\n", offset);
	return 0x00000000000000A8ull;
}
#else
#define CVMX_PCIEEPX_CFG042(offset) (0x00000000000000A8ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG044(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000000B0ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000000B0ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000000B0ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000000B0ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG044 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000000B0ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG044(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000000B0ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000000B0ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000000B0ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000000B0ull;
	}
	return 0x00000300000000B0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG045(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000000B4ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000000B4ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000000B4ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000000B4ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG045 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000000B4ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG045(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000000B4ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000000B4ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000000B4ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000000B4ull;
	}
	return 0x00000300000000B4ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG046(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000000B8ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000000B8ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000000B8ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000000B8ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG046 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000000B8ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG046(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000000B8ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000000B8ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000000B8ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000000B8ull;
	}
	return 0x00000300000000B8ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG064(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000100ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000100ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000100ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000100ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000100ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000100ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000100ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG064 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000100ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG064(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000100ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000100ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000100ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000100ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000100ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000100ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000100ull;
	}
	return 0x0000030000000100ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG065(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000104ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000104ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000104ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000104ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000104ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000104ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000104ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG065 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000104ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG065(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000104ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000104ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000104ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000104ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000104ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000104ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000104ull;
	}
	return 0x0000030000000104ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG066(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000108ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000108ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000108ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000108ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000108ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000108ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000108ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG066 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000108ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG066(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000108ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000108ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000108ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000108ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000108ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000108ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000108ull;
	}
	return 0x0000030000000108ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG067(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000010Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000010Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000010Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000010Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000010Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000010Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000000000000010Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG067 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000010Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG067(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000010Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000010Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000010Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000010Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000010Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000010Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000010Cull;
	}
	return 0x000003000000010Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG068(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000110ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000110ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000110ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000110ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000110ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000110ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000110ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG068 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000110ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG068(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000110ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000110ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000110ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000110ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000110ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000110ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000110ull;
	}
	return 0x0000030000000110ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG069(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000114ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000114ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000114ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000114ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000114ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000114ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000114ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG069 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000114ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG069(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000114ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000114ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000114ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000114ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000114ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000114ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000114ull;
	}
	return 0x0000030000000114ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG070(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000118ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000118ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000118ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000118ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000118ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000118ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000118ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG070 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000118ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG070(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000118ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000118ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000118ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000118ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000118ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000118ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000118ull;
	}
	return 0x0000030000000118ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG071(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000011Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000011Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000011Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000011Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000011Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000011Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000000000000011Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG071 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000011Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG071(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000011Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000011Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000011Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000011Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000011Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000011Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000011Cull;
	}
	return 0x000003000000011Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG072(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000120ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000120ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000120ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000120ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000120ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000120ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000120ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG072 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000120ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG072(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000120ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000120ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000120ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000120ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000120ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000120ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000120ull;
	}
	return 0x0000030000000120ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG073(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000124ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000124ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000124ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000124ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000124ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000124ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000124ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG073 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000124ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG073(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000124ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000124ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000124ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000124ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000124ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000124ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000124ull;
	}
	return 0x0000030000000124ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG074(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000128ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000128ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000128ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000128ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000128ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000128ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000128ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG074 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000128ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG074(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000128ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000128ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000128ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000128ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000128ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000128ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000128ull;
	}
	return 0x0000030000000128ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG078(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000138ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000138ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000138ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000138ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG078 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000138ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG078(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000138ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000138ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000138ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000138ull;
	}
	return 0x0000030000000138ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG082(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000148ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000148ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000148ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000148ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000148ull + ((offset) & 7) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG082 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000148ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG082(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000148ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000148ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000148ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000148ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000148ull + (offset) * 0x100000000ull;
	}
	return 0x0000030000000148ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG083(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000014Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000014Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000014Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000014Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000014Cull + ((offset) & 7) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG083 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000014Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG083(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000014Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000014Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000014Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000014Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000014Cull + (offset) * 0x100000000ull;
	}
	return 0x000003000000014Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG084(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 2)))))
		cvmx_warn("CVMX_PCIEEPX_CFG084(%lu) is invalid on this chip\n", offset);
	return 0x0000030000000150ull + ((offset) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG084(offset) (0x0000030000000150ull + ((offset) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG086(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000158ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000158ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000158ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000158ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG086 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000158ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG086(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000158ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000158ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000158ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000158ull;
	}
	return 0x0000030000000158ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG087(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000015Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000015Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000015Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000015Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG087 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000015Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG087(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000015Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000015Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000015Cull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000015Cull;
	}
	return 0x000003000000015Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG088(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000160ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000160ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000160ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000160ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG088 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000160ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG088(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000160ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000160ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000160ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000160ull;
	}
	return 0x0000030000000160ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG089(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000164ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000164ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000164ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000164ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG089 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000164ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG089(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000164ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000164ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000164ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000164ull;
	}
	return 0x0000030000000164ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG090(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000168ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000168ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000168ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000168ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG090 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000168ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG090(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000168ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000168ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000168ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000168ull;
	}
	return 0x0000030000000168ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG091(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000016Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000016Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000016Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000016Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG091 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000016Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG091(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000016Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000016Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000016Cull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000016Cull;
	}
	return 0x000003000000016Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG092(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000170ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000170ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000170ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000170ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG092 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000170ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG092(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000170ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000170ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000170ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000170ull;
	}
	return 0x0000030000000170ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG094(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000178ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000178ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000178ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000178ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG094 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000178ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG094(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000178ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000178ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000178ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000178ull;
	}
	return 0x0000030000000178ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG095(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000017Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000017Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000017Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000017Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG095 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000017Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG095(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000017Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000017Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000017Cull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000017Cull;
	}
	return 0x000003000000017Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG096(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000180ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000180ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000180ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000180ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG096 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000180ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG096(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000180ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000180ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000180ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000180ull;
	}
	return 0x0000030000000180ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG097(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000184ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000184ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000184ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000184ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG097 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000184ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG097(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000184ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000184ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000184ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000184ull;
	}
	return 0x0000030000000184ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG098(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000188ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000188ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000188ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000188ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG098 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000188ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG098(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000188ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000188ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000188ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000188ull;
	}
	return 0x0000030000000188ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG099(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000018Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000018Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000018Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000018Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG099 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000018Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG099(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000018Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000018Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000018Cull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000018Cull;
	}
	return 0x000003000000018Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG100(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000190ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000190ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000190ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000190ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG100 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000190ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG100(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000190ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000190ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000190ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000190ull;
	}
	return 0x0000030000000190ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG101(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000194ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000194ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000194ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000194ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG101 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000194ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG101(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000194ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000194ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000194ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000194ull;
	}
	return 0x0000030000000194ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG102(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000198ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000198ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000198ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000198ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG102 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000198ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG102(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000198ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000198ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000198ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000198ull;
	}
	return 0x0000030000000198ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG103(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000019Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000019Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000019Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000019Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG103 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000019Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG103(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000019Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000019Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000019Cull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000019Cull;
	}
	return 0x000003000000019Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG104(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000001A0ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000001A0ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000001A0ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000001A0ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG104 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000001A0ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG104(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000001A0ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000001A0ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001A0ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001A0ull;
	}
	return 0x00000300000001A0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG105(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000001A4ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000001A4ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000001A4ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000001A4ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG105 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000001A4ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG105(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000001A4ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000001A4ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001A4ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001A4ull;
	}
	return 0x00000300000001A4ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG106(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000001A8ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000001A8ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000001A8ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000001A8ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG106 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000001A8ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG106(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000001A8ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000001A8ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001A8ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001A8ull;
	}
	return 0x00000300000001A8ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG107(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000001ACull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000001ACull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000001ACull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000001ACull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG107 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000001ACull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG107(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000001ACull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000001ACull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001ACull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001ACull;
	}
	return 0x00000300000001ACull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG108(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000001B0ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000001B0ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000001B0ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000001B0ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG108 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000001B0ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG108(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000001B0ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000001B0ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001B0ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001B0ull;
	}
	return 0x00000300000001B0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG109(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000001B4ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000001B4ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000001B4ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000001B4ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG109 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000001B4ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG109(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000001B4ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000001B4ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001B4ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001B4ull;
	}
	return 0x00000300000001B4ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG110(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000001B8ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000001B8ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000001B8ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000001B8ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG110 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000001B8ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG110(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000001B8ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000001B8ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001B8ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001B8ull;
	}
	return 0x00000300000001B8ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG111(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000001BCull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000001BCull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000001BCull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000001BCull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG111 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000001BCull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG111(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000001BCull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000001BCull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001BCull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001BCull;
	}
	return 0x00000300000001BCull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG112(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000001C0ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000001C0ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000001C0ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000001C0ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG112 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000001C0ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG112(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000001C0ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000001C0ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001C0ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000001C0ull;
	}
	return 0x00000300000001C0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG448(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000700ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000700ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000700ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000700ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000700ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000700ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000700ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG448 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000700ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG448(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000700ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000700ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000700ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000700ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000700ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000700ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000700ull;
	}
	return 0x0000030000000700ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG449(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000704ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000704ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000704ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000704ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000704ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000704ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000704ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG449 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000704ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG449(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000704ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000704ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000704ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000704ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000704ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000704ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000704ull;
	}
	return 0x0000030000000704ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG450(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000708ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000708ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000708ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000708ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000708ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000708ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000708ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG450 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000708ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG450(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000708ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000708ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000708ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000708ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000708ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000708ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000708ull;
	}
	return 0x0000030000000708ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG451(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000070Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000070Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000070Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000070Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000070Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000070Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000000000000070Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG451 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000070Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG451(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000070Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000070Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000070Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000070Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000070Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000070Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000070Cull;
	}
	return 0x000003000000070Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG452(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000710ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000710ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000710ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000710ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000710ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000710ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000710ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG452 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000710ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG452(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000710ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000710ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000710ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000710ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000710ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000710ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000710ull;
	}
	return 0x0000030000000710ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG453(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000714ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000714ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000714ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000714ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000714ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000714ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000714ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG453 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000714ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG453(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000714ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000714ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000714ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000714ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000714ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000714ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000714ull;
	}
	return 0x0000030000000714ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG454(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000718ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000718ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000718ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000718ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000718ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000718ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000718ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG454 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000718ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG454(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000718ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000718ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000718ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000718ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000718ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000718ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000718ull;
	}
	return 0x0000030000000718ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG455(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000071Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000071Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000071Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000071Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000071Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000071Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000000000000071Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG455 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000071Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG455(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000071Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000071Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000071Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000071Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000071Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000071Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000071Cull;
	}
	return 0x000003000000071Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG456(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000720ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000720ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000720ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000720ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000720ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000720ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000720ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG456 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000720ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG456(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000720ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000720ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000720ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000720ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000720ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000720ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000720ull;
	}
	return 0x0000030000000720ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG458(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000728ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000728ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000728ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000728ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000728ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000728ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000728ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG458 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000728ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG458(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000728ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000728ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000728ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000728ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000728ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000728ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000728ull;
	}
	return 0x0000030000000728ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG459(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000072Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000072Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000072Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000072Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000072Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000072Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000000000000072Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG459 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000072Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG459(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000072Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000072Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000072Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000072Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000072Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000072Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000072Cull;
	}
	return 0x000003000000072Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG460(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000730ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000730ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000730ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000730ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000730ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000730ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000730ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG460 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000730ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG460(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000730ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000730ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000730ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000730ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000730ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000730ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000730ull;
	}
	return 0x0000030000000730ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG461(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000734ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000734ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000734ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000734ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000734ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000734ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000734ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG461 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000734ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG461(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000734ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000734ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000734ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000734ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000734ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000734ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000734ull;
	}
	return 0x0000030000000734ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG462(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000738ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000738ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000738ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000738ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000738ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000738ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000738ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG462 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000738ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG462(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000738ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000738ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000738ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000738ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000738ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000738ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000738ull;
	}
	return 0x0000030000000738ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG463(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000073Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000073Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000073Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000073Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000073Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000073Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000000000000073Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG463 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000073Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG463(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000073Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000073Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000073Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000073Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000073Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000073Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000073Cull;
	}
	return 0x000003000000073Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG464(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000740ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000740ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000740ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000740ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000740ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000740ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000740ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG464 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000740ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG464(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000740ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000740ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000740ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000740ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000740ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000740ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000740ull;
	}
	return 0x0000030000000740ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG465(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000744ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000744ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000744ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000744ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000744ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000744ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000744ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG465 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000744ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG465(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000744ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000744ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000744ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000744ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000744ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000744ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000744ull;
	}
	return 0x0000030000000744ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG466(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000748ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000748ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000748ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000748ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000748ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000748ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000748ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG466 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000748ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG466(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000748ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000748ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000748ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000748ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000748ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000748ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000748ull;
	}
	return 0x0000030000000748ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG467(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000074Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000074Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000074Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000074Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000074Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000074Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000000000000074Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG467 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000074Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG467(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000074Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000074Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000074Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000074Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000074Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000074Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000074Cull;
	}
	return 0x000003000000074Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG468(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000750ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000750ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000750ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000750ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000750ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000750ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000750ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG468 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000750ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG468(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000750ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000750ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000750ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000750ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000750ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000750ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000750ull;
	}
	return 0x0000030000000750ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG490(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x00000000000007A8ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000000000007A8ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x00000300000007A8ull + ((offset) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG490 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000000000007A8ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG490(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000007A8ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x00000000000007A8ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x00000300000007A8ull + (offset) * 0x100000000ull;
	}
	return 0x00000000000007A8ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG491(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x00000000000007ACull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000000000007ACull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x00000300000007ACull + ((offset) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG491 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000000000007ACull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG491(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000007ACull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x00000000000007ACull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x00000300000007ACull + (offset) * 0x100000000ull;
	}
	return 0x00000000000007ACull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG492(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x00000000000007B0ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000000000007B0ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x00000300000007B0ull + ((offset) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG492 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000000000007B0ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG492(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000007B0ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x00000000000007B0ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x00000300000007B0ull + (offset) * 0x100000000ull;
	}
	return 0x00000000000007B0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG515(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x000003000000080Cull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x000003000000080Cull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x000003000000080Cull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x000003000000080Cull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x000003000000080Cull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000080Cull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG515 (offset = %lu) not supported on this chip\n", offset);
	return 0x000003000000080Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG515(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000080Cull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x000003000000080Cull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x000003000000080Cull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x000003000000080Cull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000003000000080Cull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000080Cull;
	}
	return 0x000003000000080Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG516(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000810ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000810ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000810ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000810ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000810ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000810ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000810ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG516 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000810ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG516(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000810ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000810ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000810ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000810ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000810ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000810ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000810ull;
	}
	return 0x0000030000000810ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG517(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000814ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return 0x0000030000000814ull + ((offset) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000814ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000814ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000814ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000814ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000000000000814ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG517 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000814ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG517(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000814ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000814ull + (offset) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000814ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000814ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000814ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000814ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000814ull;
	}
	return 0x0000030000000814ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG548(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x0000030000000890ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x0000030000000890ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x0000030000000890ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x0000030000000890ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG548 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000030000000890ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG548(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x0000030000000890ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x0000030000000890ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000890ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000890ull;
	}
	return 0x0000030000000890ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG554(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000008A8ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000008A8ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000008A8ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000008A8ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG554 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000008A8ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG554(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000008A8ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000008A8ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000008A8ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000008A8ull;
	}
	return 0x00000300000008A8ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG558(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000008B8ull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000008B8ull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000008B8ull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000008B8ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG558 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000008B8ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG558(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000008B8ull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000008B8ull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000008B8ull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000008B8ull;
	}
	return 0x00000300000008B8ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG559(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return 0x00000300000008BCull + ((offset) & 3) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return 0x00000300000008BCull + ((offset) & 3) * 0x100000000ull;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 5))
				return 0x00000300000008BCull + ((offset) & 7) * 0x100000000ull;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return 0x00000300000008BCull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG559 (offset = %lu) not supported on this chip\n", offset);
	return 0x00000300000008BCull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG559(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return 0x00000300000008BCull + (offset) * 0x100000000ull;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return 0x00000300000008BCull + (offset) * 0x100000000ull;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return 0x00000300000008BCull + (offset) * 0x100000000ull;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x00000300000008BCull;
	}
	return 0x00000300000008BCull;
}
#endif

/**
 * cvmx_pcieep#_cfg000
 *
 * This register contains the first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg000 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg000_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t devid                        : 16; /**< Device ID for CNXXXX, writable through PEM()_CFG_WR.
                                                         For EEPROM loads, also see VENDID of this register. */
	uint32_t vendid                       : 16; /**< Cavium's vendor ID, writable through PEM()_CFG_WR.
                                                         During an EPROM Load, if a value of 0xFFFF is loaded to this field and
                                                         a value of 0xFFFF is loaded to the DEVID field of this register, the value will not be
                                                         loaded, EEPROM load will stop, and the FastLinkEnable bit will be set in the
                                                         PCIEEP()_CFG452 register. */
#else
	uint32_t vendid                       : 16;
	uint32_t devid                        : 16;
#endif
	} s;
	struct cvmx_pcieepx_cfg000_s          cn52xx;
	struct cvmx_pcieepx_cfg000_s          cn52xxp1;
	struct cvmx_pcieepx_cfg000_s          cn56xx;
	struct cvmx_pcieepx_cfg000_s          cn56xxp1;
	struct cvmx_pcieepx_cfg000_s          cn61xx;
	struct cvmx_pcieepx_cfg000_s          cn63xx;
	struct cvmx_pcieepx_cfg000_s          cn63xxp1;
	struct cvmx_pcieepx_cfg000_s          cn66xx;
	struct cvmx_pcieepx_cfg000_s          cn68xx;
	struct cvmx_pcieepx_cfg000_s          cn68xxp1;
	struct cvmx_pcieepx_cfg000_s          cn70xx;
	struct cvmx_pcieepx_cfg000_s          cn70xxp1;
	struct cvmx_pcieepx_cfg000_s          cn73xx;
	struct cvmx_pcieepx_cfg000_s          cn78xx;
	struct cvmx_pcieepx_cfg000_s          cn78xxp1;
	struct cvmx_pcieepx_cfg000_s          cnf71xx;
	struct cvmx_pcieepx_cfg000_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg000 cvmx_pcieepx_cfg000_t;

/**
 * cvmx_pcieep#_cfg001
 *
 * This register contains the second 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg001 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg001_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dpe                          : 1;  /**< Detected parity error. */
	uint32_t sse                          : 1;  /**< Signaled system error. */
	uint32_t rma                          : 1;  /**< Received master abort. */
	uint32_t rta                          : 1;  /**< Received target abort. */
	uint32_t sta                          : 1;  /**< Signaled target abort. */
	uint32_t devt                         : 2;  /**< DEVSEL timing. Not applicable for PCI Express. Hardwired to 0x0. */
	uint32_t mdpe                         : 1;  /**< Master data parity error. */
	uint32_t fbb                          : 1;  /**< Fast back-to-back capable. Not applicable for PCI Express. Hardwired to 0. */
	uint32_t reserved_22_22               : 1;
	uint32_t m66                          : 1;  /**< 66 MHz capable. Not applicable for PCI Express. Hardwired to 0. */
	uint32_t cl                           : 1;  /**< Capabilities list. Indicates presence of an extended capability item. Hardwired to 1. */
	uint32_t i_stat                       : 1;  /**< INTx status. */
	uint32_t reserved_11_18               : 8;
	uint32_t i_dis                        : 1;  /**< INTx assertion disable. */
	uint32_t fbbe                         : 1;  /**< Fast back-to-back transaction enable. Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t see                          : 1;  /**< SERR# enable. */
	uint32_t ids_wcc                      : 1;  /**< IDSEL stepping/wait cycle control. Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t per                          : 1;  /**< Parity error response. */
	uint32_t vps                          : 1;  /**< VGA palette snoop. Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t mwice                        : 1;  /**< Memory write and invalidate. Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t scse                         : 1;  /**< Special cycle enable. Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t me                           : 1;  /**< Bus master enable.  If the PF or any of its VF's try to master the bus when this bit is
                                                         not set,
                                                         the request is discarded. A interrupt will be generated setting
                                                         PEM()_DBG_INFO[BMD_E].
                                                         Transactions are dropped in the Client.  Non-posted transactions returns a SWI_RSP_ERROR
                                                         to SLI/DPI soon thereafter.
                                                         Bus master enable mimics the behavior of PEM()_FLR_PF_STOPREQ. */
	uint32_t msae                         : 1;  /**< Memory space access enable. */
	uint32_t isae                         : 1;  /**< I/O space access enable. */
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
	struct cvmx_pcieepx_cfg001_s          cn52xx;
	struct cvmx_pcieepx_cfg001_s          cn52xxp1;
	struct cvmx_pcieepx_cfg001_s          cn56xx;
	struct cvmx_pcieepx_cfg001_s          cn56xxp1;
	struct cvmx_pcieepx_cfg001_s          cn61xx;
	struct cvmx_pcieepx_cfg001_s          cn63xx;
	struct cvmx_pcieepx_cfg001_s          cn63xxp1;
	struct cvmx_pcieepx_cfg001_s          cn66xx;
	struct cvmx_pcieepx_cfg001_s          cn68xx;
	struct cvmx_pcieepx_cfg001_s          cn68xxp1;
	struct cvmx_pcieepx_cfg001_s          cn70xx;
	struct cvmx_pcieepx_cfg001_s          cn70xxp1;
	struct cvmx_pcieepx_cfg001_s          cn73xx;
	struct cvmx_pcieepx_cfg001_s          cn78xx;
	struct cvmx_pcieepx_cfg001_s          cn78xxp1;
	struct cvmx_pcieepx_cfg001_s          cnf71xx;
	struct cvmx_pcieepx_cfg001_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg001 cvmx_pcieepx_cfg001_t;

/**
 * cvmx_pcieep#_cfg002
 *
 * This register contains the third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg002 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg002_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t bcc                          : 8;  /**< Base class code,
                                                         Writable through PEM()_CFG_WR. */
	uint32_t sc                           : 8;  /**< Subclass code,
                                                         Writable through PEM()_CFG_WR. */
	uint32_t pi                           : 8;  /**< Programming interface.
                                                         Writable through PEM()_CFG_WR. */
	uint32_t rid                          : 8;  /**< Revision ID, writable through PEM()_CFG_WR. However, the application must not change
                                                         this field. Possible values:
                                                         0x0 = Pass 1.0. */
#else
	uint32_t rid                          : 8;
	uint32_t pi                           : 8;
	uint32_t sc                           : 8;
	uint32_t bcc                          : 8;
#endif
	} s;
	struct cvmx_pcieepx_cfg002_s          cn52xx;
	struct cvmx_pcieepx_cfg002_s          cn52xxp1;
	struct cvmx_pcieepx_cfg002_s          cn56xx;
	struct cvmx_pcieepx_cfg002_s          cn56xxp1;
	struct cvmx_pcieepx_cfg002_s          cn61xx;
	struct cvmx_pcieepx_cfg002_s          cn63xx;
	struct cvmx_pcieepx_cfg002_s          cn63xxp1;
	struct cvmx_pcieepx_cfg002_s          cn66xx;
	struct cvmx_pcieepx_cfg002_s          cn68xx;
	struct cvmx_pcieepx_cfg002_s          cn68xxp1;
	struct cvmx_pcieepx_cfg002_s          cn70xx;
	struct cvmx_pcieepx_cfg002_s          cn70xxp1;
	struct cvmx_pcieepx_cfg002_s          cn73xx;
	struct cvmx_pcieepx_cfg002_s          cn78xx;
	struct cvmx_pcieepx_cfg002_s          cn78xxp1;
	struct cvmx_pcieepx_cfg002_s          cnf71xx;
	struct cvmx_pcieepx_cfg002_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg002 cvmx_pcieepx_cfg002_t;

/**
 * cvmx_pcieep#_cfg003
 *
 * This register contains the fourth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg003 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg003_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t bist                         : 8;  /**< The BIST register functions are not supported. All 8 bits of the BIST register are
                                                         hardwired to 0x0. */
	uint32_t mfd                          : 1;  /**< Multi function device. The multi function device bit is writable through PEM()_CFG_WR.
                                                         However, this is a single function device. Therefore, the application must not write a 1
                                                         to this bit.
                                                         _ PCIEEP(0..2): 0x1
                                                         _ PCIEEP(3..5): 0x0 */
	uint32_t chf                          : 7;  /**< Configuration header format. Hardwired to 0x0 for type 0. */
	uint32_t lt                           : 8;  /**< Master latency timer. Not applicable for PCI Express, hardwired to 0x0. */
	uint32_t cls                          : 8;  /**< Cache line size. The cache line size register is R/W for legacy compatibility purposes and
                                                         is not applicable to PCI Express device functionality. Writing to the cache line size
                                                         register does not impact functionality of the PCI Express bus. */
#else
	uint32_t cls                          : 8;
	uint32_t lt                           : 8;
	uint32_t chf                          : 7;
	uint32_t mfd                          : 1;
	uint32_t bist                         : 8;
#endif
	} s;
	struct cvmx_pcieepx_cfg003_s          cn52xx;
	struct cvmx_pcieepx_cfg003_s          cn52xxp1;
	struct cvmx_pcieepx_cfg003_s          cn56xx;
	struct cvmx_pcieepx_cfg003_s          cn56xxp1;
	struct cvmx_pcieepx_cfg003_s          cn61xx;
	struct cvmx_pcieepx_cfg003_s          cn63xx;
	struct cvmx_pcieepx_cfg003_s          cn63xxp1;
	struct cvmx_pcieepx_cfg003_s          cn66xx;
	struct cvmx_pcieepx_cfg003_s          cn68xx;
	struct cvmx_pcieepx_cfg003_s          cn68xxp1;
	struct cvmx_pcieepx_cfg003_s          cn70xx;
	struct cvmx_pcieepx_cfg003_s          cn70xxp1;
	struct cvmx_pcieepx_cfg003_s          cn73xx;
	struct cvmx_pcieepx_cfg003_s          cn78xx;
	struct cvmx_pcieepx_cfg003_s          cn78xxp1;
	struct cvmx_pcieepx_cfg003_s          cnf71xx;
	struct cvmx_pcieepx_cfg003_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg003 cvmx_pcieepx_cfg003_t;

/**
 * cvmx_pcieep#_cfg004
 *
 * This register contains the fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg004 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg004_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_4_31                : 28;
	uint32_t pf                           : 1;  /**< Prefetchable. This field is writable through PEM()_CFG_WR. However, the application
                                                         must not change this field. */
	uint32_t typ                          : 2;  /**< BAR type.
                                                         0x0 = 32-bit BAR.
                                                         0x2 = 64-bit BAR.
                                                         This field is writable through PEM()_CFG_WR. However, the application must not change
                                                         this field. */
	uint32_t mspc                         : 1;  /**< Memory space indicator.
                                                         0 = BAR 0 is a memory BAR.
                                                         1 = BAR 0 is an I/O BAR.
                                                         This field is writable through PEM()_CFG_WR. However, the application must not change
                                                         this field. */
#else
	uint32_t mspc                         : 1;
	uint32_t typ                          : 2;
	uint32_t pf                           : 1;
	uint32_t reserved_4_31                : 28;
#endif
	} s;
	struct cvmx_pcieepx_cfg004_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lbab                         : 18; /**< Lower bits of the BAR 0 base address */
	uint32_t reserved_4_13                : 10;
	uint32_t pf                           : 1;  /**< Prefetchable
                                                         This field is writable through PESC(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t typ                          : 2;  /**< BAR type
                                                            o 00 = 32-bit BAR
                                                            o 10 = 64-bit BAR
                                                         This field is writable through PESC(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t mspc                         : 1;  /**< Memory Space Indicator
                                                            o 0 = BAR 0 is a memory BAR
                                                            o 1 = BAR 0 is an I/O BAR
                                                         This field is writable through PESC(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
#else
	uint32_t mspc                         : 1;
	uint32_t typ                          : 2;
	uint32_t pf                           : 1;
	uint32_t reserved_4_13                : 10;
	uint32_t lbab                         : 18;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg004_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg004_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg004_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg004_cn52xx     cn61xx;
	struct cvmx_pcieepx_cfg004_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg004_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg004_cn52xx     cn66xx;
	struct cvmx_pcieepx_cfg004_cn52xx     cn68xx;
	struct cvmx_pcieepx_cfg004_cn52xx     cn68xxp1;
	struct cvmx_pcieepx_cfg004_cn52xx     cn70xx;
	struct cvmx_pcieepx_cfg004_cn52xx     cn70xxp1;
	struct cvmx_pcieepx_cfg004_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lbab                         : 9;  /**< Lower bits of the BAR 0 base address. */
	uint32_t reserved_4_22                : 19;
	uint32_t pf                           : 1;  /**< Prefetchable. This field is writable through PEM()_CFG_WR. However, the application
                                                         must not change this field. */
	uint32_t typ                          : 2;  /**< BAR type.
                                                         0x0 = 32-bit BAR.
                                                         0x2 = 64-bit BAR.
                                                         This field is writable through PEM()_CFG_WR. However, the application must not change
                                                         this field. */
	uint32_t mspc                         : 1;  /**< Memory space indicator.
                                                         0 = BAR 0 is a memory BAR.
                                                         1 = BAR 0 is an I/O BAR.
                                                         This field is writable through PEM()_CFG_WR. However, the application must not change
                                                         this field. */
#else
	uint32_t mspc                         : 1;
	uint32_t typ                          : 2;
	uint32_t pf                           : 1;
	uint32_t reserved_4_22                : 19;
	uint32_t lbab                         : 9;
#endif
	} cn73xx;
	struct cvmx_pcieepx_cfg004_cn73xx     cn78xx;
	struct cvmx_pcieepx_cfg004_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lbab                         : 17; /**< Lower bits of the BAR 0 base address. */
	uint32_t reserved_4_14                : 11;
	uint32_t pf                           : 1;  /**< Prefetchable. This field is writable through PEM()_CFG_WR. However, the application
                                                         must not change this field. */
	uint32_t typ                          : 2;  /**< BAR type.
                                                         0x0 = 32-bit BAR.
                                                         0x2 = 64-bit BAR.
                                                         This field is writable through PEM()_CFG_WR. However, the application must not change
                                                         this field. */
	uint32_t mspc                         : 1;  /**< Memory space indicator.
                                                         0 = BAR 0 is a memory BAR.
                                                         1 = BAR 0 is an I/O BAR.
                                                         This field is writable through PEM()_CFG_WR. However, the application must not change
                                                         this field. */
#else
	uint32_t mspc                         : 1;
	uint32_t typ                          : 2;
	uint32_t pf                           : 1;
	uint32_t reserved_4_14                : 11;
	uint32_t lbab                         : 17;
#endif
	} cn78xxp1;
	struct cvmx_pcieepx_cfg004_cn52xx     cnf71xx;
	struct cvmx_pcieepx_cfg004_cn73xx     cnf75xx;
};
typedef union cvmx_pcieepx_cfg004 cvmx_pcieepx_cfg004_t;

/**
 * cvmx_pcieep#_cfg004_mask
 *
 * The BAR 0 mask register is invisible to host software and not readable from the application.
 * The BAR 0 mask register is only writable through PEM()_CFG_WR.
 */
union cvmx_pcieepx_cfg004_mask {
	uint32_t u32;
	struct cvmx_pcieepx_cfg004_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lmask                        : 31; /**< BAR mask low. */
	uint32_t enb                          : 1;  /**< BAR enable. 0: BAR 0 is disabled, 1: BAR 0 is enabled.
                                                         Bit 0 is interpreted as BAR enable when writing to the BAR mask register rather than as a
                                                         mask bit because bit 0 of a BAR is always masked from writing by host software. Bit 0 must
                                                         be written prior to writing the other mask bits. */
#else
	uint32_t enb                          : 1;
	uint32_t lmask                        : 31;
#endif
	} s;
	struct cvmx_pcieepx_cfg004_mask_s     cn52xx;
	struct cvmx_pcieepx_cfg004_mask_s     cn52xxp1;
	struct cvmx_pcieepx_cfg004_mask_s     cn56xx;
	struct cvmx_pcieepx_cfg004_mask_s     cn56xxp1;
	struct cvmx_pcieepx_cfg004_mask_s     cn61xx;
	struct cvmx_pcieepx_cfg004_mask_s     cn63xx;
	struct cvmx_pcieepx_cfg004_mask_s     cn63xxp1;
	struct cvmx_pcieepx_cfg004_mask_s     cn66xx;
	struct cvmx_pcieepx_cfg004_mask_s     cn68xx;
	struct cvmx_pcieepx_cfg004_mask_s     cn68xxp1;
	struct cvmx_pcieepx_cfg004_mask_s     cn70xx;
	struct cvmx_pcieepx_cfg004_mask_s     cn70xxp1;
	struct cvmx_pcieepx_cfg004_mask_s     cn73xx;
	struct cvmx_pcieepx_cfg004_mask_s     cn78xx;
	struct cvmx_pcieepx_cfg004_mask_s     cn78xxp1;
	struct cvmx_pcieepx_cfg004_mask_s     cnf71xx;
	struct cvmx_pcieepx_cfg004_mask_s     cnf75xx;
};
typedef union cvmx_pcieepx_cfg004_mask cvmx_pcieepx_cfg004_mask_t;

/**
 * cvmx_pcieep#_cfg005
 *
 * This register contains the sixth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg005 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg005_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ubab                         : 32; /**< Contains the upper 32 bits of the BAR 0 base address. */
#else
	uint32_t ubab                         : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg005_s          cn52xx;
	struct cvmx_pcieepx_cfg005_s          cn52xxp1;
	struct cvmx_pcieepx_cfg005_s          cn56xx;
	struct cvmx_pcieepx_cfg005_s          cn56xxp1;
	struct cvmx_pcieepx_cfg005_s          cn61xx;
	struct cvmx_pcieepx_cfg005_s          cn63xx;
	struct cvmx_pcieepx_cfg005_s          cn63xxp1;
	struct cvmx_pcieepx_cfg005_s          cn66xx;
	struct cvmx_pcieepx_cfg005_s          cn68xx;
	struct cvmx_pcieepx_cfg005_s          cn68xxp1;
	struct cvmx_pcieepx_cfg005_s          cn70xx;
	struct cvmx_pcieepx_cfg005_s          cn70xxp1;
	struct cvmx_pcieepx_cfg005_s          cn73xx;
	struct cvmx_pcieepx_cfg005_s          cn78xx;
	struct cvmx_pcieepx_cfg005_s          cn78xxp1;
	struct cvmx_pcieepx_cfg005_s          cnf71xx;
	struct cvmx_pcieepx_cfg005_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg005 cvmx_pcieepx_cfg005_t;

/**
 * cvmx_pcieep#_cfg005_mask
 *
 * The BAR 0 mask register is invisible to host software and not readable from the application.
 * The BAR 0 mask register is only writable through PEM()_CFG_WR.
 */
union cvmx_pcieepx_cfg005_mask {
	uint32_t u32;
	struct cvmx_pcieepx_cfg005_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t umask                        : 32; /**< BAR mask high. */
#else
	uint32_t umask                        : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg005_mask_s     cn52xx;
	struct cvmx_pcieepx_cfg005_mask_s     cn52xxp1;
	struct cvmx_pcieepx_cfg005_mask_s     cn56xx;
	struct cvmx_pcieepx_cfg005_mask_s     cn56xxp1;
	struct cvmx_pcieepx_cfg005_mask_s     cn61xx;
	struct cvmx_pcieepx_cfg005_mask_s     cn63xx;
	struct cvmx_pcieepx_cfg005_mask_s     cn63xxp1;
	struct cvmx_pcieepx_cfg005_mask_s     cn66xx;
	struct cvmx_pcieepx_cfg005_mask_s     cn68xx;
	struct cvmx_pcieepx_cfg005_mask_s     cn68xxp1;
	struct cvmx_pcieepx_cfg005_mask_s     cn70xx;
	struct cvmx_pcieepx_cfg005_mask_s     cn70xxp1;
	struct cvmx_pcieepx_cfg005_mask_s     cn73xx;
	struct cvmx_pcieepx_cfg005_mask_s     cn78xx;
	struct cvmx_pcieepx_cfg005_mask_s     cn78xxp1;
	struct cvmx_pcieepx_cfg005_mask_s     cnf71xx;
	struct cvmx_pcieepx_cfg005_mask_s     cnf75xx;
};
typedef union cvmx_pcieepx_cfg005_mask cvmx_pcieepx_cfg005_mask_t;

/**
 * cvmx_pcieep#_cfg006
 *
 * This register contains the seventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg006 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg006_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lbab                         : 6;  /**< Lower bits of the BAR 1 base address. */
	uint32_t reserved_4_25                : 22;
	uint32_t pf                           : 1;  /**< Prefetchable. This field is writable through PEM()_CFG_WR. However, the application
                                                         must not change this field. */
	uint32_t typ                          : 2;  /**< BAR type.
                                                         0x0 = 32-bit BAR.
                                                         0x2 = 64-bit BAR.
                                                         This field is writable through PEM()_CFG_WR. However, the application must not change
                                                         this field. */
	uint32_t mspc                         : 1;  /**< Memory space indicator.
                                                         0 = BAR 1 is a memory BAR.
                                                         1 = BAR 1 is an I/O BAR.
                                                         This field is writable through PEM()_CFG_WR. However, the application must not change
                                                         this field. */
#else
	uint32_t mspc                         : 1;
	uint32_t typ                          : 2;
	uint32_t pf                           : 1;
	uint32_t reserved_4_25                : 22;
	uint32_t lbab                         : 6;
#endif
	} s;
	struct cvmx_pcieepx_cfg006_s          cn52xx;
	struct cvmx_pcieepx_cfg006_s          cn52xxp1;
	struct cvmx_pcieepx_cfg006_s          cn56xx;
	struct cvmx_pcieepx_cfg006_s          cn56xxp1;
	struct cvmx_pcieepx_cfg006_s          cn61xx;
	struct cvmx_pcieepx_cfg006_s          cn63xx;
	struct cvmx_pcieepx_cfg006_s          cn63xxp1;
	struct cvmx_pcieepx_cfg006_s          cn66xx;
	struct cvmx_pcieepx_cfg006_s          cn68xx;
	struct cvmx_pcieepx_cfg006_s          cn68xxp1;
	struct cvmx_pcieepx_cfg006_s          cn70xx;
	struct cvmx_pcieepx_cfg006_s          cn70xxp1;
	struct cvmx_pcieepx_cfg006_s          cn73xx;
	struct cvmx_pcieepx_cfg006_s          cn78xx;
	struct cvmx_pcieepx_cfg006_s          cn78xxp1;
	struct cvmx_pcieepx_cfg006_s          cnf71xx;
	struct cvmx_pcieepx_cfg006_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg006 cvmx_pcieepx_cfg006_t;

/**
 * cvmx_pcieep#_cfg006_mask
 *
 * The BAR 1 mask register is invisible to host software and not readable from the application.
 * The BAR 1 mask register is only writable through PEM()_CFG_WR.
 */
union cvmx_pcieepx_cfg006_mask {
	uint32_t u32;
	struct cvmx_pcieepx_cfg006_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lmask                        : 31; /**< BAR mask low */
	uint32_t enb                          : 1;  /**< BAR enable. 0: BAR 1 is disabled, 1: BAR 1 is enabled.
                                                         Bit 0 is interpreted as BAR enable when writing to the BAR mask register rather than as a
                                                         mask bit because bit 0 of a BAR is always masked from writing by host software. Bit 0 must
                                                         be written prior to writing the other mask bits. */
#else
	uint32_t enb                          : 1;
	uint32_t lmask                        : 31;
#endif
	} s;
	struct cvmx_pcieepx_cfg006_mask_s     cn52xx;
	struct cvmx_pcieepx_cfg006_mask_s     cn52xxp1;
	struct cvmx_pcieepx_cfg006_mask_s     cn56xx;
	struct cvmx_pcieepx_cfg006_mask_s     cn56xxp1;
	struct cvmx_pcieepx_cfg006_mask_s     cn61xx;
	struct cvmx_pcieepx_cfg006_mask_s     cn63xx;
	struct cvmx_pcieepx_cfg006_mask_s     cn63xxp1;
	struct cvmx_pcieepx_cfg006_mask_s     cn66xx;
	struct cvmx_pcieepx_cfg006_mask_s     cn68xx;
	struct cvmx_pcieepx_cfg006_mask_s     cn68xxp1;
	struct cvmx_pcieepx_cfg006_mask_s     cn70xx;
	struct cvmx_pcieepx_cfg006_mask_s     cn70xxp1;
	struct cvmx_pcieepx_cfg006_mask_s     cn73xx;
	struct cvmx_pcieepx_cfg006_mask_s     cn78xx;
	struct cvmx_pcieepx_cfg006_mask_s     cn78xxp1;
	struct cvmx_pcieepx_cfg006_mask_s     cnf71xx;
	struct cvmx_pcieepx_cfg006_mask_s     cnf75xx;
};
typedef union cvmx_pcieepx_cfg006_mask cvmx_pcieepx_cfg006_mask_t;

/**
 * cvmx_pcieep#_cfg007
 *
 * This register contains the eighth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg007 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg007_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ubab                         : 32; /**< Contains the upper 32 bits of the BAR 1 base address. */
#else
	uint32_t ubab                         : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg007_s          cn52xx;
	struct cvmx_pcieepx_cfg007_s          cn52xxp1;
	struct cvmx_pcieepx_cfg007_s          cn56xx;
	struct cvmx_pcieepx_cfg007_s          cn56xxp1;
	struct cvmx_pcieepx_cfg007_s          cn61xx;
	struct cvmx_pcieepx_cfg007_s          cn63xx;
	struct cvmx_pcieepx_cfg007_s          cn63xxp1;
	struct cvmx_pcieepx_cfg007_s          cn66xx;
	struct cvmx_pcieepx_cfg007_s          cn68xx;
	struct cvmx_pcieepx_cfg007_s          cn68xxp1;
	struct cvmx_pcieepx_cfg007_s          cn70xx;
	struct cvmx_pcieepx_cfg007_s          cn70xxp1;
	struct cvmx_pcieepx_cfg007_s          cn73xx;
	struct cvmx_pcieepx_cfg007_s          cn78xx;
	struct cvmx_pcieepx_cfg007_s          cn78xxp1;
	struct cvmx_pcieepx_cfg007_s          cnf71xx;
	struct cvmx_pcieepx_cfg007_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg007 cvmx_pcieepx_cfg007_t;

/**
 * cvmx_pcieep#_cfg007_mask
 *
 * The BAR 1 mask register is invisible to host software and not readable from the application.
 * The BAR 1 mask register is only writable through PEM()_CFG_WR.
 */
union cvmx_pcieepx_cfg007_mask {
	uint32_t u32;
	struct cvmx_pcieepx_cfg007_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t umask                        : 32; /**< BAR mask high. */
#else
	uint32_t umask                        : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg007_mask_s     cn52xx;
	struct cvmx_pcieepx_cfg007_mask_s     cn52xxp1;
	struct cvmx_pcieepx_cfg007_mask_s     cn56xx;
	struct cvmx_pcieepx_cfg007_mask_s     cn56xxp1;
	struct cvmx_pcieepx_cfg007_mask_s     cn61xx;
	struct cvmx_pcieepx_cfg007_mask_s     cn63xx;
	struct cvmx_pcieepx_cfg007_mask_s     cn63xxp1;
	struct cvmx_pcieepx_cfg007_mask_s     cn66xx;
	struct cvmx_pcieepx_cfg007_mask_s     cn68xx;
	struct cvmx_pcieepx_cfg007_mask_s     cn68xxp1;
	struct cvmx_pcieepx_cfg007_mask_s     cn70xx;
	struct cvmx_pcieepx_cfg007_mask_s     cn70xxp1;
	struct cvmx_pcieepx_cfg007_mask_s     cn73xx;
	struct cvmx_pcieepx_cfg007_mask_s     cn78xx;
	struct cvmx_pcieepx_cfg007_mask_s     cn78xxp1;
	struct cvmx_pcieepx_cfg007_mask_s     cnf71xx;
	struct cvmx_pcieepx_cfg007_mask_s     cnf75xx;
};
typedef union cvmx_pcieepx_cfg007_mask cvmx_pcieepx_cfg007_mask_t;

/**
 * cvmx_pcieep#_cfg008
 *
 * This register contains the ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg008 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg008_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lbab                         : 12; /**< Lower bits of the BAR 2 base address */
	uint32_t reserved_4_19                : 16;
	uint32_t pf                           : 1;  /**< Prefetchable. This field is writable through PEM()_CFG_WR. However, the application
                                                         must not change this field. */
	uint32_t typ                          : 2;  /**< BAR type.
                                                         0x0 = 32-bit BAR.
                                                         0x2 = 64-bit BAR.
                                                         This field is writable through PEM()_CFG_WR. However, the application must not change
                                                         this field. */
	uint32_t mspc                         : 1;  /**< Memory space indicator.
                                                         0 = BAR 2 is a memory BAR.
                                                         1 = BAR 2 is an I/O BAR.
                                                         This field is writable through PEM()_CFG_WR. However, the application must not change
                                                         this field. */
#else
	uint32_t mspc                         : 1;
	uint32_t typ                          : 2;
	uint32_t pf                           : 1;
	uint32_t reserved_4_19                : 16;
	uint32_t lbab                         : 12;
#endif
	} s;
	struct cvmx_pcieepx_cfg008_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_4_31                : 28;
	uint32_t pf                           : 1;  /**< Prefetchable
                                                         This field is writable through PESC(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t typ                          : 2;  /**< BAR type
                                                            o 00 = 32-bit BAR
                                                            o 10 = 64-bit BAR
                                                         This field is writable through PESC(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t mspc                         : 1;  /**< Memory Space Indicator
                                                            o 0 = BAR 0 is a memory BAR
                                                            o 1 = BAR 0 is an I/O BAR
                                                         This field is writable through PESC(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
#else
	uint32_t mspc                         : 1;
	uint32_t typ                          : 2;
	uint32_t pf                           : 1;
	uint32_t reserved_4_31                : 28;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg008_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg008_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg008_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg008_cn52xx     cn61xx;
	struct cvmx_pcieepx_cfg008_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg008_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg008_cn52xx     cn66xx;
	struct cvmx_pcieepx_cfg008_cn52xx     cn68xx;
	struct cvmx_pcieepx_cfg008_cn52xx     cn68xxp1;
	struct cvmx_pcieepx_cfg008_s          cn70xx;
	struct cvmx_pcieepx_cfg008_s          cn70xxp1;
	struct cvmx_pcieepx_cfg008_s          cn73xx;
	struct cvmx_pcieepx_cfg008_s          cn78xx;
	struct cvmx_pcieepx_cfg008_s          cn78xxp1;
	struct cvmx_pcieepx_cfg008_cn52xx     cnf71xx;
	struct cvmx_pcieepx_cfg008_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg008 cvmx_pcieepx_cfg008_t;

/**
 * cvmx_pcieep#_cfg008_mask
 *
 * The BAR 2 mask register is invisible to host software and not readable from the application.
 * The BAR 2 mask register is only writable through PEM()_CFG_WR.
 */
union cvmx_pcieepx_cfg008_mask {
	uint32_t u32;
	struct cvmx_pcieepx_cfg008_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lmask                        : 31; /**< BAR mask low. */
	uint32_t enb                          : 1;  /**< BAR enable. 0: BAR 2 is disabled, 1: BAR 2 is enabled.
                                                         Bit 0 is interpreted as BAR enable when writing to the BAR mask register rather than as a
                                                         mask bit because bit 0 of a BAR is always masked from writing by host software. Bit 0 must
                                                         be written prior to writing the other mask bits. */
#else
	uint32_t enb                          : 1;
	uint32_t lmask                        : 31;
#endif
	} s;
	struct cvmx_pcieepx_cfg008_mask_s     cn52xx;
	struct cvmx_pcieepx_cfg008_mask_s     cn52xxp1;
	struct cvmx_pcieepx_cfg008_mask_s     cn56xx;
	struct cvmx_pcieepx_cfg008_mask_s     cn56xxp1;
	struct cvmx_pcieepx_cfg008_mask_s     cn61xx;
	struct cvmx_pcieepx_cfg008_mask_s     cn63xx;
	struct cvmx_pcieepx_cfg008_mask_s     cn63xxp1;
	struct cvmx_pcieepx_cfg008_mask_s     cn66xx;
	struct cvmx_pcieepx_cfg008_mask_s     cn68xx;
	struct cvmx_pcieepx_cfg008_mask_s     cn68xxp1;
	struct cvmx_pcieepx_cfg008_mask_s     cn70xx;
	struct cvmx_pcieepx_cfg008_mask_s     cn70xxp1;
	struct cvmx_pcieepx_cfg008_mask_s     cn73xx;
	struct cvmx_pcieepx_cfg008_mask_s     cn78xx;
	struct cvmx_pcieepx_cfg008_mask_s     cn78xxp1;
	struct cvmx_pcieepx_cfg008_mask_s     cnf71xx;
	struct cvmx_pcieepx_cfg008_mask_s     cnf75xx;
};
typedef union cvmx_pcieepx_cfg008_mask cvmx_pcieepx_cfg008_mask_t;

/**
 * cvmx_pcieep#_cfg009
 *
 * This register contains the tenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg009 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg009_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg009_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ubab                         : 25; /**< Contains the upper 32 bits of the BAR 2 base address. */
	uint32_t reserved_0_6                 : 7;
#else
	uint32_t reserved_0_6                 : 7;
	uint32_t ubab                         : 25;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg009_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg009_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg009_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg009_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ubab                         : 23; /**< Contains the upper 32 bits of the BAR 2 base address. */
	uint32_t reserved_0_8                 : 9;
#else
	uint32_t reserved_0_8                 : 9;
	uint32_t ubab                         : 23;
#endif
	} cn61xx;
	struct cvmx_pcieepx_cfg009_cn61xx     cn63xx;
	struct cvmx_pcieepx_cfg009_cn61xx     cn63xxp1;
	struct cvmx_pcieepx_cfg009_cn61xx     cn66xx;
	struct cvmx_pcieepx_cfg009_cn61xx     cn68xx;
	struct cvmx_pcieepx_cfg009_cn61xx     cn68xxp1;
	struct cvmx_pcieepx_cfg009_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ubab                         : 32; /**< Contains the upper 32 bits of the BAR 2 base address. */
#else
	uint32_t ubab                         : 32;
#endif
	} cn70xx;
	struct cvmx_pcieepx_cfg009_cn70xx     cn70xxp1;
	struct cvmx_pcieepx_cfg009_cn70xx     cn73xx;
	struct cvmx_pcieepx_cfg009_cn70xx     cn78xx;
	struct cvmx_pcieepx_cfg009_cn70xx     cn78xxp1;
	struct cvmx_pcieepx_cfg009_cn61xx     cnf71xx;
	struct cvmx_pcieepx_cfg009_cn70xx     cnf75xx;
};
typedef union cvmx_pcieepx_cfg009 cvmx_pcieepx_cfg009_t;

/**
 * cvmx_pcieep#_cfg009_mask
 *
 * The BAR 2 mask register is invisible to host software and not readable from the application.
 * The BAR 2 mask register is only writable through PEM()_CFG_WR.
 */
union cvmx_pcieepx_cfg009_mask {
	uint32_t u32;
	struct cvmx_pcieepx_cfg009_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t umask                        : 32; /**< BAR mask high. */
#else
	uint32_t umask                        : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg009_mask_s     cn52xx;
	struct cvmx_pcieepx_cfg009_mask_s     cn52xxp1;
	struct cvmx_pcieepx_cfg009_mask_s     cn56xx;
	struct cvmx_pcieepx_cfg009_mask_s     cn56xxp1;
	struct cvmx_pcieepx_cfg009_mask_s     cn61xx;
	struct cvmx_pcieepx_cfg009_mask_s     cn63xx;
	struct cvmx_pcieepx_cfg009_mask_s     cn63xxp1;
	struct cvmx_pcieepx_cfg009_mask_s     cn66xx;
	struct cvmx_pcieepx_cfg009_mask_s     cn68xx;
	struct cvmx_pcieepx_cfg009_mask_s     cn68xxp1;
	struct cvmx_pcieepx_cfg009_mask_s     cn70xx;
	struct cvmx_pcieepx_cfg009_mask_s     cn70xxp1;
	struct cvmx_pcieepx_cfg009_mask_s     cn73xx;
	struct cvmx_pcieepx_cfg009_mask_s     cn78xx;
	struct cvmx_pcieepx_cfg009_mask_s     cn78xxp1;
	struct cvmx_pcieepx_cfg009_mask_s     cnf71xx;
	struct cvmx_pcieepx_cfg009_mask_s     cnf75xx;
};
typedef union cvmx_pcieepx_cfg009_mask cvmx_pcieepx_cfg009_mask_t;

/**
 * cvmx_pcieep#_cfg010
 *
 * This register contains the eleventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg010 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg010_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t cisp                         : 32; /**< CardBus CIS pointer. Optional, writable through PEM()_CFG_WR. */
#else
	uint32_t cisp                         : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg010_s          cn52xx;
	struct cvmx_pcieepx_cfg010_s          cn52xxp1;
	struct cvmx_pcieepx_cfg010_s          cn56xx;
	struct cvmx_pcieepx_cfg010_s          cn56xxp1;
	struct cvmx_pcieepx_cfg010_s          cn61xx;
	struct cvmx_pcieepx_cfg010_s          cn63xx;
	struct cvmx_pcieepx_cfg010_s          cn63xxp1;
	struct cvmx_pcieepx_cfg010_s          cn66xx;
	struct cvmx_pcieepx_cfg010_s          cn68xx;
	struct cvmx_pcieepx_cfg010_s          cn68xxp1;
	struct cvmx_pcieepx_cfg010_s          cn70xx;
	struct cvmx_pcieepx_cfg010_s          cn70xxp1;
	struct cvmx_pcieepx_cfg010_s          cn73xx;
	struct cvmx_pcieepx_cfg010_s          cn78xx;
	struct cvmx_pcieepx_cfg010_s          cn78xxp1;
	struct cvmx_pcieepx_cfg010_s          cnf71xx;
	struct cvmx_pcieepx_cfg010_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg010 cvmx_pcieepx_cfg010_t;

/**
 * cvmx_pcieep#_cfg011
 *
 * This register contains the twelfth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg011 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg011_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ssid                         : 16; /**< Subsystem ID.
                                                         Assigned by PCI-SIG, writable through PEM()_CFG_WR. */
	uint32_t ssvid                        : 16; /**< Subsystem vendor ID.
                                                         Assigned by PCI-SIG, writable through PEM()_CFG_WR. */
#else
	uint32_t ssvid                        : 16;
	uint32_t ssid                         : 16;
#endif
	} s;
	struct cvmx_pcieepx_cfg011_s          cn52xx;
	struct cvmx_pcieepx_cfg011_s          cn52xxp1;
	struct cvmx_pcieepx_cfg011_s          cn56xx;
	struct cvmx_pcieepx_cfg011_s          cn56xxp1;
	struct cvmx_pcieepx_cfg011_s          cn61xx;
	struct cvmx_pcieepx_cfg011_s          cn63xx;
	struct cvmx_pcieepx_cfg011_s          cn63xxp1;
	struct cvmx_pcieepx_cfg011_s          cn66xx;
	struct cvmx_pcieepx_cfg011_s          cn68xx;
	struct cvmx_pcieepx_cfg011_s          cn68xxp1;
	struct cvmx_pcieepx_cfg011_s          cn70xx;
	struct cvmx_pcieepx_cfg011_s          cn70xxp1;
	struct cvmx_pcieepx_cfg011_s          cn73xx;
	struct cvmx_pcieepx_cfg011_s          cn78xx;
	struct cvmx_pcieepx_cfg011_s          cn78xxp1;
	struct cvmx_pcieepx_cfg011_s          cnf71xx;
	struct cvmx_pcieepx_cfg011_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg011 cvmx_pcieepx_cfg011_t;

/**
 * cvmx_pcieep#_cfg012
 *
 * This register contains the thirteenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg012 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg012_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t eraddr                       : 16; /**< Expansion ROM address. */
	uint32_t reserved_1_15                : 15;
	uint32_t er_en                        : 1;  /**< Expansion ROM enable. */
#else
	uint32_t er_en                        : 1;
	uint32_t reserved_1_15                : 15;
	uint32_t eraddr                       : 16;
#endif
	} s;
	struct cvmx_pcieepx_cfg012_s          cn52xx;
	struct cvmx_pcieepx_cfg012_s          cn52xxp1;
	struct cvmx_pcieepx_cfg012_s          cn56xx;
	struct cvmx_pcieepx_cfg012_s          cn56xxp1;
	struct cvmx_pcieepx_cfg012_s          cn61xx;
	struct cvmx_pcieepx_cfg012_s          cn63xx;
	struct cvmx_pcieepx_cfg012_s          cn63xxp1;
	struct cvmx_pcieepx_cfg012_s          cn66xx;
	struct cvmx_pcieepx_cfg012_s          cn68xx;
	struct cvmx_pcieepx_cfg012_s          cn68xxp1;
	struct cvmx_pcieepx_cfg012_s          cn70xx;
	struct cvmx_pcieepx_cfg012_s          cn70xxp1;
	struct cvmx_pcieepx_cfg012_s          cn73xx;
	struct cvmx_pcieepx_cfg012_s          cn78xx;
	struct cvmx_pcieepx_cfg012_s          cn78xxp1;
	struct cvmx_pcieepx_cfg012_s          cnf71xx;
	struct cvmx_pcieepx_cfg012_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg012 cvmx_pcieepx_cfg012_t;

/**
 * cvmx_pcieep#_cfg012_mask
 *
 * The ROM mask register is invisible to host software and not readable from the application. The
 * ROM mask register is only writable through PEM()_CFG_WR.
 */
union cvmx_pcieepx_cfg012_mask {
	uint32_t u32;
	struct cvmx_pcieepx_cfg012_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t mask                         : 31; /**< BAR mask low */
	uint32_t enb                          : 1;  /**< BAR enable. 0 = BAR ROM is disabled; 1 = BAR ROM is enabled. Bit 0 is interpreted as BAR
                                                         enable when writing to the BAR mask register rather than as a mask bit because bit 0 of a
                                                         BAR is always masked from writing by host software. Bit 0 must be written prior to writing
                                                         the other mask bits. */
#else
	uint32_t enb                          : 1;
	uint32_t mask                         : 31;
#endif
	} s;
	struct cvmx_pcieepx_cfg012_mask_s     cn52xx;
	struct cvmx_pcieepx_cfg012_mask_s     cn52xxp1;
	struct cvmx_pcieepx_cfg012_mask_s     cn56xx;
	struct cvmx_pcieepx_cfg012_mask_s     cn56xxp1;
	struct cvmx_pcieepx_cfg012_mask_s     cn61xx;
	struct cvmx_pcieepx_cfg012_mask_s     cn63xx;
	struct cvmx_pcieepx_cfg012_mask_s     cn63xxp1;
	struct cvmx_pcieepx_cfg012_mask_s     cn66xx;
	struct cvmx_pcieepx_cfg012_mask_s     cn68xx;
	struct cvmx_pcieepx_cfg012_mask_s     cn68xxp1;
	struct cvmx_pcieepx_cfg012_mask_s     cn70xx;
	struct cvmx_pcieepx_cfg012_mask_s     cn70xxp1;
	struct cvmx_pcieepx_cfg012_mask_s     cn73xx;
	struct cvmx_pcieepx_cfg012_mask_s     cn78xx;
	struct cvmx_pcieepx_cfg012_mask_s     cn78xxp1;
	struct cvmx_pcieepx_cfg012_mask_s     cnf71xx;
	struct cvmx_pcieepx_cfg012_mask_s     cnf75xx;
};
typedef union cvmx_pcieepx_cfg012_mask cvmx_pcieepx_cfg012_mask_t;

/**
 * cvmx_pcieep#_cfg013
 *
 * This register contains the fourteenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg013 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg013_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_8_31                : 24;
	uint32_t cp                           : 8;  /**< First capability pointer. Points to power management capability structure by default,
                                                         writable through PEM()_CFG_WR. However, the application must not change this field. */
#else
	uint32_t cp                           : 8;
	uint32_t reserved_8_31                : 24;
#endif
	} s;
	struct cvmx_pcieepx_cfg013_s          cn52xx;
	struct cvmx_pcieepx_cfg013_s          cn52xxp1;
	struct cvmx_pcieepx_cfg013_s          cn56xx;
	struct cvmx_pcieepx_cfg013_s          cn56xxp1;
	struct cvmx_pcieepx_cfg013_s          cn61xx;
	struct cvmx_pcieepx_cfg013_s          cn63xx;
	struct cvmx_pcieepx_cfg013_s          cn63xxp1;
	struct cvmx_pcieepx_cfg013_s          cn66xx;
	struct cvmx_pcieepx_cfg013_s          cn68xx;
	struct cvmx_pcieepx_cfg013_s          cn68xxp1;
	struct cvmx_pcieepx_cfg013_s          cn70xx;
	struct cvmx_pcieepx_cfg013_s          cn70xxp1;
	struct cvmx_pcieepx_cfg013_s          cn73xx;
	struct cvmx_pcieepx_cfg013_s          cn78xx;
	struct cvmx_pcieepx_cfg013_s          cn78xxp1;
	struct cvmx_pcieepx_cfg013_s          cnf71xx;
	struct cvmx_pcieepx_cfg013_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg013 cvmx_pcieepx_cfg013_t;

/**
 * cvmx_pcieep#_cfg015
 *
 * This register contains the sixteenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg015 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg015_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ml                           : 8;  /**< Maximum latency (hardwired to 0x0). */
	uint32_t mg                           : 8;  /**< Minimum grant (hardwired to 0x0). */
	uint32_t inta                         : 8;  /**< Interrupt pin. Identifies the legacy interrupt message that the device (or device
                                                         function) uses. The interrupt pin register is writable through PEM()_CFG_WR. In a
                                                         single-function configuration, only INTA is used. Therefore, the application must not
                                                         change this field. */
	uint32_t il                           : 8;  /**< Interrupt line. */
#else
	uint32_t il                           : 8;
	uint32_t inta                         : 8;
	uint32_t mg                           : 8;
	uint32_t ml                           : 8;
#endif
	} s;
	struct cvmx_pcieepx_cfg015_s          cn52xx;
	struct cvmx_pcieepx_cfg015_s          cn52xxp1;
	struct cvmx_pcieepx_cfg015_s          cn56xx;
	struct cvmx_pcieepx_cfg015_s          cn56xxp1;
	struct cvmx_pcieepx_cfg015_s          cn61xx;
	struct cvmx_pcieepx_cfg015_s          cn63xx;
	struct cvmx_pcieepx_cfg015_s          cn63xxp1;
	struct cvmx_pcieepx_cfg015_s          cn66xx;
	struct cvmx_pcieepx_cfg015_s          cn68xx;
	struct cvmx_pcieepx_cfg015_s          cn68xxp1;
	struct cvmx_pcieepx_cfg015_s          cn70xx;
	struct cvmx_pcieepx_cfg015_s          cn70xxp1;
	struct cvmx_pcieepx_cfg015_s          cn73xx;
	struct cvmx_pcieepx_cfg015_s          cn78xx;
	struct cvmx_pcieepx_cfg015_s          cn78xxp1;
	struct cvmx_pcieepx_cfg015_s          cnf71xx;
	struct cvmx_pcieepx_cfg015_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg015 cvmx_pcieepx_cfg015_t;

/**
 * cvmx_pcieep#_cfg016
 *
 * This register contains the seventeenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg016 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg016_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pmes                         : 5;  /**< PME_Support.
                                                         _ Bit 11: If set, PME Messages can be generated from D0.
                                                         _ Bit 12: If set, PME Messages can be generated from D1.
                                                         _ Bit 13: If set, PME Messages can be generated from D2.
                                                         _ Bit 14: If set, PME Messages can be generated from D3hot.
                                                         _ Bit 15: If set, PME Messages can be generated from D3cold.
                                                         The PME_Support field is writable through PEM()_CFG_WR. However, the application must
                                                         not change this field. */
	uint32_t d2s                          : 1;  /**< D2 support, writable through PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t d1s                          : 1;  /**< D1 support, writable through PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t auxc                         : 3;  /**< AUX current, writable through PEM()_CFG_WR. However, the application must not change
                                                         this field. */
	uint32_t dsi                          : 1;  /**< Device specific initialization (DSI), writable through
                                                         PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t reserved_20_20               : 1;
	uint32_t pme_clock                    : 1;  /**< PME clock, hardwired to 0. */
	uint32_t pmsv                         : 3;  /**< Power management specification version, writable through
                                                         PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t ncp                          : 8;  /**< Next capability pointer. Points to the MSI capabilities by default,
                                                         writable through PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t pmcid                        : 8;  /**< Power management capability ID. */
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
	struct cvmx_pcieepx_cfg016_s          cn52xx;
	struct cvmx_pcieepx_cfg016_s          cn52xxp1;
	struct cvmx_pcieepx_cfg016_s          cn56xx;
	struct cvmx_pcieepx_cfg016_s          cn56xxp1;
	struct cvmx_pcieepx_cfg016_s          cn61xx;
	struct cvmx_pcieepx_cfg016_s          cn63xx;
	struct cvmx_pcieepx_cfg016_s          cn63xxp1;
	struct cvmx_pcieepx_cfg016_s          cn66xx;
	struct cvmx_pcieepx_cfg016_s          cn68xx;
	struct cvmx_pcieepx_cfg016_s          cn68xxp1;
	struct cvmx_pcieepx_cfg016_s          cn70xx;
	struct cvmx_pcieepx_cfg016_s          cn70xxp1;
	struct cvmx_pcieepx_cfg016_s          cn73xx;
	struct cvmx_pcieepx_cfg016_s          cn78xx;
	struct cvmx_pcieepx_cfg016_s          cn78xxp1;
	struct cvmx_pcieepx_cfg016_s          cnf71xx;
	struct cvmx_pcieepx_cfg016_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg016 cvmx_pcieepx_cfg016_t;

/**
 * cvmx_pcieep#_cfg017
 *
 * This register contains the eighteenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg017 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg017_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pmdia                        : 8;  /**< Data register for additional information (not supported). */
	uint32_t bpccee                       : 1;  /**< Bus power/clock control enable, hardwired to 0. */
	uint32_t bd3h                         : 1;  /**< B2/B3 support, hardwired to 0. */
	uint32_t reserved_16_21               : 6;
	uint32_t pmess                        : 1;  /**< PME status. Indicates whether or not a previously enabled PME event occurred. */
	uint32_t pmedsia                      : 2;  /**< Data scale (not supported). */
	uint32_t pmds                         : 4;  /**< Data select (not supported). */
	uint32_t pmeens                       : 1;  /**< PME enable. A value of 1 indicates that the device is enabled to generate PME. */
	uint32_t reserved_4_7                 : 4;
	uint32_t nsr                          : 1;  /**< No soft reset, writable through PEM()_CFG_WR. */
	uint32_t reserved_2_2                 : 1;
	uint32_t ps                           : 2;  /**< Power state. Controls the device power state:
                                                         0x0 = D0.
                                                         0x1 = D1.
                                                         0x2 = D2.
                                                         0x3 = D3.
                                                         The written value is ignored if the specific state is not supported. */
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
	struct cvmx_pcieepx_cfg017_s          cn52xx;
	struct cvmx_pcieepx_cfg017_s          cn52xxp1;
	struct cvmx_pcieepx_cfg017_s          cn56xx;
	struct cvmx_pcieepx_cfg017_s          cn56xxp1;
	struct cvmx_pcieepx_cfg017_s          cn61xx;
	struct cvmx_pcieepx_cfg017_s          cn63xx;
	struct cvmx_pcieepx_cfg017_s          cn63xxp1;
	struct cvmx_pcieepx_cfg017_s          cn66xx;
	struct cvmx_pcieepx_cfg017_s          cn68xx;
	struct cvmx_pcieepx_cfg017_s          cn68xxp1;
	struct cvmx_pcieepx_cfg017_s          cn70xx;
	struct cvmx_pcieepx_cfg017_s          cn70xxp1;
	struct cvmx_pcieepx_cfg017_s          cn73xx;
	struct cvmx_pcieepx_cfg017_s          cn78xx;
	struct cvmx_pcieepx_cfg017_s          cn78xxp1;
	struct cvmx_pcieepx_cfg017_s          cnf71xx;
	struct cvmx_pcieepx_cfg017_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg017 cvmx_pcieepx_cfg017_t;

/**
 * cvmx_pcieep#_cfg020
 *
 * This register contains the twenty-first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg020 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg020_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_25_31               : 7;
	uint32_t pvm                          : 1;  /**< Per-vector masking capable. */
	uint32_t m64                          : 1;  /**< 64-bit address capable, writable through PEM()_CFG_WR. However, the application must
                                                         not change this field. */
	uint32_t mme                          : 3;  /**< Multiple message enabled. Indicates that multiple message mode is enabled by system
                                                         software. The number of messages enabled must be less than or equal to the multiple
                                                         message capable (MMC) value. */
	uint32_t mmc                          : 3;  /**< Multiple message capable, writable through PEM()_CFG_WR. However, the application must
                                                         not change this field. */
	uint32_t msien                        : 1;  /**< MSI enabled. When set, INTx must be disabled. */
	uint32_t ncp                          : 8;  /**< Next capability pointer. Points to PCI Express capabilities by default, writable through
                                                         PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t msicid                       : 8;  /**< MSI capability ID. */
#else
	uint32_t msicid                       : 8;
	uint32_t ncp                          : 8;
	uint32_t msien                        : 1;
	uint32_t mmc                          : 3;
	uint32_t mme                          : 3;
	uint32_t m64                          : 1;
	uint32_t pvm                          : 1;
	uint32_t reserved_25_31               : 7;
#endif
	} s;
	struct cvmx_pcieepx_cfg020_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t m64                          : 1;  /**< 64-bit Address Capable, writable through PESC(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t mme                          : 3;  /**< Multiple Message Enabled
                                                         Indicates that multiple Message mode is enabled by system
                                                         software. The number of Messages enabled must be less than
                                                         or equal to the Multiple Message Capable value. */
	uint32_t mmc                          : 3;  /**< Multiple Message Capable, writable through PESC(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t msien                        : 1;  /**< MSI Enabled
                                                         When set, INTx must be disabled. */
	uint32_t ncp                          : 8;  /**< Next Capability Pointer
                                                         Points to PCI Express Capabilities by default,
                                                         writable through PESC(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t msicid                       : 8;  /**< MSI Capability ID */
#else
	uint32_t msicid                       : 8;
	uint32_t ncp                          : 8;
	uint32_t msien                        : 1;
	uint32_t mmc                          : 3;
	uint32_t mme                          : 3;
	uint32_t m64                          : 1;
	uint32_t reserved_24_31               : 8;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg020_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg020_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg020_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg020_s          cn61xx;
	struct cvmx_pcieepx_cfg020_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg020_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg020_s          cn66xx;
	struct cvmx_pcieepx_cfg020_s          cn68xx;
	struct cvmx_pcieepx_cfg020_s          cn68xxp1;
	struct cvmx_pcieepx_cfg020_s          cn70xx;
	struct cvmx_pcieepx_cfg020_s          cn70xxp1;
	struct cvmx_pcieepx_cfg020_s          cn73xx;
	struct cvmx_pcieepx_cfg020_s          cn78xx;
	struct cvmx_pcieepx_cfg020_s          cn78xxp1;
	struct cvmx_pcieepx_cfg020_s          cnf71xx;
	struct cvmx_pcieepx_cfg020_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg020 cvmx_pcieepx_cfg020_t;

/**
 * cvmx_pcieep#_cfg021
 *
 * This register contains the twenty-second 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg021 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg021_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lmsi                         : 30; /**< Lower 32-bit address. */
	uint32_t reserved_0_1                 : 2;
#else
	uint32_t reserved_0_1                 : 2;
	uint32_t lmsi                         : 30;
#endif
	} s;
	struct cvmx_pcieepx_cfg021_s          cn52xx;
	struct cvmx_pcieepx_cfg021_s          cn52xxp1;
	struct cvmx_pcieepx_cfg021_s          cn56xx;
	struct cvmx_pcieepx_cfg021_s          cn56xxp1;
	struct cvmx_pcieepx_cfg021_s          cn61xx;
	struct cvmx_pcieepx_cfg021_s          cn63xx;
	struct cvmx_pcieepx_cfg021_s          cn63xxp1;
	struct cvmx_pcieepx_cfg021_s          cn66xx;
	struct cvmx_pcieepx_cfg021_s          cn68xx;
	struct cvmx_pcieepx_cfg021_s          cn68xxp1;
	struct cvmx_pcieepx_cfg021_s          cn70xx;
	struct cvmx_pcieepx_cfg021_s          cn70xxp1;
	struct cvmx_pcieepx_cfg021_s          cn73xx;
	struct cvmx_pcieepx_cfg021_s          cn78xx;
	struct cvmx_pcieepx_cfg021_s          cn78xxp1;
	struct cvmx_pcieepx_cfg021_s          cnf71xx;
	struct cvmx_pcieepx_cfg021_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg021 cvmx_pcieepx_cfg021_t;

/**
 * cvmx_pcieep#_cfg022
 *
 * This register contains the twenty-third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg022 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg022_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t umsi                         : 32; /**< Upper 32-bit address. */
#else
	uint32_t umsi                         : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg022_s          cn52xx;
	struct cvmx_pcieepx_cfg022_s          cn52xxp1;
	struct cvmx_pcieepx_cfg022_s          cn56xx;
	struct cvmx_pcieepx_cfg022_s          cn56xxp1;
	struct cvmx_pcieepx_cfg022_s          cn61xx;
	struct cvmx_pcieepx_cfg022_s          cn63xx;
	struct cvmx_pcieepx_cfg022_s          cn63xxp1;
	struct cvmx_pcieepx_cfg022_s          cn66xx;
	struct cvmx_pcieepx_cfg022_s          cn68xx;
	struct cvmx_pcieepx_cfg022_s          cn68xxp1;
	struct cvmx_pcieepx_cfg022_s          cn70xx;
	struct cvmx_pcieepx_cfg022_s          cn70xxp1;
	struct cvmx_pcieepx_cfg022_s          cn73xx;
	struct cvmx_pcieepx_cfg022_s          cn78xx;
	struct cvmx_pcieepx_cfg022_s          cn78xxp1;
	struct cvmx_pcieepx_cfg022_s          cnf71xx;
	struct cvmx_pcieepx_cfg022_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg022 cvmx_pcieepx_cfg022_t;

/**
 * cvmx_pcieep#_cfg023
 *
 * This register contains the twenty-fourth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg023 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg023_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t msimd                        : 16; /**< MSI data. Pattern assigned by system software. Bits [4:0] are ORed with MSI_VECTOR to
                                                         generate 32 MSI messages per function. */
#else
	uint32_t msimd                        : 16;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_pcieepx_cfg023_s          cn52xx;
	struct cvmx_pcieepx_cfg023_s          cn52xxp1;
	struct cvmx_pcieepx_cfg023_s          cn56xx;
	struct cvmx_pcieepx_cfg023_s          cn56xxp1;
	struct cvmx_pcieepx_cfg023_s          cn61xx;
	struct cvmx_pcieepx_cfg023_s          cn63xx;
	struct cvmx_pcieepx_cfg023_s          cn63xxp1;
	struct cvmx_pcieepx_cfg023_s          cn66xx;
	struct cvmx_pcieepx_cfg023_s          cn68xx;
	struct cvmx_pcieepx_cfg023_s          cn68xxp1;
	struct cvmx_pcieepx_cfg023_s          cn70xx;
	struct cvmx_pcieepx_cfg023_s          cn70xxp1;
	struct cvmx_pcieepx_cfg023_s          cn73xx;
	struct cvmx_pcieepx_cfg023_s          cn78xx;
	struct cvmx_pcieepx_cfg023_s          cn78xxp1;
	struct cvmx_pcieepx_cfg023_s          cnf71xx;
	struct cvmx_pcieepx_cfg023_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg023 cvmx_pcieepx_cfg023_t;

/**
 * cvmx_pcieep#_cfg024
 *
 * This register contains the twenty-fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg024 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg024_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msimm                        : 32; /**< MSI mask bits. For each mask bit that is set, the function is prohibited from sending the
                                                         associated message. */
#else
	uint32_t msimm                        : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg024_s          cn70xx;
	struct cvmx_pcieepx_cfg024_s          cn70xxp1;
	struct cvmx_pcieepx_cfg024_s          cn73xx;
	struct cvmx_pcieepx_cfg024_s          cn78xx;
	struct cvmx_pcieepx_cfg024_s          cn78xxp1;
	struct cvmx_pcieepx_cfg024_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg024 cvmx_pcieepx_cfg024_t;

/**
 * cvmx_pcieep#_cfg025
 *
 * This register contains the twenty-sixth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg025 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg025_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msimp                        : 32; /**< MSI pending bits. For each pending bit that is set, the function has a pending associated message. */
#else
	uint32_t msimp                        : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg025_s          cn70xx;
	struct cvmx_pcieepx_cfg025_s          cn70xxp1;
	struct cvmx_pcieepx_cfg025_s          cn73xx;
	struct cvmx_pcieepx_cfg025_s          cn78xx;
	struct cvmx_pcieepx_cfg025_s          cn78xxp1;
	struct cvmx_pcieepx_cfg025_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg025 cvmx_pcieepx_cfg025_t;

/**
 * cvmx_pcieep#_cfg028
 *
 * This register contains the twenty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg028 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg028_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t imn                          : 5;  /**< Interrupt message number. Updated by hardware, writable through PEM()_CFG_WR. However,
                                                         the application must not change this field. */
	uint32_t si                           : 1;  /**< Slot implemented. This bit is writable through PEM()_CFG_WR. However, it must be 0 for
                                                         an endpoint device. Therefore, the application must not write a 1 to this bit. */
	uint32_t dpt                          : 4;  /**< Device port type.
                                                         0000 = PCI Express endpoint.
                                                         0001 = Legacy PCI Express endpoint.
                                                         All other encodings are not supported */
	uint32_t pciecv                       : 4;  /**< PCI Express capability version. */
	uint32_t ncp                          : 8;  /**< Next capability pointer. Points to the MSI-X Capabilities by default, writable through
                                                         PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t pcieid                       : 8;  /**< PCI Express capability ID. */
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
	struct cvmx_pcieepx_cfg028_s          cn52xx;
	struct cvmx_pcieepx_cfg028_s          cn52xxp1;
	struct cvmx_pcieepx_cfg028_s          cn56xx;
	struct cvmx_pcieepx_cfg028_s          cn56xxp1;
	struct cvmx_pcieepx_cfg028_s          cn61xx;
	struct cvmx_pcieepx_cfg028_s          cn63xx;
	struct cvmx_pcieepx_cfg028_s          cn63xxp1;
	struct cvmx_pcieepx_cfg028_s          cn66xx;
	struct cvmx_pcieepx_cfg028_s          cn68xx;
	struct cvmx_pcieepx_cfg028_s          cn68xxp1;
	struct cvmx_pcieepx_cfg028_s          cn70xx;
	struct cvmx_pcieepx_cfg028_s          cn70xxp1;
	struct cvmx_pcieepx_cfg028_s          cn73xx;
	struct cvmx_pcieepx_cfg028_s          cn78xx;
	struct cvmx_pcieepx_cfg028_s          cn78xxp1;
	struct cvmx_pcieepx_cfg028_s          cnf71xx;
	struct cvmx_pcieepx_cfg028_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg028 cvmx_pcieepx_cfg028_t;

/**
 * cvmx_pcieep#_cfg029
 *
 * This register contains the thirtieth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg029 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg029_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_28_31               : 4;
	uint32_t cspls                        : 2;  /**< Captured slot power limit scale. From message from RC, upstream port only. */
	uint32_t csplv                        : 8;  /**< Captured slot power limit value. From message from RC, upstream port only. */
	uint32_t reserved_16_17               : 2;
	uint32_t rber                         : 1;  /**< Role-based error reporting, writable through PEM()_CFG_WR. However, the application
                                                         must not change this field. */
	uint32_t reserved_12_14               : 3;
	uint32_t el1al                        : 3;  /**< Endpoint L1 acceptable latency, writable through PEM()_CFG_WR. */
	uint32_t el0al                        : 3;  /**< Endpoint L0s acceptable latency, writable through PEM()_CFG_WR. */
	uint32_t etfs                         : 1;  /**< Extended tag field supported. This bit is writable through
                                                         PEM()_CFG_WR. */
	uint32_t pfs                          : 2;  /**< Phantom function supported. This field is writable through
                                                         PEM()_CFG_WR. However, phantom function is not supported. Therefore, the application
                                                         must not write any value other than 0x0 to this field. */
	uint32_t mpss                         : 3;  /**< Max_Payload_Size supported, writable through PEM()_CFG_WR. However, the application
                                                         must not change this field. */
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
	uint32_t reserved_28_31               : 4;
#endif
	} s;
	struct cvmx_pcieepx_cfg029_s          cn52xx;
	struct cvmx_pcieepx_cfg029_s          cn52xxp1;
	struct cvmx_pcieepx_cfg029_s          cn56xx;
	struct cvmx_pcieepx_cfg029_s          cn56xxp1;
	struct cvmx_pcieepx_cfg029_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_29_31               : 3;
	uint32_t flr_cap                      : 1;  /**< Function Level Reset Capable
                                                         not supported */
	uint32_t cspls                        : 2;  /**< Captured Slot Power Limit Scale
                                                         From Message from RC, upstream port only. */
	uint32_t csplv                        : 8;  /**< Captured Slot Power Limit Value
                                                         From Message from RC, upstream port only. */
	uint32_t reserved_16_17               : 2;
	uint32_t rber                         : 1;  /**< Role-Based Error Reporting, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t reserved_12_14               : 3;
	uint32_t el1al                        : 3;  /**< Endpoint L1 Acceptable Latency, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t el0al                        : 3;  /**< Endpoint L0s Acceptable Latency, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t etfs                         : 1;  /**< Extended Tag Field Supported
                                                         This bit is writable through PEM(0..1)_CFG_WR.
                                                         However, the application
                                                         must not write a 1 to this bit. */
	uint32_t pfs                          : 2;  /**< Phantom Function Supported
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, Phantom
                                                         Function is not supported. Therefore, the application must not
                                                         write any value other than 0x0 to this field. */
	uint32_t mpss                         : 3;  /**< Max_Payload_Size Supported, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
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
	} cn61xx;
	struct cvmx_pcieepx_cfg029_s          cn63xx;
	struct cvmx_pcieepx_cfg029_s          cn63xxp1;
	struct cvmx_pcieepx_cfg029_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_29_31               : 3;
	uint32_t flr                          : 1;  /**< Function Level Reset Capability
                                                         When set, core support of SR-IOV */
	uint32_t cspls                        : 2;  /**< Captured Slot Power Limit Scale
                                                         From Message from RC, upstream port only. */
	uint32_t csplv                        : 8;  /**< Captured Slot Power Limit Value
                                                         From Message from RC, upstream port only. */
	uint32_t reserved_16_17               : 2;
	uint32_t rber                         : 1;  /**< Role-Based Error Reporting, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t reserved_12_14               : 3;
	uint32_t el1al                        : 3;  /**< Endpoint L1 Acceptable Latency, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t el0al                        : 3;  /**< Endpoint L0s Acceptable Latency, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t etfs                         : 1;  /**< Extended Tag Field Supported
                                                         This bit is writable through PEM(0..1)_CFG_WR.
                                                         However, the application
                                                         must not write a 1 to this bit. */
	uint32_t pfs                          : 2;  /**< Phantom Function Supported
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, Phantom
                                                         Function is not supported. Therefore, the application must not
                                                         write any value other than 0x0 to this field. */
	uint32_t mpss                         : 3;  /**< Max_Payload_Size Supported, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
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
	uint32_t flr                          : 1;
	uint32_t reserved_29_31               : 3;
#endif
	} cn66xx;
	struct cvmx_pcieepx_cfg029_cn66xx     cn68xx;
	struct cvmx_pcieepx_cfg029_cn66xx     cn68xxp1;
	struct cvmx_pcieepx_cfg029_cn61xx     cn70xx;
	struct cvmx_pcieepx_cfg029_cn61xx     cn70xxp1;
	struct cvmx_pcieepx_cfg029_cn61xx     cn73xx;
	struct cvmx_pcieepx_cfg029_cn61xx     cn78xx;
	struct cvmx_pcieepx_cfg029_cn61xx     cn78xxp1;
	struct cvmx_pcieepx_cfg029_cn61xx     cnf71xx;
	struct cvmx_pcieepx_cfg029_cn61xx     cnf75xx;
};
typedef union cvmx_pcieepx_cfg029 cvmx_pcieepx_cfg029_t;

/**
 * cvmx_pcieep#_cfg030
 *
 * This register contains the thirty-first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg030 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg030_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_22_31               : 10;
	uint32_t tp                           : 1;  /**< Transaction pending. Set to 1 when nonposted requests are not yet completed and set to 0
                                                         when they are completed. */
	uint32_t ap_d                         : 1;  /**< AUX power detected. Set to 1 if AUX power detected. */
	uint32_t ur_d                         : 1;  /**< Unsupported request detected. Errors are logged in this register regardless of whether or
                                                         not error reporting is enabled in the device control register. UR_D occurs when we receive
                                                         something unsupported. Unsupported requests are nonfatal errors, so UR_D should cause
                                                         NFE_D. Receiving a vendor-defined message should cause an unsupported request. */
	uint32_t fe_d                         : 1;  /**< Fatal error detected. Errors are logged in this register regardless of whether or not
                                                         error reporting is enabled in the device control register. This field is set if we receive
                                                         any of the errors in PCIEEP()_CFG066 that has a severity set to fatal. Malformed TLPs
                                                         generally fit into this category. */
	uint32_t nfe_d                        : 1;  /**< Nonfatal error detected. Errors are logged in this register regardless of whether or not
                                                         error reporting is enabled in the device control register. This field is set if we receive
                                                         any of the errors in PCIEEP()_CFG066 that has a severity set to nonfatal and does not
                                                         meet advisory nonfatal criteria, which most poisoned TLPs should. */
	uint32_t ce_d                         : 1;  /**< Correctable error detected. Errors are logged in this register regardless of whether or
                                                         not error reporting is enabled in the device control register. This field is set if we
                                                         receive any of the errors in PCIEEP()_CFG068, for example a replay-timer timeout.
                                                         Also, it can be set if we get any of the errors in PCIEEP()_CFG066 that has a severity
                                                         set to Nonfatal and meets the Advisory Nonfatal criteria, which most ECRC errors should. */
	uint32_t i_flr                        : 1;  /**< Initiate function level reset.
                                                         [I_FLR] must not be written to one via the indirect PEM()_CFG_WR. It should only ever
                                                         be written to one via a direct PCIe access. */
	uint32_t mrrs                         : 3;  /**< Max read request size.
                                                         0x0 =128 bytes.
                                                         0x1 = 256 bytes.
                                                         0x2 = 512 bytes.
                                                         0x3 = 1024 bytes.
                                                         0x4 = 2048 bytes.
                                                         0x5 = 4096 bytes.
                                                         SLI_S2M_PORT()_CTL[MRRS] and DPI_SLI_PRT()_CFG[MRRS] must also be set properly.
                                                         SLI_S2M_PORT()_CTL[MRRS] and DPI_SLI_PRT()_CFG[MRRS] must not exceed the desired
                                                         max read request size. */
	uint32_t ns_en                        : 1;  /**< Enable no snoop. */
	uint32_t ap_en                        : 1;  /**< AUX power PM enable (not supported). */
	uint32_t pf_en                        : 1;  /**< Phantom function enable. This bit should never be set; PEM requests never use phantom functions. */
	uint32_t etf_en                       : 1;  /**< Extended tag field enable. */
	uint32_t mps                          : 3;  /**< Max payload size. Legal values: 0x0 = 128 B, 0x1 = 256 B.
                                                         Larger sizes are not supported by CNXXXX.
                                                         DPI_SLI_PRT()_CFG[MPS] must be set to the same value as this field for proper
                                                         functionality. */
	uint32_t ro_en                        : 1;  /**< Enable relaxed ordering. */
	uint32_t ur_en                        : 1;  /**< Unsupported request reporting enable. */
	uint32_t fe_en                        : 1;  /**< Fatal error reporting enable. */
	uint32_t nfe_en                       : 1;  /**< Nonfatal error reporting enable. */
	uint32_t ce_en                        : 1;  /**< Correctable error reporting enable. */
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
	struct cvmx_pcieepx_cfg030_cn52xx {
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
                                                          Errors are logged in this register regardless of whether
                                                          error reporting is enabled in the Device Control register.
                                                         FE_D is set if receive any of the errors in PCIE_CFG066 that
                                                         has a severity set to Fatal.  Malformed TLP's generally fit
                                                         into this category. */
	uint32_t nfe_d                        : 1;  /**< Non-Fatal Error detected
                                                          Errors are logged in this register regardless of whether
                                                          error reporting is enabled in the Device Control register.
                                                         NFE_D is set if we receive any of the errors in PCIE_CFG066
                                                         that has a severity set to Nonfatal and does NOT meet Advisory
                                                         Nonfatal criteria (PCIe 1.1 spec, Section 6.2.3.2.4), which
                                                         most poisoned TLP's should be. */
	uint32_t ce_d                         : 1;  /**< Correctable Error Detected
                                                          Errors are logged in this register regardless of whether
                                                          error reporting is enabled in the Device Control register.
                                                         CE_D is set if we receive any of the errors in PCIE_CFG068
                                                         for example a Replay Timer Timeout.  Also, it can be set if
                                                         we get any of the errors in PCIE_CFG066 that has a severity
                                                         set to Nonfatal and meets the Advisory Nonfatal criteria
                                                         (PCIe 1.1 spec, Section 6.2.3.2.4), which most ECRC errors
                                                         should be. */
	uint32_t reserved_15_15               : 1;
	uint32_t mrrs                         : 3;  /**< Max Read Request Size
                                                          0 = 128B
                                                          1 = 256B
                                                          2 = 512B
                                                          3 = 1024B
                                                          4 = 2048B
                                                          5 = 4096B
                                                         Note: NPEI_CTL_STATUS2[MRRS] also must be set properly.
                                                         NPEI_CTL_STATUS2[MRRS] must not exceed the
                                                         desired max read request size. */
	uint32_t ns_en                        : 1;  /**< Enable No Snoop */
	uint32_t ap_en                        : 1;  /**< AUX Power PM Enable */
	uint32_t pf_en                        : 1;  /**< Phantom Function Enable
                                                         This bit should never be set - OCTEON requests never use
                                                         phantom functions. */
	uint32_t etf_en                       : 1;  /**< Extended Tag Field Enable
                                                         This bit should never be set - OCTEON requests never use
                                                         extended tags. */
	uint32_t mps                          : 3;  /**< Max Payload Size
                                                          Legal values:
                                                           0  = 128B
                                                           1  = 256B
                                                          Larger sizes not supported by OCTEON.
                                                         Note: NPEI_CTL_STATUS2[MPS] must be set to the same
                                                               value for proper functionality. */
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
	uint32_t reserved_15_15               : 1;
	uint32_t ce_d                         : 1;
	uint32_t nfe_d                        : 1;
	uint32_t fe_d                         : 1;
	uint32_t ur_d                         : 1;
	uint32_t ap_d                         : 1;
	uint32_t tp                           : 1;
	uint32_t reserved_22_31               : 10;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg030_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg030_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg030_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg030_s          cn61xx;
	struct cvmx_pcieepx_cfg030_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg030_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg030_s          cn66xx;
	struct cvmx_pcieepx_cfg030_s          cn68xx;
	struct cvmx_pcieepx_cfg030_s          cn68xxp1;
	struct cvmx_pcieepx_cfg030_s          cn70xx;
	struct cvmx_pcieepx_cfg030_s          cn70xxp1;
	struct cvmx_pcieepx_cfg030_s          cn73xx;
	struct cvmx_pcieepx_cfg030_s          cn78xx;
	struct cvmx_pcieepx_cfg030_s          cn78xxp1;
	struct cvmx_pcieepx_cfg030_s          cnf71xx;
	struct cvmx_pcieepx_cfg030_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg030 cvmx_pcieepx_cfg030_t;

/**
 * cvmx_pcieep#_cfg031
 *
 * This register contains the thirty-second 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg031 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg031_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pnum                         : 8;  /**< Port number, writable through PEM()_CFG_WR. */
	uint32_t reserved_23_23               : 1;
	uint32_t aspm                         : 1;  /**< ASPM optionality compliance. */
	uint32_t lbnc                         : 1;  /**< Link bandwidth notification capability. Set to 0 for endpoint devices. */
	uint32_t dllarc                       : 1;  /**< Data link layer active reporting capable. */
	uint32_t sderc                        : 1;  /**< Surprise down error reporting capable. Not supported; hardwired to 0. */
	uint32_t cpm                          : 1;  /**< Clock power management. The default value is the value that software specifies during
                                                         hardware configuration, writable through PEM()_CFG_WR. However, the application must not
                                                         change this field. */
	uint32_t l1el                         : 3;  /**< L1 exit latency. The default value is the value that software specifies during hardware
                                                         configuration, writable through PEM()_CFG_WR. */
	uint32_t l0el                         : 3;  /**< L0s exit latency. The default value is the value that software specifies during hardware
                                                         configuration, writable through PEM()_CFG_WR. */
	uint32_t aslpms                       : 2;  /**< Active state link PM support. The default value is the value that software specifies
                                                         during hardware configuration, writable through PEM()_CFG_WR. However, the application
                                                         must not change this field. */
	uint32_t mlw                          : 6;  /**< Maximum link width.
                                                         The reset value of this field is determined by the value read from the PEM
                                                         csr PEM()_CFG[LANES4]. If LANES4 is set the reset value is 0x4, otherwise 0x2.
                                                         This field is writable through PEM()_CFG_WR.
                                                         Zeroing both [MLW] and [MLS] out of reset, using the EEPROM, will prevent the
                                                         link state machine from advancing past CONFIG.  This can be useful to allow
                                                         software to locally boot and perform preconfiguration and bug fixes.  Setting
                                                         [MLW] and [MLS] to nonzero values will then allow the lttsm to advance and the
                                                         link to come up. */
	uint32_t mls                          : 4;  /**< Maximum link speed. The reset value of this field is controlled by the value read from
                                                         PEM()_CFG[MD].
                                                         _ MD is 0x0, reset to 0x1: 2.5 GHz supported.
                                                         _ MD is 0x1, reset to 0x2: 5.0 GHz and 2.5 GHz supported.
                                                         _ MD is 0x2, reset to 0x3: 8.0 GHz, 5.0 GHz and 2.5 GHz supported.
                                                         _ MD is 0x3, reset to 0x3: 8.0 GHz, 5.0 GHz and 2.5 GHz supported (RC Mode).
                                                         This field is writable through PEM()_CFG_WR.
                                                         Zeroing both [MLW] and [MLS] out of reset, using the EEPROM, will prevent the
                                                         link state machine from advancing past CONFIG.  This can be useful to allow
                                                         software to locally boot and perform preconfiguration and bug fixes.  Setting
                                                         [MLW] and [MLS] to nonzero values will then allow the lttsm to advance and the
                                                         link to come up. */
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
	struct cvmx_pcieepx_cfg031_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pnum                         : 8;  /**< Port Number, writable through PESC(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t reserved_22_23               : 2;
	uint32_t lbnc                         : 1;  /**< Link Bandwith Notification Capability */
	uint32_t dllarc                       : 1;  /**< Data Link Layer Active Reporting Capable */
	uint32_t sderc                        : 1;  /**< Surprise Down Error Reporting Capable
                                                         Not supported, hardwired to 0x0. */
	uint32_t cpm                          : 1;  /**< Clock Power Management
                                                         The default value is the value you specify during hardware
                                                         configuration, writable through PESC(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t l1el                         : 3;  /**< L1 Exit Latency
                                                         The default value is the value you specify during hardware
                                                         configuration, writable through PESC(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t l0el                         : 3;  /**< L0s Exit Latency
                                                         The default value is the value you specify during hardware
                                                         configuration, writable through PESC(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t aslpms                       : 2;  /**< Active State Link PM Support
                                                         The default value is the value you specify during hardware
                                                         configuration, writable through PESC(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t mlw                          : 6;  /**< Maximum Link Width
                                                         The default value is the value you specify during hardware
                                                         configuration (x1, x2, x4, x8, or x16), writable through PESC(0..1)_CFG_WR.
                                                         This value will be set to 0x4 or 0x2 depending on the max
                                                         number of lanes (QLM_CFG == 0 set to 0x2 else 0x4). */
	uint32_t mls                          : 4;  /**< Maximum Link Speed
                                                         Default value is 0x1 for 2.5 Gbps Link.
                                                         This field is writable through PESC(0..1)_CFG_WR.
                                                         However, 0x1 is the
                                                         only supported value. Therefore, the application must not write
                                                         any value other than 0x1 to this field. */
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
	uint32_t reserved_22_23               : 2;
	uint32_t pnum                         : 8;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg031_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg031_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg031_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg031_s          cn61xx;
	struct cvmx_pcieepx_cfg031_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg031_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg031_s          cn66xx;
	struct cvmx_pcieepx_cfg031_s          cn68xx;
	struct cvmx_pcieepx_cfg031_cn52xx     cn68xxp1;
	struct cvmx_pcieepx_cfg031_s          cn70xx;
	struct cvmx_pcieepx_cfg031_s          cn70xxp1;
	struct cvmx_pcieepx_cfg031_s          cn73xx;
	struct cvmx_pcieepx_cfg031_s          cn78xx;
	struct cvmx_pcieepx_cfg031_s          cn78xxp1;
	struct cvmx_pcieepx_cfg031_s          cnf71xx;
	struct cvmx_pcieepx_cfg031_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg031 cvmx_pcieepx_cfg031_t;

/**
 * cvmx_pcieep#_cfg032
 *
 * This register contains the thirty-third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg032 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg032_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lab                          : 1;  /**< Link autonomous bandwidth status. */
	uint32_t lbm                          : 1;  /**< Link bandwidth management status. */
	uint32_t dlla                         : 1;  /**< Data link layer active. Not applicable for an upstream port or endpoint device, hardwired to 0. */
	uint32_t scc                          : 1;  /**< Slot clock configuration. Indicates that the component uses the same physical reference
                                                         clock that the platform provides on the connector. Writable through PEM()_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t lt                           : 1;  /**< Link training. Not applicable for an upstream port or endpoint device, hardwired to 0. */
	uint32_t reserved_26_26               : 1;
	uint32_t nlw                          : 6;  /**< Negotiated link width. Set automatically by hardware after link initialization. Value is
                                                         undefined when link is not up. */
	uint32_t ls                           : 4;  /**< Current link speed. The encoded value specifies a bit location in the supported link
                                                         speeds vector (in the link capabilities 2 register) that corresponds to the current link
                                                         speed.
                                                         0x1 = Supported link speeds vector field bit 0.
                                                         0x2 = Supported link speeds vector field bit 1.
                                                         0x3 = Supported link speeds vector field bit 2. */
	uint32_t reserved_12_15               : 4;
	uint32_t lab_int_enb                  : 1;  /**< Link autonomous bandwidth interrupt enable. This bit is not applicable and is reserved for
                                                         endpoints. */
	uint32_t lbm_int_enb                  : 1;  /**< Link bandwidth management interrupt enable. This bit is not applicable and is reserved for
                                                         endpoints. */
	uint32_t hawd                         : 1;  /**< Hardware autonomous width disable (not supported). */
	uint32_t ecpm                         : 1;  /**< Enable clock power management. Hardwired to 0 if clock power management is disabled in the
                                                         link capabilities register. */
	uint32_t es                           : 1;  /**< Extended synch. */
	uint32_t ccc                          : 1;  /**< Common clock configuration. */
	uint32_t rl                           : 1;  /**< Retrain link. Not applicable for an upstream port or endpoint device. Hardwired to 0. */
	uint32_t ld                           : 1;  /**< Link disable. Not applicable for an upstream port or endpoint device. Hardwired to 0. */
	uint32_t rcb                          : 1;  /**< Read completion boundary (RCB). */
	uint32_t reserved_2_2                 : 1;
	uint32_t aslpc                        : 2;  /**< Active state link PM control. */
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
	struct cvmx_pcieepx_cfg032_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t dlla                         : 1;  /**< Data Link Layer Active
                                                         Not applicable for an upstream Port or Endpoint device,
                                                         hardwired to 0. */
	uint32_t scc                          : 1;  /**< Slot Clock Configuration
                                                         Indicates that the component uses the same physical reference
                                                         clock that the platform provides on the connector.
                                                         Writable through PESC(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t lt                           : 1;  /**< Link Training
                                                         Not applicable for an upstream Port or Endpoint device,
                                                         hardwired to 0. */
	uint32_t reserved_26_26               : 1;
	uint32_t nlw                          : 6;  /**< Negotiated Link Width
                                                         Set automatically by hardware after Link initialization. */
	uint32_t ls                           : 4;  /**< Link Speed
                                                         The negotiated Link speed: 2.5 Gbps */
	uint32_t reserved_10_15               : 6;
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
	uint32_t reserved_10_15               : 6;
	uint32_t ls                           : 4;
	uint32_t nlw                          : 6;
	uint32_t reserved_26_26               : 1;
	uint32_t lt                           : 1;
	uint32_t scc                          : 1;
	uint32_t dlla                         : 1;
	uint32_t reserved_30_31               : 2;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg032_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg032_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg032_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg032_s          cn61xx;
	struct cvmx_pcieepx_cfg032_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg032_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg032_s          cn66xx;
	struct cvmx_pcieepx_cfg032_s          cn68xx;
	struct cvmx_pcieepx_cfg032_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t dlla                         : 1;  /**< Data Link Layer Active
                                                         Not applicable for an upstream Port or Endpoint device,
                                                         hardwired to 0. */
	uint32_t scc                          : 1;  /**< Slot Clock Configuration
                                                         Indicates that the component uses the same physical reference
                                                         clock that the platform provides on the connector.
                                                         Writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t lt                           : 1;  /**< Link Training
                                                         Not applicable for an upstream Port or Endpoint device,
                                                         hardwired to 0. */
	uint32_t reserved_26_26               : 1;
	uint32_t nlw                          : 6;  /**< Negotiated Link Width
                                                         Set automatically by hardware after Link initialization. */
	uint32_t ls                           : 4;  /**< Link Speed
                                                         1 == The negotiated Link speed: 2.5 Gbps
                                                         2 == The negotiated Link speed: 5.0 Gbps
                                                         4 == The negotiated Link speed: 8.0 Gbps (Not Supported) */
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
	uint32_t reserved_30_31               : 2;
#endif
	} cn68xxp1;
	struct cvmx_pcieepx_cfg032_s          cn70xx;
	struct cvmx_pcieepx_cfg032_s          cn70xxp1;
	struct cvmx_pcieepx_cfg032_s          cn73xx;
	struct cvmx_pcieepx_cfg032_s          cn78xx;
	struct cvmx_pcieepx_cfg032_s          cn78xxp1;
	struct cvmx_pcieepx_cfg032_s          cnf71xx;
	struct cvmx_pcieepx_cfg032_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg032 cvmx_pcieepx_cfg032_t;

/**
 * cvmx_pcieep#_cfg033
 *
 * PCIE_CFG033 = Thirty-fourth 32-bits of PCIE type 0 config space
 * (Slot Capabilities Register)
 */
union cvmx_pcieepx_cfg033 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg033_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ps_num                       : 13; /**< Physical Slot Number, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t nccs                         : 1;  /**< No Command Complete Support, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t emip                         : 1;  /**< Electromechanical Interlock Present, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t sp_ls                        : 2;  /**< Slot Power Limit Scale, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t sp_lv                        : 8;  /**< Slot Power Limit Value, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t hp_c                         : 1;  /**< Hot-Plug Capable, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t hp_s                         : 1;  /**< Hot-Plug Surprise, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t pip                          : 1;  /**< Power Indicator Present, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t aip                          : 1;  /**< Attention Indicator Present, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t mrlsp                        : 1;  /**< MRL Sensor Present, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t pcp                          : 1;  /**< Power Controller Present, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t abp                          : 1;  /**< Attention Button Present, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
#else
	uint32_t abp                          : 1;
	uint32_t pcp                          : 1;
	uint32_t mrlsp                        : 1;
	uint32_t aip                          : 1;
	uint32_t pip                          : 1;
	uint32_t hp_s                         : 1;
	uint32_t hp_c                         : 1;
	uint32_t sp_lv                        : 8;
	uint32_t sp_ls                        : 2;
	uint32_t emip                         : 1;
	uint32_t nccs                         : 1;
	uint32_t ps_num                       : 13;
#endif
	} s;
	struct cvmx_pcieepx_cfg033_s          cn52xx;
	struct cvmx_pcieepx_cfg033_s          cn52xxp1;
	struct cvmx_pcieepx_cfg033_s          cn56xx;
	struct cvmx_pcieepx_cfg033_s          cn56xxp1;
	struct cvmx_pcieepx_cfg033_s          cn63xx;
	struct cvmx_pcieepx_cfg033_s          cn63xxp1;
};
typedef union cvmx_pcieepx_cfg033 cvmx_pcieepx_cfg033_t;

/**
 * cvmx_pcieep#_cfg034
 *
 * PCIE_CFG034 = Thirty-fifth 32-bits of PCIE type 0 config space
 * (Slot Control Register/Slot Status Register)
 */
union cvmx_pcieepx_cfg034 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg034_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_25_31               : 7;
	uint32_t dlls_c                       : 1;  /**< Data Link Layer State Changed
                                                         Not applicable for an upstream Port or Endpoint device,
                                                         hardwired to 0. */
	uint32_t emis                         : 1;  /**< Electromechanical Interlock Status */
	uint32_t pds                          : 1;  /**< Presence Detect State */
	uint32_t mrlss                        : 1;  /**< MRL Sensor State */
	uint32_t ccint_d                      : 1;  /**< Command Completed */
	uint32_t pd_c                         : 1;  /**< Presence Detect Changed */
	uint32_t mrls_c                       : 1;  /**< MRL Sensor Changed */
	uint32_t pf_d                         : 1;  /**< Power Fault Detected */
	uint32_t abp_d                        : 1;  /**< Attention Button Pressed */
	uint32_t reserved_13_15               : 3;
	uint32_t dlls_en                      : 1;  /**< Data Link Layer State Changed Enable
                                                         Not applicable for an upstream Port or Endpoint device,
                                                         hardwired to 0. */
	uint32_t emic                         : 1;  /**< Electromechanical Interlock Control */
	uint32_t pcc                          : 1;  /**< Power Controller Control */
	uint32_t pic                          : 2;  /**< Power Indicator Control */
	uint32_t aic                          : 2;  /**< Attention Indicator Control */
	uint32_t hpint_en                     : 1;  /**< Hot-Plug Interrupt Enable */
	uint32_t ccint_en                     : 1;  /**< Command Completed Interrupt Enable */
	uint32_t pd_en                        : 1;  /**< Presence Detect Changed Enable */
	uint32_t mrls_en                      : 1;  /**< MRL Sensor Changed Enable */
	uint32_t pf_en                        : 1;  /**< Power Fault Detected Enable */
	uint32_t abp_en                       : 1;  /**< Attention Button Pressed Enable */
#else
	uint32_t abp_en                       : 1;
	uint32_t pf_en                        : 1;
	uint32_t mrls_en                      : 1;
	uint32_t pd_en                        : 1;
	uint32_t ccint_en                     : 1;
	uint32_t hpint_en                     : 1;
	uint32_t aic                          : 2;
	uint32_t pic                          : 2;
	uint32_t pcc                          : 1;
	uint32_t emic                         : 1;
	uint32_t dlls_en                      : 1;
	uint32_t reserved_13_15               : 3;
	uint32_t abp_d                        : 1;
	uint32_t pf_d                         : 1;
	uint32_t mrls_c                       : 1;
	uint32_t pd_c                         : 1;
	uint32_t ccint_d                      : 1;
	uint32_t mrlss                        : 1;
	uint32_t pds                          : 1;
	uint32_t emis                         : 1;
	uint32_t dlls_c                       : 1;
	uint32_t reserved_25_31               : 7;
#endif
	} s;
	struct cvmx_pcieepx_cfg034_s          cn52xx;
	struct cvmx_pcieepx_cfg034_s          cn52xxp1;
	struct cvmx_pcieepx_cfg034_s          cn56xx;
	struct cvmx_pcieepx_cfg034_s          cn56xxp1;
	struct cvmx_pcieepx_cfg034_s          cn63xx;
	struct cvmx_pcieepx_cfg034_s          cn63xxp1;
};
typedef union cvmx_pcieepx_cfg034 cvmx_pcieepx_cfg034_t;

/**
 * cvmx_pcieep#_cfg037
 *
 * This register contains the thirty-eighth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg037 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg037_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t meetp                        : 2;  /**< Max end-end TLP prefixes.
                                                         0x1 = 1.
                                                         0x2 = 2.
                                                         0x3 = 3.
                                                         0x0 = 4. */
	uint32_t eetps                        : 1;  /**< End-end TLP prefix supported (not supported). */
	uint32_t effs                         : 1;  /**< Extended fmt field supported (not supported). */
	uint32_t obffs                        : 2;  /**< Optimized buffer flush fill (OBFF) supported (not supported). */
	uint32_t reserved_12_17               : 6;
	uint32_t ltrs                         : 1;  /**< Latency tolerance reporting (LTR) mechanism supported (not supported). */
	uint32_t noroprpr                     : 1;  /**< No RO-enabled PR-PR passing. (This bit applies to RCs.) */
	uint32_t atom128s                     : 1;  /**< 128-bit AtomicOp supported (not supported). */
	uint32_t atom64s                      : 1;  /**< 64-bit AtomicOp supported.
                                                         Note that inbound AtomicOps targeting BAR0 are not supported and are dropped as an
                                                         unsupported request. */
	uint32_t atom32s                      : 1;  /**< 32-bit AtomicOp supported.
                                                         Note that inbound AtomicOps targeting BAR0 are not supported and are dropped as an
                                                         unsupported request. */
	uint32_t atom_ops                     : 1;  /**< AtomicOp routing supported (not applicable for EP). */
	uint32_t ari                          : 1;  /**< Alternate routing ID forwarding supported (not applicable for EP). */
	uint32_t ctds                         : 1;  /**< Completion timeout disable supported. */
	uint32_t ctrs                         : 4;  /**< Completion timeout ranges supported. */
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
	uint32_t reserved_12_17               : 6;
	uint32_t obffs                        : 2;
	uint32_t effs                         : 1;
	uint32_t eetps                        : 1;
	uint32_t meetp                        : 2;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_pcieepx_cfg037_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_5_31                : 27;
	uint32_t ctds                         : 1;  /**< Completion Timeout Disable Supported */
	uint32_t ctrs                         : 4;  /**< Completion Timeout Ranges Supported
                                                         Value of 0 indicates that Completion Timeout Programming
                                                         is not supported
                                                         Completion timeout is 16.7ms. */
#else
	uint32_t ctrs                         : 4;
	uint32_t ctds                         : 1;
	uint32_t reserved_5_31                : 27;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg037_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg037_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg037_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg037_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_14_31               : 18;
	uint32_t tph                          : 2;  /**< TPH Completer Supported
                                                         (Not Supported) */
	uint32_t reserved_11_11               : 1;
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
	uint32_t ari                          : 1;  /**< Alternate Routing ID Forwarding Supported
                                                         (Not Supported) */
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
	uint32_t reserved_11_11               : 1;
	uint32_t tph                          : 2;
	uint32_t reserved_14_31               : 18;
#endif
	} cn61xx;
	struct cvmx_pcieepx_cfg037_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg037_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg037_cn61xx     cn66xx;
	struct cvmx_pcieepx_cfg037_cn61xx     cn68xx;
	struct cvmx_pcieepx_cfg037_cn61xx     cn68xxp1;
	struct cvmx_pcieepx_cfg037_cn61xx     cn70xx;
	struct cvmx_pcieepx_cfg037_cn61xx     cn70xxp1;
	struct cvmx_pcieepx_cfg037_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t meetp                        : 2;  /**< Max end-end TLP prefixes.
                                                         0x1 = 1.
                                                         0x2 = 2.
                                                         0x3 = 3.
                                                         0x0 = 4. */
	uint32_t eetps                        : 1;  /**< End-end TLP prefix supported (not supported). */
	uint32_t effs                         : 1;  /**< Extended fmt field supported (not supported). */
	uint32_t obffs                        : 2;  /**< Optimized buffer flush fill (OBFF) supported (not supported). */
	uint32_t reserved_14_17               : 4;
	uint32_t tphs                         : 2;  /**< TPH completer supported (not supported). */
	uint32_t ltrs                         : 1;  /**< Latency tolerance reporting (LTR) mechanism supported (not supported). */
	uint32_t noroprpr                     : 1;  /**< No RO-enabled PR-PR passing. (This bit applies to RCs.) */
	uint32_t atom128s                     : 1;  /**< 128-bit AtomicOp supported (not supported). */
	uint32_t atom64s                      : 1;  /**< 64-bit AtomicOp supported.
                                                         _ PF0:          0x1
                                                         _ PF1:          0x1
                                                         _ PF2:          0x0
                                                         Note that inbound AtomicOps targeting BAR0 are not supported and are dropped as an
                                                         unsupported request. */
	uint32_t atom32s                      : 1;  /**< 32-bit AtomicOp supported.
                                                         _ PF0:          0x1
                                                         _ PF1:          0x1
                                                         _ PF2:          0x0
                                                         Note that inbound AtomicOps targeting BAR0 are not supported and are dropped as an
                                                         unsupported request. */
	uint32_t atom_ops                     : 1;  /**< AtomicOp routing supported (not applicable for EP). */
	uint32_t ari                          : 1;  /**< Alternate routing ID forwarding supported (not applicable for EP). */
	uint32_t ctds                         : 1;  /**< Completion timeout disable supported. */
	uint32_t ctrs                         : 4;  /**< Completion timeout ranges supported. */
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
	} cn73xx;
	struct cvmx_pcieepx_cfg037_cn73xx     cn78xx;
	struct cvmx_pcieepx_cfg037_cn73xx     cn78xxp1;
	struct cvmx_pcieepx_cfg037_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
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
	uint32_t ari                          : 1;  /**< Alternate Routing ID Forwarding Supported
                                                         (Not Supported) */
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
	uint32_t reserved_20_31               : 12;
#endif
	} cnf71xx;
	struct cvmx_pcieepx_cfg037_cn73xx     cnf75xx;
};
typedef union cvmx_pcieepx_cfg037 cvmx_pcieepx_cfg037_t;

/**
 * cvmx_pcieep#_cfg038
 *
 * This register contains the thirty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg038 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg038_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t eetpb                        : 1;  /**< Unsupported end-end TLP prefix blocking. */
	uint32_t obffe                        : 2;  /**< Optimized buffer flush fill (OBFF) enable (not supported). */
	uint32_t reserved_11_12               : 2;
	uint32_t ltre                         : 1;  /**< Latency tolerance reporting (LTR) mechanism enable (not supported). */
	uint32_t id0_cp                       : 1;  /**< ID based ordering completion enable (not supported). */
	uint32_t id0_rq                       : 1;  /**< ID based ordering request enable (not supported). */
	uint32_t atom_op_eb                   : 1;  /**< AtomicOp egress blocking. */
	uint32_t atom_op                      : 1;  /**< AtomicOp requester enable. */
	uint32_t ari                          : 1;  /**< Alternate routing ID forwarding supported. */
	uint32_t ctd                          : 1;  /**< Completion timeout disable. */
	uint32_t ctv                          : 4;  /**< Completion timeout value.
                                                         0x0 = Default range: 16 ms to 55 ms.
                                                         0x1 = 50 us to 100 us.
                                                         0x2 = 1 ms to 10 ms.
                                                         0x3 = 16 ms to 55 ms.
                                                         0x6 = 65 ms to 210 ms.
                                                         0x9 = 260 ms to 900 ms.
                                                         0xA = 1 s to 3.5 s.
                                                         0xD = 4 s to 13 s.
                                                         0xE = 17 s to 64 s.
                                                         Values not defined are reserved. */
#else
	uint32_t ctv                          : 4;
	uint32_t ctd                          : 1;
	uint32_t ari                          : 1;
	uint32_t atom_op                      : 1;
	uint32_t atom_op_eb                   : 1;
	uint32_t id0_rq                       : 1;
	uint32_t id0_cp                       : 1;
	uint32_t ltre                         : 1;
	uint32_t reserved_11_12               : 2;
	uint32_t obffe                        : 2;
	uint32_t eetpb                        : 1;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_pcieepx_cfg038_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_5_31                : 27;
	uint32_t ctd                          : 1;  /**< Completion Timeout Disable */
	uint32_t ctv                          : 4;  /**< Completion Timeout Value
                                                         Completion Timeout Programming is not supported
                                                         Completion timeout is 16.7ms. */
#else
	uint32_t ctv                          : 4;
	uint32_t ctd                          : 1;
	uint32_t reserved_5_31                : 27;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg038_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg038_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg038_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg038_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_10_31               : 22;
	uint32_t id0_cp                       : 1;  /**< ID Based Ordering Completion Enable
                                                         (Not Supported) */
	uint32_t id0_rq                       : 1;  /**< ID Based Ordering Request Enable
                                                         (Not Supported) */
	uint32_t atom_op_eb                   : 1;  /**< AtomicOp Egress Blocking
                                                         (Not Supported)m */
	uint32_t atom_op                      : 1;  /**< AtomicOp Requester Enable
                                                         (Not Supported) */
	uint32_t ari                          : 1;  /**< Alternate Routing ID Forwarding Supported
                                                         (Not Supported) */
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
	uint32_t reserved_10_31               : 22;
#endif
	} cn61xx;
	struct cvmx_pcieepx_cfg038_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg038_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg038_cn61xx     cn66xx;
	struct cvmx_pcieepx_cfg038_cn61xx     cn68xx;
	struct cvmx_pcieepx_cfg038_cn61xx     cn68xxp1;
	struct cvmx_pcieepx_cfg038_cn61xx     cn70xx;
	struct cvmx_pcieepx_cfg038_cn61xx     cn70xxp1;
	struct cvmx_pcieepx_cfg038_s          cn73xx;
	struct cvmx_pcieepx_cfg038_s          cn78xx;
	struct cvmx_pcieepx_cfg038_s          cn78xxp1;
	struct cvmx_pcieepx_cfg038_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_15_31               : 17;
	uint32_t obffe                        : 2;  /**< Optimized Buffer Flush Fill (OBFF) Enable
                                                         (Not Supported) */
	uint32_t reserved_11_12               : 2;
	uint32_t ltre                         : 1;  /**< Latency Tolerance Reporting (LTR) Mechanism Enable
                                                         (Not Supported) */
	uint32_t id0_cp                       : 1;  /**< ID Based Ordering Completion Enable
                                                         (Not Supported) */
	uint32_t id0_rq                       : 1;  /**< ID Based Ordering Request Enable
                                                         (Not Supported) */
	uint32_t atom_op_eb                   : 1;  /**< AtomicOp Egress Blocking
                                                         (Not Supported)m */
	uint32_t atom_op                      : 1;  /**< AtomicOp Requester Enable
                                                         (Not Supported) */
	uint32_t ari                          : 1;  /**< Alternate Routing ID Forwarding Supported
                                                         (Not Supported) */
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
	uint32_t ltre                         : 1;
	uint32_t reserved_11_12               : 2;
	uint32_t obffe                        : 2;
	uint32_t reserved_15_31               : 17;
#endif
	} cnf71xx;
	struct cvmx_pcieepx_cfg038_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg038 cvmx_pcieepx_cfg038_t;

/**
 * cvmx_pcieep#_cfg039
 *
 * This register contains the fortieth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg039 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg039_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_9_31                : 23;
	uint32_t cls                          : 1;  /**< Crosslink supported. */
	uint32_t slsv                         : 7;  /**< Supported link speeds vector. Indicates the supported link speeds of the associated port.
                                                         For each bit, a value of 1 b indicates that the corresponding link speed is supported;
                                                         otherwise, the link speed is not supported. Bit definitions are:
                                                         _ Bit <1> =  2.5 GT/s.
                                                         _ Bit <2> = 5.0 GT/s.
                                                         _ Bit <3> = 8.0 GT/s.
                                                         _ Bits <7:4> are reserved.
                                                         The reset value of this field is controlled by the value read from PEM()_CFG[MD].
                                                         _ MD is 0x0, reset to 0x1: 2.5 GHz supported.
                                                         _ MD is 0x1, reset to 0x3: 5.0 GHz and 2.5 GHz supported.
                                                         _ MD is 0x2, reset to 0x7: 8.0 GHz, 5.0 GHz and 2.5 GHz supported.
                                                         _ MD is 0x3, reset to 0x7: 8.0 GHz, 5.0 GHz and 2.5 GHz supported (RC Mode). */
	uint32_t reserved_0_0                 : 1;
#else
	uint32_t reserved_0_0                 : 1;
	uint32_t slsv                         : 7;
	uint32_t cls                          : 1;
	uint32_t reserved_9_31                : 23;
#endif
	} s;
	struct cvmx_pcieepx_cfg039_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg039_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg039_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg039_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg039_s          cn61xx;
	struct cvmx_pcieepx_cfg039_s          cn63xx;
	struct cvmx_pcieepx_cfg039_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg039_s          cn66xx;
	struct cvmx_pcieepx_cfg039_s          cn68xx;
	struct cvmx_pcieepx_cfg039_s          cn68xxp1;
	struct cvmx_pcieepx_cfg039_s          cn70xx;
	struct cvmx_pcieepx_cfg039_s          cn70xxp1;
	struct cvmx_pcieepx_cfg039_s          cn73xx;
	struct cvmx_pcieepx_cfg039_s          cn78xx;
	struct cvmx_pcieepx_cfg039_s          cn78xxp1;
	struct cvmx_pcieepx_cfg039_s          cnf71xx;
	struct cvmx_pcieepx_cfg039_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg039 cvmx_pcieepx_cfg039_t;

/**
 * cvmx_pcieep#_cfg040
 *
 * This register contains the forty-first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg040 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg040_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_22_31               : 10;
	uint32_t ler                          : 1;  /**< Link equalization request */
	uint32_t ep3s                         : 1;  /**< Equalization phase 3 successful */
	uint32_t ep2s                         : 1;  /**< Equalization phase 2 successful */
	uint32_t ep1s                         : 1;  /**< Equalization phase 1 successful */
	uint32_t eqc                          : 1;  /**< Equalization complete */
	uint32_t cdl                          : 1;  /**< Current deemphasis level. When the link is operating at 5 GT/s speed, this bit reflects
                                                         the level of deemphasis. Encodings:
                                                         1 = -3.5 dB.
                                                         0 = -6 dB.
                                                         The value in this bit is undefined when the link is operating at 2.5 GT/s speed. */
	uint32_t cde                          : 4;  /**< Compliance deemphasis. This bit sets the deemphasis level in polling. Compliance state if
                                                         the entry occurred due to the TX compliance receive bit being 1. Encodings:
                                                         1 = -3.5 dB.
                                                         0 = -6 dB.
                                                         When the link is operating at 2.5 GT/s, the setting of this bit has no effect. */
	uint32_t csos                         : 1;  /**< Compliance SOS. When set to 1, the LTSSM is required to send SKP ordered sets periodically
                                                         in between the (modified) compliance patterns.
                                                         When the link is operating at 2.5 GT/s, the setting of this bit has no effect. */
	uint32_t emc                          : 1;  /**< Enter modified compliance. When this bit is set to 1, the device transmits a modified
                                                         compliance pattern if the LTSSM enters polling compliance state. */
	uint32_t tm                           : 3;  /**< Transmit margin. This field controls the value of the non-deemphasized voltage level at
                                                         the transmitter pins:
                                                         0x0 =  800-1200 mV for full swing 400-600 mV for half-swing.
                                                         0x1-0x2 = Values must be monotonic with a nonzero slope.
                                                         0x3 = 200-400 mV for full-swing and 100-200 mV for halfswing.
                                                         0x4-0x7 = Reserved.
                                                         This field is reset to 0x0 on entry to the LTSSM polling compliance substate. When
                                                         operating in 5.0 GT/s mode with full swing, the deemphasis ratio must be maintained within
                                                         +/- 1 dB from the specification-defined operational value either -3.5 or -6 dB. */
	uint32_t sde                          : 1;  /**< Selectable deemphasis. Not applicable for an upstream port or endpoint device. Hardwired to 0. */
	uint32_t hasd                         : 1;  /**< Hardware autonomous speed disable. When asserted, the application must disable hardware
                                                         from changing the link speed for device-specific reasons other than attempting to correct
                                                         unreliable link operation by reducing link speed. Initial transition to the highest
                                                         supported common link speed is not blocked by this signal. */
	uint32_t ec                           : 1;  /**< Enter compliance. Software is permitted to force a link to enter compliance mode at the
                                                         speed indicated in the target link speed field by setting this bit to 1 in both components
                                                         on a link and then initiating a hot reset on the link. */
	uint32_t tls                          : 4;  /**< Target link speed. For downstream ports, this field sets an upper limit on link
                                                         operational speed by restricting the values advertised by the upstream component in its
                                                         training sequences:
                                                         0x1 = 2.5 Gb/s target link speed.
                                                         0x2 = 5 Gb/s target link speed.
                                                         0x3 = 8 Gb/s target link speed.
                                                         All other encodings are reserved.
                                                         If a value is written to this field that does not correspond to a speed included in the
                                                         supported link speeds field, the result is undefined. For both upstream and downstream
                                                         ports, this field is used to set the target compliance mode speed when software is using
                                                         the enter compliance bit to force a link into compliance mode.
                                                         The reset value of this field is controlled by the value read from PEM()_CFG[MD].
                                                         _ MD is 0x0, reset to 0x1: 2.5 GHz supported.
                                                         _ MD is 0x1, reset to 0x2: 5.0 GHz and 2.5 GHz supported.
                                                         _ MD is 0x2, reset to 0x3: 8.0 GHz, 5.0 GHz and 2.5 GHz supported.
                                                         _ MD is 0x3, reset to 0x3: 8.0 GHz, 5.0 GHz and 2.5 GHz supported (RC Mode). */
#else
	uint32_t tls                          : 4;
	uint32_t ec                           : 1;
	uint32_t hasd                         : 1;
	uint32_t sde                          : 1;
	uint32_t tm                           : 3;
	uint32_t emc                          : 1;
	uint32_t csos                         : 1;
	uint32_t cde                          : 4;
	uint32_t cdl                          : 1;
	uint32_t eqc                          : 1;
	uint32_t ep1s                         : 1;
	uint32_t ep2s                         : 1;
	uint32_t ep3s                         : 1;
	uint32_t ler                          : 1;
	uint32_t reserved_22_31               : 10;
#endif
	} s;
	struct cvmx_pcieepx_cfg040_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg040_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg040_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg040_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg040_cn61xx {
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
	uint32_t tls                          : 4;  /**< Target Link Speed
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
                                                         The reset value of this field is controlled by a value sent from
                                                         the lsb of the MIO_QLM#_SPD register.
                                                         qlm#_spd[0]   RST_VALUE   NOTE
                                                         1             0001b       2.5 GHz supported
                                                         0             0010b       5.0 GHz and 2.5 GHz supported */
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
	} cn61xx;
	struct cvmx_pcieepx_cfg040_cn61xx     cn63xx;
	struct cvmx_pcieepx_cfg040_cn61xx     cn63xxp1;
	struct cvmx_pcieepx_cfg040_cn61xx     cn66xx;
	struct cvmx_pcieepx_cfg040_cn61xx     cn68xx;
	struct cvmx_pcieepx_cfg040_cn61xx     cn68xxp1;
	struct cvmx_pcieepx_cfg040_cn61xx     cn70xx;
	struct cvmx_pcieepx_cfg040_cn61xx     cn70xxp1;
	struct cvmx_pcieepx_cfg040_s          cn73xx;
	struct cvmx_pcieepx_cfg040_s          cn78xx;
	struct cvmx_pcieepx_cfg040_s          cn78xxp1;
	struct cvmx_pcieepx_cfg040_cn61xx     cnf71xx;
	struct cvmx_pcieepx_cfg040_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg040 cvmx_pcieepx_cfg040_t;

/**
 * cvmx_pcieep#_cfg041
 *
 * PCIE_CFG041 = Fourty-second 32-bits of PCIE type 0 config space
 * (Slot Capabilities 2 Register)
 */
union cvmx_pcieepx_cfg041 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg041_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg041_s          cn52xx;
	struct cvmx_pcieepx_cfg041_s          cn52xxp1;
	struct cvmx_pcieepx_cfg041_s          cn56xx;
	struct cvmx_pcieepx_cfg041_s          cn56xxp1;
	struct cvmx_pcieepx_cfg041_s          cn63xx;
	struct cvmx_pcieepx_cfg041_s          cn63xxp1;
};
typedef union cvmx_pcieepx_cfg041 cvmx_pcieepx_cfg041_t;

/**
 * cvmx_pcieep#_cfg042
 *
 * PCIE_CFG042 = Fourty-third 32-bits of PCIE type 0 config space
 * (Slot Control 2 Register/Slot Status 2 Register)
 */
union cvmx_pcieepx_cfg042 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg042_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg042_s          cn52xx;
	struct cvmx_pcieepx_cfg042_s          cn52xxp1;
	struct cvmx_pcieepx_cfg042_s          cn56xx;
	struct cvmx_pcieepx_cfg042_s          cn56xxp1;
	struct cvmx_pcieepx_cfg042_s          cn63xx;
	struct cvmx_pcieepx_cfg042_s          cn63xxp1;
};
typedef union cvmx_pcieepx_cfg042 cvmx_pcieepx_cfg042_t;

/**
 * cvmx_pcieep#_cfg044
 *
 * This register contains the forty-fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg044 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg044_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msixen                       : 1;  /**< MSI-X enable. If MSI-X is enabled, MSI and INTx must be disabled. */
	uint32_t funm                         : 1;  /**< Function mask.
                                                         0 = Each vectors mask bit determines whether the vector is masked or not.
                                                         1 = All vectors associated with the function are masked, regardless of their respective
                                                         per-vector mask bits. */
	uint32_t reserved_27_29               : 3;
	uint32_t msixts                       : 11; /**< MSI-X table size encoded as (table size - 1). Writable through PEM()_CFG_WR.
                                                         This field is writable by issuing a PEM()_CFG_WR to PCIEEP(0)_CFG044
                                                         when PEM()_CFG_WR[ADDR[31]] is set. */
	uint32_t ncp                          : 8;  /**< Next capability pointer */
	uint32_t msixcid                      : 8;  /**< MSI-X capability ID */
#else
	uint32_t msixcid                      : 8;
	uint32_t ncp                          : 8;
	uint32_t msixts                       : 11;
	uint32_t reserved_27_29               : 3;
	uint32_t funm                         : 1;
	uint32_t msixen                       : 1;
#endif
	} s;
	struct cvmx_pcieepx_cfg044_s          cn73xx;
	struct cvmx_pcieepx_cfg044_s          cn78xx;
	struct cvmx_pcieepx_cfg044_s          cn78xxp1;
	struct cvmx_pcieepx_cfg044_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg044 cvmx_pcieepx_cfg044_t;

/**
 * cvmx_pcieep#_cfg045
 *
 * This register contains the forty-sixth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg045 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg045_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msixtoffs                    : 29; /**< MSI-X table offset register. Base address of the MSI-X table, as an offset from the base
                                                         address of the BAR indicated by the Table BIR bits.
                                                         Writable through PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t msixtbir                     : 3;  /**< MSI-X table BAR indicator register (BIR). Indicates which BAR is used to map the MSI-X
                                                         table into memory space.
                                                         Writable through PEM()_CFG_WR. However, the application must not change this field. */
#else
	uint32_t msixtbir                     : 3;
	uint32_t msixtoffs                    : 29;
#endif
	} s;
	struct cvmx_pcieepx_cfg045_s          cn73xx;
	struct cvmx_pcieepx_cfg045_s          cn78xx;
	struct cvmx_pcieepx_cfg045_s          cn78xxp1;
	struct cvmx_pcieepx_cfg045_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg045 cvmx_pcieepx_cfg045_t;

/**
 * cvmx_pcieep#_cfg046
 *
 * This register contains the forty-seventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg046 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg046_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msixpoffs                    : 29; /**< MSI-X table offset register. Base address of the MSI-X PBA, as an offset from the base
                                                         address of the BAR indicated by the table PBA bits.
                                                         Writable through PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t msixpbir                     : 3;  /**< MSI-X PBA BAR indicator register (BIR). Indicates which BAR is used to map the MSI-X
                                                         pending bit array into memory space.
                                                         Writable through PEM()_CFG_WR. However, the application must not change this field. */
#else
	uint32_t msixpbir                     : 3;
	uint32_t msixpoffs                    : 29;
#endif
	} s;
	struct cvmx_pcieepx_cfg046_s          cn73xx;
	struct cvmx_pcieepx_cfg046_s          cn78xx;
	struct cvmx_pcieepx_cfg046_s          cn78xxp1;
	struct cvmx_pcieepx_cfg046_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg046 cvmx_pcieepx_cfg046_t;

/**
 * cvmx_pcieep#_cfg064
 *
 * This register contains the sixty-fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg064 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg064_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next capability offset. Points to the ARI capabilities by default. */
	uint32_t cv                           : 4;  /**< Capability version. */
	uint32_t pcieec                       : 16; /**< PCI Express extended capability. */
#else
	uint32_t pcieec                       : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} s;
	struct cvmx_pcieepx_cfg064_s          cn52xx;
	struct cvmx_pcieepx_cfg064_s          cn52xxp1;
	struct cvmx_pcieepx_cfg064_s          cn56xx;
	struct cvmx_pcieepx_cfg064_s          cn56xxp1;
	struct cvmx_pcieepx_cfg064_s          cn61xx;
	struct cvmx_pcieepx_cfg064_s          cn63xx;
	struct cvmx_pcieepx_cfg064_s          cn63xxp1;
	struct cvmx_pcieepx_cfg064_s          cn66xx;
	struct cvmx_pcieepx_cfg064_s          cn68xx;
	struct cvmx_pcieepx_cfg064_s          cn68xxp1;
	struct cvmx_pcieepx_cfg064_s          cn70xx;
	struct cvmx_pcieepx_cfg064_s          cn70xxp1;
	struct cvmx_pcieepx_cfg064_s          cn73xx;
	struct cvmx_pcieepx_cfg064_s          cn78xx;
	struct cvmx_pcieepx_cfg064_s          cn78xxp1;
	struct cvmx_pcieepx_cfg064_s          cnf71xx;
	struct cvmx_pcieepx_cfg064_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg064 cvmx_pcieepx_cfg064_t;

/**
 * cvmx_pcieep#_cfg065
 *
 * This register contains the sixty-sixth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg065 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg065_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t tpbes                        : 1;  /**< Unsupported TLP prefix blocked error status. */
	uint32_t uatombs                      : 1;  /**< Unsupported AtomicOp egress blocked status. */
	uint32_t reserved_23_23               : 1;
	uint32_t ucies                        : 1;  /**< Uncorrectable internal error status. */
	uint32_t reserved_21_21               : 1;
	uint32_t ures                         : 1;  /**< Unsupported request error status. */
	uint32_t ecrces                       : 1;  /**< ECRC error status. */
	uint32_t mtlps                        : 1;  /**< Malformed TLP status. */
	uint32_t ros                          : 1;  /**< Receiver overflow status. */
	uint32_t ucs                          : 1;  /**< Unexpected completion status. */
	uint32_t cas                          : 1;  /**< Completer abort status. */
	uint32_t cts                          : 1;  /**< Completion timeout status. */
	uint32_t fcpes                        : 1;  /**< Flow control protocol error status. */
	uint32_t ptlps                        : 1;  /**< Poisoned TLP status. */
	uint32_t reserved_6_11                : 6;
	uint32_t sdes                         : 1;  /**< Surprise Down Error Status (not supported) */
	uint32_t dlpes                        : 1;  /**< Data link protocol error status. */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpes                        : 1;
	uint32_t sdes                         : 1;
	uint32_t reserved_6_11                : 6;
	uint32_t ptlps                        : 1;
	uint32_t fcpes                        : 1;
	uint32_t cts                          : 1;
	uint32_t cas                          : 1;
	uint32_t ucs                          : 1;
	uint32_t ros                          : 1;
	uint32_t mtlps                        : 1;
	uint32_t ecrces                       : 1;
	uint32_t ures                         : 1;
	uint32_t reserved_21_21               : 1;
	uint32_t ucies                        : 1;
	uint32_t reserved_23_23               : 1;
	uint32_t uatombs                      : 1;
	uint32_t tpbes                        : 1;
	uint32_t reserved_26_31               : 6;
#endif
	} s;
	struct cvmx_pcieepx_cfg065_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_21_31               : 11;
	uint32_t ures                         : 1;  /**< Unsupported Request Error Status */
	uint32_t ecrces                       : 1;  /**< ECRC Error Status */
	uint32_t mtlps                        : 1;  /**< Malformed TLP Status */
	uint32_t ros                          : 1;  /**< Receiver Overflow Status */
	uint32_t ucs                          : 1;  /**< Unexpected Completion Status */
	uint32_t cas                          : 1;  /**< Completer Abort Status */
	uint32_t cts                          : 1;  /**< Completion Timeout Status */
	uint32_t fcpes                        : 1;  /**< Flow Control Protocol Error Status */
	uint32_t ptlps                        : 1;  /**< Poisoned TLP Status */
	uint32_t reserved_6_11                : 6;
	uint32_t sdes                         : 1;  /**< Surprise Down Error Status (not supported) */
	uint32_t dlpes                        : 1;  /**< Data Link Protocol Error Status */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpes                        : 1;
	uint32_t sdes                         : 1;
	uint32_t reserved_6_11                : 6;
	uint32_t ptlps                        : 1;
	uint32_t fcpes                        : 1;
	uint32_t cts                          : 1;
	uint32_t cas                          : 1;
	uint32_t ucs                          : 1;
	uint32_t ros                          : 1;
	uint32_t mtlps                        : 1;
	uint32_t ecrces                       : 1;
	uint32_t ures                         : 1;
	uint32_t reserved_21_31               : 11;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg065_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg065_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg065_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg065_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_25_31               : 7;
	uint32_t uatombs                      : 1;  /**< Unsupported AtomicOp Egress Blocked Status */
	uint32_t reserved_21_23               : 3;
	uint32_t ures                         : 1;  /**< Unsupported Request Error Status */
	uint32_t ecrces                       : 1;  /**< ECRC Error Status */
	uint32_t mtlps                        : 1;  /**< Malformed TLP Status */
	uint32_t ros                          : 1;  /**< Receiver Overflow Status */
	uint32_t ucs                          : 1;  /**< Unexpected Completion Status */
	uint32_t cas                          : 1;  /**< Completer Abort Status */
	uint32_t cts                          : 1;  /**< Completion Timeout Status */
	uint32_t fcpes                        : 1;  /**< Flow Control Protocol Error Status */
	uint32_t ptlps                        : 1;  /**< Poisoned TLP Status */
	uint32_t reserved_6_11                : 6;
	uint32_t sdes                         : 1;  /**< Surprise Down Error Status (not supported) */
	uint32_t dlpes                        : 1;  /**< Data Link Protocol Error Status */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpes                        : 1;
	uint32_t sdes                         : 1;
	uint32_t reserved_6_11                : 6;
	uint32_t ptlps                        : 1;
	uint32_t fcpes                        : 1;
	uint32_t cts                          : 1;
	uint32_t cas                          : 1;
	uint32_t ucs                          : 1;
	uint32_t ros                          : 1;
	uint32_t mtlps                        : 1;
	uint32_t ecrces                       : 1;
	uint32_t ures                         : 1;
	uint32_t reserved_21_23               : 3;
	uint32_t uatombs                      : 1;
	uint32_t reserved_25_31               : 7;
#endif
	} cn61xx;
	struct cvmx_pcieepx_cfg065_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg065_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg065_cn61xx     cn66xx;
	struct cvmx_pcieepx_cfg065_cn61xx     cn68xx;
	struct cvmx_pcieepx_cfg065_cn52xx     cn68xxp1;
	struct cvmx_pcieepx_cfg065_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_25_31               : 7;
	uint32_t uatombs                      : 1;  /**< Unsupported AtomicOp Egress Blocked Status */
	uint32_t reserved_23_23               : 1;
	uint32_t ucies                        : 1;  /**< Uncorrectable Internal Error Status */
	uint32_t reserved_21_21               : 1;
	uint32_t ures                         : 1;  /**< Unsupported Request Error Status */
	uint32_t ecrces                       : 1;  /**< ECRC Error Status */
	uint32_t mtlps                        : 1;  /**< Malformed TLP Status */
	uint32_t ros                          : 1;  /**< Receiver Overflow Status */
	uint32_t ucs                          : 1;  /**< Unexpected Completion Status */
	uint32_t cas                          : 1;  /**< Completer Abort Status */
	uint32_t cts                          : 1;  /**< Completion Timeout Status */
	uint32_t fcpes                        : 1;  /**< Flow Control Protocol Error Status */
	uint32_t ptlps                        : 1;  /**< Poisoned TLP Status */
	uint32_t reserved_6_11                : 6;
	uint32_t sdes                         : 1;  /**< Surprise Down Error Status (not supported) */
	uint32_t dlpes                        : 1;  /**< Data Link Protocol Error Status */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpes                        : 1;
	uint32_t sdes                         : 1;
	uint32_t reserved_6_11                : 6;
	uint32_t ptlps                        : 1;
	uint32_t fcpes                        : 1;
	uint32_t cts                          : 1;
	uint32_t cas                          : 1;
	uint32_t ucs                          : 1;
	uint32_t ros                          : 1;
	uint32_t mtlps                        : 1;
	uint32_t ecrces                       : 1;
	uint32_t ures                         : 1;
	uint32_t reserved_21_21               : 1;
	uint32_t ucies                        : 1;
	uint32_t reserved_23_23               : 1;
	uint32_t uatombs                      : 1;
	uint32_t reserved_25_31               : 7;
#endif
	} cn70xx;
	struct cvmx_pcieepx_cfg065_cn70xx     cn70xxp1;
	struct cvmx_pcieepx_cfg065_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t tpbes                        : 1;  /**< Unsupported TLP prefix blocked error status. */
	uint32_t uatombs                      : 1;  /**< Unsupported AtomicOp egress blocked status. */
	uint32_t reserved_23_23               : 1;
	uint32_t ucies                        : 1;  /**< Uncorrectable internal error status. */
	uint32_t reserved_21_21               : 1;
	uint32_t ures                         : 1;  /**< Unsupported request error status. */
	uint32_t ecrces                       : 1;  /**< ECRC error status. */
	uint32_t mtlps                        : 1;  /**< Malformed TLP status. */
	uint32_t ros                          : 1;  /**< Receiver overflow status. */
	uint32_t ucs                          : 1;  /**< Unexpected completion status. */
	uint32_t cas                          : 1;  /**< Completer abort status. */
	uint32_t cts                          : 1;  /**< Completion timeout status. */
	uint32_t fcpes                        : 1;  /**< Flow control protocol error status. */
	uint32_t ptlps                        : 1;  /**< Poisoned TLP status. */
	uint32_t reserved_5_11                : 7;
	uint32_t dlpes                        : 1;  /**< Data link protocol error status. */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpes                        : 1;
	uint32_t reserved_5_11                : 7;
	uint32_t ptlps                        : 1;
	uint32_t fcpes                        : 1;
	uint32_t cts                          : 1;
	uint32_t cas                          : 1;
	uint32_t ucs                          : 1;
	uint32_t ros                          : 1;
	uint32_t mtlps                        : 1;
	uint32_t ecrces                       : 1;
	uint32_t ures                         : 1;
	uint32_t reserved_21_21               : 1;
	uint32_t ucies                        : 1;
	uint32_t reserved_23_23               : 1;
	uint32_t uatombs                      : 1;
	uint32_t tpbes                        : 1;
	uint32_t reserved_26_31               : 6;
#endif
	} cn73xx;
	struct cvmx_pcieepx_cfg065_cn73xx     cn78xx;
	struct cvmx_pcieepx_cfg065_cn73xx     cn78xxp1;
	struct cvmx_pcieepx_cfg065_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_25_31               : 7;
	uint32_t uatombs                      : 1;  /**< Unsupported AtomicOp Egress Blocked Status */
	uint32_t reserved_23_23               : 1;
	uint32_t ucies                        : 1;  /**< Uncorrectable Internal Error Status */
	uint32_t reserved_21_21               : 1;
	uint32_t ures                         : 1;  /**< Unsupported Request Error Status */
	uint32_t ecrces                       : 1;  /**< ECRC Error Status */
	uint32_t mtlps                        : 1;  /**< Malformed TLP Status */
	uint32_t ros                          : 1;  /**< Receiver Overflow Status */
	uint32_t ucs                          : 1;  /**< Unexpected Completion Status */
	uint32_t cas                          : 1;  /**< Completer Abort Status */
	uint32_t cts                          : 1;  /**< Completion Timeout Status */
	uint32_t fcpes                        : 1;  /**< Flow Control Protocol Error Status */
	uint32_t ptlps                        : 1;  /**< Poisoned TLP Status */
	uint32_t reserved_5_11                : 7;
	uint32_t dlpes                        : 1;  /**< Data Link Protocol Error Status */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpes                        : 1;
	uint32_t reserved_5_11                : 7;
	uint32_t ptlps                        : 1;
	uint32_t fcpes                        : 1;
	uint32_t cts                          : 1;
	uint32_t cas                          : 1;
	uint32_t ucs                          : 1;
	uint32_t ros                          : 1;
	uint32_t mtlps                        : 1;
	uint32_t ecrces                       : 1;
	uint32_t ures                         : 1;
	uint32_t reserved_21_21               : 1;
	uint32_t ucies                        : 1;
	uint32_t reserved_23_23               : 1;
	uint32_t uatombs                      : 1;
	uint32_t reserved_25_31               : 7;
#endif
	} cnf71xx;
	struct cvmx_pcieepx_cfg065_cn73xx     cnf75xx;
};
typedef union cvmx_pcieepx_cfg065 cvmx_pcieepx_cfg065_t;

/**
 * cvmx_pcieep#_cfg066
 *
 * This register contains the sixty-seventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg066 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg066_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t tpbem                        : 1;  /**< Unsupported TLP prefix blocked error mask. */
	uint32_t uatombm                      : 1;  /**< Unsupported AtomicOp egress blocked mask. */
	uint32_t reserved_23_23               : 1;
	uint32_t uciem                        : 1;  /**< Uncorrectable internal error mask. */
	uint32_t reserved_21_21               : 1;
	uint32_t urem                         : 1;  /**< Unsupported request error mask. */
	uint32_t ecrcem                       : 1;  /**< ECRC error mask. */
	uint32_t mtlpm                        : 1;  /**< Malformed TLP mask. */
	uint32_t rom                          : 1;  /**< Receiver overflow mask. */
	uint32_t ucm                          : 1;  /**< Unexpected completion mask. */
	uint32_t cam                          : 1;  /**< Completer abort mask. */
	uint32_t ctm                          : 1;  /**< Completion timeout mask. */
	uint32_t fcpem                        : 1;  /**< Flow control protocol error mask. */
	uint32_t ptlpm                        : 1;  /**< Poisoned TLP mask. */
	uint32_t reserved_6_11                : 6;
	uint32_t sdem                         : 1;  /**< Surprise Down Error Mask (not supported) */
	uint32_t dlpem                        : 1;  /**< Data link protocol error mask. */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpem                        : 1;
	uint32_t sdem                         : 1;
	uint32_t reserved_6_11                : 6;
	uint32_t ptlpm                        : 1;
	uint32_t fcpem                        : 1;
	uint32_t ctm                          : 1;
	uint32_t cam                          : 1;
	uint32_t ucm                          : 1;
	uint32_t rom                          : 1;
	uint32_t mtlpm                        : 1;
	uint32_t ecrcem                       : 1;
	uint32_t urem                         : 1;
	uint32_t reserved_21_21               : 1;
	uint32_t uciem                        : 1;
	uint32_t reserved_23_23               : 1;
	uint32_t uatombm                      : 1;
	uint32_t tpbem                        : 1;
	uint32_t reserved_26_31               : 6;
#endif
	} s;
	struct cvmx_pcieepx_cfg066_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_21_31               : 11;
	uint32_t urem                         : 1;  /**< Unsupported Request Error Mask */
	uint32_t ecrcem                       : 1;  /**< ECRC Error Mask */
	uint32_t mtlpm                        : 1;  /**< Malformed TLP Mask */
	uint32_t rom                          : 1;  /**< Receiver Overflow Mask */
	uint32_t ucm                          : 1;  /**< Unexpected Completion Mask */
	uint32_t cam                          : 1;  /**< Completer Abort Mask */
	uint32_t ctm                          : 1;  /**< Completion Timeout Mask */
	uint32_t fcpem                        : 1;  /**< Flow Control Protocol Error Mask */
	uint32_t ptlpm                        : 1;  /**< Poisoned TLP Mask */
	uint32_t reserved_6_11                : 6;
	uint32_t sdem                         : 1;  /**< Surprise Down Error Mask (not supported) */
	uint32_t dlpem                        : 1;  /**< Data Link Protocol Error Mask */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpem                        : 1;
	uint32_t sdem                         : 1;
	uint32_t reserved_6_11                : 6;
	uint32_t ptlpm                        : 1;
	uint32_t fcpem                        : 1;
	uint32_t ctm                          : 1;
	uint32_t cam                          : 1;
	uint32_t ucm                          : 1;
	uint32_t rom                          : 1;
	uint32_t mtlpm                        : 1;
	uint32_t ecrcem                       : 1;
	uint32_t urem                         : 1;
	uint32_t reserved_21_31               : 11;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg066_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg066_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg066_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg066_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_25_31               : 7;
	uint32_t uatombm                      : 1;  /**< Unsupported AtomicOp Egress Blocked Mask */
	uint32_t reserved_21_23               : 3;
	uint32_t urem                         : 1;  /**< Unsupported Request Error Mask */
	uint32_t ecrcem                       : 1;  /**< ECRC Error Mask */
	uint32_t mtlpm                        : 1;  /**< Malformed TLP Mask */
	uint32_t rom                          : 1;  /**< Receiver Overflow Mask */
	uint32_t ucm                          : 1;  /**< Unexpected Completion Mask */
	uint32_t cam                          : 1;  /**< Completer Abort Mask */
	uint32_t ctm                          : 1;  /**< Completion Timeout Mask */
	uint32_t fcpem                        : 1;  /**< Flow Control Protocol Error Mask */
	uint32_t ptlpm                        : 1;  /**< Poisoned TLP Mask */
	uint32_t reserved_6_11                : 6;
	uint32_t sdem                         : 1;  /**< Surprise Down Error Mask (not supported) */
	uint32_t dlpem                        : 1;  /**< Data Link Protocol Error Mask */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpem                        : 1;
	uint32_t sdem                         : 1;
	uint32_t reserved_6_11                : 6;
	uint32_t ptlpm                        : 1;
	uint32_t fcpem                        : 1;
	uint32_t ctm                          : 1;
	uint32_t cam                          : 1;
	uint32_t ucm                          : 1;
	uint32_t rom                          : 1;
	uint32_t mtlpm                        : 1;
	uint32_t ecrcem                       : 1;
	uint32_t urem                         : 1;
	uint32_t reserved_21_23               : 3;
	uint32_t uatombm                      : 1;
	uint32_t reserved_25_31               : 7;
#endif
	} cn61xx;
	struct cvmx_pcieepx_cfg066_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg066_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg066_cn61xx     cn66xx;
	struct cvmx_pcieepx_cfg066_cn61xx     cn68xx;
	struct cvmx_pcieepx_cfg066_cn52xx     cn68xxp1;
	struct cvmx_pcieepx_cfg066_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_25_31               : 7;
	uint32_t uatombm                      : 1;  /**< Unsupported AtomicOp Egress Blocked Mask */
	uint32_t reserved_23_23               : 1;
	uint32_t uciem                        : 1;  /**< Uncorrectable Internal Error Mask */
	uint32_t reserved_21_21               : 1;
	uint32_t urem                         : 1;  /**< Unsupported Request Error Mask */
	uint32_t ecrcem                       : 1;  /**< ECRC Error Mask */
	uint32_t mtlpm                        : 1;  /**< Malformed TLP Mask */
	uint32_t rom                          : 1;  /**< Receiver Overflow Mask */
	uint32_t ucm                          : 1;  /**< Unexpected Completion Mask */
	uint32_t cam                          : 1;  /**< Completer Abort Mask */
	uint32_t ctm                          : 1;  /**< Completion Timeout Mask */
	uint32_t fcpem                        : 1;  /**< Flow Control Protocol Error Mask */
	uint32_t ptlpm                        : 1;  /**< Poisoned TLP Mask */
	uint32_t reserved_6_11                : 6;
	uint32_t sdem                         : 1;  /**< Surprise Down Error Mask (not supported) */
	uint32_t dlpem                        : 1;  /**< Data Link Protocol Error Mask */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpem                        : 1;
	uint32_t sdem                         : 1;
	uint32_t reserved_6_11                : 6;
	uint32_t ptlpm                        : 1;
	uint32_t fcpem                        : 1;
	uint32_t ctm                          : 1;
	uint32_t cam                          : 1;
	uint32_t ucm                          : 1;
	uint32_t rom                          : 1;
	uint32_t mtlpm                        : 1;
	uint32_t ecrcem                       : 1;
	uint32_t urem                         : 1;
	uint32_t reserved_21_21               : 1;
	uint32_t uciem                        : 1;
	uint32_t reserved_23_23               : 1;
	uint32_t uatombm                      : 1;
	uint32_t reserved_25_31               : 7;
#endif
	} cn70xx;
	struct cvmx_pcieepx_cfg066_cn70xx     cn70xxp1;
	struct cvmx_pcieepx_cfg066_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t tpbem                        : 1;  /**< Unsupported TLP prefix blocked error mask. */
	uint32_t uatombm                      : 1;  /**< Unsupported AtomicOp egress blocked mask. */
	uint32_t reserved_23_23               : 1;
	uint32_t uciem                        : 1;  /**< Uncorrectable internal error mask. */
	uint32_t reserved_21_21               : 1;
	uint32_t urem                         : 1;  /**< Unsupported request error mask. */
	uint32_t ecrcem                       : 1;  /**< ECRC error mask. */
	uint32_t mtlpm                        : 1;  /**< Malformed TLP mask. */
	uint32_t rom                          : 1;  /**< Receiver overflow mask. */
	uint32_t ucm                          : 1;  /**< Unexpected completion mask. */
	uint32_t cam                          : 1;  /**< Completer abort mask. */
	uint32_t ctm                          : 1;  /**< Completion timeout mask. */
	uint32_t fcpem                        : 1;  /**< Flow control protocol error mask. */
	uint32_t ptlpm                        : 1;  /**< Poisoned TLP mask. */
	uint32_t reserved_5_11                : 7;
	uint32_t dlpem                        : 1;  /**< Data link protocol error mask. */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpem                        : 1;
	uint32_t reserved_5_11                : 7;
	uint32_t ptlpm                        : 1;
	uint32_t fcpem                        : 1;
	uint32_t ctm                          : 1;
	uint32_t cam                          : 1;
	uint32_t ucm                          : 1;
	uint32_t rom                          : 1;
	uint32_t mtlpm                        : 1;
	uint32_t ecrcem                       : 1;
	uint32_t urem                         : 1;
	uint32_t reserved_21_21               : 1;
	uint32_t uciem                        : 1;
	uint32_t reserved_23_23               : 1;
	uint32_t uatombm                      : 1;
	uint32_t tpbem                        : 1;
	uint32_t reserved_26_31               : 6;
#endif
	} cn73xx;
	struct cvmx_pcieepx_cfg066_cn73xx     cn78xx;
	struct cvmx_pcieepx_cfg066_cn73xx     cn78xxp1;
	struct cvmx_pcieepx_cfg066_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_25_31               : 7;
	uint32_t uatombm                      : 1;  /**< Unsupported AtomicOp Egress Blocked Mask */
	uint32_t reserved_23_23               : 1;
	uint32_t uciem                        : 1;  /**< Uncorrectable Internal Error Mask */
	uint32_t reserved_21_21               : 1;
	uint32_t urem                         : 1;  /**< Unsupported Request Error Mask */
	uint32_t ecrcem                       : 1;  /**< ECRC Error Mask */
	uint32_t mtlpm                        : 1;  /**< Malformed TLP Mask */
	uint32_t rom                          : 1;  /**< Receiver Overflow Mask */
	uint32_t ucm                          : 1;  /**< Unexpected Completion Mask */
	uint32_t cam                          : 1;  /**< Completer Abort Mask */
	uint32_t ctm                          : 1;  /**< Completion Timeout Mask */
	uint32_t fcpem                        : 1;  /**< Flow Control Protocol Error Mask */
	uint32_t ptlpm                        : 1;  /**< Poisoned TLP Mask */
	uint32_t reserved_5_11                : 7;
	uint32_t dlpem                        : 1;  /**< Data Link Protocol Error Mask */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpem                        : 1;
	uint32_t reserved_5_11                : 7;
	uint32_t ptlpm                        : 1;
	uint32_t fcpem                        : 1;
	uint32_t ctm                          : 1;
	uint32_t cam                          : 1;
	uint32_t ucm                          : 1;
	uint32_t rom                          : 1;
	uint32_t mtlpm                        : 1;
	uint32_t ecrcem                       : 1;
	uint32_t urem                         : 1;
	uint32_t reserved_21_21               : 1;
	uint32_t uciem                        : 1;
	uint32_t reserved_23_23               : 1;
	uint32_t uatombm                      : 1;
	uint32_t reserved_25_31               : 7;
#endif
	} cnf71xx;
	struct cvmx_pcieepx_cfg066_cn73xx     cnf75xx;
};
typedef union cvmx_pcieepx_cfg066 cvmx_pcieepx_cfg066_t;

/**
 * cvmx_pcieep#_cfg067
 *
 * This register contains the sixty-eighth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg067 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg067_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t tpbes                        : 1;  /**< Unsupported TLP prefix blocked error severity. */
	uint32_t uatombs                      : 1;  /**< Unsupported AtomicOp egress blocked severity. */
	uint32_t reserved_23_23               : 1;
	uint32_t ucies                        : 1;  /**< Uncorrectable internal error severity. */
	uint32_t reserved_21_21               : 1;
	uint32_t ures                         : 1;  /**< Unsupported request error severity. */
	uint32_t ecrces                       : 1;  /**< ECRC error severity. */
	uint32_t mtlps                        : 1;  /**< Malformed TLP severity. */
	uint32_t ros                          : 1;  /**< Receiver overflow severity. */
	uint32_t ucs                          : 1;  /**< Unexpected completion severity. */
	uint32_t cas                          : 1;  /**< Completer abort severity. */
	uint32_t cts                          : 1;  /**< Completion timeout severity. */
	uint32_t fcpes                        : 1;  /**< Flow control protocol error severity. */
	uint32_t ptlps                        : 1;  /**< Poisoned TLP severity. */
	uint32_t reserved_6_11                : 6;
	uint32_t sdes                         : 1;  /**< Surprise Down Error Severity (not supported) */
	uint32_t dlpes                        : 1;  /**< Data link protocol error severity. */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpes                        : 1;
	uint32_t sdes                         : 1;
	uint32_t reserved_6_11                : 6;
	uint32_t ptlps                        : 1;
	uint32_t fcpes                        : 1;
	uint32_t cts                          : 1;
	uint32_t cas                          : 1;
	uint32_t ucs                          : 1;
	uint32_t ros                          : 1;
	uint32_t mtlps                        : 1;
	uint32_t ecrces                       : 1;
	uint32_t ures                         : 1;
	uint32_t reserved_21_21               : 1;
	uint32_t ucies                        : 1;
	uint32_t reserved_23_23               : 1;
	uint32_t uatombs                      : 1;
	uint32_t tpbes                        : 1;
	uint32_t reserved_26_31               : 6;
#endif
	} s;
	struct cvmx_pcieepx_cfg067_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_21_31               : 11;
	uint32_t ures                         : 1;  /**< Unsupported Request Error Severity */
	uint32_t ecrces                       : 1;  /**< ECRC Error Severity */
	uint32_t mtlps                        : 1;  /**< Malformed TLP Severity */
	uint32_t ros                          : 1;  /**< Receiver Overflow Severity */
	uint32_t ucs                          : 1;  /**< Unexpected Completion Severity */
	uint32_t cas                          : 1;  /**< Completer Abort Severity */
	uint32_t cts                          : 1;  /**< Completion Timeout Severity */
	uint32_t fcpes                        : 1;  /**< Flow Control Protocol Error Severity */
	uint32_t ptlps                        : 1;  /**< Poisoned TLP Severity */
	uint32_t reserved_6_11                : 6;
	uint32_t sdes                         : 1;  /**< Surprise Down Error Severity (not supported) */
	uint32_t dlpes                        : 1;  /**< Data Link Protocol Error Severity */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpes                        : 1;
	uint32_t sdes                         : 1;
	uint32_t reserved_6_11                : 6;
	uint32_t ptlps                        : 1;
	uint32_t fcpes                        : 1;
	uint32_t cts                          : 1;
	uint32_t cas                          : 1;
	uint32_t ucs                          : 1;
	uint32_t ros                          : 1;
	uint32_t mtlps                        : 1;
	uint32_t ecrces                       : 1;
	uint32_t ures                         : 1;
	uint32_t reserved_21_31               : 11;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg067_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg067_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg067_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg067_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_25_31               : 7;
	uint32_t uatombs                      : 1;  /**< Unsupported AtomicOp Egress Blocked Severity */
	uint32_t reserved_21_23               : 3;
	uint32_t ures                         : 1;  /**< Unsupported Request Error Severity */
	uint32_t ecrces                       : 1;  /**< ECRC Error Severity */
	uint32_t mtlps                        : 1;  /**< Malformed TLP Severity */
	uint32_t ros                          : 1;  /**< Receiver Overflow Severity */
	uint32_t ucs                          : 1;  /**< Unexpected Completion Severity */
	uint32_t cas                          : 1;  /**< Completer Abort Severity */
	uint32_t cts                          : 1;  /**< Completion Timeout Severity */
	uint32_t fcpes                        : 1;  /**< Flow Control Protocol Error Severity */
	uint32_t ptlps                        : 1;  /**< Poisoned TLP Severity */
	uint32_t reserved_6_11                : 6;
	uint32_t sdes                         : 1;  /**< Surprise Down Error Severity (not supported) */
	uint32_t dlpes                        : 1;  /**< Data Link Protocol Error Severity */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpes                        : 1;
	uint32_t sdes                         : 1;
	uint32_t reserved_6_11                : 6;
	uint32_t ptlps                        : 1;
	uint32_t fcpes                        : 1;
	uint32_t cts                          : 1;
	uint32_t cas                          : 1;
	uint32_t ucs                          : 1;
	uint32_t ros                          : 1;
	uint32_t mtlps                        : 1;
	uint32_t ecrces                       : 1;
	uint32_t ures                         : 1;
	uint32_t reserved_21_23               : 3;
	uint32_t uatombs                      : 1;
	uint32_t reserved_25_31               : 7;
#endif
	} cn61xx;
	struct cvmx_pcieepx_cfg067_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg067_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg067_cn61xx     cn66xx;
	struct cvmx_pcieepx_cfg067_cn61xx     cn68xx;
	struct cvmx_pcieepx_cfg067_cn52xx     cn68xxp1;
	struct cvmx_pcieepx_cfg067_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_25_31               : 7;
	uint32_t uatombs                      : 1;  /**< Unsupported AtomicOp Egress Blocked Severity */
	uint32_t reserved_23_23               : 1;
	uint32_t ucies                        : 1;  /**< Uncorrectable Internal Error Severity */
	uint32_t reserved_21_21               : 1;
	uint32_t ures                         : 1;  /**< Unsupported Request Error Severity */
	uint32_t ecrces                       : 1;  /**< ECRC Error Severity */
	uint32_t mtlps                        : 1;  /**< Malformed TLP Severity */
	uint32_t ros                          : 1;  /**< Receiver Overflow Severity */
	uint32_t ucs                          : 1;  /**< Unexpected Completion Severity */
	uint32_t cas                          : 1;  /**< Completer Abort Severity */
	uint32_t cts                          : 1;  /**< Completion Timeout Severity */
	uint32_t fcpes                        : 1;  /**< Flow Control Protocol Error Severity */
	uint32_t ptlps                        : 1;  /**< Poisoned TLP Severity */
	uint32_t reserved_6_11                : 6;
	uint32_t sdes                         : 1;  /**< Surprise Down Error Severity (not supported) */
	uint32_t dlpes                        : 1;  /**< Data Link Protocol Error Severity */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpes                        : 1;
	uint32_t sdes                         : 1;
	uint32_t reserved_6_11                : 6;
	uint32_t ptlps                        : 1;
	uint32_t fcpes                        : 1;
	uint32_t cts                          : 1;
	uint32_t cas                          : 1;
	uint32_t ucs                          : 1;
	uint32_t ros                          : 1;
	uint32_t mtlps                        : 1;
	uint32_t ecrces                       : 1;
	uint32_t ures                         : 1;
	uint32_t reserved_21_21               : 1;
	uint32_t ucies                        : 1;
	uint32_t reserved_23_23               : 1;
	uint32_t uatombs                      : 1;
	uint32_t reserved_25_31               : 7;
#endif
	} cn70xx;
	struct cvmx_pcieepx_cfg067_cn70xx     cn70xxp1;
	struct cvmx_pcieepx_cfg067_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t tpbes                        : 1;  /**< Unsupported TLP prefix blocked error severity. */
	uint32_t uatombs                      : 1;  /**< Unsupported AtomicOp egress blocked severity. */
	uint32_t reserved_23_23               : 1;
	uint32_t ucies                        : 1;  /**< Uncorrectable internal error severity. */
	uint32_t reserved_21_21               : 1;
	uint32_t ures                         : 1;  /**< Unsupported request error severity. */
	uint32_t ecrces                       : 1;  /**< ECRC error severity. */
	uint32_t mtlps                        : 1;  /**< Malformed TLP severity. */
	uint32_t ros                          : 1;  /**< Receiver overflow severity. */
	uint32_t ucs                          : 1;  /**< Unexpected completion severity. */
	uint32_t cas                          : 1;  /**< Completer abort severity. */
	uint32_t cts                          : 1;  /**< Completion timeout severity. */
	uint32_t fcpes                        : 1;  /**< Flow control protocol error severity. */
	uint32_t ptlps                        : 1;  /**< Poisoned TLP severity. */
	uint32_t reserved_11_5                : 7;
	uint32_t dlpes                        : 1;  /**< Data link protocol error severity. */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpes                        : 1;
	uint32_t reserved_11_5                : 7;
	uint32_t ptlps                        : 1;
	uint32_t fcpes                        : 1;
	uint32_t cts                          : 1;
	uint32_t cas                          : 1;
	uint32_t ucs                          : 1;
	uint32_t ros                          : 1;
	uint32_t mtlps                        : 1;
	uint32_t ecrces                       : 1;
	uint32_t ures                         : 1;
	uint32_t reserved_21_21               : 1;
	uint32_t ucies                        : 1;
	uint32_t reserved_23_23               : 1;
	uint32_t uatombs                      : 1;
	uint32_t tpbes                        : 1;
	uint32_t reserved_26_31               : 6;
#endif
	} cn73xx;
	struct cvmx_pcieepx_cfg067_cn73xx     cn78xx;
	struct cvmx_pcieepx_cfg067_cn73xx     cn78xxp1;
	struct cvmx_pcieepx_cfg067_cnf71xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_25_31               : 7;
	uint32_t uatombs                      : 1;  /**< Unsupported AtomicOp Egress Blocked Severity */
	uint32_t reserved_23_23               : 1;
	uint32_t ucies                        : 1;  /**< Uncorrectable Internal Error Severity */
	uint32_t reserved_21_21               : 1;
	uint32_t ures                         : 1;  /**< Unsupported Request Error Severity */
	uint32_t ecrces                       : 1;  /**< ECRC Error Severity */
	uint32_t mtlps                        : 1;  /**< Malformed TLP Severity */
	uint32_t ros                          : 1;  /**< Receiver Overflow Severity */
	uint32_t ucs                          : 1;  /**< Unexpected Completion Severity */
	uint32_t cas                          : 1;  /**< Completer Abort Severity */
	uint32_t cts                          : 1;  /**< Completion Timeout Severity */
	uint32_t fcpes                        : 1;  /**< Flow Control Protocol Error Severity */
	uint32_t ptlps                        : 1;  /**< Poisoned TLP Severity */
	uint32_t reserved_5_11                : 7;
	uint32_t dlpes                        : 1;  /**< Data Link Protocol Error Severity */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t dlpes                        : 1;
	uint32_t reserved_5_11                : 7;
	uint32_t ptlps                        : 1;
	uint32_t fcpes                        : 1;
	uint32_t cts                          : 1;
	uint32_t cas                          : 1;
	uint32_t ucs                          : 1;
	uint32_t ros                          : 1;
	uint32_t mtlps                        : 1;
	uint32_t ecrces                       : 1;
	uint32_t ures                         : 1;
	uint32_t reserved_21_21               : 1;
	uint32_t ucies                        : 1;
	uint32_t reserved_23_23               : 1;
	uint32_t uatombs                      : 1;
	uint32_t reserved_25_31               : 7;
#endif
	} cnf71xx;
	struct cvmx_pcieepx_cfg067_cn73xx     cnf75xx;
};
typedef union cvmx_pcieepx_cfg067 cvmx_pcieepx_cfg067_t;

/**
 * cvmx_pcieep#_cfg068
 *
 * This register contains the sixty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg068 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg068_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_15_31               : 17;
	uint32_t cies                         : 1;  /**< Corrected internal error status. */
	uint32_t anfes                        : 1;  /**< Advisory nonfatal error status. */
	uint32_t rtts                         : 1;  /**< Replay timer timeout status. */
	uint32_t reserved_9_11                : 3;
	uint32_t rnrs                         : 1;  /**< REPLAY_NUM rollover status. */
	uint32_t bdllps                       : 1;  /**< Bad DLLP status. */
	uint32_t btlps                        : 1;  /**< Bad TLP status. */
	uint32_t reserved_1_5                 : 5;
	uint32_t res                          : 1;  /**< Receiver error status. */
#else
	uint32_t res                          : 1;
	uint32_t reserved_1_5                 : 5;
	uint32_t btlps                        : 1;
	uint32_t bdllps                       : 1;
	uint32_t rnrs                         : 1;
	uint32_t reserved_9_11                : 3;
	uint32_t rtts                         : 1;
	uint32_t anfes                        : 1;
	uint32_t cies                         : 1;
	uint32_t reserved_15_31               : 17;
#endif
	} s;
	struct cvmx_pcieepx_cfg068_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_14_31               : 18;
	uint32_t anfes                        : 1;  /**< Advisory Non-Fatal Error Status */
	uint32_t rtts                         : 1;  /**< Reply Timer Timeout Status */
	uint32_t reserved_9_11                : 3;
	uint32_t rnrs                         : 1;  /**< REPLAY_NUM Rollover Status */
	uint32_t bdllps                       : 1;  /**< Bad DLLP Status */
	uint32_t btlps                        : 1;  /**< Bad TLP Status */
	uint32_t reserved_1_5                 : 5;
	uint32_t res                          : 1;  /**< Receiver Error Status */
#else
	uint32_t res                          : 1;
	uint32_t reserved_1_5                 : 5;
	uint32_t btlps                        : 1;
	uint32_t bdllps                       : 1;
	uint32_t rnrs                         : 1;
	uint32_t reserved_9_11                : 3;
	uint32_t rtts                         : 1;
	uint32_t anfes                        : 1;
	uint32_t reserved_14_31               : 18;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg068_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg068_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg068_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg068_cn52xx     cn61xx;
	struct cvmx_pcieepx_cfg068_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg068_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg068_cn52xx     cn66xx;
	struct cvmx_pcieepx_cfg068_cn52xx     cn68xx;
	struct cvmx_pcieepx_cfg068_cn52xx     cn68xxp1;
	struct cvmx_pcieepx_cfg068_s          cn70xx;
	struct cvmx_pcieepx_cfg068_s          cn70xxp1;
	struct cvmx_pcieepx_cfg068_s          cn73xx;
	struct cvmx_pcieepx_cfg068_s          cn78xx;
	struct cvmx_pcieepx_cfg068_s          cn78xxp1;
	struct cvmx_pcieepx_cfg068_s          cnf71xx;
	struct cvmx_pcieepx_cfg068_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg068 cvmx_pcieepx_cfg068_t;

/**
 * cvmx_pcieep#_cfg069
 *
 * This register contains the seventieth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg069 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg069_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_15_31               : 17;
	uint32_t ciem                         : 1;  /**< Corrected internal error mask. */
	uint32_t anfem                        : 1;  /**< Advisory nonfatal error mask. */
	uint32_t rttm                         : 1;  /**< Replay timer timeout mask. */
	uint32_t reserved_9_11                : 3;
	uint32_t rnrm                         : 1;  /**< REPLAY_NUM rollover mask. */
	uint32_t bdllpm                       : 1;  /**< Bad DLLP mask. */
	uint32_t btlpm                        : 1;  /**< Bad TLP mask. */
	uint32_t reserved_1_5                 : 5;
	uint32_t rem                          : 1;  /**< Receiver error mask. */
#else
	uint32_t rem                          : 1;
	uint32_t reserved_1_5                 : 5;
	uint32_t btlpm                        : 1;
	uint32_t bdllpm                       : 1;
	uint32_t rnrm                         : 1;
	uint32_t reserved_9_11                : 3;
	uint32_t rttm                         : 1;
	uint32_t anfem                        : 1;
	uint32_t ciem                         : 1;
	uint32_t reserved_15_31               : 17;
#endif
	} s;
	struct cvmx_pcieepx_cfg069_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_14_31               : 18;
	uint32_t anfem                        : 1;  /**< Advisory Non-Fatal Error Mask */
	uint32_t rttm                         : 1;  /**< Reply Timer Timeout Mask */
	uint32_t reserved_9_11                : 3;
	uint32_t rnrm                         : 1;  /**< REPLAY_NUM Rollover Mask */
	uint32_t bdllpm                       : 1;  /**< Bad DLLP Mask */
	uint32_t btlpm                        : 1;  /**< Bad TLP Mask */
	uint32_t reserved_1_5                 : 5;
	uint32_t rem                          : 1;  /**< Receiver Error Mask */
#else
	uint32_t rem                          : 1;
	uint32_t reserved_1_5                 : 5;
	uint32_t btlpm                        : 1;
	uint32_t bdllpm                       : 1;
	uint32_t rnrm                         : 1;
	uint32_t reserved_9_11                : 3;
	uint32_t rttm                         : 1;
	uint32_t anfem                        : 1;
	uint32_t reserved_14_31               : 18;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg069_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg069_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg069_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg069_cn52xx     cn61xx;
	struct cvmx_pcieepx_cfg069_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg069_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg069_cn52xx     cn66xx;
	struct cvmx_pcieepx_cfg069_cn52xx     cn68xx;
	struct cvmx_pcieepx_cfg069_cn52xx     cn68xxp1;
	struct cvmx_pcieepx_cfg069_s          cn70xx;
	struct cvmx_pcieepx_cfg069_s          cn70xxp1;
	struct cvmx_pcieepx_cfg069_s          cn73xx;
	struct cvmx_pcieepx_cfg069_s          cn78xx;
	struct cvmx_pcieepx_cfg069_s          cn78xxp1;
	struct cvmx_pcieepx_cfg069_s          cnf71xx;
	struct cvmx_pcieepx_cfg069_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg069 cvmx_pcieepx_cfg069_t;

/**
 * cvmx_pcieep#_cfg070
 *
 * This register contains the seventy-first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg070 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg070_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_12_31               : 20;
	uint32_t tlp_plp                      : 1;  /**< TLP prefix log present (not supported). */
	uint32_t reserved_9_10                : 2;
	uint32_t ce                           : 1;  /**< ECRC check enable. */
	uint32_t cc                           : 1;  /**< ECRC check capable. */
	uint32_t ge                           : 1;  /**< ECRC generation enable. */
	uint32_t gc                           : 1;  /**< ECRC generation capability. */
	uint32_t fep                          : 5;  /**< First error pointer. */
#else
	uint32_t fep                          : 5;
	uint32_t gc                           : 1;
	uint32_t ge                           : 1;
	uint32_t cc                           : 1;
	uint32_t ce                           : 1;
	uint32_t reserved_9_10                : 2;
	uint32_t tlp_plp                      : 1;
	uint32_t reserved_12_31               : 20;
#endif
	} s;
	struct cvmx_pcieepx_cfg070_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_9_31                : 23;
	uint32_t ce                           : 1;  /**< ECRC Check Enable */
	uint32_t cc                           : 1;  /**< ECRC Check Capable */
	uint32_t ge                           : 1;  /**< ECRC Generation Enable */
	uint32_t gc                           : 1;  /**< ECRC Generation Capability */
	uint32_t fep                          : 5;  /**< First Error Pointer */
#else
	uint32_t fep                          : 5;
	uint32_t gc                           : 1;
	uint32_t ge                           : 1;
	uint32_t cc                           : 1;
	uint32_t ce                           : 1;
	uint32_t reserved_9_31                : 23;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg070_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg070_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg070_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg070_cn52xx     cn61xx;
	struct cvmx_pcieepx_cfg070_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg070_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg070_cn52xx     cn66xx;
	struct cvmx_pcieepx_cfg070_cn52xx     cn68xx;
	struct cvmx_pcieepx_cfg070_cn52xx     cn68xxp1;
	struct cvmx_pcieepx_cfg070_cn52xx     cn70xx;
	struct cvmx_pcieepx_cfg070_cn52xx     cn70xxp1;
	struct cvmx_pcieepx_cfg070_s          cn73xx;
	struct cvmx_pcieepx_cfg070_s          cn78xx;
	struct cvmx_pcieepx_cfg070_s          cn78xxp1;
	struct cvmx_pcieepx_cfg070_cn52xx     cnf71xx;
	struct cvmx_pcieepx_cfg070_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg070 cvmx_pcieepx_cfg070_t;

/**
 * cvmx_pcieep#_cfg071
 *
 * This register contains the seventy-second 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg071 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg071_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dword1                       : 32; /**< Header log register (first DWORD). */
#else
	uint32_t dword1                       : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg071_s          cn52xx;
	struct cvmx_pcieepx_cfg071_s          cn52xxp1;
	struct cvmx_pcieepx_cfg071_s          cn56xx;
	struct cvmx_pcieepx_cfg071_s          cn56xxp1;
	struct cvmx_pcieepx_cfg071_s          cn61xx;
	struct cvmx_pcieepx_cfg071_s          cn63xx;
	struct cvmx_pcieepx_cfg071_s          cn63xxp1;
	struct cvmx_pcieepx_cfg071_s          cn66xx;
	struct cvmx_pcieepx_cfg071_s          cn68xx;
	struct cvmx_pcieepx_cfg071_s          cn68xxp1;
	struct cvmx_pcieepx_cfg071_s          cn70xx;
	struct cvmx_pcieepx_cfg071_s          cn70xxp1;
	struct cvmx_pcieepx_cfg071_s          cn73xx;
	struct cvmx_pcieepx_cfg071_s          cn78xx;
	struct cvmx_pcieepx_cfg071_s          cn78xxp1;
	struct cvmx_pcieepx_cfg071_s          cnf71xx;
	struct cvmx_pcieepx_cfg071_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg071 cvmx_pcieepx_cfg071_t;

/**
 * cvmx_pcieep#_cfg072
 *
 * This register contains the seventy-third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg072 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg072_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dword2                       : 32; /**< Header log register (second DWORD). */
#else
	uint32_t dword2                       : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg072_s          cn52xx;
	struct cvmx_pcieepx_cfg072_s          cn52xxp1;
	struct cvmx_pcieepx_cfg072_s          cn56xx;
	struct cvmx_pcieepx_cfg072_s          cn56xxp1;
	struct cvmx_pcieepx_cfg072_s          cn61xx;
	struct cvmx_pcieepx_cfg072_s          cn63xx;
	struct cvmx_pcieepx_cfg072_s          cn63xxp1;
	struct cvmx_pcieepx_cfg072_s          cn66xx;
	struct cvmx_pcieepx_cfg072_s          cn68xx;
	struct cvmx_pcieepx_cfg072_s          cn68xxp1;
	struct cvmx_pcieepx_cfg072_s          cn70xx;
	struct cvmx_pcieepx_cfg072_s          cn70xxp1;
	struct cvmx_pcieepx_cfg072_s          cn73xx;
	struct cvmx_pcieepx_cfg072_s          cn78xx;
	struct cvmx_pcieepx_cfg072_s          cn78xxp1;
	struct cvmx_pcieepx_cfg072_s          cnf71xx;
	struct cvmx_pcieepx_cfg072_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg072 cvmx_pcieepx_cfg072_t;

/**
 * cvmx_pcieep#_cfg073
 *
 * This register contains the seventy-fourth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg073 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg073_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dword3                       : 32; /**< Header log register (third DWORD). */
#else
	uint32_t dword3                       : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg073_s          cn52xx;
	struct cvmx_pcieepx_cfg073_s          cn52xxp1;
	struct cvmx_pcieepx_cfg073_s          cn56xx;
	struct cvmx_pcieepx_cfg073_s          cn56xxp1;
	struct cvmx_pcieepx_cfg073_s          cn61xx;
	struct cvmx_pcieepx_cfg073_s          cn63xx;
	struct cvmx_pcieepx_cfg073_s          cn63xxp1;
	struct cvmx_pcieepx_cfg073_s          cn66xx;
	struct cvmx_pcieepx_cfg073_s          cn68xx;
	struct cvmx_pcieepx_cfg073_s          cn68xxp1;
	struct cvmx_pcieepx_cfg073_s          cn70xx;
	struct cvmx_pcieepx_cfg073_s          cn70xxp1;
	struct cvmx_pcieepx_cfg073_s          cn73xx;
	struct cvmx_pcieepx_cfg073_s          cn78xx;
	struct cvmx_pcieepx_cfg073_s          cn78xxp1;
	struct cvmx_pcieepx_cfg073_s          cnf71xx;
	struct cvmx_pcieepx_cfg073_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg073 cvmx_pcieepx_cfg073_t;

/**
 * cvmx_pcieep#_cfg074
 *
 * This register contains the seventy-fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg074 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg074_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dword4                       : 32; /**< Header log register (fourth DWORD). */
#else
	uint32_t dword4                       : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg074_s          cn52xx;
	struct cvmx_pcieepx_cfg074_s          cn52xxp1;
	struct cvmx_pcieepx_cfg074_s          cn56xx;
	struct cvmx_pcieepx_cfg074_s          cn56xxp1;
	struct cvmx_pcieepx_cfg074_s          cn61xx;
	struct cvmx_pcieepx_cfg074_s          cn63xx;
	struct cvmx_pcieepx_cfg074_s          cn63xxp1;
	struct cvmx_pcieepx_cfg074_s          cn66xx;
	struct cvmx_pcieepx_cfg074_s          cn68xx;
	struct cvmx_pcieepx_cfg074_s          cn68xxp1;
	struct cvmx_pcieepx_cfg074_s          cn70xx;
	struct cvmx_pcieepx_cfg074_s          cn70xxp1;
	struct cvmx_pcieepx_cfg074_s          cn73xx;
	struct cvmx_pcieepx_cfg074_s          cn78xx;
	struct cvmx_pcieepx_cfg074_s          cn78xxp1;
	struct cvmx_pcieepx_cfg074_s          cnf71xx;
	struct cvmx_pcieepx_cfg074_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg074 cvmx_pcieepx_cfg074_t;

/**
 * cvmx_pcieep#_cfg078
 *
 * This register contains the seventy-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg078 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg078_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t tlp_pfx_log                  : 32; /**< TLP prefix log register. */
#else
	uint32_t tlp_pfx_log                  : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg078_s          cn73xx;
	struct cvmx_pcieepx_cfg078_s          cn78xx;
	struct cvmx_pcieepx_cfg078_s          cn78xxp1;
	struct cvmx_pcieepx_cfg078_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg078 cvmx_pcieepx_cfg078_t;

/**
 * cvmx_pcieep#_cfg082
 *
 * This register contains the eighty-third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg082 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg082_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next capability offset. Points to the secondary PCI Express capabilities by default. */
	uint32_t cv                           : 4;  /**< Capability version. */
	uint32_t reserved_0_15                : 16;
#else
	uint32_t reserved_0_15                : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} s;
	struct cvmx_pcieepx_cfg082_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next Capability Offset */
	uint32_t cv                           : 4;  /**< Capability Version */
	uint32_t pcieec                       : 16; /**< PCIE Express Extended Capability */
#else
	uint32_t pcieec                       : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} cn70xx;
	struct cvmx_pcieepx_cfg082_cn70xx     cn70xxp1;
	struct cvmx_pcieepx_cfg082_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next capability offset. Points to the secondary PCI Express capabilities by default. */
	uint32_t cv                           : 4;  /**< Capability version. */
	uint32_t ariid                        : 16; /**< PCI Express extended capability. */
#else
	uint32_t ariid                        : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} cn73xx;
	struct cvmx_pcieepx_cfg082_cn73xx     cn78xx;
	struct cvmx_pcieepx_cfg082_cn73xx     cn78xxp1;
	struct cvmx_pcieepx_cfg082_cn73xx     cnf75xx;
};
typedef union cvmx_pcieepx_cfg082 cvmx_pcieepx_cfg082_t;

/**
 * cvmx_pcieep#_cfg083
 *
 * This register contains the eighty-fourth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg083 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg083_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_2_31                : 30;
	uint32_t acsfgc                       : 1;  /**< ACS function groups capability. */
	uint32_t mfvcfgc                      : 1;  /**< MFVC function groups capability. */
#else
	uint32_t mfvcfgc                      : 1;
	uint32_t acsfgc                       : 1;
	uint32_t reserved_2_31                : 30;
#endif
	} s;
	struct cvmx_pcieepx_cfg083_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t srs                          : 22; /**< "Supported Resource Sizes
                                                         The OCTEON advertises the maximum allowable BAR size (512GB - 0xf_ffff)
                                                         when the fus__bar2_size_conf is intact. When the fuse is blown,
                                                         the OCTEON advertised a BAR size of 2TB (0x3f_ffff).
                                                         The BAR is disabled at runtime by writing all zeros through
                                                         PEM#_CFG_WR to this field" */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t srs                          : 22;
	uint32_t reserved_26_31               : 6;
#endif
	} cn70xx;
	struct cvmx_pcieepx_cfg083_cn70xx     cn70xxp1;
	struct cvmx_pcieepx_cfg083_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_23_31               : 9;
	uint32_t fg                           : 3;  /**< Function group. */
	uint32_t reserved_18_19               : 2;
	uint32_t acsfge                       : 1;  /**< ACS function groups enable (A). */
	uint32_t mfvcfge                      : 1;  /**< MFVC function groups enable (M). */
	uint32_t nfn                          : 8;  /**< Next function number. */
	uint32_t reserved_2_7                 : 6;
	uint32_t acsfgc                       : 1;  /**< ACS function groups capability. */
	uint32_t mfvcfgc                      : 1;  /**< MFVC function groups capability. */
#else
	uint32_t mfvcfgc                      : 1;
	uint32_t acsfgc                       : 1;
	uint32_t reserved_2_7                 : 6;
	uint32_t nfn                          : 8;
	uint32_t mfvcfge                      : 1;
	uint32_t acsfge                       : 1;
	uint32_t reserved_18_19               : 2;
	uint32_t fg                           : 3;
	uint32_t reserved_23_31               : 9;
#endif
	} cn73xx;
	struct cvmx_pcieepx_cfg083_cn73xx     cn78xx;
	struct cvmx_pcieepx_cfg083_cn73xx     cn78xxp1;
	struct cvmx_pcieepx_cfg083_cn73xx     cnf75xx;
};
typedef union cvmx_pcieepx_cfg083 cvmx_pcieepx_cfg083_t;

/**
 * cvmx_pcieep#_cfg084
 *
 * PCIE_CFG084 = Eighty-fifth 32-bits of PCIE type 0 config space
 * (PCI Express Resizable BAR (RBAR) Control Register)
 */
union cvmx_pcieepx_cfg084 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg084_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_13_31               : 19;
	uint32_t rbars                        : 5;  /**< BAR Size
                                                         The OCTEON advertises the minimum allowable BAR size of 0x0 (1MB)
                                                         but will accept values as large as 0x15 (2TB) */
	uint32_t nrbar                        : 3;  /**< Number of Resizable BARs */
	uint32_t reserved_3_4                 : 2;
	uint32_t rbari                        : 3;  /**< BAR Index. Points to BAR2. */
#else
	uint32_t rbari                        : 3;
	uint32_t reserved_3_4                 : 2;
	uint32_t nrbar                        : 3;
	uint32_t rbars                        : 5;
	uint32_t reserved_13_31               : 19;
#endif
	} s;
	struct cvmx_pcieepx_cfg084_s          cn70xx;
	struct cvmx_pcieepx_cfg084_s          cn70xxp1;
};
typedef union cvmx_pcieepx_cfg084 cvmx_pcieepx_cfg084_t;

/**
 * cvmx_pcieep#_cfg086
 *
 * This register contains the eighty-seventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg086 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg086_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next capability offset. Points to the PCI Express SR-IOV capability header by default.
                                                         Writable through PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t cv                           : 4;  /**< Capability version
                                                         Writable through PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t pcieec                       : 16; /**< PCIE Express extended capability
                                                         Writable through PEM()_CFG_WR. However, the application must not change this field. */
#else
	uint32_t pcieec                       : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} s;
	struct cvmx_pcieepx_cfg086_s          cn73xx;
	struct cvmx_pcieepx_cfg086_s          cn78xx;
	struct cvmx_pcieepx_cfg086_s          cn78xxp1;
	struct cvmx_pcieepx_cfg086_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg086 cvmx_pcieepx_cfg086_t;

/**
 * cvmx_pcieep#_cfg087
 *
 * This register contains the eighty-eighth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg087 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg087_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg087_s          cn73xx;
	struct cvmx_pcieepx_cfg087_s          cn78xx;
	struct cvmx_pcieepx_cfg087_s          cn78xxp1;
	struct cvmx_pcieepx_cfg087_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg087 cvmx_pcieepx_cfg087_t;

/**
 * cvmx_pcieep#_cfg088
 *
 * This register contains the eighty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg088 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg088_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_8_31                : 24;
	uint32_t les                          : 8;  /**< Lane error status bits. */
#else
	uint32_t les                          : 8;
	uint32_t reserved_8_31                : 24;
#endif
	} s;
	struct cvmx_pcieepx_cfg088_s          cn73xx;
	struct cvmx_pcieepx_cfg088_s          cn78xx;
	struct cvmx_pcieepx_cfg088_s          cn78xxp1;
	struct cvmx_pcieepx_cfg088_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg088 cvmx_pcieepx_cfg088_t;

/**
 * cvmx_pcieep#_cfg089
 *
 * This register contains the ninetieth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg089 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg089_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t l1dph                        : 3;  /**< Lane 1 downstream component receiver preset hint. Writable through PEM()_CFG_WR. */
	uint32_t l1dtp                        : 4;  /**< Lane 1 downstream component transmitter preset. Writable through PEM()_CFG_WR. */
	uint32_t reserved_15_23               : 9;
	uint32_t l0dph                        : 3;  /**< Lane 0 downstream component receiver preset hint. Writable through PEM()_CFG_WR. */
	uint32_t l0dtp                        : 4;  /**< Lane 0 downstream component transmitter preset. Writable through PEM()_CFG_WR. */
	uint32_t reserved_0_7                 : 8;
#else
	uint32_t reserved_0_7                 : 8;
	uint32_t l0dtp                        : 4;
	uint32_t l0dph                        : 3;
	uint32_t reserved_15_23               : 9;
	uint32_t l1dtp                        : 4;
	uint32_t l1dph                        : 3;
	uint32_t reserved_31_31               : 1;
#endif
	} s;
	struct cvmx_pcieepx_cfg089_s          cn73xx;
	struct cvmx_pcieepx_cfg089_s          cn78xx;
	struct cvmx_pcieepx_cfg089_s          cn78xxp1;
	struct cvmx_pcieepx_cfg089_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg089 cvmx_pcieepx_cfg089_t;

/**
 * cvmx_pcieep#_cfg090
 *
 * This register contains the ninety-first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg090 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg090_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t l3dph                        : 3;  /**< Lane 3 downstream component receiver preset hint. Writable through PEM()_CFG_WR. */
	uint32_t l3dtp                        : 4;  /**< Lane 3 downstream component transmitter preset. Writable through PEM()_CFG_WR. */
	uint32_t reserved_15_23               : 9;
	uint32_t l2dph                        : 3;  /**< Lane 2 downstream component receiver preset hint. Writable through PEM()_CFG_WR. */
	uint32_t l2dtp                        : 4;  /**< Lane 2 downstream component transmitter preset. Writable through PEM()_CFG_WR. */
	uint32_t reserved_0_7                 : 8;
#else
	uint32_t reserved_0_7                 : 8;
	uint32_t l2dtp                        : 4;
	uint32_t l2dph                        : 3;
	uint32_t reserved_15_23               : 9;
	uint32_t l3dtp                        : 4;
	uint32_t l3dph                        : 3;
	uint32_t reserved_31_31               : 1;
#endif
	} s;
	struct cvmx_pcieepx_cfg090_s          cn73xx;
	struct cvmx_pcieepx_cfg090_s          cn78xx;
	struct cvmx_pcieepx_cfg090_s          cn78xxp1;
	struct cvmx_pcieepx_cfg090_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg090 cvmx_pcieepx_cfg090_t;

/**
 * cvmx_pcieep#_cfg091
 *
 * This register contains the ninety-second 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg091 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg091_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t l5dph                        : 3;  /**< Lane 5 downstream component receiver preset hint. Writable through PEM()_CFG_WR. */
	uint32_t l5dtp                        : 4;  /**< Lane 5 downstream component transmitter preset. Writable through PEM()_CFG_WR. */
	uint32_t reserved_15_23               : 9;
	uint32_t l4dph                        : 3;  /**< Lane 4 downstream component receiver preset hint. Writable through PEM()_CFG_WR. */
	uint32_t l4dtp                        : 4;  /**< Lane 4 downstream component transmitter preset. Writable through PEM()_CFG_WR. */
	uint32_t reserved_0_7                 : 8;
#else
	uint32_t reserved_0_7                 : 8;
	uint32_t l4dtp                        : 4;
	uint32_t l4dph                        : 3;
	uint32_t reserved_15_23               : 9;
	uint32_t l5dtp                        : 4;
	uint32_t l5dph                        : 3;
	uint32_t reserved_31_31               : 1;
#endif
	} s;
	struct cvmx_pcieepx_cfg091_s          cn73xx;
	struct cvmx_pcieepx_cfg091_s          cn78xx;
	struct cvmx_pcieepx_cfg091_s          cn78xxp1;
	struct cvmx_pcieepx_cfg091_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg091 cvmx_pcieepx_cfg091_t;

/**
 * cvmx_pcieep#_cfg092
 *
 * This register contains the ninety-fourth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg092 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg092_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t l7dph                        : 3;  /**< Lane 7 downstream component receiver preset hint. Writable through PEM()_CFG_WR. */
	uint32_t l7dtp                        : 4;  /**< Lane 7 downstream component transmitter preset. Writable through PEM()_CFG_WR. */
	uint32_t reserved_15_23               : 9;
	uint32_t l6dph                        : 3;  /**< Lane 6 downstream component receiver preset hint. Writable through PEM()_CFG_WR. */
	uint32_t l6dtp                        : 4;  /**< Lane 6 downstream component transmitter preset. Writable through PEM()_CFG_WR. */
	uint32_t reserved_0_7                 : 8;
#else
	uint32_t reserved_0_7                 : 8;
	uint32_t l6dtp                        : 4;
	uint32_t l6dph                        : 3;
	uint32_t reserved_15_23               : 9;
	uint32_t l7dtp                        : 4;
	uint32_t l7dph                        : 3;
	uint32_t reserved_31_31               : 1;
#endif
	} s;
	struct cvmx_pcieepx_cfg092_s          cn73xx;
	struct cvmx_pcieepx_cfg092_s          cn78xx;
	struct cvmx_pcieepx_cfg092_s          cn78xxp1;
	struct cvmx_pcieepx_cfg092_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg092 cvmx_pcieepx_cfg092_t;

/**
 * cvmx_pcieep#_cfg094
 *
 * This register contains the ninety-fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg094 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg094_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next capability offset. Points to the resizable BAR capabilities by default */
	uint32_t cv                           : 4;  /**< Capability version. */
	uint32_t pcieec                       : 16; /**< PCIE Express extended capability. */
#else
	uint32_t pcieec                       : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} s;
	struct cvmx_pcieepx_cfg094_s          cn73xx;
	struct cvmx_pcieepx_cfg094_s          cn78xx;
	struct cvmx_pcieepx_cfg094_s          cn78xxp1;
	struct cvmx_pcieepx_cfg094_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg094 cvmx_pcieepx_cfg094_t;

/**
 * cvmx_pcieep#_cfg095
 *
 * This register contains the ninety-sixth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg095 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg095_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t vfmimn                       : 11; /**< VF migration interrupt message number. */
	uint32_t reserved_2_20                : 19;
	uint32_t arichp                       : 1;  /**< ARI capable hierarchy preserved. Writable through PEM()_CFG_WR. However, the application
                                                         must not change this field. */
	uint32_t vfmc                         : 1;  /**< VF migration capable. */
#else
	uint32_t vfmc                         : 1;
	uint32_t arichp                       : 1;
	uint32_t reserved_2_20                : 19;
	uint32_t vfmimn                       : 11;
#endif
	} s;
	struct cvmx_pcieepx_cfg095_s          cn73xx;
	struct cvmx_pcieepx_cfg095_s          cn78xx;
	struct cvmx_pcieepx_cfg095_s          cn78xxp1;
	struct cvmx_pcieepx_cfg095_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg095 cvmx_pcieepx_cfg095_t;

/**
 * cvmx_pcieep#_cfg096
 *
 * This register contains the ninety-seventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg096 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg096_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_17_31               : 15;
	uint32_t ms                           : 1;  /**< VF migration status. */
	uint32_t reserved_5_15                : 11;
	uint32_t ach                          : 1;  /**< ARI capable hierarchy.
                                                         0 = All PFs have non-ARI capable hierarchy.
                                                         1 = All PFs have ARI capable hierarchy.
                                                         The value in this field in PF0 is used for all other physical functions. */
	uint32_t mse                          : 1;  /**< VF MSE. */
	uint32_t mie                          : 1;  /**< VF migration interrupt enable. */
	uint32_t me                           : 1;  /**< VF migration enable. */
	uint32_t vfe                          : 1;  /**< VF enable. */
#else
	uint32_t vfe                          : 1;
	uint32_t me                           : 1;
	uint32_t mie                          : 1;
	uint32_t mse                          : 1;
	uint32_t ach                          : 1;
	uint32_t reserved_5_15                : 11;
	uint32_t ms                           : 1;
	uint32_t reserved_17_31               : 15;
#endif
	} s;
	struct cvmx_pcieepx_cfg096_s          cn73xx;
	struct cvmx_pcieepx_cfg096_s          cn78xx;
	struct cvmx_pcieepx_cfg096_s          cn78xxp1;
	struct cvmx_pcieepx_cfg096_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg096 cvmx_pcieepx_cfg096_t;

/**
 * cvmx_pcieep#_cfg097
 *
 * This register contains the ninety-eighth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg097 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg097_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t tvf                          : 16; /**< Total VFs.  Read-only copy of PCIEEP()_CFG097[IVF]. */
	uint32_t ivf                          : 16; /**< Initial VFs.
                                                         Reset values:
                                                         _ ARI:          0x40.
                                                         _ non-ARI:      0x40.
                                                         There are two InitialVFs registers; one for each ARI Capable
                                                         and non-ARI capable hierarchies.  The PCIEEP()_CFG096[ARI] determines which one is
                                                         being used for SR-IOV, and which one is accessed by a read request.
                                                         This field is writable through PEM()_CFG_WR, PEM()_CFG_WR[ADDR[31]] determines
                                                         which IVF register is updated.
                                                         0 = accesses non-ARI Capable Hierarchy copy of IVF.
                                                         1 = accesses ARI Capable Hierarchy copy of IVF. */
#else
	uint32_t ivf                          : 16;
	uint32_t tvf                          : 16;
#endif
	} s;
	struct cvmx_pcieepx_cfg097_s          cn73xx;
	struct cvmx_pcieepx_cfg097_s          cn78xx;
	struct cvmx_pcieepx_cfg097_s          cn78xxp1;
	struct cvmx_pcieepx_cfg097_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg097 cvmx_pcieepx_cfg097_t;

/**
 * cvmx_pcieep#_cfg098
 *
 * This register contains the ninety-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg098 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg098_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t fdl                          : 8;  /**< Function dependency link. */
	uint32_t nvf                          : 16; /**< Number of VFs that are visible. */
#else
	uint32_t nvf                          : 16;
	uint32_t fdl                          : 8;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_pcieepx_cfg098_s          cn73xx;
	struct cvmx_pcieepx_cfg098_s          cn78xx;
	struct cvmx_pcieepx_cfg098_s          cn78xxp1;
	struct cvmx_pcieepx_cfg098_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg098 cvmx_pcieepx_cfg098_t;

/**
 * cvmx_pcieep#_cfg099
 *
 * This register contains the one hundredth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg099 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg099_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t vfs                          : 16; /**< VF stride.
                                                         Reset values:
                                                         _ ARI:          0x1.
                                                         _ non-ARI:      0x1.
                                                         There are two VF Stride registers per PF; one for each ARI Capable
                                                         and non-ARI capable hierarchies.  The PCIEEP()_CFG096[ACH] determines which one is
                                                         being used for SR-IOV, and which one is accessed by a read request.
                                                         This field is writable through PEM()_CFG_WR, PEM()_CFG_WR[ADDR[31]] determines
                                                         which VFS register is updated.
                                                         0 = accesses non-ARI capable hierarchy copy of VFS.
                                                         1 = accesses ARI capable hierarchy copy of VFS. */
	uint32_t fo                           : 16; /**< First VF offset.
                                                         Reset values:
                                                         _ ARI:          0x1.
                                                         _ non-ARI:      0x1.
                                                         There are two first VF offset registers;  one for each ARI capable
                                                         and non-ARI capable hierarchies.  The PCIEEP()_CFG096[ARI] determines which one is
                                                         being used for SR-IOV, and which one is accessed by a read request.
                                                         This field is writable through PEM()_CFG_WR, PEM()_CFG_WR[ADDR[31]] determines
                                                         which FO register is updated.
                                                         0 = accesses non-ARI capable hierarchy copy of FO.
                                                         1 = accesses ARI capable hierarchy copy of FO. */
#else
	uint32_t fo                           : 16;
	uint32_t vfs                          : 16;
#endif
	} s;
	struct cvmx_pcieepx_cfg099_s          cn73xx;
	struct cvmx_pcieepx_cfg099_s          cn78xx;
	struct cvmx_pcieepx_cfg099_s          cn78xxp1;
	struct cvmx_pcieepx_cfg099_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg099 cvmx_pcieepx_cfg099_t;

/**
 * cvmx_pcieep#_cfg100
 *
 * This register contains the one hundred first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg100 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg100_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t vfdev                        : 16; /**< VF device ID. */
	uint32_t reserved_0_15                : 16;
#else
	uint32_t reserved_0_15                : 16;
	uint32_t vfdev                        : 16;
#endif
	} s;
	struct cvmx_pcieepx_cfg100_s          cn73xx;
	struct cvmx_pcieepx_cfg100_s          cn78xx;
	struct cvmx_pcieepx_cfg100_s          cn78xxp1;
	struct cvmx_pcieepx_cfg100_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg100 cvmx_pcieepx_cfg100_t;

/**
 * cvmx_pcieep#_cfg101
 *
 * This register contains the one hundred second 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg101 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg101_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t supps                        : 32; /**< Supported page sizes. */
#else
	uint32_t supps                        : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg101_s          cn73xx;
	struct cvmx_pcieepx_cfg101_s          cn78xx;
	struct cvmx_pcieepx_cfg101_s          cn78xxp1;
	struct cvmx_pcieepx_cfg101_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg101 cvmx_pcieepx_cfg101_t;

/**
 * cvmx_pcieep#_cfg102
 *
 * This register contains the one hundred third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg102 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg102_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ps                           : 32; /**< System page size. */
#else
	uint32_t ps                           : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg102_s          cn73xx;
	struct cvmx_pcieepx_cfg102_s          cn78xx;
	struct cvmx_pcieepx_cfg102_s          cn78xxp1;
	struct cvmx_pcieepx_cfg102_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg102 cvmx_pcieepx_cfg102_t;

/**
 * cvmx_pcieep#_cfg103
 *
 * This register contains the one hundred fourth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg103 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg103_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_4_31                : 28;
	uint32_t pf                           : 1;  /**< Prefetchable. */
	uint32_t typ                          : 2;  /**< BAR type:
                                                         0x0 = 32-bit BAR.
                                                         0x2 = 64-bit BAR. */
	uint32_t mspc                         : 1;  /**< Memory space indicator:
                                                         0 = BAR 0 is a memory BAR.
                                                         1 = BAR 0 is an I/O BAR. */
#else
	uint32_t mspc                         : 1;
	uint32_t typ                          : 2;
	uint32_t pf                           : 1;
	uint32_t reserved_4_31                : 28;
#endif
	} s;
	struct cvmx_pcieepx_cfg103_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lbab                         : 12; /**< Lower bits of the VF BAR 0 base address. */
	uint32_t reserved_4_19                : 16;
	uint32_t pf                           : 1;  /**< Prefetchable. */
	uint32_t typ                          : 2;  /**< BAR type:
                                                         0x0 = 32-bit BAR.
                                                         0x2 = 64-bit BAR. */
	uint32_t mspc                         : 1;  /**< Memory space indicator:
                                                         0 = BAR 0 is a memory BAR.
                                                         1 = BAR 0 is an I/O BAR. */
#else
	uint32_t mspc                         : 1;
	uint32_t typ                          : 2;
	uint32_t pf                           : 1;
	uint32_t reserved_4_19                : 16;
	uint32_t lbab                         : 12;
#endif
	} cn73xx;
	struct cvmx_pcieepx_cfg103_cn73xx     cn78xx;
	struct cvmx_pcieepx_cfg103_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lbab                         : 17; /**< Lower bits of the VF BAR 0 base address. */
	uint32_t reserved_4_14                : 11;
	uint32_t pf                           : 1;  /**< Prefetchable. */
	uint32_t typ                          : 2;  /**< BAR type:
                                                         0x0 = 32-bit BAR.
                                                         0x2 = 64-bit BAR. */
	uint32_t mspc                         : 1;  /**< Memory space indicator:
                                                         0 = BAR 0 is a memory BAR.
                                                         1 = BAR 0 is an I/O BAR. */
#else
	uint32_t mspc                         : 1;
	uint32_t typ                          : 2;
	uint32_t pf                           : 1;
	uint32_t reserved_4_14                : 11;
	uint32_t lbab                         : 17;
#endif
	} cn78xxp1;
	struct cvmx_pcieepx_cfg103_cn73xx     cnf75xx;
};
typedef union cvmx_pcieepx_cfg103 cvmx_pcieepx_cfg103_t;

/**
 * cvmx_pcieep#_cfg104
 *
 * This register contains the one hundred seventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg104 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg104_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ubab                         : 32; /**< Upper bits of the VF BAR 0 base address. */
#else
	uint32_t ubab                         : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg104_s          cn73xx;
	struct cvmx_pcieepx_cfg104_s          cn78xx;
	struct cvmx_pcieepx_cfg104_s          cn78xxp1;
	struct cvmx_pcieepx_cfg104_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg104 cvmx_pcieepx_cfg104_t;

/**
 * cvmx_pcieep#_cfg105
 *
 * This register contains the one hundred sixth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg105 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg105_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg105_s          cn73xx;
	struct cvmx_pcieepx_cfg105_s          cn78xx;
	struct cvmx_pcieepx_cfg105_s          cn78xxp1;
	struct cvmx_pcieepx_cfg105_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg105 cvmx_pcieepx_cfg105_t;

/**
 * cvmx_pcieep#_cfg106
 *
 * This register contains the one hundred seventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg106 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg106_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg106_s          cn73xx;
	struct cvmx_pcieepx_cfg106_s          cn78xx;
	struct cvmx_pcieepx_cfg106_s          cn78xxp1;
	struct cvmx_pcieepx_cfg106_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg106 cvmx_pcieepx_cfg106_t;

/**
 * cvmx_pcieep#_cfg107
 *
 * This register contains the one hundred eighth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg107 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg107_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg107_s          cn73xx;
	struct cvmx_pcieepx_cfg107_s          cn78xx;
	struct cvmx_pcieepx_cfg107_s          cn78xxp1;
	struct cvmx_pcieepx_cfg107_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg107 cvmx_pcieepx_cfg107_t;

/**
 * cvmx_pcieep#_cfg108
 *
 * This register contains the one hundred ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg108 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg108_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_0_31                : 32;
#else
	uint32_t reserved_0_31                : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg108_s          cn73xx;
	struct cvmx_pcieepx_cfg108_s          cn78xx;
	struct cvmx_pcieepx_cfg108_s          cn78xxp1;
	struct cvmx_pcieepx_cfg108_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg108 cvmx_pcieepx_cfg108_t;

/**
 * cvmx_pcieep#_cfg109
 *
 * This register contains the one hundred tenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg109 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg109_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t mso                          : 29; /**< VF migration state offset. */
	uint32_t msbir                        : 3;  /**< VF migration state BIR. */
#else
	uint32_t msbir                        : 3;
	uint32_t mso                          : 29;
#endif
	} s;
	struct cvmx_pcieepx_cfg109_s          cn73xx;
	struct cvmx_pcieepx_cfg109_s          cn78xx;
	struct cvmx_pcieepx_cfg109_s          cn78xxp1;
	struct cvmx_pcieepx_cfg109_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg109 cvmx_pcieepx_cfg109_t;

/**
 * cvmx_pcieep#_cfg110
 *
 * This register contains the one hundred eleventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg110 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg110_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next capability offset. */
	uint32_t cv                           : 4;  /**< Capability version. */
	uint32_t pcieec                       : 16; /**< PCI Express extended capability. */
#else
	uint32_t pcieec                       : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} s;
	struct cvmx_pcieepx_cfg110_s          cn73xx;
	struct cvmx_pcieepx_cfg110_s          cn78xx;
	struct cvmx_pcieepx_cfg110_s          cn78xxp1;
	struct cvmx_pcieepx_cfg110_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg110 cvmx_pcieepx_cfg110_t;

/**
 * cvmx_pcieep#_cfg111
 *
 * This register contains the one hundred twelfth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg111 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg111_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t srs                          : 26; /**< "Supported resource sizes. PEM advertises the maximum allowable BAR size (512 GB -
                                                         0xF_FFFF) when the fus__bar2_size_conf is in tact. When the fuse is blown, the CNXXXX
                                                         advertises a BAR size of 32TB (0x3FF_FFFF). The BAR is disabled at runtime by writing all
                                                         zeros through PEM()_CFG_WR to this field." */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t srs                          : 26;
	uint32_t reserved_30_31               : 2;
#endif
	} s;
	struct cvmx_pcieepx_cfg111_s          cn73xx;
	struct cvmx_pcieepx_cfg111_s          cn78xx;
	struct cvmx_pcieepx_cfg111_s          cn78xxp1;
	struct cvmx_pcieepx_cfg111_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg111 cvmx_pcieepx_cfg111_t;

/**
 * cvmx_pcieep#_cfg112
 *
 * This register contains the one hundred thirteenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg112 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg112_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_13_31               : 19;
	uint32_t rbars                        : 5;  /**< BAR Size. PEM advertises the minimum allowable BAR size of 0x0 (1MB) but will accept
                                                         values as large as 0x19 (32TB). */
	uint32_t nrbar                        : 3;  /**< Number of resizable BARs */
	uint32_t reserved_3_4                 : 2;
	uint32_t rbari                        : 3;  /**< BAR Index. Points to BAR2. */
#else
	uint32_t rbari                        : 3;
	uint32_t reserved_3_4                 : 2;
	uint32_t nrbar                        : 3;
	uint32_t rbars                        : 5;
	uint32_t reserved_13_31               : 19;
#endif
	} s;
	struct cvmx_pcieepx_cfg112_s          cn73xx;
	struct cvmx_pcieepx_cfg112_s          cn78xx;
	struct cvmx_pcieepx_cfg112_s          cn78xxp1;
	struct cvmx_pcieepx_cfg112_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg112 cvmx_pcieepx_cfg112_t;

/**
 * cvmx_pcieep#_cfg448
 *
 * This register contains the four hundred forty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg448 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg448_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t rtl                          : 16; /**< Replay time limit. The replay timer expires when it reaches this limit. The PCI Express
                                                         bus initiates a replay upon reception of a nak or when the replay timer expires. This
                                                         value is set correctly by the hardware out of reset or when the negotiated link width or
                                                         payload size changes. If the user changes this value through a CSR write or by an EEPROM
                                                         load, they should refer to the PCIe specification for the correct value. */
	uint32_t rtltl                        : 16; /**< Round trip latency time limit. The ack/nak latency timer expires when it reaches this
                                                         limit. This value is set correctly by the hardware out of reset or when the negotiated
                                                         link width or payload size changes. If the user changes this value through a CSR write or
                                                         by an EEPROM load, they should refer to the PCIe specification for the correct value. */
#else
	uint32_t rtltl                        : 16;
	uint32_t rtl                          : 16;
#endif
	} s;
	struct cvmx_pcieepx_cfg448_s          cn52xx;
	struct cvmx_pcieepx_cfg448_s          cn52xxp1;
	struct cvmx_pcieepx_cfg448_s          cn56xx;
	struct cvmx_pcieepx_cfg448_s          cn56xxp1;
	struct cvmx_pcieepx_cfg448_s          cn61xx;
	struct cvmx_pcieepx_cfg448_s          cn63xx;
	struct cvmx_pcieepx_cfg448_s          cn63xxp1;
	struct cvmx_pcieepx_cfg448_s          cn66xx;
	struct cvmx_pcieepx_cfg448_s          cn68xx;
	struct cvmx_pcieepx_cfg448_s          cn68xxp1;
	struct cvmx_pcieepx_cfg448_s          cn70xx;
	struct cvmx_pcieepx_cfg448_s          cn70xxp1;
	struct cvmx_pcieepx_cfg448_s          cn73xx;
	struct cvmx_pcieepx_cfg448_s          cn78xx;
	struct cvmx_pcieepx_cfg448_s          cn78xxp1;
	struct cvmx_pcieepx_cfg448_s          cnf71xx;
	struct cvmx_pcieepx_cfg448_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg448 cvmx_pcieepx_cfg448_t;

/**
 * cvmx_pcieep#_cfg449
 *
 * This register contains the four hundred fiftieth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg449 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg449_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t omr                          : 32; /**< Other message register. This register can be used for either of the following purposes:
                                                         * To send a specific PCI Express message, the application writes the payload of the
                                                         message into this register, then sets bit 0 of the port link control register to send the
                                                         message.
                                                         * To store a corruption pattern for corrupting the LCRC on all TLPs, the application
                                                         places a 32-bit corruption pattern into this register and enables this function by setting
                                                         bit 25 of the port link control register. When enabled, the transmit LCRC result is XORed
                                                         with this pattern before inserting it into the packet. */
#else
	uint32_t omr                          : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg449_s          cn52xx;
	struct cvmx_pcieepx_cfg449_s          cn52xxp1;
	struct cvmx_pcieepx_cfg449_s          cn56xx;
	struct cvmx_pcieepx_cfg449_s          cn56xxp1;
	struct cvmx_pcieepx_cfg449_s          cn61xx;
	struct cvmx_pcieepx_cfg449_s          cn63xx;
	struct cvmx_pcieepx_cfg449_s          cn63xxp1;
	struct cvmx_pcieepx_cfg449_s          cn66xx;
	struct cvmx_pcieepx_cfg449_s          cn68xx;
	struct cvmx_pcieepx_cfg449_s          cn68xxp1;
	struct cvmx_pcieepx_cfg449_s          cn70xx;
	struct cvmx_pcieepx_cfg449_s          cn70xxp1;
	struct cvmx_pcieepx_cfg449_s          cn73xx;
	struct cvmx_pcieepx_cfg449_s          cn78xx;
	struct cvmx_pcieepx_cfg449_s          cn78xxp1;
	struct cvmx_pcieepx_cfg449_s          cnf71xx;
	struct cvmx_pcieepx_cfg449_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg449 cvmx_pcieepx_cfg449_t;

/**
 * cvmx_pcieep#_cfg450
 *
 * This register contains the four hundred fifty-first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg450 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg450_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lpec                         : 8;  /**< Low power entrance count. The power management state waits this many clock cycles for the
                                                         associated completion of a CfgWr to PCIEEP()_CFG017 register, power state (PS) field
                                                         register
                                                         to go low-power. This register is intended for applications that do not let the PCI
                                                         Express bus handle a completion for configuration request to the power management control
                                                         and status (PCIEEP()_CFG017) register. */
	uint32_t reserved_22_23               : 2;
	uint32_t link_state                   : 6;  /**< Link state. The link state that the PCI Express bus is forced to when bit 15 (force link)
                                                         is set. State encoding:
                                                         0x0 = DETECT_QUIET.
                                                         0x1 = DETECT_ACT.
                                                         0x2 = POLL_ACTIVE.
                                                         0x3 = POLL_COMPLIANCE.
                                                         0x4 = POLL_CONFIG.
                                                         0x5 = PRE_DETECT_QUIET.
                                                         0x6 = DETECT_WAIT.
                                                         0x7 = CFG_LINKWD_START.
                                                         0x8 = CFG_LINKWD_ACEPT.
                                                         0x9 = CFG_LANENUM_WAIT.
                                                         0xA = CFG_LANENUM_ACEPT.
                                                         0xB = CFG_COMPLETE.
                                                         0xC = CFG_IDLE.
                                                         0xD = RCVRY_LOCK.
                                                         0xE = RCVRY_SPEED.
                                                         0xF = RCVRY_RCVRCFG.
                                                         0x10 = RCVRY_IDLE.
                                                         0x11 = L0.
                                                         0x12 = L0S.
                                                         0x13 = L123_SEND_EIDLE.
                                                         0x14 = L1_IDLE.
                                                         0x15 = L2_IDLE.
                                                         0x16 = L2_WAKE.
                                                         0x17 = DISABLED_ENTRY.
                                                         0x18 = DISABLED_IDLE.
                                                         0x19 = DISABLED.
                                                         0x1A = LPBK_ENTRY.
                                                         0x1B = LPBK_ACTIVE.
                                                         0x1C = LPBK_EXIT.
                                                         0x1D = LPBK_EXIT_TIMEOUT.
                                                         0x1E = HOT_RESET_ENTRY.
                                                         0x1F = HOT_RESET. */
	uint32_t force_link                   : 1;  /**< Force link. Forces the link to the state specified by [LINK_STATE]. The force link
                                                         pulse triggers link renegotiation.
                                                         As the force link is a pulse, writing a 1 to it does trigger the forced link state event,
                                                         even though reading it always returns a 0. */
	uint32_t reserved_8_14                : 7;
	uint32_t link_num                     : 8;  /**< Link number. Not used for endpoint. */
#else
	uint32_t link_num                     : 8;
	uint32_t reserved_8_14                : 7;
	uint32_t force_link                   : 1;
	uint32_t link_state                   : 6;
	uint32_t reserved_22_23               : 2;
	uint32_t lpec                         : 8;
#endif
	} s;
	struct cvmx_pcieepx_cfg450_s          cn52xx;
	struct cvmx_pcieepx_cfg450_s          cn52xxp1;
	struct cvmx_pcieepx_cfg450_s          cn56xx;
	struct cvmx_pcieepx_cfg450_s          cn56xxp1;
	struct cvmx_pcieepx_cfg450_s          cn61xx;
	struct cvmx_pcieepx_cfg450_s          cn63xx;
	struct cvmx_pcieepx_cfg450_s          cn63xxp1;
	struct cvmx_pcieepx_cfg450_s          cn66xx;
	struct cvmx_pcieepx_cfg450_s          cn68xx;
	struct cvmx_pcieepx_cfg450_s          cn68xxp1;
	struct cvmx_pcieepx_cfg450_cn70xx {
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
	uint32_t reserved_12_14               : 3;
	uint32_t link_cmd                     : 4;  /**< Link Command
                                                         The Link command that the PCI Express Core will be forced to
                                                         transmit when bit 15 (Force Link) is set.
                                                         Command encoding:
                                                         o PEM_SEND_IDLE              1h
                                                         o PEM_SEND_EIDLE             2h
                                                         o PEM_XMT_IN_EIDLE           3h
                                                         o PEM_MOD_COMPL_PATTERN      4h
                                                         o PEM_SEND_RCVR_DETECT_SEQ   5h
                                                         o PEM_SEND_TS1               6h
                                                         o PEM_SEND_TS2               7h
                                                         o PEM_COMPLIANCE_PATTERN     8h
                                                         o PEM_SEND_SDS               9h
                                                         o PEM_SEND_BEACON            ah
                                                         o PEM_SEND_N_FTS             bh
                                                         o PEM_NORM                   ch
                                                         o PEM_SEND_SKP               dh
                                                         o PEM_SEND_EIES              eh
                                                         o PEM_SEND_EIES_SYM          fh */
	uint32_t link_num                     : 8;  /**< Link Number
                                                         Not used for Endpoint */
#else
	uint32_t link_num                     : 8;
	uint32_t link_cmd                     : 4;
	uint32_t reserved_12_14               : 3;
	uint32_t force_link                   : 1;
	uint32_t link_state                   : 6;
	uint32_t reserved_22_23               : 2;
	uint32_t lpec                         : 8;
#endif
	} cn70xx;
	struct cvmx_pcieepx_cfg450_cn70xx     cn70xxp1;
	struct cvmx_pcieepx_cfg450_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lpec                         : 8;  /**< Low power entrance count. The power management state waits this many clock cycles for the
                                                         associated completion of a CfgWr to PCIEEP()_CFG017 register, power state (PS) field
                                                         register
                                                         to go low-power. This register is intended for applications that do not let the PCI
                                                         Express bus handle a completion for configuration request to the power management control
                                                         and status (PCIEEP()_CFG017) register. */
	uint32_t reserved_22_23               : 2;
	uint32_t link_state                   : 6;  /**< Link state. The link state that the PCI Express bus is forced to when bit 15 (force link)
                                                         is set. State encoding:
                                                         0x0 = DETECT_QUIET.
                                                         0x1 = DETECT_ACT.
                                                         0x2 = POLL_ACTIVE.
                                                         0x3 = POLL_COMPLIANCE.
                                                         0x4 = POLL_CONFIG.
                                                         0x5 = PRE_DETECT_QUIET.
                                                         0x6 = DETECT_WAIT.
                                                         0x7 = CFG_LINKWD_START.
                                                         0x8 = CFG_LINKWD_ACEPT.
                                                         0x9 = CFG_LANENUM_WAIT.
                                                         0xA = CFG_LANENUM_ACEPT.
                                                         0xB = CFG_COMPLETE.
                                                         0xC = CFG_IDLE.
                                                         0xD = RCVRY_LOCK.
                                                         0xE = RCVRY_SPEED.
                                                         0xF = RCVRY_RCVRCFG.
                                                         0x10 = RCVRY_IDLE.
                                                         0x11 = L0.
                                                         0x12 = L0S.
                                                         0x13 = L123_SEND_EIDLE.
                                                         0x14 = L1_IDLE.
                                                         0x15 = L2_IDLE.
                                                         0x16 = L2_WAKE.
                                                         0x17 = DISABLED_ENTRY.
                                                         0x18 = DISABLED_IDLE.
                                                         0x19 = DISABLED.
                                                         0x1A = LPBK_ENTRY.
                                                         0x1B = LPBK_ACTIVE.
                                                         0x1C = LPBK_EXIT.
                                                         0x1D = LPBK_EXIT_TIMEOUT.
                                                         0x1E = HOT_RESET_ENTRY.
                                                         0x1F = HOT_RESET. */
	uint32_t force_link                   : 1;  /**< Force link. Forces the link to the state specified by [LINK_STATE]. The force link
                                                         pulse triggers link renegotiation.
                                                         As the force link is a pulse, writing a 1 to it does trigger the forced link state event,
                                                         even though reading it always returns a 0. */
	uint32_t reserved_12_14               : 3;
	uint32_t forced_ltssm                 : 4;  /**< Forced link command. */
	uint32_t link_num                     : 8;  /**< Link number. Not used for endpoint. */
#else
	uint32_t link_num                     : 8;
	uint32_t forced_ltssm                 : 4;
	uint32_t reserved_12_14               : 3;
	uint32_t force_link                   : 1;
	uint32_t link_state                   : 6;
	uint32_t reserved_22_23               : 2;
	uint32_t lpec                         : 8;
#endif
	} cn73xx;
	struct cvmx_pcieepx_cfg450_cn73xx     cn78xx;
	struct cvmx_pcieepx_cfg450_cn73xx     cn78xxp1;
	struct cvmx_pcieepx_cfg450_s          cnf71xx;
	struct cvmx_pcieepx_cfg450_cn73xx     cnf75xx;
};
typedef union cvmx_pcieepx_cfg450 cvmx_pcieepx_cfg450_t;

/**
 * cvmx_pcieep#_cfg451
 *
 * This register contains the four hundred fifty-second 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg451 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg451_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t easpml1                      : 1;  /**< Enter ASPM L1 without receive in L0s. Allow core to enter ASPM L1 even when link partner
                                                         did not go to L0s (receive is not in L0s). When not set, core goes to ASPM L1 only after
                                                         idle period, during which both receive and transmit are in L0s. */
	uint32_t l1el                         : 3;  /**< L1 entrance latency. Values correspond to:
                                                         0x0 = 1 ms.
                                                         0x1 = 2 ms.
                                                         0x2 = 4 ms.
                                                         0x3 = 8 ms.
                                                         0x4 = 16 ms.
                                                         0x5 = 32 ms.
                                                         0x6 or 0x7 = 64 ms. */
	uint32_t l0el                         : 3;  /**< L0s entrance latency. Values correspond to:
                                                         0x0 = 1 ms.
                                                         0x1 = 2 ms.
                                                         0x2 = 3 ms.
                                                         0x3 = 4 ms.
                                                         0x4 = 5 ms.
                                                         0x5 = 6 ms.
                                                         0x6 or 0x7 = 7 ms. */
	uint32_t n_fts_cc                     : 8;  /**< N_FTS when common clock is used.
                                                         The number of fast training sequence (FTS) ordered sets to be transmitted when
                                                         transitioning from L0s to L0. The maximum number of FTS ordered sets that a component can
                                                         request is 255.
                                                         A value of zero is not supported; a value of zero can cause the LTSSM to go into the
                                                         recovery state when exiting from L0s. */
	uint32_t n_fts                        : 8;  /**< N_FTS. The number of fast training sequence (FTS) ordered sets to be transmitted when
                                                         transitioning from L0s to L0. The maximum number of FTS ordered sets that a component can
                                                         request is 255.
                                                         A value of zero is not supported; a value of zero can cause the LTSSM to go into the
                                                         recovery state when exiting from L0s. */
	uint32_t ack_freq                     : 8;  /**< Ack frequency. The number of pending Acks specified here (up to 255) before sending an Ack. */
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
	struct cvmx_pcieepx_cfg451_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
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
	uint32_t reserved_30_31               : 2;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg451_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg451_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg451_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg451_s          cn61xx;
	struct cvmx_pcieepx_cfg451_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg451_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg451_s          cn66xx;
	struct cvmx_pcieepx_cfg451_s          cn68xx;
	struct cvmx_pcieepx_cfg451_s          cn68xxp1;
	struct cvmx_pcieepx_cfg451_s          cn70xx;
	struct cvmx_pcieepx_cfg451_s          cn70xxp1;
	struct cvmx_pcieepx_cfg451_s          cn73xx;
	struct cvmx_pcieepx_cfg451_s          cn78xx;
	struct cvmx_pcieepx_cfg451_s          cn78xxp1;
	struct cvmx_pcieepx_cfg451_s          cnf71xx;
	struct cvmx_pcieepx_cfg451_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg451 cvmx_pcieepx_cfg451_t;

/**
 * cvmx_pcieep#_cfg452
 *
 * This register contains the four hundred fifty-third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg452 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg452_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t eccrc                        : 1;  /**< Enable Corrupted CRC
                                                         Causes corrupt LCRC for TLPs when set,
                                                         using the pattern contained in the Other Message register.
                                                         This is a test feature, not to be used in normal operation. */
	uint32_t reserved_22_24               : 3;
	uint32_t lme                          : 6;  /**< Link mode enable set as follows:
                                                         0x1 = x1.
                                                         0x3 = x2.
                                                         0x7 = x4.
                                                         0xF = x8   (not supported).
                                                         0x1F = x16 (not supported).
                                                         0x3F = x32 (not supported).
                                                         This field indicates the maximum number of lanes supported by the PCIe port. The value can
                                                         be set less than 0xF to limit the number of lanes that the PCIe will attempt to use. If
                                                         the value of 0xF set by the hardware is not desired, this field can be programmed to a
                                                         smaller value (i.e. EEPROM). See also PCIEEP()_CFG031[MLW].
                                                         The value of this field does not indicate the number of lanes in use by the PCIe. This
                                                         field sets the maximum number of lanes in the PCIe core that could be used. As per the
                                                         PCIe specification, the PCIe core can negotiate a smaller link width, so all of x8, x4,
                                                         x2, and x1 are supported when
                                                         LME = 0xF, for example. */
	uint32_t reserved_12_15               : 4;
	uint32_t link_rate                    : 4;  /**< Reserved. */
	uint32_t flm                          : 1;  /**< Fast link mode. Sets all internal timers to fast mode for simulation purposes. If during
                                                         an EEPROM load, the first word loaded is 0xFFFFFFFF, the EEPROM load is terminated and
                                                         this bit is set. */
	uint32_t reserved_6_6                 : 1;
	uint32_t dllle                        : 1;  /**< DLL link enable. Enables link initialization. If DLL link enable = 0, the PCI Express bus
                                                         does not transmit InitFC DLLPs and does not establish a link. */
	uint32_t reserved_4_4                 : 1;
	uint32_t ra                           : 1;  /**< Reset assert. Triggers a recovery and forces the LTSSM to the hot reset state (downstream
                                                         port only). */
	uint32_t le                           : 1;  /**< Loopback enable. Initiate loopback mode as a master. On a 0->1 transition, the PCIe core
                                                         sends TS ordered sets with the loopback bit set to cause the link partner to enter into
                                                         loopback mode as a slave. Normal transmission is not possible when LE=1. To exit loopback
                                                         mode, take the link through a reset sequence. */
	uint32_t sd                           : 1;  /**< Scramble disable. Setting this bit turns off data scrambling. */
	uint32_t omr                          : 1;  /**< Other message request. When software writes a 1 to this bit, the PCI Express bus transmits
                                                         the message contained in the other message register. */
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
	uint32_t reserved_22_24               : 3;
	uint32_t eccrc                        : 1;
	uint32_t reserved_26_31               : 6;
#endif
	} s;
	struct cvmx_pcieepx_cfg452_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t eccrc                        : 1;  /**< Enable Corrupted CRC
                                                         Causes corrupt LCRC for TLPs when set,
                                                         using the pattern contained in the Other Message register.
                                                         This is a test feature, not to be used in normal operation. */
	uint32_t reserved_22_24               : 3;
	uint32_t lme                          : 6;  /**< Link Mode Enable
                                                         o 000001: x1
                                                         o 000011: x2
                                                         o 000111: x4
                                                         o 001111: x8  (not supported)
                                                         o 011111: x16 (not supported)
                                                         o 111111: x32 (not supported)
                                                         This field indicates the MAXIMUM number of lanes supported
                                                         by the PCIe port. It is set to 0x7 or 0x3 depending
                                                         on the value of the QLM_CFG bits (0x7 when QLM_CFG == 0x3
                                                         otherwise 0x3). The value can be set less than 0x7 or 0x3
                                                         to limit the number of lanes the PCIe will attempt to use.
                                                         If the value of 0x7 or 0x3 set by the HW is not desired,
                                                         this field can be programmed to a smaller value (i.e. EEPROM)
                                                         See also MLW.
                                                         (Note: The value of this field does NOT indicate the number
                                                          of lanes in use by the PCIe. LME sets the max number of lanes
                                                          in the PCIe core that COULD be used. As per the PCIe specs,
                                                          the PCIe core can negotiate a smaller link width, so all
                                                          of x4, x2, and x1 are supported when LME=0x7,
                                                          for example.) */
	uint32_t reserved_8_15                : 8;
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
	uint32_t reserved_8_15                : 8;
	uint32_t lme                          : 6;
	uint32_t reserved_22_24               : 3;
	uint32_t eccrc                        : 1;
	uint32_t reserved_26_31               : 6;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg452_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg452_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg452_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg452_cn61xx {
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
                                                         If the value of 0x7 set by the HW is not desired,
                                                         this field can be programmed to a smaller value (i.e. EEPROM)
                                                         See also MLW.
                                                         (Note: The value of this field does NOT indicate the number
                                                          of lanes in use by the PCIe. LME sets the max number of lanes
                                                          in the PCIe core that COULD be used. As per the PCIe specs,
                                                          the PCIe core can negotiate a smaller link width, so all
                                                          of x4, x2, and x1 are supported when LME=0x7,
                                                          for example.) */
	uint32_t reserved_8_15                : 8;
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
	uint32_t reserved_8_15                : 8;
	uint32_t lme                          : 6;
	uint32_t reserved_22_31               : 10;
#endif
	} cn61xx;
	struct cvmx_pcieepx_cfg452_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg452_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg452_cn61xx     cn66xx;
	struct cvmx_pcieepx_cfg452_cn61xx     cn68xx;
	struct cvmx_pcieepx_cfg452_cn61xx     cn68xxp1;
	struct cvmx_pcieepx_cfg452_cn70xx {
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
                                                         If the value of 0x7 set by the HW is not desired,
                                                         this field can be programmed to a smaller value (i.e. EEPROM)
                                                         See also MLW.
                                                         (Note: The value of this field does NOT indicate the number
                                                         of lanes in use by the PCIe. LME sets the max number of lanes
                                                         in the PCIe core that COULD be used. As per the PCIe specs,
                                                         the PCIe core can negotiate a smaller link width, so all
                                                         of x4, x2, and x1 are supported when LME=0x7,
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
	} cn70xx;
	struct cvmx_pcieepx_cfg452_cn70xx     cn70xxp1;
	struct cvmx_pcieepx_cfg452_cn70xx     cn73xx;
	struct cvmx_pcieepx_cfg452_cn70xx     cn78xx;
	struct cvmx_pcieepx_cfg452_cn70xx     cn78xxp1;
	struct cvmx_pcieepx_cfg452_cn61xx     cnf71xx;
	struct cvmx_pcieepx_cfg452_cn70xx     cnf75xx;
};
typedef union cvmx_pcieepx_cfg452 cvmx_pcieepx_cfg452_t;

/**
 * cvmx_pcieep#_cfg453
 *
 * This register contains the four hundred fifty-fourth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg453 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg453_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dlld                         : 1;  /**< Disable lane-to-lane deskew. Disables the internal lane-to-lane deskew logic. */
	uint32_t reserved_26_30               : 5;
	uint32_t ack_nak                      : 1;  /**< Ack/Nak disable. Prevents the PCI Express bus from sending Ack and Nak DLLPs. */
	uint32_t fcd                          : 1;  /**< Flow control disable. Prevents the PCI Express bus from sending FC DLLPs. */
	uint32_t ilst                         : 24; /**< Insert lane skew for transmit. Causes skew between lanes for test purposes. There are
                                                         three bits per lane. The value is in units of one symbol time. For example, the value 010
                                                         b
                                                         for a lane forces a skew of two symbol times for that lane. The maximum skew value for any
                                                         lane is 5 symbol times. */
#else
	uint32_t ilst                         : 24;
	uint32_t fcd                          : 1;
	uint32_t ack_nak                      : 1;
	uint32_t reserved_26_30               : 5;
	uint32_t dlld                         : 1;
#endif
	} s;
	struct cvmx_pcieepx_cfg453_s          cn52xx;
	struct cvmx_pcieepx_cfg453_s          cn52xxp1;
	struct cvmx_pcieepx_cfg453_s          cn56xx;
	struct cvmx_pcieepx_cfg453_s          cn56xxp1;
	struct cvmx_pcieepx_cfg453_s          cn61xx;
	struct cvmx_pcieepx_cfg453_s          cn63xx;
	struct cvmx_pcieepx_cfg453_s          cn63xxp1;
	struct cvmx_pcieepx_cfg453_s          cn66xx;
	struct cvmx_pcieepx_cfg453_s          cn68xx;
	struct cvmx_pcieepx_cfg453_s          cn68xxp1;
	struct cvmx_pcieepx_cfg453_s          cn70xx;
	struct cvmx_pcieepx_cfg453_s          cn70xxp1;
	struct cvmx_pcieepx_cfg453_s          cn73xx;
	struct cvmx_pcieepx_cfg453_s          cn78xx;
	struct cvmx_pcieepx_cfg453_s          cn78xxp1;
	struct cvmx_pcieepx_cfg453_s          cnf71xx;
	struct cvmx_pcieepx_cfg453_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg453 cvmx_pcieepx_cfg453_t;

/**
 * cvmx_pcieep#_cfg454
 *
 * This register contains the four hundred fifty-fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg454 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg454_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t cx_nfunc                     : 3;  /**< Number of Functions (minus 1)
                                                         Configuration Requests targeted at function numbers above this
                                                         value will be returned with unsupported request */
	uint32_t tmfcwt                       : 5;  /**< Used to be 'timer modifier for flow control watchdog timer.' This field is no longer used.
                                                         and has moved to the queue status register -- PCIEEP()_CFG463. This field remains to
                                                         prevent software from breaking. */
	uint32_t tmanlt                       : 5;  /**< Timer modifier for Ack/Nak latency timer. Increases the timer value for the Ack/Nak
                                                         latency timer, in increments of 64 clock cycles. */
	uint32_t tmrt                         : 5;  /**< Timer modifier for replay timer. Increases the timer value for the replay timer, in
                                                         increments of 64 clock cycles. */
	uint32_t reserved_11_13               : 3;
	uint32_t nskps                        : 3;  /**< Number of SKP Symbols */
	uint32_t reserved_0_7                 : 8;
#else
	uint32_t reserved_0_7                 : 8;
	uint32_t nskps                        : 3;
	uint32_t reserved_11_13               : 3;
	uint32_t tmrt                         : 5;
	uint32_t tmanlt                       : 5;
	uint32_t tmfcwt                       : 5;
	uint32_t cx_nfunc                     : 3;
#endif
	} s;
	struct cvmx_pcieepx_cfg454_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_29_31               : 3;
	uint32_t tmfcwt                       : 5;  /**< Timer Modifier for Flow Control Watchdog Timer
                                                         Increases the timer value for the Flow Control watchdog timer,
                                                         in increments of 16 clock cycles. */
	uint32_t tmanlt                       : 5;  /**< Timer Modifier for Ack/Nak Latency Timer
                                                         Increases the timer value for the Ack/Nak latency timer, in
                                                         increments of 64 clock cycles. */
	uint32_t tmrt                         : 5;  /**< Timer Modifier for Replay Timer
                                                         Increases the timer value for the replay timer, in increments
                                                         of 64 clock cycles. */
	uint32_t reserved_11_13               : 3;
	uint32_t nskps                        : 3;  /**< Number of SKP Symbols */
	uint32_t reserved_4_7                 : 4;
	uint32_t ntss                         : 4;  /**< Number of TS Symbols
                                                         Sets the number of TS identifier symbols that are sent in TS1
                                                         and TS2 ordered sets. */
#else
	uint32_t ntss                         : 4;
	uint32_t reserved_4_7                 : 4;
	uint32_t nskps                        : 3;
	uint32_t reserved_11_13               : 3;
	uint32_t tmrt                         : 5;
	uint32_t tmanlt                       : 5;
	uint32_t tmfcwt                       : 5;
	uint32_t reserved_29_31               : 3;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg454_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg454_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg454_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg454_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t cx_nfunc                     : 3;  /**< Number of Functions (minus 1)
                                                         Configuration Requests targeted at function numbers above this
                                                         value will be returned with unsupported request */
	uint32_t tmfcwt                       : 5;  /**< Timer Modifier for Flow Control Watchdog Timer
                                                         Increases the timer value for the Flow Control watchdog timer,
                                                         in increments of 16 clock cycles. */
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
	uint32_t cx_nfunc                     : 3;
#endif
	} cn61xx;
	struct cvmx_pcieepx_cfg454_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg454_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg454_cn61xx     cn66xx;
	struct cvmx_pcieepx_cfg454_cn61xx     cn68xx;
	struct cvmx_pcieepx_cfg454_cn52xx     cn68xxp1;
	struct cvmx_pcieepx_cfg454_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
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
	uint32_t reserved_24_31               : 8;
#endif
	} cn70xx;
	struct cvmx_pcieepx_cfg454_cn70xx     cn70xxp1;
	struct cvmx_pcieepx_cfg454_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_29_31               : 3;
	uint32_t tmfcwt                       : 5;  /**< Used to be 'timer modifier for flow control watchdog timer.' This field is no longer used.
                                                         and has moved to the queue status register -- PCIEEP()_CFG463. This field remains to
                                                         prevent software from breaking. */
	uint32_t tmanlt                       : 5;  /**< Timer modifier for Ack/Nak latency timer. Increases the timer value for the Ack/Nak
                                                         latency timer, in increments of 64 clock cycles. */
	uint32_t tmrt                         : 5;  /**< Timer modifier for replay timer. Increases the timer value for the replay timer, in
                                                         increments of 64 clock cycles. */
	uint32_t reserved_8_13                : 6;
	uint32_t mfuncn                       : 8;  /**< Max number of functions supported. Used for SR-IOV. */
#else
	uint32_t mfuncn                       : 8;
	uint32_t reserved_8_13                : 6;
	uint32_t tmrt                         : 5;
	uint32_t tmanlt                       : 5;
	uint32_t tmfcwt                       : 5;
	uint32_t reserved_29_31               : 3;
#endif
	} cn73xx;
	struct cvmx_pcieepx_cfg454_cn73xx     cn78xx;
	struct cvmx_pcieepx_cfg454_cn73xx     cn78xxp1;
	struct cvmx_pcieepx_cfg454_cn61xx     cnf71xx;
	struct cvmx_pcieepx_cfg454_cn73xx     cnf75xx;
};
typedef union cvmx_pcieepx_cfg454 cvmx_pcieepx_cfg454_t;

/**
 * cvmx_pcieep#_cfg455
 *
 * This register contains the four hundred fifty-sixth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg455 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg455_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t m_cfg0_filt                  : 1;  /**< Mask filtering of received configuration requests (RC mode only). */
	uint32_t m_io_filt                    : 1;  /**< Mask filtering of received I/O requests (RC mode only). */
	uint32_t msg_ctrl                     : 1;  /**< Message control. The application must not change this field. */
	uint32_t m_cpl_ecrc_filt              : 1;  /**< Mask ECRC error filtering for completions. */
	uint32_t m_ecrc_filt                  : 1;  /**< Mask ECRC error filtering. */
	uint32_t m_cpl_len_err                : 1;  /**< Mask length mismatch error for received completions. */
	uint32_t m_cpl_attr_err               : 1;  /**< Mask attributes mismatch error for received completions. */
	uint32_t m_cpl_tc_err                 : 1;  /**< Mask traffic class mismatch error for received completions. */
	uint32_t m_cpl_fun_err                : 1;  /**< Mask function mismatch error for received completions. */
	uint32_t m_cpl_rid_err                : 1;  /**< Mask requester ID mismatch error for received completions. */
	uint32_t m_cpl_tag_err                : 1;  /**< Mask tag error rules for received completions. */
	uint32_t m_lk_filt                    : 1;  /**< Mask locked request filtering. */
	uint32_t m_cfg1_filt                  : 1;  /**< Mask type 1 configuration request filtering. */
	uint32_t m_bar_match                  : 1;  /**< Mask BAR match filtering. */
	uint32_t m_pois_filt                  : 1;  /**< Mask poisoned TLP filtering. */
	uint32_t m_fun                        : 1;  /**< Mask function. */
	uint32_t dfcwt                        : 1;  /**< Disable FC watchdog timer. */
	uint32_t reserved_11_14               : 4;
	uint32_t skpiv                        : 11; /**< SKP interval value. */
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
	struct cvmx_pcieepx_cfg455_s          cn52xx;
	struct cvmx_pcieepx_cfg455_s          cn52xxp1;
	struct cvmx_pcieepx_cfg455_s          cn56xx;
	struct cvmx_pcieepx_cfg455_s          cn56xxp1;
	struct cvmx_pcieepx_cfg455_s          cn61xx;
	struct cvmx_pcieepx_cfg455_s          cn63xx;
	struct cvmx_pcieepx_cfg455_s          cn63xxp1;
	struct cvmx_pcieepx_cfg455_s          cn66xx;
	struct cvmx_pcieepx_cfg455_s          cn68xx;
	struct cvmx_pcieepx_cfg455_s          cn68xxp1;
	struct cvmx_pcieepx_cfg455_s          cn70xx;
	struct cvmx_pcieepx_cfg455_s          cn70xxp1;
	struct cvmx_pcieepx_cfg455_s          cn73xx;
	struct cvmx_pcieepx_cfg455_s          cn78xx;
	struct cvmx_pcieepx_cfg455_s          cn78xxp1;
	struct cvmx_pcieepx_cfg455_s          cnf71xx;
	struct cvmx_pcieepx_cfg455_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg455 cvmx_pcieepx_cfg455_t;

/**
 * cvmx_pcieep#_cfg456
 *
 * This register contains the four hundred fifty-seventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg456 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg456_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_4_31                : 28;
	uint32_t m_handle_flush               : 1;  /**< Mask core filter to handle flush request. */
	uint32_t m_dabort_4ucpl               : 1;  /**< Mask DLLP abort for unexpected CPL. */
	uint32_t m_vend1_drp                  : 1;  /**< Mask vendor MSG type 1 dropped silently. */
	uint32_t m_vend0_drp                  : 1;  /**< Mask vendor MSG type 0 dropped with UR error reporting. */
#else
	uint32_t m_vend0_drp                  : 1;
	uint32_t m_vend1_drp                  : 1;
	uint32_t m_dabort_4ucpl               : 1;
	uint32_t m_handle_flush               : 1;
	uint32_t reserved_4_31                : 28;
#endif
	} s;
	struct cvmx_pcieepx_cfg456_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_2_31                : 30;
	uint32_t m_vend1_drp                  : 1;  /**< Mask Vendor MSG Type 1 dropped silently */
	uint32_t m_vend0_drp                  : 1;  /**< Mask Vendor MSG Type 0 dropped with UR error reporting. */
#else
	uint32_t m_vend0_drp                  : 1;
	uint32_t m_vend1_drp                  : 1;
	uint32_t reserved_2_31                : 30;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg456_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg456_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg456_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg456_s          cn61xx;
	struct cvmx_pcieepx_cfg456_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg456_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg456_s          cn66xx;
	struct cvmx_pcieepx_cfg456_s          cn68xx;
	struct cvmx_pcieepx_cfg456_cn52xx     cn68xxp1;
	struct cvmx_pcieepx_cfg456_s          cn70xx;
	struct cvmx_pcieepx_cfg456_s          cn70xxp1;
	struct cvmx_pcieepx_cfg456_s          cn73xx;
	struct cvmx_pcieepx_cfg456_s          cn78xx;
	struct cvmx_pcieepx_cfg456_s          cn78xxp1;
	struct cvmx_pcieepx_cfg456_s          cnf71xx;
	struct cvmx_pcieepx_cfg456_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg456 cvmx_pcieepx_cfg456_t;

/**
 * cvmx_pcieep#_cfg458
 *
 * This register contains the four hundred fifty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg458 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg458_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dbg_info_l32                 : 32; /**< Debug info lower 32 bits. */
#else
	uint32_t dbg_info_l32                 : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg458_s          cn52xx;
	struct cvmx_pcieepx_cfg458_s          cn52xxp1;
	struct cvmx_pcieepx_cfg458_s          cn56xx;
	struct cvmx_pcieepx_cfg458_s          cn56xxp1;
	struct cvmx_pcieepx_cfg458_s          cn61xx;
	struct cvmx_pcieepx_cfg458_s          cn63xx;
	struct cvmx_pcieepx_cfg458_s          cn63xxp1;
	struct cvmx_pcieepx_cfg458_s          cn66xx;
	struct cvmx_pcieepx_cfg458_s          cn68xx;
	struct cvmx_pcieepx_cfg458_s          cn68xxp1;
	struct cvmx_pcieepx_cfg458_s          cn70xx;
	struct cvmx_pcieepx_cfg458_s          cn70xxp1;
	struct cvmx_pcieepx_cfg458_s          cn73xx;
	struct cvmx_pcieepx_cfg458_s          cn78xx;
	struct cvmx_pcieepx_cfg458_s          cn78xxp1;
	struct cvmx_pcieepx_cfg458_s          cnf71xx;
	struct cvmx_pcieepx_cfg458_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg458 cvmx_pcieepx_cfg458_t;

/**
 * cvmx_pcieep#_cfg459
 *
 * This register contains the four hundred sixtieth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg459 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg459_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dbg_info_u32                 : 32; /**< Debug info upper 32 bits. */
#else
	uint32_t dbg_info_u32                 : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg459_s          cn52xx;
	struct cvmx_pcieepx_cfg459_s          cn52xxp1;
	struct cvmx_pcieepx_cfg459_s          cn56xx;
	struct cvmx_pcieepx_cfg459_s          cn56xxp1;
	struct cvmx_pcieepx_cfg459_s          cn61xx;
	struct cvmx_pcieepx_cfg459_s          cn63xx;
	struct cvmx_pcieepx_cfg459_s          cn63xxp1;
	struct cvmx_pcieepx_cfg459_s          cn66xx;
	struct cvmx_pcieepx_cfg459_s          cn68xx;
	struct cvmx_pcieepx_cfg459_s          cn68xxp1;
	struct cvmx_pcieepx_cfg459_s          cn70xx;
	struct cvmx_pcieepx_cfg459_s          cn70xxp1;
	struct cvmx_pcieepx_cfg459_s          cn73xx;
	struct cvmx_pcieepx_cfg459_s          cn78xx;
	struct cvmx_pcieepx_cfg459_s          cn78xxp1;
	struct cvmx_pcieepx_cfg459_s          cnf71xx;
	struct cvmx_pcieepx_cfg459_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg459 cvmx_pcieepx_cfg459_t;

/**
 * cvmx_pcieep#_cfg460
 *
 * This register contains the four hundred sixty-first 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg460 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg460_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t tphfcc                       : 8;  /**< Transmit posted header FC credits. The posted header credits advertised by the receiver at
                                                         the other end of the link, updated with each UpdateFC DLLP. */
	uint32_t tpdfcc                       : 12; /**< Transmit posted data FC credits. The posted data credits advertised by the receiver at the
                                                         other end of the link, updated with each UpdateFC DLLP. */
#else
	uint32_t tpdfcc                       : 12;
	uint32_t tphfcc                       : 8;
	uint32_t reserved_20_31               : 12;
#endif
	} s;
	struct cvmx_pcieepx_cfg460_s          cn52xx;
	struct cvmx_pcieepx_cfg460_s          cn52xxp1;
	struct cvmx_pcieepx_cfg460_s          cn56xx;
	struct cvmx_pcieepx_cfg460_s          cn56xxp1;
	struct cvmx_pcieepx_cfg460_s          cn61xx;
	struct cvmx_pcieepx_cfg460_s          cn63xx;
	struct cvmx_pcieepx_cfg460_s          cn63xxp1;
	struct cvmx_pcieepx_cfg460_s          cn66xx;
	struct cvmx_pcieepx_cfg460_s          cn68xx;
	struct cvmx_pcieepx_cfg460_s          cn68xxp1;
	struct cvmx_pcieepx_cfg460_s          cn70xx;
	struct cvmx_pcieepx_cfg460_s          cn70xxp1;
	struct cvmx_pcieepx_cfg460_s          cn73xx;
	struct cvmx_pcieepx_cfg460_s          cn78xx;
	struct cvmx_pcieepx_cfg460_s          cn78xxp1;
	struct cvmx_pcieepx_cfg460_s          cnf71xx;
	struct cvmx_pcieepx_cfg460_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg460 cvmx_pcieepx_cfg460_t;

/**
 * cvmx_pcieep#_cfg461
 *
 * This register contains the four hundred sixty-second 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg461 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg461_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t tchfcc                       : 8;  /**< Transmit nonposted header FC credits. The nonposted header credits advertised by the
                                                         receiver at the other end of the link, updated with each UpdateFC DLLP. */
	uint32_t tcdfcc                       : 12; /**< Transmit nonposted data FC credits. The nonposted data credits advertised by the receiver
                                                         at the other end of the link, updated with each UpdateFC DLLP. */
#else
	uint32_t tcdfcc                       : 12;
	uint32_t tchfcc                       : 8;
	uint32_t reserved_20_31               : 12;
#endif
	} s;
	struct cvmx_pcieepx_cfg461_s          cn52xx;
	struct cvmx_pcieepx_cfg461_s          cn52xxp1;
	struct cvmx_pcieepx_cfg461_s          cn56xx;
	struct cvmx_pcieepx_cfg461_s          cn56xxp1;
	struct cvmx_pcieepx_cfg461_s          cn61xx;
	struct cvmx_pcieepx_cfg461_s          cn63xx;
	struct cvmx_pcieepx_cfg461_s          cn63xxp1;
	struct cvmx_pcieepx_cfg461_s          cn66xx;
	struct cvmx_pcieepx_cfg461_s          cn68xx;
	struct cvmx_pcieepx_cfg461_s          cn68xxp1;
	struct cvmx_pcieepx_cfg461_s          cn70xx;
	struct cvmx_pcieepx_cfg461_s          cn70xxp1;
	struct cvmx_pcieepx_cfg461_s          cn73xx;
	struct cvmx_pcieepx_cfg461_s          cn78xx;
	struct cvmx_pcieepx_cfg461_s          cn78xxp1;
	struct cvmx_pcieepx_cfg461_s          cnf71xx;
	struct cvmx_pcieepx_cfg461_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg461 cvmx_pcieepx_cfg461_t;

/**
 * cvmx_pcieep#_cfg462
 *
 * This register contains the four hundred sixty-third 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg462 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg462_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t tchfcc                       : 8;  /**< Transmit completion header FC credits. The completion header credits advertised by the
                                                         receiver at the other end of the link, updated with each UpdateFC DLLP. */
	uint32_t tcdfcc                       : 12; /**< Transmit completion data FC credits. The completion data credits advertised by the
                                                         receiver at the other end of the link, updated with each UpdateFC DLLP. */
#else
	uint32_t tcdfcc                       : 12;
	uint32_t tchfcc                       : 8;
	uint32_t reserved_20_31               : 12;
#endif
	} s;
	struct cvmx_pcieepx_cfg462_s          cn52xx;
	struct cvmx_pcieepx_cfg462_s          cn52xxp1;
	struct cvmx_pcieepx_cfg462_s          cn56xx;
	struct cvmx_pcieepx_cfg462_s          cn56xxp1;
	struct cvmx_pcieepx_cfg462_s          cn61xx;
	struct cvmx_pcieepx_cfg462_s          cn63xx;
	struct cvmx_pcieepx_cfg462_s          cn63xxp1;
	struct cvmx_pcieepx_cfg462_s          cn66xx;
	struct cvmx_pcieepx_cfg462_s          cn68xx;
	struct cvmx_pcieepx_cfg462_s          cn68xxp1;
	struct cvmx_pcieepx_cfg462_s          cn70xx;
	struct cvmx_pcieepx_cfg462_s          cn70xxp1;
	struct cvmx_pcieepx_cfg462_s          cn73xx;
	struct cvmx_pcieepx_cfg462_s          cn78xx;
	struct cvmx_pcieepx_cfg462_s          cn78xxp1;
	struct cvmx_pcieepx_cfg462_s          cnf71xx;
	struct cvmx_pcieepx_cfg462_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg462 cvmx_pcieepx_cfg462_t;

/**
 * cvmx_pcieep#_cfg463
 *
 * This register contains the four hundred sixty-fourth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg463 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg463_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t fcltoe                       : 1;  /**< FC latency timer override enable. When this bit is set, the value in
                                                         PCIEEP()_CFG463[FCLTOV] will override the FC latency timer value that the core calculates
                                                         according to the PCIe specification. */
	uint32_t reserved_29_30               : 2;
	uint32_t fcltov                       : 13; /**< FC latency timer override value. When you set PCIEEP()_CFG463[FCLTOE], the value in this
                                                         field will override the FC latency timer value that the core calculates according to the
                                                         PCIe specification. */
	uint32_t reserved_3_15                : 13;
	uint32_t rqne                         : 1;  /**< Received queue not empty. Indicates there is data in one or more of the receive buffers. */
	uint32_t trbne                        : 1;  /**< Transmit retry buffer not empty. Indicates that there is data in the transmit retry buffer. */
	uint32_t rtlpfccnr                    : 1;  /**< Received TLP FC credits not returned. Indicates that the PCI Express bus has sent a TLP
                                                         but has not yet received an UpdateFC DLLP indicating that the credits for that TLP have
                                                         been restored by the receiver at the other end of the link. */
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
	struct cvmx_pcieepx_cfg463_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_3_31                : 29;
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
	uint32_t reserved_3_31                : 29;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg463_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg463_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg463_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg463_cn52xx     cn61xx;
	struct cvmx_pcieepx_cfg463_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg463_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg463_cn52xx     cn66xx;
	struct cvmx_pcieepx_cfg463_cn52xx     cn68xx;
	struct cvmx_pcieepx_cfg463_cn52xx     cn68xxp1;
	struct cvmx_pcieepx_cfg463_cn52xx     cn70xx;
	struct cvmx_pcieepx_cfg463_cn52xx     cn70xxp1;
	struct cvmx_pcieepx_cfg463_s          cn73xx;
	struct cvmx_pcieepx_cfg463_s          cn78xx;
	struct cvmx_pcieepx_cfg463_s          cn78xxp1;
	struct cvmx_pcieepx_cfg463_cn52xx     cnf71xx;
	struct cvmx_pcieepx_cfg463_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg463 cvmx_pcieepx_cfg463_t;

/**
 * cvmx_pcieep#_cfg464
 *
 * This register contains the four hundred sixty-fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg464 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg464_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t wrr_vc3                      : 8;  /**< WRR Weight for VC3. */
	uint32_t wrr_vc2                      : 8;  /**< WRR Weight for VC2. */
	uint32_t wrr_vc1                      : 8;  /**< WRR Weight for VC1. */
	uint32_t wrr_vc0                      : 8;  /**< WRR Weight for VC0. */
#else
	uint32_t wrr_vc0                      : 8;
	uint32_t wrr_vc1                      : 8;
	uint32_t wrr_vc2                      : 8;
	uint32_t wrr_vc3                      : 8;
#endif
	} s;
	struct cvmx_pcieepx_cfg464_s          cn52xx;
	struct cvmx_pcieepx_cfg464_s          cn52xxp1;
	struct cvmx_pcieepx_cfg464_s          cn56xx;
	struct cvmx_pcieepx_cfg464_s          cn56xxp1;
	struct cvmx_pcieepx_cfg464_s          cn61xx;
	struct cvmx_pcieepx_cfg464_s          cn63xx;
	struct cvmx_pcieepx_cfg464_s          cn63xxp1;
	struct cvmx_pcieepx_cfg464_s          cn66xx;
	struct cvmx_pcieepx_cfg464_s          cn68xx;
	struct cvmx_pcieepx_cfg464_s          cn68xxp1;
	struct cvmx_pcieepx_cfg464_s          cn70xx;
	struct cvmx_pcieepx_cfg464_s          cn70xxp1;
	struct cvmx_pcieepx_cfg464_s          cn73xx;
	struct cvmx_pcieepx_cfg464_s          cn78xx;
	struct cvmx_pcieepx_cfg464_s          cn78xxp1;
	struct cvmx_pcieepx_cfg464_s          cnf71xx;
	struct cvmx_pcieepx_cfg464_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg464 cvmx_pcieepx_cfg464_t;

/**
 * cvmx_pcieep#_cfg465
 *
 * This register contains the four hundred sixty-sixth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg465 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg465_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t wrr_vc7                      : 8;  /**< WRR Weight for VC7. */
	uint32_t wrr_vc6                      : 8;  /**< WRR Weight for VC6. */
	uint32_t wrr_vc5                      : 8;  /**< WRR Weight for VC5. */
	uint32_t wrr_vc4                      : 8;  /**< WRR Weight for VC4. */
#else
	uint32_t wrr_vc4                      : 8;
	uint32_t wrr_vc5                      : 8;
	uint32_t wrr_vc6                      : 8;
	uint32_t wrr_vc7                      : 8;
#endif
	} s;
	struct cvmx_pcieepx_cfg465_s          cn52xx;
	struct cvmx_pcieepx_cfg465_s          cn52xxp1;
	struct cvmx_pcieepx_cfg465_s          cn56xx;
	struct cvmx_pcieepx_cfg465_s          cn56xxp1;
	struct cvmx_pcieepx_cfg465_s          cn61xx;
	struct cvmx_pcieepx_cfg465_s          cn63xx;
	struct cvmx_pcieepx_cfg465_s          cn63xxp1;
	struct cvmx_pcieepx_cfg465_s          cn66xx;
	struct cvmx_pcieepx_cfg465_s          cn68xx;
	struct cvmx_pcieepx_cfg465_s          cn68xxp1;
	struct cvmx_pcieepx_cfg465_s          cn70xx;
	struct cvmx_pcieepx_cfg465_s          cn70xxp1;
	struct cvmx_pcieepx_cfg465_s          cn73xx;
	struct cvmx_pcieepx_cfg465_s          cn78xx;
	struct cvmx_pcieepx_cfg465_s          cn78xxp1;
	struct cvmx_pcieepx_cfg465_s          cnf71xx;
	struct cvmx_pcieepx_cfg465_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg465 cvmx_pcieepx_cfg465_t;

/**
 * cvmx_pcieep#_cfg466
 *
 * This register contains the four hundred sixty-seventh 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg466 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg466_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t rx_queue_order               : 1;  /**< VC ordering for receive queues. Determines the VC ordering rule for the receive queues,
                                                         used only in the segmented-buffer configuration, writable through
                                                         PEM()_CFG_WR:
                                                         0 = Round robin.
                                                         1 = Strict ordering, higher numbered VCs have higher priority.
                                                         The application must not change this field. */
	uint32_t type_ordering                : 1;  /**< TLP type ordering for VC0. Determines the TLP type ordering rule for VC0 receive queues,
                                                         used only in the segmented-buffer configuration, writable through
                                                         PEM()_CFG_WR:
                                                         0 = Strict ordering for received TLPs: Posted, then completion, then NonPosted.
                                                         1 = Ordering of received TLPs follows the rules in PCI Express Base Specification.
                                                         The application must not change this field. */
	uint32_t reserved_24_29               : 6;
	uint32_t queue_mode                   : 3;  /**< VC0 posted TLP queue mode. The operating mode of the posted receive queue for VC0, used
                                                         only in the segmented-buffer configuration, writable through PEM()_CFG_WR. However,
                                                         the application must not change this field.
                                                         Only one bit can be set at a time:
                                                         _ Bit 23 = Bypass.
                                                         _ Bit 22 = Cut-through.
                                                         _ Bit 21 = Store-and-forward. */
	uint32_t reserved_20_20               : 1;
	uint32_t header_credits               : 8;  /**< VC0 posted header credits. The number of initial posted header credits for VC0, used for
                                                         all receive queue buffer configurations. This field is writable through
                                                         PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t data_credits                 : 12; /**< VC0 posted data credits. The number of initial posted data credits for VC0, used for all
                                                         receive queue buffer configurations. This field is writable through
                                                         PEM()_CFG_WR. However, the application must not change this field. */
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
	struct cvmx_pcieepx_cfg466_s          cn52xx;
	struct cvmx_pcieepx_cfg466_s          cn52xxp1;
	struct cvmx_pcieepx_cfg466_s          cn56xx;
	struct cvmx_pcieepx_cfg466_s          cn56xxp1;
	struct cvmx_pcieepx_cfg466_s          cn61xx;
	struct cvmx_pcieepx_cfg466_s          cn63xx;
	struct cvmx_pcieepx_cfg466_s          cn63xxp1;
	struct cvmx_pcieepx_cfg466_s          cn66xx;
	struct cvmx_pcieepx_cfg466_s          cn68xx;
	struct cvmx_pcieepx_cfg466_s          cn68xxp1;
	struct cvmx_pcieepx_cfg466_s          cn70xx;
	struct cvmx_pcieepx_cfg466_s          cn70xxp1;
	struct cvmx_pcieepx_cfg466_s          cn73xx;
	struct cvmx_pcieepx_cfg466_s          cn78xx;
	struct cvmx_pcieepx_cfg466_s          cn78xxp1;
	struct cvmx_pcieepx_cfg466_s          cnf71xx;
	struct cvmx_pcieepx_cfg466_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg466 cvmx_pcieepx_cfg466_t;

/**
 * cvmx_pcieep#_cfg467
 *
 * This register contains the four hundred sixty-eighth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg467 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg467_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t queue_mode                   : 3;  /**< VC0 nonposted TLP queue mode. The operating mode of the nonposted receive queue for VC0,
                                                         used only in the segmented-buffer configuration, writable through
                                                         PEM()_CFG_WR.
                                                         Only one bit can be set at a time:
                                                         _ Bit 23 = Bypass.
                                                         _ Bit 22 = Cut-through.
                                                         _ Bit 21 = Store-and-forward.
                                                         The application must not change this field. */
	uint32_t reserved_20_20               : 1;
	uint32_t header_credits               : 8;  /**< VC0 nonposted header credits. The number of initial nonposted header credits for VC0, used
                                                         for all receive queue buffer configurations. This field is writable through
                                                         PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t data_credits                 : 12; /**< VC0 nonposted data credits. The number of initial nonposted data credits for VC0, used for
                                                         all receive queue buffer configurations. This field is writable through PEM()_CFG_WR.
                                                         However, the application must not change this field. */
#else
	uint32_t data_credits                 : 12;
	uint32_t header_credits               : 8;
	uint32_t reserved_20_20               : 1;
	uint32_t queue_mode                   : 3;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_pcieepx_cfg467_s          cn52xx;
	struct cvmx_pcieepx_cfg467_s          cn52xxp1;
	struct cvmx_pcieepx_cfg467_s          cn56xx;
	struct cvmx_pcieepx_cfg467_s          cn56xxp1;
	struct cvmx_pcieepx_cfg467_s          cn61xx;
	struct cvmx_pcieepx_cfg467_s          cn63xx;
	struct cvmx_pcieepx_cfg467_s          cn63xxp1;
	struct cvmx_pcieepx_cfg467_s          cn66xx;
	struct cvmx_pcieepx_cfg467_s          cn68xx;
	struct cvmx_pcieepx_cfg467_s          cn68xxp1;
	struct cvmx_pcieepx_cfg467_s          cn70xx;
	struct cvmx_pcieepx_cfg467_s          cn70xxp1;
	struct cvmx_pcieepx_cfg467_s          cn73xx;
	struct cvmx_pcieepx_cfg467_s          cn78xx;
	struct cvmx_pcieepx_cfg467_s          cn78xxp1;
	struct cvmx_pcieepx_cfg467_s          cnf71xx;
	struct cvmx_pcieepx_cfg467_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg467 cvmx_pcieepx_cfg467_t;

/**
 * cvmx_pcieep#_cfg468
 *
 * This register contains the four hundred sixty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg468 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg468_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t queue_mode                   : 3;  /**< VC0 completion TLP queue mode. The operating mode of the completion receive queue for VC0,
                                                         used only in the segmented-buffer configuration, writable through
                                                         PEM()_CFG_WR.
                                                         Only one bit can be set at a time:
                                                         _ Bit 23 = Bypass.
                                                         _ Bit 22 = Cut-through.
                                                         _ Bit 21 = Store-and-forward.
                                                         The application must not change this field. */
	uint32_t reserved_20_20               : 1;
	uint32_t header_credits               : 8;  /**< VC0 completion header credits. The number of initial completion header credits for VC0,
                                                         used for all receive queue buffer configurations. This field is writable through
                                                         PEM()_CFG_WR. However, the application must not change this field. */
	uint32_t data_credits                 : 12; /**< VC0 completion data credits. The number of initial completion data credits for VC0, used
                                                         for all receive queue buffer configurations. This field is writable through
                                                         PEM()_CFG_WR. However, the application must not change this field. */
#else
	uint32_t data_credits                 : 12;
	uint32_t header_credits               : 8;
	uint32_t reserved_20_20               : 1;
	uint32_t queue_mode                   : 3;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_pcieepx_cfg468_s          cn52xx;
	struct cvmx_pcieepx_cfg468_s          cn52xxp1;
	struct cvmx_pcieepx_cfg468_s          cn56xx;
	struct cvmx_pcieepx_cfg468_s          cn56xxp1;
	struct cvmx_pcieepx_cfg468_s          cn61xx;
	struct cvmx_pcieepx_cfg468_s          cn63xx;
	struct cvmx_pcieepx_cfg468_s          cn63xxp1;
	struct cvmx_pcieepx_cfg468_s          cn66xx;
	struct cvmx_pcieepx_cfg468_s          cn68xx;
	struct cvmx_pcieepx_cfg468_s          cn68xxp1;
	struct cvmx_pcieepx_cfg468_s          cn70xx;
	struct cvmx_pcieepx_cfg468_s          cn70xxp1;
	struct cvmx_pcieepx_cfg468_s          cn73xx;
	struct cvmx_pcieepx_cfg468_s          cn78xx;
	struct cvmx_pcieepx_cfg468_s          cn78xxp1;
	struct cvmx_pcieepx_cfg468_s          cnf71xx;
	struct cvmx_pcieepx_cfg468_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg468 cvmx_pcieepx_cfg468_t;

/**
 * cvmx_pcieep#_cfg490
 *
 * PCIE_CFG490 = Four hundred ninety-first 32-bits of PCIE type 0 config space
 * (VC0 Posted Buffer Depth)
 */
union cvmx_pcieepx_cfg490 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg490_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t header_depth                 : 10; /**< VC0 Posted Header Queue Depth
                                                         Sets the number of entries in the Posted header queue for VC0
                                                         when using the segmented-buffer configuration, writable through
                                                         PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t reserved_14_15               : 2;
	uint32_t data_depth                   : 14; /**< VC0 Posted Data Queue Depth
                                                         Sets the number of entries in the Posted data queue for VC0
                                                         when using the segmented-buffer configuration, writable
                                                         through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
#else
	uint32_t data_depth                   : 14;
	uint32_t reserved_14_15               : 2;
	uint32_t header_depth                 : 10;
	uint32_t reserved_26_31               : 6;
#endif
	} s;
	struct cvmx_pcieepx_cfg490_s          cn52xx;
	struct cvmx_pcieepx_cfg490_s          cn52xxp1;
	struct cvmx_pcieepx_cfg490_s          cn56xx;
	struct cvmx_pcieepx_cfg490_s          cn56xxp1;
	struct cvmx_pcieepx_cfg490_s          cn61xx;
	struct cvmx_pcieepx_cfg490_s          cn63xx;
	struct cvmx_pcieepx_cfg490_s          cn63xxp1;
	struct cvmx_pcieepx_cfg490_s          cn66xx;
	struct cvmx_pcieepx_cfg490_s          cn68xx;
	struct cvmx_pcieepx_cfg490_s          cn68xxp1;
	struct cvmx_pcieepx_cfg490_s          cn70xx;
	struct cvmx_pcieepx_cfg490_s          cn70xxp1;
	struct cvmx_pcieepx_cfg490_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg490 cvmx_pcieepx_cfg490_t;

/**
 * cvmx_pcieep#_cfg491
 *
 * PCIE_CFG491 = Four hundred ninety-second 32-bits of PCIE type 0 config space
 * (VC0 Non-Posted Buffer Depth)
 */
union cvmx_pcieepx_cfg491 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg491_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t header_depth                 : 10; /**< VC0 Non-Posted Header Queue Depth
                                                         Sets the number of entries in the Non-Posted header queue for
                                                         VC0 when using the segmented-buffer configuration, writable
                                                         through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t reserved_14_15               : 2;
	uint32_t data_depth                   : 14; /**< VC0 Non-Posted Data Queue Depth
                                                         Sets the number of entries in the Non-Posted data queue for VC0
                                                         when using the segmented-buffer configuration, writable
                                                         through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
#else
	uint32_t data_depth                   : 14;
	uint32_t reserved_14_15               : 2;
	uint32_t header_depth                 : 10;
	uint32_t reserved_26_31               : 6;
#endif
	} s;
	struct cvmx_pcieepx_cfg491_s          cn52xx;
	struct cvmx_pcieepx_cfg491_s          cn52xxp1;
	struct cvmx_pcieepx_cfg491_s          cn56xx;
	struct cvmx_pcieepx_cfg491_s          cn56xxp1;
	struct cvmx_pcieepx_cfg491_s          cn61xx;
	struct cvmx_pcieepx_cfg491_s          cn63xx;
	struct cvmx_pcieepx_cfg491_s          cn63xxp1;
	struct cvmx_pcieepx_cfg491_s          cn66xx;
	struct cvmx_pcieepx_cfg491_s          cn68xx;
	struct cvmx_pcieepx_cfg491_s          cn68xxp1;
	struct cvmx_pcieepx_cfg491_s          cn70xx;
	struct cvmx_pcieepx_cfg491_s          cn70xxp1;
	struct cvmx_pcieepx_cfg491_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg491 cvmx_pcieepx_cfg491_t;

/**
 * cvmx_pcieep#_cfg492
 *
 * PCIE_CFG492 = Four hundred ninety-third 32-bits of PCIE type 0 config space
 * (VC0 Completion Buffer Depth)
 */
union cvmx_pcieepx_cfg492 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg492_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t header_depth                 : 10; /**< VC0 Completion Header Queue Depth
                                                         Sets the number of entries in the Completion header queue for
                                                         VC0 when using the segmented-buffer configuration, writable
                                                         through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t reserved_14_15               : 2;
	uint32_t data_depth                   : 14; /**< VC0 Completion Data Queue Depth
                                                         Sets the number of entries in the Completion data queue for VC0
                                                         when using the segmented-buffer configuration, writable
                                                         through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
#else
	uint32_t data_depth                   : 14;
	uint32_t reserved_14_15               : 2;
	uint32_t header_depth                 : 10;
	uint32_t reserved_26_31               : 6;
#endif
	} s;
	struct cvmx_pcieepx_cfg492_s          cn52xx;
	struct cvmx_pcieepx_cfg492_s          cn52xxp1;
	struct cvmx_pcieepx_cfg492_s          cn56xx;
	struct cvmx_pcieepx_cfg492_s          cn56xxp1;
	struct cvmx_pcieepx_cfg492_s          cn61xx;
	struct cvmx_pcieepx_cfg492_s          cn63xx;
	struct cvmx_pcieepx_cfg492_s          cn63xxp1;
	struct cvmx_pcieepx_cfg492_s          cn66xx;
	struct cvmx_pcieepx_cfg492_s          cn68xx;
	struct cvmx_pcieepx_cfg492_s          cn68xxp1;
	struct cvmx_pcieepx_cfg492_s          cn70xx;
	struct cvmx_pcieepx_cfg492_s          cn70xxp1;
	struct cvmx_pcieepx_cfg492_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg492 cvmx_pcieepx_cfg492_t;

/**
 * cvmx_pcieep#_cfg515
 *
 * This register contains the five hundred sixteenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg515 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg515_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_21_31               : 11;
	uint32_t s_d_e                        : 1;  /**< SEL_DE_EMPHASIS. Used to set the deemphasis level for upstream ports. */
	uint32_t ctcrb                        : 1;  /**< Config TX compliance receive bit. When set to 1, signals LTSSM to transmit TS ordered sets
                                                         with the compliance receive bit assert (equal to 1). */
	uint32_t cpyts                        : 1;  /**< Config PHY TX swing. Indicates the voltage level that the PHY should drive. When set to 1,
                                                         indicates full swing. When set to 0, indicates low swing. */
	uint32_t dsc                          : 1;  /**< Directed speed change. A write of 1 initiates a speed change.
                                                         When the speed change occurs, the controller will clear the contents of this field. */
	uint32_t reserved_8_16                : 9;
	uint32_t n_fts                        : 8;  /**< N_FTS. Sets the number of fast training sequences (N_FTS) that the core advertises as its
                                                         N_FTS during GEN2 Link training. This value is used to inform the link partner about the
                                                         PHY's ability to recover synchronization after a low power state.
                                                         Do not set [N_FTS] to zero; doing so can cause the LTSSM to go into the recovery
                                                         state when exiting from L0s. */
#else
	uint32_t n_fts                        : 8;
	uint32_t reserved_8_16                : 9;
	uint32_t dsc                          : 1;
	uint32_t cpyts                        : 1;
	uint32_t ctcrb                        : 1;
	uint32_t s_d_e                        : 1;
	uint32_t reserved_21_31               : 11;
#endif
	} s;
	struct cvmx_pcieepx_cfg515_cn61xx {
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
	} cn61xx;
	struct cvmx_pcieepx_cfg515_cn61xx     cn63xx;
	struct cvmx_pcieepx_cfg515_cn61xx     cn63xxp1;
	struct cvmx_pcieepx_cfg515_cn61xx     cn66xx;
	struct cvmx_pcieepx_cfg515_cn61xx     cn68xx;
	struct cvmx_pcieepx_cfg515_cn61xx     cn68xxp1;
	struct cvmx_pcieepx_cfg515_cn61xx     cn70xx;
	struct cvmx_pcieepx_cfg515_cn61xx     cn70xxp1;
	struct cvmx_pcieepx_cfg515_cn61xx     cn73xx;
	struct cvmx_pcieepx_cfg515_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_21_31               : 11;
	uint32_t s_d_e                        : 1;  /**< SEL_DE_EMPHASIS. Used to set the deemphasis level for upstream ports. */
	uint32_t ctcrb                        : 1;  /**< Config TX compliance receive bit. When set to 1, signals LTSSM to transmit TS ordered sets
                                                         with the compliance receive bit assert (equal to 1). */
	uint32_t cpyts                        : 1;  /**< Config PHY TX swing. Indicates the voltage level that the PHY should drive. When set to 1,
                                                         indicates full swing. When set to 0, indicates low swing. */
	uint32_t dsc                          : 1;  /**< Directed speed change. A write of 1 initiates a speed change.
                                                         When the speed change occurs, the controller will clear the contents of this field. */
	uint32_t alaneflip                    : 1;  /**< Enable auto flipping of the lanes. */
	uint32_t pdetlane                     : 3;  /**< Predetermined lane for auto flip. This field defines which
                                                         physical lane is connected to logical Lane0 by the flip
                                                         operation performed in detect.
                                                           0x0 = Auto flipping not supported.
                                                           0x1 = Connect logical Lane0 to physical lane 1.
                                                           0x2 = Connect logical Lane0 to physical lane 3.
                                                           0x3 = Connect logical Lane0 to physical lane 7.
                                                           0x4 = Connect logical Lane0 to physical lane 15.
                                                           0x5 - 0x7 = Reserved. */
	uint32_t nlanes                       : 5;  /**< Predetermined number of lanes.  Defines the number of
                                                         lanes which are connected and not bad. Used to limit the
                                                         effective link width to ignore "broken" or "unused" lanes that
                                                         detect a receiver. Indicates the number of lanes to check for
                                                         exit from electrical idle in Polling.Active and L2.Idle.
                                                         0x1 = 1 lane.
                                                         0x2 = 2 lanes.
                                                         0x3 = 3 lanes.
                                                         - ...
                                                         0x8 = 8 lanes.
                                                         0x9-0x1F = Reserved.
                                                         When there are unused lanes in the system, then this value must reflect the
                                                         number of lanes. PCIEEP()_CFG452[LME] must also be changed likewise. */
	uint32_t n_fts                        : 8;  /**< N_FTS. Sets the number of fast training sequences (N_FTS) that the core advertises as its
                                                         N_FTS during GEN2 Link training. This value is used to inform the link partner about the
                                                         PHY's ability to recover synchronization after a low power state.
                                                         Do not set [N_FTS] to zero; doing so can cause the LTSSM to go into the recovery
                                                         state when exiting from L0s. */
#else
	uint32_t n_fts                        : 8;
	uint32_t nlanes                       : 5;
	uint32_t pdetlane                     : 3;
	uint32_t alaneflip                    : 1;
	uint32_t dsc                          : 1;
	uint32_t cpyts                        : 1;
	uint32_t ctcrb                        : 1;
	uint32_t s_d_e                        : 1;
	uint32_t reserved_21_31               : 11;
#endif
	} cn78xx;
	struct cvmx_pcieepx_cfg515_cn61xx     cn78xxp1;
	struct cvmx_pcieepx_cfg515_cn61xx     cnf71xx;
	struct cvmx_pcieepx_cfg515_cn78xx     cnf75xx;
};
typedef union cvmx_pcieepx_cfg515 cvmx_pcieepx_cfg515_t;

/**
 * cvmx_pcieep#_cfg516
 *
 * This register contains the five hundred seventeenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg516 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg516_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t phy_stat                     : 32; /**< PHY status. */
#else
	uint32_t phy_stat                     : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg516_s          cn52xx;
	struct cvmx_pcieepx_cfg516_s          cn52xxp1;
	struct cvmx_pcieepx_cfg516_s          cn56xx;
	struct cvmx_pcieepx_cfg516_s          cn56xxp1;
	struct cvmx_pcieepx_cfg516_s          cn61xx;
	struct cvmx_pcieepx_cfg516_s          cn63xx;
	struct cvmx_pcieepx_cfg516_s          cn63xxp1;
	struct cvmx_pcieepx_cfg516_s          cn66xx;
	struct cvmx_pcieepx_cfg516_s          cn68xx;
	struct cvmx_pcieepx_cfg516_s          cn68xxp1;
	struct cvmx_pcieepx_cfg516_s          cn70xx;
	struct cvmx_pcieepx_cfg516_s          cn70xxp1;
	struct cvmx_pcieepx_cfg516_s          cn73xx;
	struct cvmx_pcieepx_cfg516_s          cn78xx;
	struct cvmx_pcieepx_cfg516_s          cn78xxp1;
	struct cvmx_pcieepx_cfg516_s          cnf71xx;
	struct cvmx_pcieepx_cfg516_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg516 cvmx_pcieepx_cfg516_t;

/**
 * cvmx_pcieep#_cfg517
 *
 * This register contains the five hundred eighteenth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg517 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg517_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t phy_ctrl                     : 32; /**< PHY control. */
#else
	uint32_t phy_ctrl                     : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg517_s          cn52xx;
	struct cvmx_pcieepx_cfg517_s          cn52xxp1;
	struct cvmx_pcieepx_cfg517_s          cn56xx;
	struct cvmx_pcieepx_cfg517_s          cn56xxp1;
	struct cvmx_pcieepx_cfg517_s          cn61xx;
	struct cvmx_pcieepx_cfg517_s          cn63xx;
	struct cvmx_pcieepx_cfg517_s          cn63xxp1;
	struct cvmx_pcieepx_cfg517_s          cn66xx;
	struct cvmx_pcieepx_cfg517_s          cn68xx;
	struct cvmx_pcieepx_cfg517_s          cn68xxp1;
	struct cvmx_pcieepx_cfg517_s          cn70xx;
	struct cvmx_pcieepx_cfg517_s          cn70xxp1;
	struct cvmx_pcieepx_cfg517_s          cn73xx;
	struct cvmx_pcieepx_cfg517_s          cn78xx;
	struct cvmx_pcieepx_cfg517_s          cn78xxp1;
	struct cvmx_pcieepx_cfg517_s          cnf71xx;
	struct cvmx_pcieepx_cfg517_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg517 cvmx_pcieepx_cfg517_t;

/**
 * cvmx_pcieep#_cfg548
 *
 * This register contains the five hundred forty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg548 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg548_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t rss                          : 2;  /**< Data rate for shadow register.  Hard-wired for Gen3. */
	uint32_t eiedd                        : 1;  /**< Eq InvalidRequest and RxEqEval Different Time Assertion Disable.  Disable the assertion of
                                                         Eq InvalidRequest and RxEqEval at different time. */
	uint32_t reserved_19_22               : 4;
	uint32_t dcbd                         : 1;  /**< Disable balance disable. Disable DC balance feature. */
	uint32_t dtdd                         : 1;  /**< DLLP transmission delay disable. Disable delay transmission of DLLPs before equalization. */
	uint32_t ed                           : 1;  /**< Equalization disable. Disable equalization feature. */
	uint32_t reserved_13_15               : 3;
	uint32_t rxeq_ph01_en                 : 1;  /**< Rx Equalization Phase 0/Phase 1 Hold Enable. */
	uint32_t erd                          : 1;  /**< Equalization redo disable. Disable requesting reset of EIEOS count during equalization. */
	uint32_t ecrd                         : 1;  /**< Equalization EIEOS count reset disable. Disable requesting reset of EIEOS count during
                                                         equalization. */
	uint32_t ep2p3d                       : 1;  /**< Equalization phase 2 and phase 3 disable. This applies to downstream ports only. */
	uint32_t dsg3                         : 1;  /**< Disable scrambler for Gen3 data rate. The Gen3 scrambler/descrambler within the core needs
                                                         to be disabled when the scrambling function is implemented outside of the core (within the
                                                         PHY). */
	uint32_t reserved_1_7                 : 7;
	uint32_t grizdnc                      : 1;  /**< Gen3 receiver impedance ZRX-DC not compliant. */
#else
	uint32_t grizdnc                      : 1;
	uint32_t reserved_1_7                 : 7;
	uint32_t dsg3                         : 1;
	uint32_t ep2p3d                       : 1;
	uint32_t ecrd                         : 1;
	uint32_t erd                          : 1;
	uint32_t rxeq_ph01_en                 : 1;
	uint32_t reserved_13_15               : 3;
	uint32_t ed                           : 1;
	uint32_t dtdd                         : 1;
	uint32_t dcbd                         : 1;
	uint32_t reserved_19_22               : 4;
	uint32_t eiedd                        : 1;
	uint32_t rss                          : 2;
	uint32_t reserved_26_31               : 6;
#endif
	} s;
	struct cvmx_pcieepx_cfg548_s          cn73xx;
	struct cvmx_pcieepx_cfg548_s          cn78xx;
	struct cvmx_pcieepx_cfg548_s          cn78xxp1;
	struct cvmx_pcieepx_cfg548_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg548 cvmx_pcieepx_cfg548_t;

/**
 * cvmx_pcieep#_cfg554
 *
 * This register contains the five hundred fifty-fifth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg554 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg554_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_27_31               : 5;
	uint32_t scefpm                       : 1;  /**< Request core to send back-to-back EIEOS in Recovery.RcvrLock state until
                                                         presets to coefficient mapping is complete. */
	uint32_t reserved_25_25               : 1;
	uint32_t iif                          : 1;  /**< Include initial FOM. Include, or not, the FOM feedback from the initial preset evaluation
                                                         performed in the EQ master, when finding the highest FOM among all preset evaluations. */
	uint32_t prv                          : 16; /**< Preset request vector. Requesting of presets during the initial part of the EQ master
                                                         phase. Encoding scheme as follows:
                                                         Bit [15:0] = 0x0: No preset is requested and evaluated in the EQ master phase.
                                                         Bit [i] = 1: Preset=i is requested and evaluated in the EQ master phase.
                                                         _ 0b0000000000000000 = No preset req/evaluated in EQ master phase.
                                                         _ 0b00000xxxxxxxxxx1 = Preset 0 req/evaluated in EQ master phase.
                                                         _ 0b00000xxxxxxxxx1x = Preset 1 req/evaluated in EQ master phase.
                                                         _ 0b00000xxxxxxxx1xx = Preset 2 req/evaluated in EQ master phase.
                                                         _ 0b00000xxxxxxx1xxx = Preset 3 req/evaluated in EQ master phase.
                                                         _ 0b00000xxxxxx1xxxx = Preset 4 req/evaluated in EQ master phase.
                                                         _ 0b00000xxxxx1xxxxx = Preset 5 req/evaluated in EQ master phase.
                                                         _ 0b00000xxxx1xxxxxx = Preset 6 req/evaluated in EQ master phase.
                                                         _ 0b00000xxx1xxxxxxx = Preset 7 req/evaluated in EQ master phase.
                                                         _ 0b00000xx1xxxxxxxx = Preset 8 req/evaluated in EQ master phase.
                                                         _ 0b00000x1xxxxxxxxx = Preset 9 req/evaluated in EQ master phase.
                                                         _ 0b000001xxxxxxxxxx = Preset 10 req/evaluated in EQ master phase.
                                                         _ All other encodings = Reserved. */
	uint32_t reserved_6_7                 : 2;
	uint32_t p23td                        : 1;  /**< Phase2_3 2 ms timeout disable. Determine behavior in Phase2 for USP (Phase3 if DSP) when
                                                         the PHY does not respond within 2 ms to the assertion of RxEqEval:
                                                         0 = Abort the current evaluation; stop any attempt to modify the remote transmitter
                                                         settings. Phase2 will be terminated by the 24 ms timeout.
                                                         1 = Ignore the 2 ms timeout and continue as normal. This is used to support PHYs that
                                                         require more than 2 ms to respond to the assertion of RxEqEval. */
	uint32_t bt                           : 1;  /**< Behavior after 24 ms timeout (when optimal settings are not found).
                                                         For a USP: determine the next LTSSM state from Phase2:
                                                         0 = Recovery.Speed.
                                                         1 = Recovry.Equalization.Phase3.
                                                         For a DSP: determine the next LTSSM state from Phase3:
                                                         0 = Recovery.Speed.
                                                         1 = Recovry.Equalization.RcrLock.
                                                         When optimal settings are not found:
                                                         * Equalization phase 3 successful status bit is not set in the link status register.
                                                         * Equalization phase 3 complete status bit is set in the link status register. */
	uint32_t fm                           : 4;  /**< Feedback mode.
                                                         0 = Direction of change (not supported).
                                                         1 = Figure of merit.
                                                         2-15 = Reserved. */
#else
	uint32_t fm                           : 4;
	uint32_t bt                           : 1;
	uint32_t p23td                        : 1;
	uint32_t reserved_6_7                 : 2;
	uint32_t prv                          : 16;
	uint32_t iif                          : 1;
	uint32_t reserved_25_25               : 1;
	uint32_t scefpm                       : 1;
	uint32_t reserved_27_31               : 5;
#endif
	} s;
	struct cvmx_pcieepx_cfg554_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_25_31               : 7;
	uint32_t iif                          : 1;  /**< Include initial FOM. Include, or not, the FOM feedback from the initial preset evaluation
                                                         performed in the EQ master, when finding the highest FOM among all preset evaluations. */
	uint32_t prv                          : 16; /**< Preset request vector. Requesting of presets during the initial part of the EQ master
                                                         phase. Encoding scheme as follows:
                                                         Bit [15:0] = 0x0: No preset is requested and evaluated in the EQ master phase.
                                                         Bit [i] = 1: Preset=i is requested and evaluated in the EQ master phase.
                                                         _ 0b0000000000000000 = No preset req/evaluated in EQ master phase.
                                                         _ 0b00000xxxxxxxxxx1 = Preset 0 req/evaluated in EQ master phase.
                                                         _ 0b00000xxxxxxxxx1x = Preset 1 req/evaluated in EQ master phase.
                                                         _ 0b00000xxxxxxxx1xx = Preset 2 req/evaluated in EQ master phase.
                                                         _ 0b00000xxxxxxx1xxx = Preset 3 req/evaluated in EQ master phase.
                                                         _ 0b00000xxxxxx1xxxx = Preset 4 req/evaluated in EQ master phase.
                                                         _ 0b00000xxxxx1xxxxx = Preset 5 req/evaluated in EQ master phase.
                                                         _ 0b00000xxxx1xxxxxx = Preset 6 req/evaluated in EQ master phase.
                                                         _ 0b00000xxx1xxxxxxx = Preset 7 req/evaluated in EQ master phase.
                                                         _ 0b00000xx1xxxxxxxx = Preset 8 req/evaluated in EQ master phase.
                                                         _ 0b00000x1xxxxxxxxx = Preset 9 req/evaluated in EQ master phase.
                                                         _ 0b000001xxxxxxxxxx = Preset 10 req/evaluated in EQ master phase.
                                                         _ All other encodings = Reserved. */
	uint32_t reserved_6_7                 : 2;
	uint32_t p23td                        : 1;  /**< Phase2_3 2 ms timeout disable. Determine behavior in Phase2 for USP (Phase3 if DSP) when
                                                         the PHY does not respond within 2 ms to the assertion of RxEqEval:
                                                         0 = Abort the current evaluation; stop any attempt to modify the remote transmitter
                                                         settings. Phase2 will be terminated by the 24 ms timeout.
                                                         1 = Ignore the 2 ms timeout and continue as normal. This is used to support PHYs that
                                                         require more than 2 ms to respond to the assertion of RxEqEval. */
	uint32_t bt                           : 1;  /**< Behavior after 24 ms timeout (when optimal settings are not found).
                                                         For a USP: determine the next LTSSM state from Phase2:
                                                         0 = Recovery.Speed.
                                                         1 = Recovry.Equalization.Phase3.
                                                         For a DSP: determine the next LTSSM state from Phase3:
                                                         0 = Recovery.Speed.
                                                         1 = Recovry.Equalization.RcrLock.
                                                         When optimal settings are not found:
                                                         * Equalization phase 3 successful status bit is not set in the link status register.
                                                         * Equalization phase 3 complete status bit is set in the link status register. */
	uint32_t fm                           : 4;  /**< Feedback mode.
                                                         0 = Direction of change (not supported).
                                                         1 = Figure of merit.
                                                         2-15 = Reserved. */
#else
	uint32_t fm                           : 4;
	uint32_t bt                           : 1;
	uint32_t p23td                        : 1;
	uint32_t reserved_6_7                 : 2;
	uint32_t prv                          : 16;
	uint32_t iif                          : 1;
	uint32_t reserved_25_31               : 7;
#endif
	} cn73xx;
	struct cvmx_pcieepx_cfg554_s          cn78xx;
	struct cvmx_pcieepx_cfg554_s          cn78xxp1;
	struct cvmx_pcieepx_cfg554_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg554 cvmx_pcieepx_cfg554_t;

/**
 * cvmx_pcieep#_cfg558
 *
 * This register contains the five hundred fifty-ninth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg558 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg558_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ple                          : 1;  /**< Pipe loopback enable. */
	uint32_t rxstatus                     : 31; /**< Reserved. */
#else
	uint32_t rxstatus                     : 31;
	uint32_t ple                          : 1;
#endif
	} s;
	struct cvmx_pcieepx_cfg558_s          cn73xx;
	struct cvmx_pcieepx_cfg558_s          cn78xx;
	struct cvmx_pcieepx_cfg558_s          cn78xxp1;
	struct cvmx_pcieepx_cfg558_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg558 cvmx_pcieepx_cfg558_t;

/**
 * cvmx_pcieep#_cfg559
 *
 * This register contains the five hundred sixtieth 32-bits of PCIe type 0 configuration space.
 *
 */
union cvmx_pcieepx_cfg559 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg559_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_1_31                : 31;
	uint32_t dbi_ro_wr_en                 : 1;  /**< Write to RO registers using DBI.  When you set this bit, then some
                                                         RO bits are writable from the DBI. */
#else
	uint32_t dbi_ro_wr_en                 : 1;
	uint32_t reserved_1_31                : 31;
#endif
	} s;
	struct cvmx_pcieepx_cfg559_s          cn73xx;
	struct cvmx_pcieepx_cfg559_s          cn78xx;
	struct cvmx_pcieepx_cfg559_s          cn78xxp1;
	struct cvmx_pcieepx_cfg559_s          cnf75xx;
};
typedef union cvmx_pcieepx_cfg559 cvmx_pcieepx_cfg559_t;

#endif
