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
static inline uint64_t CVMX_PCIEEPX_CFG000(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000000ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000000ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000000ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000000ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG000 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000000ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG000(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000000ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000000ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000000ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000000ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG001(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000004ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000004ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000004ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000004ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG001 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000004ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG001(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000004ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000004ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000004ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000004ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000004ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG002(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000008ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000008ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000008ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000008ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG002 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000008ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG002(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000008ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000008ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000008ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000008ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000008ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG003(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000000000000Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x000000000000000Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003000000000Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003000000000Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG003 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000000000000Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG003(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000000Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000000Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000000Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003000000000Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000000000000Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG004(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000010ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000010ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000010ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000010ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG004 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000010ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG004(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000010ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000010ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000010ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000010ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000010ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG004_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000080000010ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000080000010ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030080000010ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030080000010ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG004_MASK (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000080000010ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG004_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000010ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000010ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000010ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000010ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000080000010ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG005(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000014ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000014ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000014ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000014ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG005 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000014ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG005(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000014ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000014ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000014ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000014ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000014ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG005_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000080000014ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000080000014ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030080000014ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030080000014ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG005_MASK (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000080000014ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG005_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000014ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000014ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000014ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000014ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000080000014ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG006(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000018ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000018ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000018ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000018ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG006 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000018ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG006(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000018ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000018ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000018ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000018ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000018ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG006_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000080000018ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000080000018ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030080000018ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030080000018ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG006_MASK (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000080000018ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG006_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000018ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000018ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000018ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000018ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000080000018ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG007(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000000000001Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x000000000000001Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003000000001Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003000000001Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG007 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000000000001Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG007(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000001Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000001Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000001Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003000000001Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000000000001Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG007_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000008000001Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x000000008000001Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003008000001Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003008000001Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG007_MASK (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000008000001Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG007_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000008000001Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000008000001Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003008000001Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003008000001Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000008000001Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG008(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000020ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000020ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000020ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000020ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG008 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000020ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG008(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000020ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000020ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000020ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000020ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000020ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG008_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000080000020ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000080000020ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030080000020ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030080000020ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG008_MASK (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000080000020ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG008_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000020ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000020ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000020ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000020ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000080000020ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG009(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000024ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000024ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000024ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000024ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG009 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000024ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG009(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000024ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000024ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000024ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000024ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000024ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG009_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000080000024ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000080000024ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030080000024ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030080000024ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG009_MASK (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000080000024ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG009_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000024ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000024ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000024ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000024ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000080000024ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG010(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000028ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000028ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000028ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000028ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG010 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000028ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG010(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000028ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000028ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000028ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000028ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000028ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG011(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000000000002Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x000000000000002Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003000000002Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003000000002Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG011 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000000000002Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG011(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000002Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000002Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000002Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003000000002Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000000000002Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG012(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000030ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000030ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000030ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000030ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG012 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000030ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG012(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000030ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000030ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000030ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000030ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000030ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG012_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000080000030ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000080000030ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030080000030ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030080000030ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG012_MASK (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000080000030ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG012_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000030ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000080000030ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000030ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030080000030ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000080000030ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG013(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000034ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000034ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000034ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000034ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG013 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000034ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG013(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000034ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000034ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000034ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000034ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000034ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG015(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000000000003Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x000000000000003Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003000000003Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003000000003Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG015 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000000000003Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG015(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000003Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000003Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000003Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003000000003Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000000000003Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG016(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000040ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000040ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000040ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000040ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG016 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000040ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG016(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000040ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000040ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000040ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000040ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000040ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG017(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000044ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000044ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000044ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000044ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG017 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000044ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG017(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000044ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000044ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000044ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000044ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000044ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG020(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000050ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000050ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000050ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000050ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG020 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000050ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG020(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000050ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000050ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000050ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000050ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000050ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG021(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000054ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000054ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000054ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000054ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG021 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000054ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG021(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000054ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000054ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000054ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000054ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000054ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG022(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000058ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000058ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000058ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000058ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG022 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000058ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG022(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000058ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000058ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000058ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000058ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000058ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG023(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000000000005Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x000000000000005Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003000000005Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003000000005Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG023 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000000000005Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG023(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000005Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000005Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000005Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003000000005Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000000000005Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG024(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG024(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000060ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG024(block_id) (0x0000030000000060ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG025(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG025(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000064ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG025(block_id) (0x0000030000000064ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG028(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000070ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000070ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000070ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000070ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG028 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000070ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG028(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000070ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000070ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000070ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000070ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000070ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG029(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000074ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000074ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000074ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000074ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG029 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000074ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG029(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000074ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000074ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000074ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000074ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000074ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG030(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000078ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000078ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000078ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000078ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG030 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000078ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG030(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000078ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000078ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000078ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000078ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000078ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG031(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000000000007Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x000000000000007Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003000000007Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003000000007Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG031 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000000000007Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG031(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000007Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000007Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000007Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003000000007Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000000000007Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG032(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000080ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000080ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000080ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000080ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG032 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000080ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG032(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000080ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000080ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000080ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000080ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000080ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG033(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_PCIEEPX_CFG033(%lu) is invalid on this chip\n", block_id);
	return 0x0000000000000084ull;
}
#else
#define CVMX_PCIEEPX_CFG033(block_id) (0x0000000000000084ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG034(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_PCIEEPX_CFG034(%lu) is invalid on this chip\n", block_id);
	return 0x0000000000000088ull;
}
#else
#define CVMX_PCIEEPX_CFG034(block_id) (0x0000000000000088ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG037(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000094ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000094ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000094ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000094ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG037 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000094ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG037(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000094ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000094ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000094ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000094ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000094ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG038(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000098ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000098ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000098ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000098ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG038 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000098ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG038(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000098ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000098ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000098ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000098ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000098ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG039(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000000000009Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x000000000000009Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003000000009Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003000000009Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG039 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000000000009Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG039(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000009Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000009Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000009Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003000000009Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000000000009Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG040(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x00000000000000A0ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x00000000000000A0ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x00000300000000A0ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x00000300000000A0ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG040 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x00000000000000A0ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG040(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x00000000000000A0ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x00000000000000A0ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x00000300000000A0ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x00000300000000A0ull + (block_id) * 0x100000000ull;
	}
	return 0x00000000000000A0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG041(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_PCIEEPX_CFG041(%lu) is invalid on this chip\n", block_id);
	return 0x00000000000000A4ull;
}
#else
#define CVMX_PCIEEPX_CFG041(block_id) (0x00000000000000A4ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG042(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_PCIEEPX_CFG042(%lu) is invalid on this chip\n", block_id);
	return 0x00000000000000A8ull;
}
#else
#define CVMX_PCIEEPX_CFG042(block_id) (0x00000000000000A8ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG044(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG044(%lu) is invalid on this chip\n", block_id);
	return 0x00000300000000B0ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG044(block_id) (0x00000300000000B0ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG045(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG045(%lu) is invalid on this chip\n", block_id);
	return 0x00000300000000B4ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG045(block_id) (0x00000300000000B4ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG046(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG046(%lu) is invalid on this chip\n", block_id);
	return 0x00000300000000B8ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG046(block_id) (0x00000300000000B8ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG064(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000100ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000100ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000100ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000100ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG064 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000100ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG064(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000100ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000100ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000100ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000100ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000100ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG065(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000104ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000104ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000104ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000104ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG065 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000104ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG065(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000104ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000104ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000104ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000104ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000104ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG066(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000108ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000108ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000108ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000108ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG066 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000108ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG066(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000108ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000108ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000108ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000108ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000108ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG067(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000000000010Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x000000000000010Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003000000010Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003000000010Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG067 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000000000010Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG067(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000010Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000010Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000010Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003000000010Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000000000010Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG068(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000110ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000110ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000110ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000110ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG068 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000110ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG068(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000110ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000110ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000110ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000110ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000110ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG069(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000114ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000114ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000114ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000114ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG069 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000114ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG069(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000114ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000114ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000114ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000114ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000114ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG070(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000118ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000118ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000118ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000118ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG070 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000118ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG070(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000118ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000118ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000118ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000118ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000118ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG071(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000000000011Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x000000000000011Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003000000011Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003000000011Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG071 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000000000011Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG071(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000011Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000011Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000011Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003000000011Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000000000011Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG072(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000120ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000120ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000120ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000120ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG072 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000120ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG072(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000120ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000120ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000120ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000120ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000120ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG073(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000124ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000124ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000124ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000124ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG073 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000124ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG073(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000124ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000124ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000124ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000124ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000124ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG074(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000128ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000128ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000128ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000128ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG074 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000128ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG074(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000128ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000128ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000128ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000128ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000128ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG078(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG078(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000138ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG078(block_id) (0x0000030000000138ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG082(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG082(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000148ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG082(block_id) (0x0000030000000148ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG083(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG083(%lu) is invalid on this chip\n", block_id);
	return 0x000003000000014Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG083(block_id) (0x000003000000014Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG084(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2)))))
		cvmx_warn("CVMX_PCIEEPX_CFG084(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000150ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG084(block_id) (0x0000030000000150ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG086(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG086(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000158ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG086(block_id) (0x0000030000000158ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG087(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG087(%lu) is invalid on this chip\n", block_id);
	return 0x000003000000015Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG087(block_id) (0x000003000000015Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG088(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG088(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000160ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG088(block_id) (0x0000030000000160ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG089(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG089(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000164ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG089(block_id) (0x0000030000000164ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG090(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG090(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000168ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG090(block_id) (0x0000030000000168ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG091(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG091(%lu) is invalid on this chip\n", block_id);
	return 0x000003000000016Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG091(block_id) (0x000003000000016Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG092(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG092(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000170ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG092(block_id) (0x0000030000000170ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG094(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG094(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000178ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG094(block_id) (0x0000030000000178ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG095(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG095(%lu) is invalid on this chip\n", block_id);
	return 0x000003000000017Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG095(block_id) (0x000003000000017Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG096(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG096(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000180ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG096(block_id) (0x0000030000000180ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG097(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG097(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000184ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG097(block_id) (0x0000030000000184ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG098(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG098(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000188ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG098(block_id) (0x0000030000000188ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG099(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG099(%lu) is invalid on this chip\n", block_id);
	return 0x000003000000018Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG099(block_id) (0x000003000000018Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG100(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG100(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000190ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG100(block_id) (0x0000030000000190ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG101(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG101(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000194ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG101(block_id) (0x0000030000000194ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG102(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG102(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000198ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG102(block_id) (0x0000030000000198ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG103(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG103(%lu) is invalid on this chip\n", block_id);
	return 0x000003000000019Cull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG103(block_id) (0x000003000000019Cull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG104(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG104(%lu) is invalid on this chip\n", block_id);
	return 0x00000300000001A0ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG104(block_id) (0x00000300000001A0ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG105(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG105(%lu) is invalid on this chip\n", block_id);
	return 0x00000300000001A4ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG105(block_id) (0x00000300000001A4ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG106(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG106(%lu) is invalid on this chip\n", block_id);
	return 0x00000300000001A8ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG106(block_id) (0x00000300000001A8ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG107(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG107(%lu) is invalid on this chip\n", block_id);
	return 0x00000300000001ACull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG107(block_id) (0x00000300000001ACull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG108(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG108(%lu) is invalid on this chip\n", block_id);
	return 0x00000300000001B0ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG108(block_id) (0x00000300000001B0ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG109(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG109(%lu) is invalid on this chip\n", block_id);
	return 0x00000300000001B4ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG109(block_id) (0x00000300000001B4ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG110(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG110(%lu) is invalid on this chip\n", block_id);
	return 0x00000300000001B8ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG110(block_id) (0x00000300000001B8ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG111(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG111(%lu) is invalid on this chip\n", block_id);
	return 0x00000300000001BCull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG111(block_id) (0x00000300000001BCull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG112(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG112(%lu) is invalid on this chip\n", block_id);
	return 0x00000300000001C0ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG112(block_id) (0x00000300000001C0ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG448(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000700ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000700ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000700ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000700ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG448 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000700ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG448(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000700ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000700ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000700ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000700ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000700ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG449(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000704ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000704ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000704ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000704ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG449 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000704ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG449(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000704ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000704ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000704ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000704ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000704ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG450(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000708ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000708ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000708ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000708ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG450 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000708ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG450(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000708ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000708ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000708ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000708ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000708ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG451(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000000000070Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x000000000000070Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003000000070Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003000000070Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG451 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000000000070Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG451(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000070Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000070Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000070Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003000000070Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000000000070Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG452(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000710ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000710ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000710ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000710ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG452 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000710ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG452(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000710ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000710ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000710ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000710ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000710ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG453(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000714ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000714ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000714ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000714ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG453 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000714ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG453(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000714ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000714ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000714ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000714ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000714ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG454(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000718ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000718ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000718ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000718ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG454 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000718ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG454(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000718ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000718ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000718ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000718ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000718ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG455(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000000000071Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x000000000000071Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003000000071Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003000000071Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG455 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000000000071Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG455(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000071Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000071Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000071Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003000000071Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000000000071Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG456(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000720ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000720ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000720ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000720ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG456 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000720ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG456(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000720ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000720ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000720ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000720ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000720ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG458(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000728ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000728ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000728ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000728ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG458 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000728ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG458(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000728ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000728ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000728ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000728ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000728ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG459(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000000000072Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x000000000000072Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003000000072Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003000000072Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG459 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000000000072Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG459(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000072Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000072Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000072Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003000000072Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000000000072Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG460(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000730ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000730ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000730ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000730ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG460 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000730ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG460(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000730ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000730ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000730ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000730ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000730ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG461(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000734ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000734ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000734ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000734ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG461 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000734ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG461(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000734ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000734ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000734ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000734ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000734ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG462(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000738ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000738ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000738ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000738ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG462 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000738ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG462(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000738ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000738ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000738ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000738ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000738ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG463(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000000000073Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x000000000000073Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003000000073Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003000000073Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG463 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000000000073Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG463(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000073Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000073Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000073Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003000000073Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000000000073Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG464(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000740ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000740ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000740ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000740ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG464 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000740ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG464(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000740ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000740ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000740ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000740ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000740ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG465(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000744ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000744ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000744ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000744ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG465 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000744ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG465(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000744ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000744ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000744ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000744ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000744ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG466(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000748ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000748ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000748ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000748ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG466 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000748ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG466(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000748ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000748ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000748ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000748ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000748ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG467(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000000000074Cull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x000000000000074Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003000000074Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003000000074Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG467 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000000000074Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG467(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000074Cull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x000000000000074Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000074Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003000000074Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000000000074Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG468(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000750ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000750ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000750ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000750ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG468 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000750ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG468(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000750ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000750ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000750ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000750ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000750ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG490(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x00000000000007A8ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x00000000000007A8ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x00000300000007A8ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG490 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x00000000000007A8ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG490(unsigned long block_id)
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
			return 0x00000300000007A8ull + (block_id) * 0x100000000ull;
	}
	return 0x00000000000007A8ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG491(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x00000000000007ACull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x00000000000007ACull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x00000300000007ACull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG491 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x00000000000007ACull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG491(unsigned long block_id)
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
			return 0x00000300000007ACull + (block_id) * 0x100000000ull;
	}
	return 0x00000000000007ACull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG492(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x00000000000007B0ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x00000000000007B0ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x00000300000007B0ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG492 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x00000000000007B0ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG492(unsigned long block_id)
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
			return 0x00000300000007B0ull + (block_id) * 0x100000000ull;
	}
	return 0x00000000000007B0ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG515(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x000000000000080Cull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x000003000000080Cull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x000003000000080Cull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG515 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x000000000000080Cull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG515(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x000000000000080Cull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x000003000000080Cull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x000003000000080Cull + (block_id) * 0x100000000ull;
	}
	return 0x000000000000080Cull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG516(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000810ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000810ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000810ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000810ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG516 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000810ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG516(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000810ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000810ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000810ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000810ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000810ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG517(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return 0x0000000000000814ull;
			break;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			if ((block_id == 0))
				return 0x0000000000000814ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return 0x0000030000000814ull + ((block_id) & 3) * 0x100000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return 0x0000030000000814ull + ((block_id) & 3) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_PCIEEPX_CFG517 (block_id = %lu) not supported on this chip\n", block_id);
	return 0x0000000000000814ull;
}
#else
static inline uint64_t CVMX_PCIEEPX_CFG517(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000814ull;
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000814ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000814ull + (block_id) * 0x100000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return 0x0000030000000814ull + (block_id) * 0x100000000ull;
	}
	return 0x0000000000000814ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG548(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG548(%lu) is invalid on this chip\n", block_id);
	return 0x0000030000000890ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG548(block_id) (0x0000030000000890ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG554(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG554(%lu) is invalid on this chip\n", block_id);
	return 0x00000300000008A8ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG554(block_id) (0x00000300000008A8ull + ((block_id) & 3) * 0x100000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PCIEEPX_CFG558(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PCIEEPX_CFG558(%lu) is invalid on this chip\n", block_id);
	return 0x00000300000008B8ull + ((block_id) & 3) * 0x100000000ull;
}
#else
#define CVMX_PCIEEPX_CFG558(block_id) (0x00000300000008B8ull + ((block_id) & 3) * 0x100000000ull)
#endif

/**
 * cvmx_pcieep#_cfg000
 *
 * PCIE_CFG000 = First 32-bits of PCIE type 0 config space (Device ID and Vendor ID Register)
 *
 */
union cvmx_pcieepx_cfg000 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg000_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t devid                        : 16; /**< Device ID, writable through PEM(0..1)_CFG_WR
                                                          However, the application must not change this field.
                                                         For EEPROM loads also see VENDID of this register. */
	uint32_t vendid                       : 16; /**< Vendor ID, writable through PEM(0..1)_CFG_WR
                                                          However, the application must not change this field.
                                                         During and EPROM Load is a value of 0xFFFF is loaded to this
                                                         field and a value of 0xFFFF is loaded to the DEVID field of
                                                         this register, the value will not be loaded, EEPROM load will
                                                         stop, and the FastLinkEnable bit will be set in the
                                                         PCIE_CFG452 register. */
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
	struct cvmx_pcieepx_cfg000_s          cn78xx;
	struct cvmx_pcieepx_cfg000_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg000 cvmx_pcieepx_cfg000_t;

/**
 * cvmx_pcieep#_cfg001
 *
 * PCIE_CFG001 = Second 32-bits of PCIE type 0 config space (Command/Status Register)
 *
 */
union cvmx_pcieepx_cfg001 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg001_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dpe                          : 1;  /**< Detected Parity Error */
	uint32_t sse                          : 1;  /**< Signaled System Error */
	uint32_t rma                          : 1;  /**< Received Master Abort */
	uint32_t rta                          : 1;  /**< Received Target Abort */
	uint32_t sta                          : 1;  /**< Signaled Target Abort */
	uint32_t devt                         : 2;  /**< DEVSEL Timing
                                                         Not applicable for PCI Express. Hardwired to 0. */
	uint32_t mdpe                         : 1;  /**< Master Data Parity Error */
	uint32_t fbb                          : 1;  /**< Fast Back-to-Back Capable
                                                         Not applicable for PCI Express. Hardwired to 0. */
	uint32_t reserved_22_22               : 1;
	uint32_t m66                          : 1;  /**< 66 MHz Capable
                                                         Not applicable for PCI Express. Hardwired to 0. */
	uint32_t cl                           : 1;  /**< Capabilities List
                                                         Indicates presence of an extended capability item.
                                                         Hardwired to 1. */
	uint32_t i_stat                       : 1;  /**< INTx Status */
	uint32_t reserved_11_18               : 8;
	uint32_t i_dis                        : 1;  /**< INTx Assertion Disable */
	uint32_t fbbe                         : 1;  /**< Fast Back-to-Back Enable
                                                         Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t see                          : 1;  /**< SERR# Enable */
	uint32_t ids_wcc                      : 1;  /**< IDSEL Stepping/Wait Cycle Control
                                                         Not applicable for PCI Express. Must be hardwired to 0 */
	uint32_t per                          : 1;  /**< Parity Error Response */
	uint32_t vps                          : 1;  /**< VGA Palette Snoop
                                                         Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t mwice                        : 1;  /**< Memory Write and Invalidate
                                                         Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t scse                         : 1;  /**< Special Cycle Enable
                                                         Not applicable for PCI Express. Must be hardwired to 0. */
	uint32_t me                           : 1;  /**< Bus Master Enable */
	uint32_t msae                         : 1;  /**< Memory Space Enable */
	uint32_t isae                         : 1;  /**< I/O Space Enable */
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
	struct cvmx_pcieepx_cfg001_s          cn78xx;
	struct cvmx_pcieepx_cfg001_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg001 cvmx_pcieepx_cfg001_t;

/**
 * cvmx_pcieep#_cfg002
 *
 * PCIE_CFG002 = Third 32-bits of PCIE type 0 config space (Revision ID/Class Code Register)
 *
 */
union cvmx_pcieepx_cfg002 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg002_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t bcc                          : 8;  /**< Base Class Code, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t sc                           : 8;  /**< Subclass Code, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t pi                           : 8;  /**< Programming Interface, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t rid                          : 8;  /**< Revision ID, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
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
	struct cvmx_pcieepx_cfg002_s          cn78xx;
	struct cvmx_pcieepx_cfg002_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg002 cvmx_pcieepx_cfg002_t;

/**
 * cvmx_pcieep#_cfg003
 *
 * PCIE_CFG003 = Fourth 32-bits of PCIE type 0 config space (Cache Line Size/Master Latency
 * Timer/Header Type Register/BIST Register)
 */
union cvmx_pcieepx_cfg003 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg003_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t bist                         : 8;  /**< The BIST register functions are not supported.
                                                         All 8 bits of the BIST register are hardwired to 0. */
	uint32_t mfd                          : 1;  /**< Multi Function Device
                                                         The Multi Function Device bit is writable through PEM(0..1)_CFG_WR.
                                                         However, this is a single function device. Therefore, the
                                                         application must not write a 1 to this bit. */
	uint32_t chf                          : 7;  /**< Configuration Header Format
                                                         Hardwired to 0 for type 0. */
	uint32_t lt                           : 8;  /**< Master Latency Timer
                                                         Not applicable for PCI Express, hardwired to 0. */
	uint32_t cls                          : 8;  /**< Cache Line Size
                                                         The Cache Line Size register is RW for legacy compatibility
                                                         purposes and is not applicable to PCI Express device
                                                         functionality.
                                                         Writing to the Cache Line Size register does not impact
                                                         functionality. */
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
	struct cvmx_pcieepx_cfg003_s          cn78xx;
	struct cvmx_pcieepx_cfg003_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg003 cvmx_pcieepx_cfg003_t;

/**
 * cvmx_pcieep#_cfg004
 *
 * PCIE_CFG004 = Fifth 32-bits of PCIE type 0 config space (Base Address Register 0 - Low)
 *
 */
union cvmx_pcieepx_cfg004 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg004_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lbab                         : 18; /**< Lower bits of the BAR 0 base address */
	uint32_t reserved_4_13                : 10;
	uint32_t pf                           : 1;  /**< Prefetchable
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t typ                          : 2;  /**< BAR type
                                                            o 00 = 32-bit BAR
                                                            o 10 = 64-bit BAR
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t mspc                         : 1;  /**< Memory Space Indicator
                                                            o 0 = BAR 0 is a memory BAR
                                                            o 1 = BAR 0 is an I/O BAR
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
#else
	uint32_t mspc                         : 1;
	uint32_t typ                          : 2;
	uint32_t pf                           : 1;
	uint32_t reserved_4_13                : 10;
	uint32_t lbab                         : 18;
#endif
	} s;
	struct cvmx_pcieepx_cfg004_s          cn52xx;
	struct cvmx_pcieepx_cfg004_s          cn52xxp1;
	struct cvmx_pcieepx_cfg004_s          cn56xx;
	struct cvmx_pcieepx_cfg004_s          cn56xxp1;
	struct cvmx_pcieepx_cfg004_s          cn61xx;
	struct cvmx_pcieepx_cfg004_s          cn63xx;
	struct cvmx_pcieepx_cfg004_s          cn63xxp1;
	struct cvmx_pcieepx_cfg004_s          cn66xx;
	struct cvmx_pcieepx_cfg004_s          cn68xx;
	struct cvmx_pcieepx_cfg004_s          cn68xxp1;
	struct cvmx_pcieepx_cfg004_s          cn70xx;
	struct cvmx_pcieepx_cfg004_s          cn78xx;
	struct cvmx_pcieepx_cfg004_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg004 cvmx_pcieepx_cfg004_t;

/**
 * cvmx_pcieep#_cfg004_mask
 *
 * "PCIE_CFG004_MASK (BAR Mask 0 - Low)
 * The BAR 0 Mask register is invisible to host software and not readable from the application.
 * The BAR 0 Mask register is only writable through PEM#_CFG_WR."
 */
union cvmx_pcieepx_cfg004_mask {
	uint32_t u32;
	struct cvmx_pcieepx_cfg004_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lmask                        : 31; /**< Bar Mask Low */
	uint32_t enb                          : 1;  /**< Bar Enable
                                                         o 0: BAR 0 is disabled
                                                         o 1: BAR 0 is enabled
                                                         Bit 0 is interpreted as BAR Enable when writing to the BAR Mask
                                                         register rather than as a mask bit because bit 0 of a BAR is
                                                         always masked from writing by host software. Bit 0 must be
                                                         written prior to writing the other mask bits. */
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
	struct cvmx_pcieepx_cfg004_mask_s     cn78xx;
	struct cvmx_pcieepx_cfg004_mask_s     cnf71xx;
};
typedef union cvmx_pcieepx_cfg004_mask cvmx_pcieepx_cfg004_mask_t;

/**
 * cvmx_pcieep#_cfg005
 *
 * PCIE_CFG005 = Sixth 32-bits of PCIE type 0 config space (Base Address Register 0 - High)
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
	struct cvmx_pcieepx_cfg005_s          cn78xx;
	struct cvmx_pcieepx_cfg005_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg005 cvmx_pcieepx_cfg005_t;

/**
 * cvmx_pcieep#_cfg005_mask
 *
 * "PCIE_CFG005_MASK = (BAR Mask 0 - High)
 * The BAR 0 Mask register is invisible to host software and not readable from the application.
 * The BAR 0 Mask register is only writable through PEM#_CFG_WR."
 */
union cvmx_pcieepx_cfg005_mask {
	uint32_t u32;
	struct cvmx_pcieepx_cfg005_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t umask                        : 32; /**< Bar Mask High */
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
	struct cvmx_pcieepx_cfg005_mask_s     cn78xx;
	struct cvmx_pcieepx_cfg005_mask_s     cnf71xx;
};
typedef union cvmx_pcieepx_cfg005_mask cvmx_pcieepx_cfg005_mask_t;

/**
 * cvmx_pcieep#_cfg006
 *
 * PCIE_CFG006 = Seventh 32-bits of PCIE type 0 config space (Base Address Register 1 - Low)
 *
 */
union cvmx_pcieepx_cfg006 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg006_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lbab                         : 6;  /**< Lower bits of the BAR 1 base address */
	uint32_t reserved_4_25                : 22;
	uint32_t pf                           : 1;  /**< Prefetchable
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t typ                          : 2;  /**< BAR type
                                                            o 00 = 32-bit BAR
                                                            o 10 = 64-bit BAR
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t mspc                         : 1;  /**< Memory Space Indicator
                                                            o 0 = BAR 1 is a memory BAR
                                                            o 1 = BAR 1 is an I/O BAR
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
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
	struct cvmx_pcieepx_cfg006_s          cn78xx;
	struct cvmx_pcieepx_cfg006_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg006 cvmx_pcieepx_cfg006_t;

/**
 * cvmx_pcieep#_cfg006_mask
 *
 * "PCIE_CFG006_MASK (BAR Mask 1 - Low)
 * The BAR 1 Mask register is invisible to host software and not readable from the application.
 * The BAR 1 Mask register is only writable through PEM#_CFG_WR."
 */
union cvmx_pcieepx_cfg006_mask {
	uint32_t u32;
	struct cvmx_pcieepx_cfg006_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lmask                        : 31; /**< Bar Mask Low */
	uint32_t enb                          : 1;  /**< Bar Enable
                                                         o 0: BAR 1 is disabled
                                                         o 1: BAR 1 is enabled
                                                         Bit 0 is interpreted as BAR Enable when writing to the BAR Mask
                                                         register rather than as a mask bit because bit 0 of a BAR is
                                                         always masked from writing by host software. Bit 0 must be
                                                         written prior to writing the other mask bits. */
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
	struct cvmx_pcieepx_cfg006_mask_s     cn78xx;
	struct cvmx_pcieepx_cfg006_mask_s     cnf71xx;
};
typedef union cvmx_pcieepx_cfg006_mask cvmx_pcieepx_cfg006_mask_t;

/**
 * cvmx_pcieep#_cfg007
 *
 * PCIE_CFG007 = Eighth 32-bits of PCIE type 0 config space (Base Address Register 1 - High)
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
	struct cvmx_pcieepx_cfg007_s          cn78xx;
	struct cvmx_pcieepx_cfg007_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg007 cvmx_pcieepx_cfg007_t;

/**
 * cvmx_pcieep#_cfg007_mask
 *
 * "PCIE_CFG007_MASK (BAR Mask 1 - High)
 * The BAR 1 Mask register is invisible to host software and not readable from the application.
 * The BAR 1 Mask register is only writable through PEM#_CFG_WR."
 */
union cvmx_pcieepx_cfg007_mask {
	uint32_t u32;
	struct cvmx_pcieepx_cfg007_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t umask                        : 32; /**< Bar Mask High */
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
	struct cvmx_pcieepx_cfg007_mask_s     cn78xx;
	struct cvmx_pcieepx_cfg007_mask_s     cnf71xx;
};
typedef union cvmx_pcieepx_cfg007_mask cvmx_pcieepx_cfg007_mask_t;

/**
 * cvmx_pcieep#_cfg008
 *
 * PCIE_CFG008 = Ninth 32-bits of PCIE type 0 config space (Base Address Register 2 - Low)
 *
 */
union cvmx_pcieepx_cfg008 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg008_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lbab                         : 12; /**< Lower bits of the BAR 2 base address */
	uint32_t reserved_4_19                : 16;
	uint32_t pf                           : 1;  /**< Prefetchable
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t typ                          : 2;  /**< BAR type
                                                            o 00 = 32-bit BAR
                                                            o 10 = 64-bit BAR
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t mspc                         : 1;  /**< Memory Space Indicator
                                                            o 0 = BAR 2 is a memory BAR
                                                            o 1 = BAR 2 is an I/O BAR
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
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
	struct cvmx_pcieepx_cfg008_s          cn78xx;
	struct cvmx_pcieepx_cfg008_cn52xx     cnf71xx;
};
typedef union cvmx_pcieepx_cfg008 cvmx_pcieepx_cfg008_t;

/**
 * cvmx_pcieep#_cfg008_mask
 *
 * "PCIE_CFG008_MASK (BAR Mask 2 - Low)
 * The BAR 2 Mask register is invisible to host software and not readable from the application.
 * The BAR 2 Mask register is only writable through PEM#_CFG_WR."
 */
union cvmx_pcieepx_cfg008_mask {
	uint32_t u32;
	struct cvmx_pcieepx_cfg008_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lmask                        : 31; /**< Bar Mask Low */
	uint32_t enb                          : 1;  /**< Bar Enable
                                                         o 0: BAR 2 is disabled
                                                         o 1: BAR 2 is enabled
                                                         Bit 0 is interpreted as BAR Enable when writing to the BAR Mask
                                                         register rather than as a mask bit because bit 0 of a BAR is
                                                         always masked from writing by host software. Bit 0 must be
                                                         written prior to writing the other mask bits. */
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
	struct cvmx_pcieepx_cfg008_mask_s     cn78xx;
	struct cvmx_pcieepx_cfg008_mask_s     cnf71xx;
};
typedef union cvmx_pcieepx_cfg008_mask cvmx_pcieepx_cfg008_mask_t;

/**
 * cvmx_pcieep#_cfg009
 *
 * PCIE_CFG009 = Tenth 32-bits of PCIE type 0 config space (Base Address Register 2 - High)
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
	struct cvmx_pcieepx_cfg009_cn70xx     cn78xx;
	struct cvmx_pcieepx_cfg009_cn61xx     cnf71xx;
};
typedef union cvmx_pcieepx_cfg009 cvmx_pcieepx_cfg009_t;

/**
 * cvmx_pcieep#_cfg009_mask
 *
 * "PCIE_CFG009_MASK (BAR Mask 2 - High)
 * The BAR 2 Mask register is invisible to host software and not readable from the application.
 * The BAR 2 Mask register is only writable through PEM#_CFG_WR."
 */
union cvmx_pcieepx_cfg009_mask {
	uint32_t u32;
	struct cvmx_pcieepx_cfg009_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t umask                        : 32; /**< Bar Mask High */
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
	struct cvmx_pcieepx_cfg009_mask_s     cn78xx;
	struct cvmx_pcieepx_cfg009_mask_s     cnf71xx;
};
typedef union cvmx_pcieepx_cfg009_mask cvmx_pcieepx_cfg009_mask_t;

/**
 * cvmx_pcieep#_cfg010
 *
 * PCIE_CFG010 = Eleventh 32-bits of PCIE type 0 config space (CardBus CIS Pointer Register)
 *
 */
union cvmx_pcieepx_cfg010 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg010_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t cisp                         : 32; /**< CardBus CIS Pointer
                                                         Optional, writable through PEM(0..1)_CFG_WR. */
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
	struct cvmx_pcieepx_cfg010_s          cn78xx;
	struct cvmx_pcieepx_cfg010_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg010 cvmx_pcieepx_cfg010_t;

/**
 * cvmx_pcieep#_cfg011
 *
 * PCIE_CFG011 = Twelfth 32-bits of PCIE type 0 config space (Subsystem ID and Subsystem Vendor
 * ID Register)
 */
union cvmx_pcieepx_cfg011 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg011_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ssid                         : 16; /**< Subsystem ID
                                                         Assigned by PCI-SIG, writable through PEM(0..1)_CFG_WR.                                                                                                           However, the application must not change this field. */
	uint32_t ssvid                        : 16; /**< Subsystem Vendor ID
                                                         Assigned by PCI-SIG, writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
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
	struct cvmx_pcieepx_cfg011_s          cn78xx;
	struct cvmx_pcieepx_cfg011_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg011 cvmx_pcieepx_cfg011_t;

/**
 * cvmx_pcieep#_cfg012
 *
 * PCIE_CFG012 = Thirteenth 32-bits of PCIE type 0 config space (Expansion ROM Base Address Register)
 *
 */
union cvmx_pcieepx_cfg012 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg012_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t eraddr                       : 16; /**< Expansion ROM Address */
	uint32_t reserved_1_15                : 15;
	uint32_t er_en                        : 1;  /**< Expansion ROM Enable */
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
	struct cvmx_pcieepx_cfg012_s          cn78xx;
	struct cvmx_pcieepx_cfg012_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg012 cvmx_pcieepx_cfg012_t;

/**
 * cvmx_pcieep#_cfg012_mask
 *
 * "PCIE_CFG012_MASK (Exapansion ROM BAR Mask)
 * The ROM Mask register is invisible to host software and not readable from the application.
 * The ROM Mask register is only writable through PEM#_CFG_WR."
 */
union cvmx_pcieepx_cfg012_mask {
	uint32_t u32;
	struct cvmx_pcieepx_cfg012_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t mask                         : 31; /**< Bar Mask Low                                                                 NS */
	uint32_t enb                          : 1;  /**< Bar Enable                                                                   NS
                                                         o 0: BAR ROM is disabled
                                                         o 1: BAR ROM is enabled
                                                         Bit 0 is interpreted as BAR Enable when writing to the BAR Mask
                                                         register rather than as a mask bit because bit 0 of a BAR is
                                                         always masked from writing by host software. Bit 0 must be
                                                         written prior to writing the other mask bits. */
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
	struct cvmx_pcieepx_cfg012_mask_s     cn78xx;
	struct cvmx_pcieepx_cfg012_mask_s     cnf71xx;
};
typedef union cvmx_pcieepx_cfg012_mask cvmx_pcieepx_cfg012_mask_t;

/**
 * cvmx_pcieep#_cfg013
 *
 * PCIE_CFG013 = Fourteenth 32-bits of PCIE type 0 config space (Capability Pointer Register)
 *
 */
union cvmx_pcieepx_cfg013 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg013_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_8_31                : 24;
	uint32_t cp                           : 8;  /**< First Capability Pointer.
                                                         Points to Power Management Capability structure by
                                                         default, writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
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
	struct cvmx_pcieepx_cfg013_s          cn78xx;
	struct cvmx_pcieepx_cfg013_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg013 cvmx_pcieepx_cfg013_t;

/**
 * cvmx_pcieep#_cfg015
 *
 * PCIE_CFG015 = Sixteenth 32-bits of PCIE type 0 config space (Interrupt Line Register/Interrupt
 * Pin/Bridge Control Register)
 */
union cvmx_pcieepx_cfg015 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg015_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ml                           : 8;  /**< Maximum Latency     (Hardwired to 0) */
	uint32_t mg                           : 8;  /**< Minimum Grant       (Hardwired to 0) */
	uint32_t inta                         : 8;  /**< Interrupt Pin
                                                         Identifies the legacy interrupt Message that the device
                                                         (or device function) uses.
                                                         The Interrupt Pin register is writable through PEM(0..1)_CFG_WR.
                                                         In a single-function configuration, only INTA is used.
                                                         Therefore, the application must not change this field. */
	uint32_t il                           : 8;  /**< Interrupt Line */
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
	struct cvmx_pcieepx_cfg015_s          cn78xx;
	struct cvmx_pcieepx_cfg015_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg015 cvmx_pcieepx_cfg015_t;

/**
 * cvmx_pcieep#_cfg016
 *
 * PCIE_CFG016 = Seventeenth 32-bits of PCIE type 0 config space
 * (Power Management Capability ID/
 * Power Management Next Item Pointer/
 * Power Management Capabilities Register)
 */
union cvmx_pcieepx_cfg016 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg016_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pmes                         : 5;  /**< PME_Support
                                                         o Bit 11: If set, PME Messages can be generated from D0
                                                         o Bit 12: If set, PME Messages can be generated from D1
                                                         o Bit 13: If set, PME Messages can be generated from D2
                                                         o Bit 14: If set, PME Messages can be generated from D3hot
                                                         o Bit 15: If set, PME Messages can be generated from D3cold
                                                         The PME_Support field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t d2s                          : 1;  /**< D2 Support, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t d1s                          : 1;  /**< D1 Support, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t auxc                         : 3;  /**< AUX Current, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t dsi                          : 1;  /**< Device Specific Initialization (DSI), writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t reserved_20_20               : 1;
	uint32_t pme_clock                    : 1;  /**< PME Clock, hardwired to 0 */
	uint32_t pmsv                         : 3;  /**< Power Management Specification Version, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t ncp                          : 8;  /**< Next Capability Pointer
                                                         Points to the MSI capabilities by default, writable
                                                         through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t pmcid                        : 8;  /**< Power Management Capability ID */
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
	struct cvmx_pcieepx_cfg016_s          cn78xx;
	struct cvmx_pcieepx_cfg016_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg016 cvmx_pcieepx_cfg016_t;

/**
 * cvmx_pcieep#_cfg017
 *
 * PCIE_CFG017 = Eighteenth 32-bits of PCIE type 0 config space (Power Management Control and
 * Status Register)
 */
union cvmx_pcieepx_cfg017 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg017_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pmdia                        : 8;  /**< Data register for additional information (not supported) */
	uint32_t bpccee                       : 1;  /**< Bus Power/Clock Control Enable, hardwired to 0 */
	uint32_t bd3h                         : 1;  /**< B2/B3 Support, hardwired to 0 */
	uint32_t reserved_16_21               : 6;
	uint32_t pmess                        : 1;  /**< PME Status
                                                         Indicates if a previously enabled PME event occurred or not. */
	uint32_t pmedsia                      : 2;  /**< Data Scale (not supported) */
	uint32_t pmds                         : 4;  /**< Data Select (not supported) */
	uint32_t pmeens                       : 1;  /**< PME Enable
                                                         A value of 1 indicates that the device is enabled to
                                                         generate PME. */
	uint32_t reserved_4_7                 : 4;
	uint32_t nsr                          : 1;  /**< No Soft Reset, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t reserved_2_2                 : 1;
	uint32_t ps                           : 2;  /**< Power State
                                                         Controls the device power state:
                                                           o 00b: D0
                                                           o 01b: D1
                                                           o 10b: D2
                                                           o 11b: D3
                                                         The written value is ignored if the specific state is
                                                         not supported. */
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
	struct cvmx_pcieepx_cfg017_s          cn78xx;
	struct cvmx_pcieepx_cfg017_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg017 cvmx_pcieepx_cfg017_t;

/**
 * cvmx_pcieep#_cfg020
 *
 * PCIE_CFG020 = Twenty-first 32-bits of PCIE type 0 config space
 * (MSI Capability ID/
 * MSI Next Item Pointer/
 * MSI Control Register)
 */
union cvmx_pcieepx_cfg020 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg020_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_25_31               : 7;
	uint32_t pvm                          : 1;  /**< Per-vector masking capable */
	uint32_t m64                          : 1;  /**< 64-bit Address Capable, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t mme                          : 3;  /**< Multiple Message Enabled
                                                         Indicates that multiple Message mode is enabled by system
                                                         software. The number of Messages enabled must be less than
                                                         or equal to the Multiple Message Capable value. */
	uint32_t mmc                          : 3;  /**< Multiple Message Capable, writable through PEM(0..1)_CFG_WR
                                                         However, the application must not change this field. */
	uint32_t msien                        : 1;  /**< MSI Enabled
                                                         When set, INTx must be disabled. */
	uint32_t ncp                          : 8;  /**< Next Capability Pointer
                                                         Points to PCI Express Capabilities by default,
                                                         writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t msicid                       : 8;  /**< MSI Capability ID */
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
	struct cvmx_pcieepx_cfg020_s          cn78xx;
	struct cvmx_pcieepx_cfg020_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg020 cvmx_pcieepx_cfg020_t;

/**
 * cvmx_pcieep#_cfg021
 *
 * PCIE_CFG021 = Twenty-second 32-bits of PCIE type 0 config space (MSI Lower 32 Bits Address
 * Register)
 */
union cvmx_pcieepx_cfg021 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg021_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lmsi                         : 30; /**< Lower 32-bit Address */
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
	struct cvmx_pcieepx_cfg021_s          cn78xx;
	struct cvmx_pcieepx_cfg021_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg021 cvmx_pcieepx_cfg021_t;

/**
 * cvmx_pcieep#_cfg022
 *
 * PCIE_CFG022 = Twenty-third 32-bits of PCIE type 0 config space (MSI Upper 32 bits Address Register)
 *
 */
union cvmx_pcieepx_cfg022 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg022_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t umsi                         : 32; /**< Upper 32-bit Address */
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
	struct cvmx_pcieepx_cfg022_s          cn78xx;
	struct cvmx_pcieepx_cfg022_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg022 cvmx_pcieepx_cfg022_t;

/**
 * cvmx_pcieep#_cfg023
 *
 * PCIE_CFG023 = Twenty-fourth 32-bits of PCIE type 0 config space (MSI Data Register)
 *
 */
union cvmx_pcieepx_cfg023 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg023_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t msimd                        : 16; /**< MSI Data
                                                         Pattern assigned by system software, bits [4:0] are Or-ed with
                                                         MSI_VECTOR to generate 32 MSI Messages per function. */
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
	struct cvmx_pcieepx_cfg023_s          cn78xx;
	struct cvmx_pcieepx_cfg023_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg023 cvmx_pcieepx_cfg023_t;

/**
 * cvmx_pcieep#_cfg024
 *
 * PCIE_CFG024 = Twenty-fifth 32-bits of PCIE type 0 config space (MSI Mask Bits Register)
 *
 */
union cvmx_pcieepx_cfg024 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg024_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msimm                        : 32; /**< MSI
                                                         For each mask bit that is set, the function is prohibited from
                                                         sending the associated message. */
#else
	uint32_t msimm                        : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg024_s          cn70xx;
	struct cvmx_pcieepx_cfg024_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg024 cvmx_pcieepx_cfg024_t;

/**
 * cvmx_pcieep#_cfg025
 *
 * PCIE_CFG025 = Twenty-sixth 32-bits of PCIE type 0 config space (MSI Pending Bits Register)
 *
 */
union cvmx_pcieepx_cfg025 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg025_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msimp                        : 32; /**< MSI
                                                         For each pending bit that is set, the function has a pending
                                                         associated message. */
#else
	uint32_t msimp                        : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg025_s          cn70xx;
	struct cvmx_pcieepx_cfg025_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg025 cvmx_pcieepx_cfg025_t;

/**
 * cvmx_pcieep#_cfg028
 *
 * PCIE_CFG028 = Twenty-ninth 32-bits of PCIE type 0 config space
 * (PCI Express Capabilities List Register/
 * PCI Express Capabilities Register)
 */
union cvmx_pcieepx_cfg028 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg028_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t imn                          : 5;  /**< Interrupt Message Number
                                                         Updated by hardware, writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t si                           : 1;  /**< Slot Implemented
                                                         This bit is writable through PEM(0..1)_CFG_WR.
                                                         However, it must be 0 for
                                                         an Endpoint device. Therefore, the application must not write a
                                                         1 to this bit. */
	uint32_t dpt                          : 4;  /**< Device Port Type */
	uint32_t pciecv                       : 4;  /**< PCI Express Capability Version */
	uint32_t ncp                          : 8;  /**< Next Capability Pointer
                                                         writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t pcieid                       : 8;  /**< PCIE Capability ID */
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
	struct cvmx_pcieepx_cfg028_s          cn78xx;
	struct cvmx_pcieepx_cfg028_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg028 cvmx_pcieepx_cfg028_t;

/**
 * cvmx_pcieep#_cfg029
 *
 * PCIE_CFG029 = Thirtieth 32-bits of PCIE type 0 config space (Device Capabilities Register)
 *
 */
union cvmx_pcieepx_cfg029 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg029_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_28_31               : 4;
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
	struct cvmx_pcieepx_cfg029_cn61xx     cn78xx;
	struct cvmx_pcieepx_cfg029_cn61xx     cnf71xx;
};
typedef union cvmx_pcieepx_cfg029 cvmx_pcieepx_cfg029_t;

/**
 * cvmx_pcieep#_cfg030
 *
 * PCIE_CFG030 = Thirty-first 32-bits of PCIE type 0 config space
 * (Device Control Register/Device Status Register)
 */
union cvmx_pcieepx_cfg030 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg030_s {
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
                                                         Nonfatal criteria , which
                                                         most poisoned TLP's should be. */
	uint32_t ce_d                         : 1;  /**< Correctable Error Detected
                                                          Errors are logged in this register regardless of whether
                                                          error reporting is enabled in the Device Control register.
                                                         CE_D is set if we receive any of the errors in PCIE_CFG068
                                                         for example a Replay Timer Timeout.  Also, it can be set if
                                                         we get any of the errors in PCIE_CFG066 that has a severity
                                                         set to Nonfatal and meets the Advisory Nonfatal criteria,
                                                         which most ECRC errors
                                                         should be. */
	uint32_t i_flr                        : 1;  /**< Initiate Function Level Reset
                                                         (Not Supported) */
	uint32_t mrrs                         : 3;  /**< Max Read Request Size
                                                          0 = 128B
                                                          1 = 256B
                                                          2 = 512B
                                                          3 = 1024B
                                                          4 = 2048B
                                                          5 = 4096B
                                                         Note: SLI_S2M_PORT#_CTL[MRRS] and DPI_SLI_PRT#_CFG[MRRS] and
                                                               also must be set properly.
                                                               SLI_S2M_PORT#_CTL[MRRS] and DPI_SLI_PRT#_CFG[MRRS] must
                                                               not exceed the desired max read request size. */
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
                                                         Note: DPI_SLI_PRT#_CFG[MPS] must be set to the same
                                                               value for proper functionality. */
	uint32_t ro_en                        : 1;  /**< Enable Relaxed Ordering
                                                         This bit is not used. */
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
	struct cvmx_pcieepx_cfg030_s          cn78xx;
	struct cvmx_pcieepx_cfg030_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg030 cvmx_pcieepx_cfg030_t;

/**
 * cvmx_pcieep#_cfg031
 *
 * PCIE_CFG031 = Thirty-second 32-bits of PCIE type 0 config space
 * (Link Capabilities Register)
 */
union cvmx_pcieepx_cfg031 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg031_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pnum                         : 8;  /**< Port Number
                                                         writable through PEM(0..1)_CFG_WR, however the application
                                                         must not change this field. */
	uint32_t reserved_23_23               : 1;
	uint32_t aspm                         : 1;  /**< ASPM Optionality Compliance */
	uint32_t lbnc                         : 1;  /**< Link Bandwidth Notification Capability
                                                         Set 0 for Endpoint devices. */
	uint32_t dllarc                       : 1;  /**< Data Link Layer Active Reporting Capable */
	uint32_t sderc                        : 1;  /**< Surprise Down Error Reporting Capable
                                                         Not supported, hardwired to 0x0. */
	uint32_t cpm                          : 1;  /**< Clock Power Management
                                                         The default value is the value you specify during hardware
                                                         configuration, writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t l1el                         : 3;  /**< L1 Exit Latency
                                                         The default value is the value you specify during hardware
                                                         configuration, writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t l0el                         : 3;  /**< L0s Exit Latency
                                                         The default value is the value you specify during hardware
                                                         configuration, writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t aslpms                       : 2;  /**< Active State Link PM Support
                                                         The default value is the value you specify during hardware
                                                         configuration, writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t mlw                          : 6;  /**< Maximum Link Width
                                                         The default value is the value you specify during hardware
                                                         configuration (x1), writable through PEM(0..1)_CFG_WR
                                                         however wider cofigurations are not supported. */
	uint32_t mls                          : 4;  /**< Maximum Link Speed
                                                         The reset value of this field is controlled by a value sent from
                                                         the lsb of the MIO_QLM#_SPD register.
                                                         qlm#_spd[1]   RST_VALUE   NOTE
                                                         1             0001b       2.5 GHz supported
                                                         0             0010b       5.0 GHz and 2.5 GHz supported
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
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
	struct cvmx_pcieepx_cfg031_s          cn78xx;
	struct cvmx_pcieepx_cfg031_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg031 cvmx_pcieepx_cfg031_t;

/**
 * cvmx_pcieep#_cfg032
 *
 * PCIE_CFG032 = Thirty-third 32-bits of PCIE type 0 config space
 * (Link Control Register/Link Status Register)
 */
union cvmx_pcieepx_cfg032 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg032_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lab                          : 1;  /**< Link Autonomous Bandwidth Status */
	uint32_t lbm                          : 1;  /**< Link Bandwidth Management Status */
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
                                                         Set automatically by hardware after Link initialization.
                                                         Value is undefined when link is not up. */
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
	struct cvmx_pcieepx_cfg032_s          cn78xx;
	struct cvmx_pcieepx_cfg032_s          cnf71xx;
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
 * PCIE_CFG037 = Thirty-eighth 32-bits of PCIE type 0 config space
 * (Device Capabilities 2 Register)
 */
union cvmx_pcieepx_cfg037 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg037_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t meetp                        : 2;  /**< Max End-End TLP Prefixes
                                                         o 01: 1
                                                         o 10: 2
                                                         o 11: 3
                                                         0 00: 4 */
	uint32_t eetps                        : 1;  /**< End-End TLP Prefix Supported
                                                         (Not Supported) */
	uint32_t effs                         : 1;  /**< Extended Fmt Field Supported
                                                         (Not Supported) */
	uint32_t obffs                        : 2;  /**< Optimized Buffer Flush Fill (OBFF) Supported
                                                         (Not Supported) */
	uint32_t reserved_12_17               : 6;
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
	struct cvmx_pcieepx_cfg037_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t meetp                        : 2;  /**< Max End-End TLP Prefixes
                                                         o 01: 1
                                                         o 10: 2
                                                         o 11: 3
                                                         0 00: 4 */
	uint32_t eetps                        : 1;  /**< End-End TLP Prefix Supported
                                                         (Not Supported) */
	uint32_t effs                         : 1;  /**< Extended Fmt Field Supported
                                                         (Not Supported) */
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
	uint32_t atom64s                      : 1;  /**< 64-bit AtomicOp Supported */
	uint32_t atom32s                      : 1;  /**< 32-bit AtomicOp Supported */
	uint32_t atom_ops                     : 1;  /**< AtomicOp Routing Supported
                                                         (Not Applicable for EP) */
	uint32_t ari                          : 1;  /**< Alternate Routing ID Forwarding Supported */
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
	uint32_t effs                         : 1;
	uint32_t eetps                        : 1;
	uint32_t meetp                        : 2;
	uint32_t reserved_24_31               : 8;
#endif
	} cn78xx;
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
};
typedef union cvmx_pcieepx_cfg037 cvmx_pcieepx_cfg037_t;

/**
 * cvmx_pcieep#_cfg038
 *
 * PCIE_CFG038 = Thirty-ninth 32-bits of PCIE type 0 config space
 * (Device Control 2 Register/Device Status 2 Register)
 */
union cvmx_pcieepx_cfg038 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg038_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t eetpb                        : 1;  /**< Unsupported End-End TLP Prefix Blocking */
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
	struct cvmx_pcieepx_cfg038_s          cn78xx;
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
};
typedef union cvmx_pcieepx_cfg038 cvmx_pcieepx_cfg038_t;

/**
 * cvmx_pcieep#_cfg039
 *
 * PCIE_CFG039 = Fourtieth 32-bits of PCIE type 0 config space
 * (Link Capabilities 2 Register)
 */
union cvmx_pcieepx_cfg039 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg039_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_9_31                : 23;
	uint32_t cls                          : 1;  /**< Crosslink Supported */
	uint32_t slsv                         : 7;  /**< Supported Link Speeds Vector
                                                         Indicates the supported Link speeds of the associated Port.
                                                         For each bit, a value of 1b indicates that the cooresponding
                                                         Link speed is supported; otherwise, the Link speed is not
                                                         supported.
                                                         Bit definitions are:
                                                         Bit 1 2.5 GT/s
                                                         Bit 2 5.0 GT/s
                                                         Bit 3 8.0 GT/s (Not Supported)
                                                         Bits 7:4 reserved
                                                         The reset value of this field is controlled by a value sent from
                                                         the lsb of the MIO_QLM#_SPD register
                                                         qlm#_spd[0]   RST_VALUE   NOTE
                                                         1             0001b       2.5 GHz supported
                                                         0             0011b       5.0 GHz and 2.5 GHz supported */
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
	struct cvmx_pcieepx_cfg039_s          cn78xx;
	struct cvmx_pcieepx_cfg039_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg039 cvmx_pcieepx_cfg039_t;

/**
 * cvmx_pcieep#_cfg040
 *
 * PCIE_CFG040 = Fourty-first 32-bits of PCIE type 0 config space
 * (Link Control 2 Register/Link Status 2 Register)
 */
union cvmx_pcieepx_cfg040 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg040_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_22_31               : 10;
	uint32_t ler                          : 1;  /**< Link Equalization Request */
	uint32_t ep3s                         : 1;  /**< Equalization Phase 3 Successful */
	uint32_t ep2s                         : 1;  /**< Equalization Phase 2 Successful */
	uint32_t ep1s                         : 1;  /**< Equalization Phase 1 Successful */
	uint32_t eqc                          : 1;  /**< Equalization Complete */
	uint32_t cdl                          : 1;  /**< Current De-emphasis Level
                                                         When the Link is operating at 5 GT/s speed, this bit
                                                         reflects the level of de-emphasis. Encodings:
                                                          1b: -3.5 dB
                                                          0b: -6 dB
                                                         Note: The value in this bit is undefined when the Link is
                                                         operating at 2.5 GT/s speed */
	uint32_t cde                          : 4;  /**< Compliance De-emphasis
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
                                                         voltage level at the Transmitter signals:
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
	struct cvmx_pcieepx_cfg040_s          cn78xx;
	struct cvmx_pcieepx_cfg040_cn61xx     cnf71xx;
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
 * PCIE_CFG044 = Fourty-fifth 32-bits of PCIE type 0 config space
 * (MSI-X Capability ID/
 * MSI-X Next Item Pointer/
 * MSI-X Control Register)
 */
union cvmx_pcieepx_cfg044 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg044_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msixen                       : 1;  /**< MSI-X Enable
                                                         If MSI-X is enabled, MSI and INTx must be disabled. */
	uint32_t funm                         : 1;  /**< Function Mask
                                                         1b: All vectors associated with the function are masked,
                                                         regardless of their respective per-vector mask bits.
                                                         0b: Each vectors Mask bit determines whether the vector
                                                         is masked or not. */
	uint32_t reserved_27_29               : 3;
	uint32_t msixts                       : 11; /**< "MSI-X Table Size
                                                         Encoded as (Table Size - 1)
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t ncp                          : 8;  /**< Next Capability Pointer */
	uint32_t msixcid                      : 8;  /**< MSI-X Capability ID */
#else
	uint32_t msixcid                      : 8;
	uint32_t ncp                          : 8;
	uint32_t msixts                       : 11;
	uint32_t reserved_27_29               : 3;
	uint32_t funm                         : 1;
	uint32_t msixen                       : 1;
#endif
	} s;
	struct cvmx_pcieepx_cfg044_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg044 cvmx_pcieepx_cfg044_t;

/**
 * cvmx_pcieep#_cfg045
 *
 * PCIE_CFG045 = Fourty-sixth 32-bits of PCIE type 0 config space
 * (MSI-X Table Offset and BIR Register)
 */
union cvmx_pcieepx_cfg045 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg045_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msixtoffs                    : 29; /**< "MSI-X Table Offset Register
                                                         Base address of the MSI-X Table, as an offset from the base
                                                         address of te BAR indicated by the Table BIR bits.
                                                         writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t msixtbir                     : 3;  /**< "MSI-X Table BAR Indicator Register (BIR)
                                                         Indicates which BAR is used to map the MSI-X Table
                                                         into memory space
                                                         000 - 100: BAR#
                                                         110 - 111: Reserved
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
#else
	uint32_t msixtbir                     : 3;
	uint32_t msixtoffs                    : 29;
#endif
	} s;
	struct cvmx_pcieepx_cfg045_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg045 cvmx_pcieepx_cfg045_t;

/**
 * cvmx_pcieep#_cfg046
 *
 * PCIE_CFG046 = Fourty-seventh 32-bits of PCIE type 0 config space
 * (MSI-X PBA Offset and BIR Register)
 */
union cvmx_pcieepx_cfg046 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg046_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t msixpoffs                    : 29; /**< "MSI-X Table Offset Register
                                                         Base address of the MSI-X PBA, as an offset from the base
                                                         address of te BAR indicated by the Table PBA bits.
                                                         writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t msixpbir                     : 3;  /**< "MSI-X PBA BAR Indicator Register (BIR)
                                                         Indicates which BAR is used to map the MSI-X Pending Bit Array
                                                         into memory space
                                                         000 - 100: BAR#
                                                         110 - 111: Reserved
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
#else
	uint32_t msixpbir                     : 3;
	uint32_t msixpoffs                    : 29;
#endif
	} s;
	struct cvmx_pcieepx_cfg046_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg046 cvmx_pcieepx_cfg046_t;

/**
 * cvmx_pcieep#_cfg064
 *
 * PCIE_CFG064 = Sixty-fifth 32-bits of PCIE type 0 config space
 * (PCI Express Extended Capability Header)
 */
union cvmx_pcieepx_cfg064 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg064_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next Capability Offset */
	uint32_t cv                           : 4;  /**< Capability Version */
	uint32_t pcieec                       : 16; /**< PCIE Express Extended Capability */
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
	struct cvmx_pcieepx_cfg064_s          cn78xx;
	struct cvmx_pcieepx_cfg064_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg064 cvmx_pcieepx_cfg064_t;

/**
 * cvmx_pcieep#_cfg065
 *
 * PCIE_CFG065 = Sixty-sixth 32-bits of PCIE type 0 config space
 * (Uncorrectable Error Status Register)
 */
union cvmx_pcieepx_cfg065 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg065_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t tpbes                        : 1;  /**< Unsupported TLP Prefix Blocked Error Status */
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
	struct cvmx_pcieepx_cfg065_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t tpbes                        : 1;  /**< Unsupported TLP Prefix Blocked Error Status */
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
	uint32_t tpbes                        : 1;
	uint32_t reserved_26_31               : 6;
#endif
	} cn78xx;
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
};
typedef union cvmx_pcieepx_cfg065 cvmx_pcieepx_cfg065_t;

/**
 * cvmx_pcieep#_cfg066
 *
 * PCIE_CFG066 = Sixty-seventh 32-bits of PCIE type 0 config space
 * (Uncorrectable Error Mask Register)
 */
union cvmx_pcieepx_cfg066 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg066_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t tpbem                        : 1;  /**< Unsupported TLP Prefix Blocked Error Mask */
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
	struct cvmx_pcieepx_cfg066_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t tpbem                        : 1;  /**< Unsupported TLP Prefix Blocked Error Mask */
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
	uint32_t tpbem                        : 1;
	uint32_t reserved_26_31               : 6;
#endif
	} cn78xx;
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
};
typedef union cvmx_pcieepx_cfg066 cvmx_pcieepx_cfg066_t;

/**
 * cvmx_pcieep#_cfg067
 *
 * PCIE_CFG067 = Sixty-eighth 32-bits of PCIE type 0 config space
 * (Uncorrectable Error Severity Register)
 */
union cvmx_pcieepx_cfg067 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg067_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t tpbes                        : 1;  /**< Unsupported TLP Prefix Blocked Error Severity */
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
	struct cvmx_pcieepx_cfg067_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_26_31               : 6;
	uint32_t tpbes                        : 1;  /**< Unsupported TLP Prefix Blocked Error Severity */
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
	uint32_t tpbes                        : 1;
	uint32_t reserved_26_31               : 6;
#endif
	} cn78xx;
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
};
typedef union cvmx_pcieepx_cfg067 cvmx_pcieepx_cfg067_t;

/**
 * cvmx_pcieep#_cfg068
 *
 * PCIE_CFG068 = Sixty-ninth 32-bits of PCIE type 0 config space
 * (Correctable Error Status Register)
 */
union cvmx_pcieepx_cfg068 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg068_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_15_31               : 17;
	uint32_t cies                         : 1;  /**< Corrected Internal Error Status */
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
	struct cvmx_pcieepx_cfg068_s          cn78xx;
	struct cvmx_pcieepx_cfg068_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg068 cvmx_pcieepx_cfg068_t;

/**
 * cvmx_pcieep#_cfg069
 *
 * PCIE_CFG069 = Seventieth 32-bits of PCIE type 0 config space
 * (Correctable Error Mask Register)
 */
union cvmx_pcieepx_cfg069 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg069_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_15_31               : 17;
	uint32_t ciem                         : 1;  /**< Corrected Internal Error Mask */
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
	struct cvmx_pcieepx_cfg069_s          cn78xx;
	struct cvmx_pcieepx_cfg069_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg069 cvmx_pcieepx_cfg069_t;

/**
 * cvmx_pcieep#_cfg070
 *
 * PCIE_CFG070 = Seventy-first 32-bits of PCIE type 0 config space
 * (Advanced Error Capabilities and Control Register)
 */
union cvmx_pcieepx_cfg070 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg070_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_12_31               : 20;
	uint32_t tlp_plp                      : 1;  /**< TLP Prefix Log Present (Not Supported) */
	uint32_t reserved_9_10                : 2;
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
	struct cvmx_pcieepx_cfg070_s          cn78xx;
	struct cvmx_pcieepx_cfg070_cn52xx     cnf71xx;
};
typedef union cvmx_pcieepx_cfg070 cvmx_pcieepx_cfg070_t;

/**
 * cvmx_pcieep#_cfg071
 *
 * PCIE_CFG071 = Seventy-second 32-bits of PCIE type 0 config space
 * (Header Log Register 1)
 */
union cvmx_pcieepx_cfg071 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg071_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dword1                       : 32; /**< Header Log Register (first DWORD) */
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
	struct cvmx_pcieepx_cfg071_s          cn78xx;
	struct cvmx_pcieepx_cfg071_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg071 cvmx_pcieepx_cfg071_t;

/**
 * cvmx_pcieep#_cfg072
 *
 * PCIE_CFG072 = Seventy-third 32-bits of PCIE type 0 config space
 * (Header Log Register 2)
 */
union cvmx_pcieepx_cfg072 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg072_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dword2                       : 32; /**< Header Log Register (second DWORD) */
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
	struct cvmx_pcieepx_cfg072_s          cn78xx;
	struct cvmx_pcieepx_cfg072_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg072 cvmx_pcieepx_cfg072_t;

/**
 * cvmx_pcieep#_cfg073
 *
 * PCIE_CFG073 = Seventy-fourth 32-bits of PCIE type 0 config space
 * (Header Log Register 3)
 */
union cvmx_pcieepx_cfg073 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg073_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dword3                       : 32; /**< Header Log Register (third DWORD) */
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
	struct cvmx_pcieepx_cfg073_s          cn78xx;
	struct cvmx_pcieepx_cfg073_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg073 cvmx_pcieepx_cfg073_t;

/**
 * cvmx_pcieep#_cfg074
 *
 * PCIE_CFG074 = Seventy-fifth 32-bits of PCIE type 0 config space
 * (Header Log Register 4)
 */
union cvmx_pcieepx_cfg074 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg074_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dword4                       : 32; /**< Header Log Register (fourth DWORD) */
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
	struct cvmx_pcieepx_cfg074_s          cn78xx;
	struct cvmx_pcieepx_cfg074_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg074 cvmx_pcieepx_cfg074_t;

/**
 * cvmx_pcieep#_cfg078
 *
 * PCIE_CFG078 = Seventy-ninth 32-bits of PCIE type 0 config space
 * (TLP Prefix Log Register)
 */
union cvmx_pcieepx_cfg078 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg078_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t tlp_pfx_log                  : 32; /**< TLP Prefix Log Register */
#else
	uint32_t tlp_pfx_log                  : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg078_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg078 cvmx_pcieepx_cfg078_t;

/**
 * cvmx_pcieep#_cfg082
 *
 * PCIE_CFG082 = Eighty-third 32-bits of PCIE type 0 config space
 * (PCI Express ARI Capability Header)
 */
union cvmx_pcieepx_cfg082 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg082_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next Capability Offset
                                                         Points to the Secondary PCI Express Capabilities by default */
	uint32_t cv                           : 4;  /**< Capability Version */
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
	struct cvmx_pcieepx_cfg082_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next Capability Offset
                                                         Points to the Secondary PCI Express Capabilities by default */
	uint32_t cv                           : 4;  /**< Capability Version */
	uint32_t ariid                        : 16; /**< PCIE Express Extended Capability */
#else
	uint32_t ariid                        : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} cn78xx;
};
typedef union cvmx_pcieepx_cfg082 cvmx_pcieepx_cfg082_t;

/**
 * cvmx_pcieep#_cfg083
 *
 * PCIE_CFG083 = Eighty-fourth 32-bits of PCIE type 0 config space
 * (PCI Express ARI Capability Register/
 * PCI Express ARI Control Register)
 */
union cvmx_pcieepx_cfg083 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg083_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_2_31                : 30;
	uint32_t acsfgc                       : 1;  /**< ACS Function Groups Capability */
	uint32_t mfvcfgc                      : 1;  /**< MFVC Function Groups Capability */
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
	struct cvmx_pcieepx_cfg083_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_23_31               : 9;
	uint32_t fg                           : 3;  /**< Function Group */
	uint32_t reserved_18_19               : 2;
	uint32_t acsfge                       : 1;  /**< ACS Function Groups Enable (A) */
	uint32_t mfvcfge                      : 1;  /**< MFVC Function Groups Enable (M) */
	uint32_t nfn                          : 8;  /**< Next Function Number */
	uint32_t reserved_2_7                 : 6;
	uint32_t acsfgc                       : 1;  /**< ACS Function Groups Capability */
	uint32_t mfvcfgc                      : 1;  /**< MFVC Function Groups Capability */
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
	} cn78xx;
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
};
typedef union cvmx_pcieepx_cfg084 cvmx_pcieepx_cfg084_t;

/**
 * cvmx_pcieep#_cfg086
 *
 * PCIE_CFG086 = Eighty-seventh 32-bits of PCIE type 0 config space
 * (PCI Express Secondary Capability (Gen3) Header)
 */
union cvmx_pcieepx_cfg086 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg086_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next Capability Offset
                                                         Points to the PCI Express SR-IOV Capability Header by default */
	uint32_t cv                           : 4;  /**< Capability Version */
	uint32_t pcieec                       : 16; /**< PCIE Express Extended Capability */
#else
	uint32_t pcieec                       : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} s;
	struct cvmx_pcieepx_cfg086_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg086 cvmx_pcieepx_cfg086_t;

/**
 * cvmx_pcieep#_cfg087
 *
 * PCIE_CFG087 = Eighty-eighth 32-bits of PCIE type 0 config space
 * (Link Control 3)
 */
union cvmx_pcieepx_cfg087 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg087_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_2_31                : 30;
	uint32_t ler                          : 1;  /**< Link Equalization Request Interrupt Enable */
	uint32_t pe                           : 1;  /**< Perform Equalization */
#else
	uint32_t pe                           : 1;
	uint32_t ler                          : 1;
	uint32_t reserved_2_31                : 30;
#endif
	} s;
	struct cvmx_pcieepx_cfg087_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg087 cvmx_pcieepx_cfg087_t;

/**
 * cvmx_pcieep#_cfg088
 *
 * PCIE_CFG088 = Eighty-ninth 32-bits of PCIE type 0 config space
 * (Lane Error Status)
 */
union cvmx_pcieepx_cfg088 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg088_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_8_31                : 24;
	uint32_t les                          : 8;  /**< Lane Error Status Bits */
#else
	uint32_t les                          : 8;
	uint32_t reserved_8_31                : 24;
#endif
	} s;
	struct cvmx_pcieepx_cfg088_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg088 cvmx_pcieepx_cfg088_t;

/**
 * cvmx_pcieep#_cfg089
 *
 * PCIE_CFG089 = Ninetieth 32-bits of PCIE type 0 config space
 * (Equalization Control Lane 0/
 * Equalization Control Lane 1)
 */
union cvmx_pcieepx_cfg089 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg089_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t l1dph                        : 3;  /**< "Lane 1 Downstream Component Receiver Preset Hint
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t l1dtp                        : 4;  /**< "Lane 1 Downstream Component Transmitter Preset
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t reserved_15_23               : 9;
	uint32_t l0dph                        : 3;  /**< "Lane 0 Downstream Component Receiver Preset Hint
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t l0dtp                        : 4;  /**< "Lane 0 Downstream Component Transmitter Preset
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
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
	struct cvmx_pcieepx_cfg089_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg089 cvmx_pcieepx_cfg089_t;

/**
 * cvmx_pcieep#_cfg090
 *
 * PCIE_CFG090 = Ninety-first 32-bits of PCIE type 0 config space
 * (Equalization Control Lane 2/
 * Equalization Control Lane 3)
 */
union cvmx_pcieepx_cfg090 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg090_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t l3dph                        : 3;  /**< "Lane 3 Downstream Component Receiver Preset Hint
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t l3dtp                        : 4;  /**< "Lane 3 Downstream Component Transmitter Preset
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t reserved_15_23               : 9;
	uint32_t l2dph                        : 3;  /**< "Lane 2 Downstream Component Receiver Preset Hint
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t l2dtp                        : 4;  /**< "Lane 2 Downstream Component Transmitter Preset
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
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
	struct cvmx_pcieepx_cfg090_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg090 cvmx_pcieepx_cfg090_t;

/**
 * cvmx_pcieep#_cfg091
 *
 * PCIE_CFG091 = Ninety-second 32-bits of PCIE type 0 config space
 * (Equalization Control Lane 4/
 * Equalization Control Lane 5)
 */
union cvmx_pcieepx_cfg091 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg091_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t l5dph                        : 3;  /**< "Lane 5 Downstream Component Receiver Preset Hint
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t l5dtp                        : 4;  /**< "Lane 5 Downstream Component Transmitter Preset
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t reserved_15_23               : 9;
	uint32_t l4dph                        : 3;  /**< "Lane 4 Downstream Component Receiver Preset Hint
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t l4dtp                        : 4;  /**< "Lane 4 Downstream Component Transmitter Preset
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
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
	struct cvmx_pcieepx_cfg091_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg091 cvmx_pcieepx_cfg091_t;

/**
 * cvmx_pcieep#_cfg092
 *
 * PCIE_CFG092 = Ninety-third 32-bits of PCIE type 0 config space
 * (Equalization Control Lane 6/
 * Equalization Control Lane 7)
 */
union cvmx_pcieepx_cfg092 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg092_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t l7dph                        : 3;  /**< "Lane 7 Downstream Component Receiver Preset Hint
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t l7dtp                        : 4;  /**< "Lane 7 Downstream Component Transmitter Preset
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t reserved_15_23               : 9;
	uint32_t l6dph                        : 3;  /**< "Lane 6 Downstream Component Receiver Preset Hint
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t l6dtp                        : 4;  /**< "Lane 6 Downstream Component Transmitter Preset
                                                         Writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
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
	struct cvmx_pcieepx_cfg092_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg092 cvmx_pcieepx_cfg092_t;

/**
 * cvmx_pcieep#_cfg094
 *
 * PCIE_CFG094 = Ninety-fifth 32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV Capability Header)
 */
union cvmx_pcieepx_cfg094 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg094_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next Capability Offset
                                                         Points to the Resizable BAR Capabilities by default */
	uint32_t cv                           : 4;  /**< Capability Version */
	uint32_t pcieec                       : 16; /**< PCIE Express Extended Capability */
#else
	uint32_t pcieec                       : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} s;
	struct cvmx_pcieepx_cfg094_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg094 cvmx_pcieepx_cfg094_t;

/**
 * cvmx_pcieep#_cfg095
 *
 * PCIE_CFG095 = Ninety-sixth 32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV Capability Register)
 */
union cvmx_pcieepx_cfg095 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg095_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t vfmimn                       : 11; /**< VF Migration Interrupt Message Number */
	uint32_t reserved_2_20                : 19;
	uint32_t arichp                       : 1;  /**< "ARI Capable Hiearchy Preserved
                                                         writable through PEM#_CFG_WR.
                                                         However, the application must not change this field." */
	uint32_t vfmc                         : 1;  /**< VF Migration Capable */
#else
	uint32_t vfmc                         : 1;
	uint32_t arichp                       : 1;
	uint32_t reserved_2_20                : 19;
	uint32_t vfmimn                       : 11;
#endif
	} s;
	struct cvmx_pcieepx_cfg095_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg095 cvmx_pcieepx_cfg095_t;

/**
 * cvmx_pcieep#_cfg096
 *
 * PCIE_CFG096 = Ninety-seventh 32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV Control Register/
 * PCI Express SR-IOV Status Register)
 */
union cvmx_pcieepx_cfg096 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg096_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_17_31               : 15;
	uint32_t ms                           : 1;  /**< VF Migration Status */
	uint32_t reserved_5_15                : 11;
	uint32_t ach                          : 1;  /**< ARI Capable Hiearchy
                                                         0b: All PF's have non-ARI Capable Hierarchy
                                                         1b: All PF's have ARI Capable Hierarchy
                                                         The value in this field in PF0 is used for all other
                                                         physical functions. */
	uint32_t mse                          : 1;  /**< VF MSE */
	uint32_t mie                          : 1;  /**< VF Migration Interrupt Enable */
	uint32_t me                           : 1;  /**< VF Migration Enable */
	uint32_t vfe                          : 1;  /**< VF Enable */
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
	struct cvmx_pcieepx_cfg096_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg096 cvmx_pcieepx_cfg096_t;

/**
 * cvmx_pcieep#_cfg097
 *
 * PCIE_CFG097 = Ninety-eighth 32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV Initial VFs Register/
 * PCI Express SR-IOV Total VFs Register)
 */
union cvmx_pcieepx_cfg097 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg097_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t tvf                          : 16; /**< Total VFs */
	uint32_t ivf                          : 16; /**< Initial VFs */
#else
	uint32_t ivf                          : 16;
	uint32_t tvf                          : 16;
#endif
	} s;
	struct cvmx_pcieepx_cfg097_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg097 cvmx_pcieepx_cfg097_t;

/**
 * cvmx_pcieep#_cfg098
 *
 * PCIE_CFG098 = Ninety-ninth  32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV Number of VFs Register/
 * PCI Express SR-IOV Function Dependency Link Register)
 */
union cvmx_pcieepx_cfg098 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg098_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t fdl                          : 8;  /**< Function Dependency Link
                                                         Enables support for VF dependency link. */
	uint32_t nvf                          : 16; /**< Number of VFs that are visible */
#else
	uint32_t nvf                          : 16;
	uint32_t fdl                          : 8;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_pcieepx_cfg098_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg098 cvmx_pcieepx_cfg098_t;

/**
 * cvmx_pcieep#_cfg099
 *
 * PCIE_CFG099 = One hundredth 32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV First VF Offset Register/
 * PCI Express SR-IOV VF Stride Register)
 */
union cvmx_pcieepx_cfg099 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg099_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t vfs                          : 16; /**< VF Stride */
	uint32_t fo                           : 16; /**< First VF Offset */
#else
	uint32_t fo                           : 16;
	uint32_t vfs                          : 16;
#endif
	} s;
	struct cvmx_pcieepx_cfg099_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg099 cvmx_pcieepx_cfg099_t;

/**
 * cvmx_pcieep#_cfg100
 *
 * PCIE_CFG100 = One hundred first 32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV VF Device ID Register)
 */
union cvmx_pcieepx_cfg100 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg100_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t vfdev                        : 16; /**< VF Device ID */
	uint32_t reserved_0_15                : 16;
#else
	uint32_t reserved_0_15                : 16;
	uint32_t vfdev                        : 16;
#endif
	} s;
	struct cvmx_pcieepx_cfg100_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg100 cvmx_pcieepx_cfg100_t;

/**
 * cvmx_pcieep#_cfg101
 *
 * PCIE_CFG101 = One hundred second 32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV Supported Page Sizes Register)
 */
union cvmx_pcieepx_cfg101 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg101_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t supps                        : 32; /**< Supported Page Sizes */
#else
	uint32_t supps                        : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg101_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg101 cvmx_pcieepx_cfg101_t;

/**
 * cvmx_pcieep#_cfg102
 *
 * PCIE_CFG102 = One hundred third 32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV System Page Size Register)
 */
union cvmx_pcieepx_cfg102 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg102_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ps                           : 32; /**< System Page Size */
#else
	uint32_t ps                           : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg102_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg102 cvmx_pcieepx_cfg102_t;

/**
 * cvmx_pcieep#_cfg103
 *
 * PCIE_CFG103 = One hundred fourth 32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV BAR 0 Register)
 */
union cvmx_pcieepx_cfg103 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg103_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lbab                         : 18; /**< Lower bits of the VF BAR 0 base address */
	uint32_t reserved_4_13                : 10;
	uint32_t pf                           : 1;  /**< Prefetchable */
	uint32_t typ                          : 2;  /**< BAR type
                                                         o 00 = 32-bit BAR
                                                         o 10 = 64-bit BAR */
	uint32_t mspc                         : 1;  /**< Memory Space Indicator
                                                         o 0 = BAR 0 is a memory BAR
                                                         o 1 = BAR 0 is an I/O BAR */
#else
	uint32_t mspc                         : 1;
	uint32_t typ                          : 2;
	uint32_t pf                           : 1;
	uint32_t reserved_4_13                : 10;
	uint32_t lbab                         : 18;
#endif
	} s;
	struct cvmx_pcieepx_cfg103_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg103 cvmx_pcieepx_cfg103_t;

/**
 * cvmx_pcieep#_cfg104
 *
 * PCIE_CFG104 = One hundred fifth 32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV BAR 1 Register)
 */
union cvmx_pcieepx_cfg104 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg104_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ubab                         : 32; /**< Upper bits of the VF BAR 0 base address */
#else
	uint32_t ubab                         : 32;
#endif
	} s;
	struct cvmx_pcieepx_cfg104_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg104 cvmx_pcieepx_cfg104_t;

/**
 * cvmx_pcieep#_cfg105
 *
 * PCIE_CFG105 = One hundred sixth 32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV BAR 2 Register)
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
	struct cvmx_pcieepx_cfg105_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg105 cvmx_pcieepx_cfg105_t;

/**
 * cvmx_pcieep#_cfg106
 *
 * PCIE_CFG106 = One hundred seventh 32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV BAR 3 Register)
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
	struct cvmx_pcieepx_cfg106_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg106 cvmx_pcieepx_cfg106_t;

/**
 * cvmx_pcieep#_cfg107
 *
 * PCIE_CFG107 = One hundred eighth 32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV BAR 4 Register)
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
	struct cvmx_pcieepx_cfg107_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg107 cvmx_pcieepx_cfg107_t;

/**
 * cvmx_pcieep#_cfg108
 *
 * PCIE_CFG108 = One hundred eleventh 32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV BAR 5 Register)
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
	struct cvmx_pcieepx_cfg108_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg108 cvmx_pcieepx_cfg108_t;

/**
 * cvmx_pcieep#_cfg109
 *
 * PCIE_CFG109 = One hundred tenth 32-bits of PCIE type 0 config space
 * (PCI Express SR-IOV migration State Array Offset Register)
 */
union cvmx_pcieepx_cfg109 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg109_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t mso                          : 29; /**< VF Migration State Offset */
	uint32_t msbir                        : 3;  /**< VF Migration State BIR */
#else
	uint32_t msbir                        : 3;
	uint32_t mso                          : 29;
#endif
	} s;
	struct cvmx_pcieepx_cfg109_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg109 cvmx_pcieepx_cfg109_t;

/**
 * cvmx_pcieep#_cfg110
 *
 * PCIE_CFG110 = One hundred eleventh 32-bits of PCIE type 0 config space
 * (PCI Express Resizable BAR (RBAR) Capability Header)
 */
union cvmx_pcieepx_cfg110 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg110_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t nco                          : 12; /**< Next Capability Offset */
	uint32_t cv                           : 4;  /**< Capability Version */
	uint32_t pcieec                       : 16; /**< PCIE Express Extended Capability */
#else
	uint32_t pcieec                       : 16;
	uint32_t cv                           : 4;
	uint32_t nco                          : 12;
#endif
	} s;
	struct cvmx_pcieepx_cfg110_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg110 cvmx_pcieepx_cfg110_t;

/**
 * cvmx_pcieep#_cfg111
 *
 * PCIE_CFG111 = One hundred twelvth 32-bits of PCIE type 0 config space
 * (PCI Express Resizable BAR (RBAR) Capability Register)
 */
union cvmx_pcieepx_cfg111 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg111_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t srs                          : 26; /**< "Supported Resource Sizes
                                                         The OCTEON advertises the maximum allowable BAR size (512GB - 0xf_ffff)
                                                         when the fus__bar2_size_conf is intact. When the fuse is blown,
                                                         the OCTEON advertised a BAR size of 32TB (0x3ff_ffff).
                                                         The BAR is disabled at runtime by writing all zeros through
                                                         PEM#_CFG_WR to this field" */
	uint32_t reserved_0_3                 : 4;
#else
	uint32_t reserved_0_3                 : 4;
	uint32_t srs                          : 26;
	uint32_t reserved_30_31               : 2;
#endif
	} s;
	struct cvmx_pcieepx_cfg111_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg111 cvmx_pcieepx_cfg111_t;

/**
 * cvmx_pcieep#_cfg112
 *
 * PCIE_CFG112 = One hundred thirteenth 32-bits of PCIE type 0 config space
 * (PCI Express Resizable BAR (RBAR) Control Register)
 */
union cvmx_pcieepx_cfg112 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg112_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_13_31               : 19;
	uint32_t rbars                        : 5;  /**< BAR Size
                                                         The OCTEON advertises the minimum allowable BAR size of 0x0 (1MB)
                                                         but will accept values as large as 0x19 (32TB) */
	uint32_t nrbar                        : 3;  /**< Number of Resizable BARs */
	uint32_t reserved_3_4                 : 2;
	uint32_t rbari                        : 3;  /**< BAR Index
                                                         Points to the BAR located at offset 0x18 (BAR2) */
#else
	uint32_t rbari                        : 3;
	uint32_t reserved_3_4                 : 2;
	uint32_t nrbar                        : 3;
	uint32_t rbars                        : 5;
	uint32_t reserved_13_31               : 19;
#endif
	} s;
	struct cvmx_pcieepx_cfg112_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg112 cvmx_pcieepx_cfg112_t;

/**
 * cvmx_pcieep#_cfg448
 *
 * PCIE_CFG448 = Four hundred forty-ninth 32-bits of PCIE type 0 config space
 * (Ack Latency Timer and Replay Timer Register)
 */
union cvmx_pcieepx_cfg448 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg448_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t rtl                          : 16; /**< Replay Time Limit
                                                         The replay timer expires when it reaches this limit. The PCI
                                                         Express bus initiates a replay upon reception of a Nak or when
                                                         the replay timer expires.
                                                         This value will be set correctly by the hardware out of reset
                                                         or when the negotiated Link-Width or Payload-Size changes. If
                                                         the user changes this value through a CSR write or by an
                                                         EEPROM load then they should refer to the PCIe Specification
                                                         for the correct value. */
	uint32_t rtltl                        : 16; /**< Round Trip Latency Time Limit
                                                         The Ack/Nak latency timer expires when it reaches this limit.
                                                         This value will be set correctly by the hardware out of reset
                                                         or when the negotiated Link-Width or Payload-Size changes. If
                                                         the user changes this value through a CSR write or by an
                                                         EEPROM load then they should refer to the PCIe Specification
                                                         for the correct value. */
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
	struct cvmx_pcieepx_cfg448_s          cn78xx;
	struct cvmx_pcieepx_cfg448_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg448 cvmx_pcieepx_cfg448_t;

/**
 * cvmx_pcieep#_cfg449
 *
 * PCIE_CFG449 = Four hundred fiftieth 32-bits of PCIE type 0 config space
 * (Other Message Register)
 */
union cvmx_pcieepx_cfg449 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg449_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t omr                          : 32; /**< Other Message Register
                                                         This register can be used for either of the following purposes:
                                                         o To send a specific PCI Express Message, the application
                                                           writes the payload of the Message into this register, then
                                                           sets bit 0 of the Port Link Control Register to send the
                                                           Message.
                                                         o To store a corruption pattern for corrupting the LCRC on all
                                                           TLPs, the application places a 32-bit corruption pattern into
                                                           this register and enables this function by setting bit 25 of
                                                           the Port Link Control Register. When enabled, the transmit
                                                           LCRC result is XOR'd with this pattern before inserting
                                                           it into the packet. */
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
	struct cvmx_pcieepx_cfg449_s          cn78xx;
	struct cvmx_pcieepx_cfg449_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg449 cvmx_pcieepx_cfg449_t;

/**
 * cvmx_pcieep#_cfg450
 *
 * PCIE_CFG450 = Four hundred fifty-first 32-bits of PCIE type 0 config space
 * (Port Force Link Register)
 */
union cvmx_pcieepx_cfg450 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg450_s {
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
	} s;
	struct cvmx_pcieepx_cfg450_cn52xx {
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
	uint32_t reserved_8_14                : 7;
	uint32_t link_num                     : 8;  /**< Link Number
                                                         Not used for Endpoint */
#else
	uint32_t link_num                     : 8;
	uint32_t reserved_8_14                : 7;
	uint32_t force_link                   : 1;
	uint32_t link_state                   : 6;
	uint32_t reserved_22_23               : 2;
	uint32_t lpec                         : 8;
#endif
	} cn52xx;
	struct cvmx_pcieepx_cfg450_cn52xx     cn52xxp1;
	struct cvmx_pcieepx_cfg450_cn52xx     cn56xx;
	struct cvmx_pcieepx_cfg450_cn52xx     cn56xxp1;
	struct cvmx_pcieepx_cfg450_cn52xx     cn61xx;
	struct cvmx_pcieepx_cfg450_cn52xx     cn63xx;
	struct cvmx_pcieepx_cfg450_cn52xx     cn63xxp1;
	struct cvmx_pcieepx_cfg450_cn52xx     cn66xx;
	struct cvmx_pcieepx_cfg450_cn52xx     cn68xx;
	struct cvmx_pcieepx_cfg450_cn52xx     cn68xxp1;
	struct cvmx_pcieepx_cfg450_s          cn70xx;
	struct cvmx_pcieepx_cfg450_cn52xx     cn78xx;
	struct cvmx_pcieepx_cfg450_cn52xx     cnf71xx;
};
typedef union cvmx_pcieepx_cfg450 cvmx_pcieepx_cfg450_t;

/**
 * cvmx_pcieep#_cfg451
 *
 * PCIE_CFG451 = Four hundred fifty-second 32-bits of PCIE type 0 config space
 * (Ack Frequency Register)
 */
union cvmx_pcieepx_cfg451 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg451_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t easpml1                      : 1;  /**< Enter ASPM L1 without receive in L0s
                                                         Allow core to enter ASPM L1 even when link partner did
                                                         not go to L0s (receive is not in L0s).
                                                         When not set, core goes to ASPM L1 only after idle period
                                                         during which both receive and transmit are in L0s. */
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
	struct cvmx_pcieepx_cfg451_s          cn78xx;
	struct cvmx_pcieepx_cfg451_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg451 cvmx_pcieepx_cfg451_t;

/**
 * cvmx_pcieep#_cfg452
 *
 * PCIE_CFG452 = Four hundred fifty-third 32-bits of PCIE type 0 config space
 * (Port Link Control Register)
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
	uint32_t lme                          : 6;  /**< Link Mode Enable
                                                         o 000001: x1
                                                         o 000011: x2  (not supported)
                                                         o 000111: x4  (not supported)
                                                         o 001111: x8  (not supported)
                                                         o 011111: x16 (not supported)
                                                         o 111111: x32 (not supported)
                                                         This field indicates the MAXIMUM number of lanes supported
                                                         by the PCIe port.
                                                         See also MLW.
                                                         (Note: The value of this field does NOT indicate the number
                                                          of lanes in use by the PCIe. LME sets the max number of lanes
                                                          in the PCIe core that COULD be used. As per the PCIe specs,
                                                          the PCIe core can negotiate a smaller link width) */
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
	struct cvmx_pcieepx_cfg452_cn70xx     cn78xx;
	struct cvmx_pcieepx_cfg452_cn61xx     cnf71xx;
};
typedef union cvmx_pcieepx_cfg452 cvmx_pcieepx_cfg452_t;

/**
 * cvmx_pcieep#_cfg453
 *
 * PCIE_CFG453 = Four hundred fifty-fourth 32-bits of PCIE type 0 config space
 * (Lane Skew Register)
 */
union cvmx_pcieepx_cfg453 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg453_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dlld                         : 1;  /**< Disable Lane-to-Lane Deskew
                                                         Disables the internal Lane-to-Lane deskew logic. */
	uint32_t reserved_26_30               : 5;
	uint32_t ack_nak                      : 1;  /**< Ack/Nak Disable
                                                         Prevents the PCI Express bus from sending Ack and Nak DLLPs. */
	uint32_t fcd                          : 1;  /**< Flow Control Disable
                                                         Prevents the PCI Express bus from sending FC DLLPs. */
	uint32_t ilst                         : 24; /**< Insert Lane Skew for Transmit
                                                         Causes skew between lanes for test purposes. There are three
                                                         bits per Lane. The value is in units of one symbol time. For
                                                         example, the value 010b for a Lane forces a skew of two symbol
                                                         times for that Lane. The maximum skew value for any Lane is 5
                                                         symbol times. */
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
	struct cvmx_pcieepx_cfg453_s          cn78xx;
	struct cvmx_pcieepx_cfg453_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg453 cvmx_pcieepx_cfg453_t;

/**
 * cvmx_pcieep#_cfg454
 *
 * PCIE_CFG454 = Four hundred fifty-fifth 32-bits of PCIE type 0 config space
 * (Symbol Number Register)
 */
union cvmx_pcieepx_cfg454 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg454_s {
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
	struct cvmx_pcieepx_cfg454_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_29_31               : 3;
	uint32_t tmfcwt                       : 5;  /**< Used to be "Timer Modifier for Flow Control Watchdog Timer"
                                                         No longer used. Repl and enhanced func moved to "Queue Status"
                                                         register - CFG463. Kept for now to prevent s/w from breaking. */
	uint32_t tmanlt                       : 5;  /**< Timer Modifier for Ack/Nak Latency Timer
                                                         Increases the timer value for the Ack/Nak latency timer, in
                                                         increments of 64 clock cycles. */
	uint32_t tmrt                         : 5;  /**< Timer Modifier for Replay Timer
                                                         Increases the timer value for the replay timer, in increments
                                                         of 64 clock cycles. */
	uint32_t reserved_8_13                : 6;
	uint32_t mfuncn                       : 8;  /**< Max Number of Functions Supported
                                                         Used for SR-IOV */
#else
	uint32_t mfuncn                       : 8;
	uint32_t reserved_8_13                : 6;
	uint32_t tmrt                         : 5;
	uint32_t tmanlt                       : 5;
	uint32_t tmfcwt                       : 5;
	uint32_t reserved_29_31               : 3;
#endif
	} cn78xx;
	struct cvmx_pcieepx_cfg454_cn61xx     cnf71xx;
};
typedef union cvmx_pcieepx_cfg454 cvmx_pcieepx_cfg454_t;

/**
 * cvmx_pcieep#_cfg455
 *
 * PCIE_CFG455 = Four hundred fifty-sixth 32-bits of PCIE type 0 config space
 * (Symbol Timer Register/Filter Mask Register 1)
 */
union cvmx_pcieepx_cfg455 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg455_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t m_cfg0_filt                  : 1;  /**< Mask filtering of received Configuration Requests (RC mode only) */
	uint32_t m_io_filt                    : 1;  /**< Mask filtering of received I/O Requests (RC mode only) */
	uint32_t msg_ctrl                     : 1;  /**< Message Control
                                                         The application must not change this field. */
	uint32_t m_cpl_ecrc_filt              : 1;  /**< Mask ECRC error filtering for Completions */
	uint32_t m_ecrc_filt                  : 1;  /**< Mask ECRC error filtering */
	uint32_t m_cpl_len_err                : 1;  /**< Mask Length mismatch error for received Completions */
	uint32_t m_cpl_attr_err               : 1;  /**< Mask Attributes mismatch error for received Completions */
	uint32_t m_cpl_tc_err                 : 1;  /**< Mask Traffic Class mismatch error for received Completions */
	uint32_t m_cpl_fun_err                : 1;  /**< Mask function mismatch error for received Completions */
	uint32_t m_cpl_rid_err                : 1;  /**< Mask Requester ID mismatch error for received Completions */
	uint32_t m_cpl_tag_err                : 1;  /**< Mask Tag error rules for received Completions */
	uint32_t m_lk_filt                    : 1;  /**< Mask Locked Request filtering */
	uint32_t m_cfg1_filt                  : 1;  /**< Mask Type 1 Configuration Request filtering */
	uint32_t m_bar_match                  : 1;  /**< Mask BAR match filtering */
	uint32_t m_pois_filt                  : 1;  /**< Mask poisoned TLP filtering */
	uint32_t m_fun                        : 1;  /**< Mask function */
	uint32_t dfcwt                        : 1;  /**< Disable FC Watchdog Timer */
	uint32_t reserved_11_14               : 4;
	uint32_t skpiv                        : 11; /**< SKP Interval Value */
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
	struct cvmx_pcieepx_cfg455_s          cn78xx;
	struct cvmx_pcieepx_cfg455_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg455 cvmx_pcieepx_cfg455_t;

/**
 * cvmx_pcieep#_cfg456
 *
 * PCIE_CFG456 = Four hundred fifty-seventh 32-bits of PCIE type 0 config space
 * (Filter Mask Register 2)
 */
union cvmx_pcieepx_cfg456 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg456_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_4_31                : 28;
	uint32_t m_handle_flush               : 1;  /**< Mask Core Filter to handle flush request */
	uint32_t m_dabort_4ucpl               : 1;  /**< Mask DLLP abort for unexpected CPL */
	uint32_t m_vend1_drp                  : 1;  /**< Mask Vendor MSG Type 1 dropped silently */
	uint32_t m_vend0_drp                  : 1;  /**< Mask Vendor MSG Type 0 dropped with UR error reporting. */
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
	struct cvmx_pcieepx_cfg456_s          cn78xx;
	struct cvmx_pcieepx_cfg456_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg456 cvmx_pcieepx_cfg456_t;

/**
 * cvmx_pcieep#_cfg458
 *
 * PCIE_CFG458 = Four hundred fifty-ninth 32-bits of PCIE type 0 config space
 * (Debug Register 0)
 */
union cvmx_pcieepx_cfg458 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg458_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dbg_info_l32                 : 32; /**< Debug Info Lower 32 Bits */
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
	struct cvmx_pcieepx_cfg458_s          cn78xx;
	struct cvmx_pcieepx_cfg458_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg458 cvmx_pcieepx_cfg458_t;

/**
 * cvmx_pcieep#_cfg459
 *
 * PCIE_CFG459 = Four hundred sixtieth 32-bits of PCIE type 0 config space
 * (Debug Register 1)
 */
union cvmx_pcieepx_cfg459 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg459_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dbg_info_u32                 : 32; /**< Debug Info Upper 32 Bits */
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
	struct cvmx_pcieepx_cfg459_s          cn78xx;
	struct cvmx_pcieepx_cfg459_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg459 cvmx_pcieepx_cfg459_t;

/**
 * cvmx_pcieep#_cfg460
 *
 * PCIE_CFG460 = Four hundred sixty-first 32-bits of PCIE type 0 config space
 * (Transmit Posted FC Credit Status)
 */
union cvmx_pcieepx_cfg460 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg460_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t tphfcc                       : 8;  /**< Transmit Posted Header FC Credits
                                                         The Posted Header credits advertised by the receiver at the
                                                         other end of the Link, updated with each UpdateFC DLLP. */
	uint32_t tpdfcc                       : 12; /**< Transmit Posted Data FC Credits
                                                         The Posted Data credits advertised by the receiver at the other
                                                         end of the Link, updated with each UpdateFC DLLP. */
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
	struct cvmx_pcieepx_cfg460_s          cn78xx;
	struct cvmx_pcieepx_cfg460_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg460 cvmx_pcieepx_cfg460_t;

/**
 * cvmx_pcieep#_cfg461
 *
 * PCIE_CFG461 = Four hundred sixty-second 32-bits of PCIE type 0 config space
 * (Transmit Non-Posted FC Credit Status)
 */
union cvmx_pcieepx_cfg461 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg461_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t tchfcc                       : 8;  /**< Transmit Non-Posted Header FC Credits
                                                         The Non-Posted Header credits advertised by the receiver at the
                                                         other end of the Link, updated with each UpdateFC DLLP. */
	uint32_t tcdfcc                       : 12; /**< Transmit Non-Posted Data FC Credits
                                                         The Non-Posted Data credits advertised by the receiver at the
                                                         other end of the Link, updated with each UpdateFC DLLP. */
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
	struct cvmx_pcieepx_cfg461_s          cn78xx;
	struct cvmx_pcieepx_cfg461_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg461 cvmx_pcieepx_cfg461_t;

/**
 * cvmx_pcieep#_cfg462
 *
 * PCIE_CFG462 = Four hundred sixty-third 32-bits of PCIE type 0 config space
 * (Transmit Completion FC Credit Status )
 */
union cvmx_pcieepx_cfg462 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg462_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t tchfcc                       : 8;  /**< Transmit Completion Header FC Credits
                                                         The Completion Header credits advertised by the receiver at the
                                                         other end of the Link, updated with each UpdateFC DLLP. */
	uint32_t tcdfcc                       : 12; /**< Transmit Completion Data FC Credits
                                                         The Completion Data credits advertised by the receiver at the
                                                         other end of the Link, updated with each UpdateFC DLLP. */
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
	struct cvmx_pcieepx_cfg462_s          cn78xx;
	struct cvmx_pcieepx_cfg462_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg462 cvmx_pcieepx_cfg462_t;

/**
 * cvmx_pcieep#_cfg463
 *
 * PCIE_CFG463 = Four hundred sixty-fourth 32-bits of PCIE type 0 config space
 * (Queue Status)
 */
union cvmx_pcieepx_cfg463 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg463_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t fcltoe                       : 1;  /**< FC Latency Timer Override Enable
                                                         When this bit is set, the value from the "FC Latency Timer Override
                                                         Value" field in this register will override the FC latency timer
                                                         value that the core calculates according to the PCIe specification. */
	uint32_t reserved_29_30               : 2;
	uint32_t fcltov                       : 13; /**< FC Latency Timer Override Value
                                                         When you set the "FC Latency Timer Override Enable" in this register,
                                                         the value in this field will override the FC latency timer value
                                                         that the core calculates according to the PCIe specification. */
	uint32_t reserved_3_15                : 13;
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
	struct cvmx_pcieepx_cfg463_s          cn78xx;
	struct cvmx_pcieepx_cfg463_cn52xx     cnf71xx;
};
typedef union cvmx_pcieepx_cfg463 cvmx_pcieepx_cfg463_t;

/**
 * cvmx_pcieep#_cfg464
 *
 * PCIE_CFG464 = Four hundred sixty-fifth 32-bits of PCIE type 0 config space
 * (VC Transmit Arbitration Register 1)
 */
union cvmx_pcieepx_cfg464 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg464_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t wrr_vc3                      : 8;  /**< WRR Weight for VC3 */
	uint32_t wrr_vc2                      : 8;  /**< WRR Weight for VC2 */
	uint32_t wrr_vc1                      : 8;  /**< WRR Weight for VC1 */
	uint32_t wrr_vc0                      : 8;  /**< WRR Weight for VC0 */
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
	struct cvmx_pcieepx_cfg464_s          cn78xx;
	struct cvmx_pcieepx_cfg464_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg464 cvmx_pcieepx_cfg464_t;

/**
 * cvmx_pcieep#_cfg465
 *
 * PCIE_CFG465 = Four hundred sixty-sixth 32-bits of PCIE type 0 config space
 * (VC Transmit Arbitration Register 2)
 */
union cvmx_pcieepx_cfg465 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg465_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t wrr_vc7                      : 8;  /**< WRR Weight for VC7 */
	uint32_t wrr_vc6                      : 8;  /**< WRR Weight for VC6 */
	uint32_t wrr_vc5                      : 8;  /**< WRR Weight for VC5 */
	uint32_t wrr_vc4                      : 8;  /**< WRR Weight for VC4 */
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
	struct cvmx_pcieepx_cfg465_s          cn78xx;
	struct cvmx_pcieepx_cfg465_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg465 cvmx_pcieepx_cfg465_t;

/**
 * cvmx_pcieep#_cfg466
 *
 * PCIE_CFG466 = Four hundred sixty-seventh 32-bits of PCIE type 0 config space
 * (VC0 Posted Receive Queue Control)
 */
union cvmx_pcieepx_cfg466 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg466_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t rx_queue_order               : 1;  /**< VC Ordering for Receive Queues
                                                         Determines the VC ordering rule for the receive queues, used
                                                         only in the segmented-buffer configuration,
                                                         writable through PEM(0..1)_CFG_WR:
                                                         o 1: Strict ordering, higher numbered VCs have higher priority
                                                         o 0: Round robin
                                                         However, the application must not change this field. */
	uint32_t type_ordering                : 1;  /**< TLP Type Ordering for VC0
                                                         Determines the TLP type ordering rule for VC0 receive queues,
                                                         used only in the segmented-buffer configuration, writable
                                                         through PEM(0..1)_CFG_WR:
                                                         o 1: Ordering of received TLPs follows the rules in
                                                              PCI Express Base Specification
                                                         o 0: Strict ordering for received TLPs: Posted, then
                                                              Completion, then Non-Posted
                                                         However, the application must not change this field. */
	uint32_t reserved_24_29               : 6;
	uint32_t queue_mode                   : 3;  /**< VC0 Posted TLP Queue Mode
                                                         The operating mode of the Posted receive queue for VC0, used
                                                         only in the segmented-buffer configuration, writable through
                                                         PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field.
                                                         Only one bit can be set at a time:
                                                         o Bit 23: Bypass
                                                         o Bit 22: Cut-through
                                                         o Bit 21: Store-and-forward */
	uint32_t reserved_20_20               : 1;
	uint32_t header_credits               : 8;  /**< VC0 Posted Header Credits
                                                         The number of initial Posted header credits for VC0, used for
                                                         all receive queue buffer configurations.
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t data_credits                 : 12; /**< VC0 Posted Data Credits
                                                         The number of initial Posted data credits for VC0, used for all
                                                         receive queue buffer configurations.
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
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
	struct cvmx_pcieepx_cfg466_s          cn78xx;
	struct cvmx_pcieepx_cfg466_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg466 cvmx_pcieepx_cfg466_t;

/**
 * cvmx_pcieep#_cfg467
 *
 * PCIE_CFG467 = Four hundred sixty-eighth 32-bits of PCIE type 0 config space
 * (VC0 Non-Posted Receive Queue Control)
 */
union cvmx_pcieepx_cfg467 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg467_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t queue_mode                   : 3;  /**< VC0 Non-Posted TLP Queue Mode
                                                         The operating mode of the Non-Posted receive queue for VC0,
                                                         used only in the segmented-buffer configuration, writable
                                                         through PEM(0..1)_CFG_WR.
                                                         Only one bit can be set at a time:
                                                         o Bit 23: Bypass
                                                         o Bit 22: Cut-through
                                                         o Bit 21: Store-and-forward
                                                         However, the application must not change this field. */
	uint32_t reserved_20_20               : 1;
	uint32_t header_credits               : 8;  /**< VC0 Non-Posted Header Credits
                                                         The number of initial Non-Posted header credits for VC0, used
                                                         for all receive queue buffer configurations.
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t data_credits                 : 12; /**< VC0 Non-Posted Data Credits
                                                         The number of initial Non-Posted data credits for VC0, used for
                                                         all receive queue buffer configurations.
                                                         This field is writable through PEM(0..1)_CFG_WR.
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
	struct cvmx_pcieepx_cfg467_s          cn78xx;
	struct cvmx_pcieepx_cfg467_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg467 cvmx_pcieepx_cfg467_t;

/**
 * cvmx_pcieep#_cfg468
 *
 * PCIE_CFG468 = Four hundred sixty-ninth 32-bits of PCIE type 0 config space
 * (VC0 Completion Receive Queue Control)
 */
union cvmx_pcieepx_cfg468 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg468_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t queue_mode                   : 3;  /**< VC0 Completion TLP Queue Mode
                                                         The operating mode of the Completion receive queue for VC0,
                                                         used only in the segmented-buffer configuration, writable
                                                         through PEM(0..1)_CFG_WR.
                                                         Only one bit can be set at a time:
                                                         o Bit 23: Bypass
                                                         o Bit 22: Cut-through
                                                         o Bit 21: Store-and-forward
                                                         However, the application must not change this field. */
	uint32_t reserved_20_20               : 1;
	uint32_t header_credits               : 8;  /**< VC0 Completion Header Credits
                                                         The number of initial Completion header credits for VC0, used
                                                         for all receive queue buffer configurations.
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
	uint32_t data_credits                 : 12; /**< VC0 Completion Data Credits
                                                         The number of initial Completion data credits for VC0, used for
                                                         all receive queue buffer configurations.
                                                         This field is writable through PEM(0..1)_CFG_WR.
                                                         However, the application must not change this field. */
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
	struct cvmx_pcieepx_cfg468_s          cn78xx;
	struct cvmx_pcieepx_cfg468_s          cnf71xx;
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
	struct cvmx_pcieepx_cfg492_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg492 cvmx_pcieepx_cfg492_t;

/**
 * cvmx_pcieep#_cfg515
 *
 * PCIE_CFG515 = Five hundred sixteenth 32-bits of PCIE type 0 config space
 * (Port Logic Register (Gen2))
 */
union cvmx_pcieepx_cfg515 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg515_s {
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
	} s;
	struct cvmx_pcieepx_cfg515_s          cn61xx;
	struct cvmx_pcieepx_cfg515_s          cn63xx;
	struct cvmx_pcieepx_cfg515_s          cn63xxp1;
	struct cvmx_pcieepx_cfg515_s          cn66xx;
	struct cvmx_pcieepx_cfg515_s          cn68xx;
	struct cvmx_pcieepx_cfg515_s          cn68xxp1;
	struct cvmx_pcieepx_cfg515_s          cn70xx;
	struct cvmx_pcieepx_cfg515_s          cn78xx;
	struct cvmx_pcieepx_cfg515_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg515 cvmx_pcieepx_cfg515_t;

/**
 * cvmx_pcieep#_cfg516
 *
 * PCIE_CFG516 = Five hundred seventeenth 32-bits of PCIE type 0 config space
 * (PHY Status Register)
 */
union cvmx_pcieepx_cfg516 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg516_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t phy_stat                     : 32; /**< PHY Status */
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
	struct cvmx_pcieepx_cfg516_s          cn78xx;
	struct cvmx_pcieepx_cfg516_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg516 cvmx_pcieepx_cfg516_t;

/**
 * cvmx_pcieep#_cfg517
 *
 * PCIE_CFG517 = Five hundred eighteenth 32-bits of PCIE type 0 config space
 * (PHY Control Register)
 */
union cvmx_pcieepx_cfg517 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg517_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t phy_ctrl                     : 32; /**< PHY Control */
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
	struct cvmx_pcieepx_cfg517_s          cn78xx;
	struct cvmx_pcieepx_cfg517_s          cnf71xx;
};
typedef union cvmx_pcieepx_cfg517 cvmx_pcieepx_cfg517_t;

/**
 * cvmx_pcieep#_cfg548
 *
 * PCIE_CFG548 = Five hundred forty ninth 32-bits of PCIE type 0 config space
 * (Gen3 Control Register)
 */
union cvmx_pcieepx_cfg548 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg548_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_19_31               : 13;
	uint32_t dcbd                         : 1;  /**< Disable Balance Disable
                                                         Disable DC Balance feature */
	uint32_t dtdd                         : 1;  /**< DLLP Transmission Delay Disable
                                                         Disable delay transmission of DLLPs before Equalization */
	uint32_t ed                           : 1;  /**< Equalization Disable
                                                         Disable Equalization feature */
	uint32_t reserved_12_15               : 4;
	uint32_t erd                          : 1;  /**< Equalization Redo Disable
                                                         Disable requesting reset of EIEOS count during Equalization */
	uint32_t ecrd                         : 1;  /**< Equalization EIEOS Count Reset Disable
                                                         Disable requesting reset of EIEOS count during Equalization */
	uint32_t ep2p3d                       : 1;  /**< Equalization Phase 2 and Phase 3 Disable
                                                         This applies to Downstream Ports only */
	uint32_t dsg3                         : 1;  /**< Disable Scrambler for Gen3 Data Rate
                                                         The Gen3 scrambler/descrambler within the core needs to be
                                                         disabled when the scrambling function is implemented outside
                                                         of the core (within the PHY) */
	uint32_t reserved_1_7                 : 7;
	uint32_t grizdnc                      : 1;  /**< Gen3 Receiver Impedance ZRX-DC Not Compliant. */
#else
	uint32_t grizdnc                      : 1;
	uint32_t reserved_1_7                 : 7;
	uint32_t dsg3                         : 1;
	uint32_t ep2p3d                       : 1;
	uint32_t ecrd                         : 1;
	uint32_t erd                          : 1;
	uint32_t reserved_12_15               : 4;
	uint32_t ed                           : 1;
	uint32_t dtdd                         : 1;
	uint32_t dcbd                         : 1;
	uint32_t reserved_19_31               : 13;
#endif
	} s;
	struct cvmx_pcieepx_cfg548_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg548 cvmx_pcieepx_cfg548_t;

/**
 * cvmx_pcieep#_cfg554
 *
 * PCIE_CFG554 = Five hundred fifty fifth 32-bits of PCIE type 0 config space
 * (Gen3 EQ Control Register)
 */
union cvmx_pcieepx_cfg554 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg554_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t prv                          : 16; /**< Preset Request Vector
                                                         Requesting of Presets during the intial part of the EQ Master
                                                         Phase. Encoding Scheme as follows:
                                                         Bit [15:0] = 0x0: No preset will be requested and evaluated
                                                         in the EQ Master Phase
                                                         Bit [i] = 1: Preset=i will be requested and evaluated in the
                                                         EQ Master Phase
                                                         o 0000000000000000: No preset req/evaluated in EQ Master Phase
                                                         o 00000xxxxxxxxxx1: Preset 0 req/evaluated in EQ Master Phase
                                                         o 00000xxxxxxxxx1x: Preset 1 req/evaluated in EQ Master Phase
                                                         o 00000xxxxxxxx1xx: Preset 2 req/evaluated in EQ Master Phase
                                                         o 00000xxxxxxx1xxx: Preset 3 req/evaluated in EQ Master Phase
                                                         o 00000xxxxxx1xxxx: Preset 4 req/evaluated in EQ Master Phase
                                                         o 00000xxxxx1xxxxx: Preset 5 req/evaluated in EQ Master Phase
                                                         o 00000xxxx1xxxxxx: Preset 6 req/evaluated in EQ Master Phase
                                                         o 00000xxx1xxxxxxx: Preset 7 req/evaluated in EQ Master Phase
                                                         o 00000xx1xxxxxxxx: Preset 8 req/evaluated in EQ Master Phase
                                                         o 00000x1xxxxxxxxx: Preset 9 req/evaluated in EQ Master Phase
                                                         o 000001xxxxxxxxxx: Preset 10 req/evaluated in EQ Master Phase
                                                         o all other encodings: Reserved */
	uint32_t reserved_6_7                 : 2;
	uint32_t p23td                        : 1;  /**< Phase2_3 2 ms Timeout Disable
                                                         Determine behavior in Phase2 for USP (Phase3 if DSP) when the
                                                         PHY does not respond within 2ms to the assertion of RxEqEval:
                                                         o 0: abort the current evaluation, stop any attempt to
                                                         modify the remote transmitter settings, Phase2 will be
                                                         terminated by the 24ms timeout
                                                         o 1: ignore the 2ms timeout and continue as normal. This is
                                                         used to support PHYs that require more than 2ms to
                                                         respond to the assertion of RxEqEval. */
	uint32_t bt                           : 1;  /**< Behavior After 24ms Timeout (When Optimal settings are not found)
                                                         FOR a USP:
                                                         Determine the next LTSSM state from Phase2
                                                         o 0: Recovery.Speed
                                                         o 1: Recovry.Equalization.Phase3
                                                         FOR a DSP:
                                                         Determine the next LTSSM state from Phase3
                                                         o 0: Recovery.Speed
                                                         o 1: Recovry.Equalization.RcrLock
                                                         When optimal settings are not found then
                                                         o Equalization Phase 3 Successful status bit is not set in the
                                                         Link Status Register
                                                         o Equalization Phase 3 Complete status bit is set in the
                                                         Link Status Register */
	uint32_t fm                           : 4;  /**< Feedback Mode
                                                         - 0: Direction of Change
                                                         - 1: Figure of Merit
                                                         - 2-15: Reserved */
#else
	uint32_t fm                           : 4;
	uint32_t bt                           : 1;
	uint32_t p23td                        : 1;
	uint32_t reserved_6_7                 : 2;
	uint32_t prv                          : 16;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_pcieepx_cfg554_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg554 cvmx_pcieepx_cfg554_t;

/**
 * cvmx_pcieep#_cfg558
 *
 * PCIE_CFG558 = Five hundred fifty ninth 32-bits of PCIE type 0 config space
 * (Gen3 PIPE Loopback Register)
 */
union cvmx_pcieepx_cfg558 {
	uint32_t u32;
	struct cvmx_pcieepx_cfg558_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ple                          : 1;  /**< Pipe Loopback Enable */
	uint32_t rxstatus                     : 31; /**< Reserved. */
#else
	uint32_t rxstatus                     : 31;
	uint32_t ple                          : 1;
#endif
	} s;
	struct cvmx_pcieepx_cfg558_s          cn78xx;
};
typedef union cvmx_pcieepx_cfg558 cvmx_pcieepx_cfg558_t;

#endif
