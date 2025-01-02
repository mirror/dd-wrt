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
 * cvmx-sriomaintx-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon sriomaintx.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_SRIOMAINTX_DEFS_H__
#define __CVMX_SRIOMAINTX_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ASMBLY_ID(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000008ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000008ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000008ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ASMBLY_ID (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000008ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ASMBLY_ID(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000008ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000008ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000008ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000008ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ASMBLY_INFO(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000000000Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000000000Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000000Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ASMBLY_INFO (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000000000Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ASMBLY_INFO(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000000000Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000000000Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000000000Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000000000Cull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_BAR1_IDXX(unsigned long offset, unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if (((offset <= 15)) && ((block_id <= 1)))
				return 0x0000010000200010ull + (((offset) & 15) + ((block_id) & 1) * 0x40000000ull) * 4;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if (((offset <= 15)) && ((block_id == 0) || (block_id == 2) || (block_id == 3)))
				return 0x0000000000200010ull + (((offset) & 15) + ((block_id) & 3) * 0x0ull) * 4;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if (((offset <= 15)) && ((block_id <= 1)))
				return 0x0000000000200010ull + (((offset) & 15) + ((block_id) & 1) * 0x40000000ull) * 4;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_BAR1_IDXX (%lu, %lu) not supported on this chip\n", offset, block_id);
	return 0x0000010000200010ull + (((offset) & 15) + ((block_id) & 1) * 0x40000000ull) * 4;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_BAR1_IDXX(unsigned long offset, unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000200010ull + ((offset) + (block_id) * 0x40000000ull) * 4;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200010ull + ((offset) + (block_id) * 0x0ull) * 4;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200010ull + ((offset) + (block_id) * 0x40000000ull) * 4;
	}
	return 0x0000010000200010ull + ((offset) + (block_id) * 0x40000000ull) * 4;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_BELL_STATUS(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000200080ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000200080ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000200080ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_BELL_STATUS (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000200080ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_BELL_STATUS(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000200080ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200080ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200080ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000200080ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_COMP_TAG(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000000006Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000000006Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000006Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_COMP_TAG (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000000006Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_COMP_TAG(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000000006Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000000006Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000000006Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000000006Cull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_CORE_ENABLES(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000200070ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000200070ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000200070ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_CORE_ENABLES (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000200070ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_CORE_ENABLES(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000200070ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200070ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200070ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000200070ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_DEV_ID(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000000ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000000ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000000ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_DEV_ID (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000000ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_DEV_ID(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000000ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000000ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000000ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000000ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_DEV_REV(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000004ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000004ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000004ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_DEV_REV (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000004ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_DEV_REV(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000004ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000004ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000004ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000004ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_DST_OPS(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000000001Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000000001Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000001Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_DST_OPS (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000000001Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_DST_OPS(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000000001Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000000001Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000000001Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000000001Cull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_ATTR_CAPT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000002048ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000002048ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000002048ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_ATTR_CAPT (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000002048ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_ATTR_CAPT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000002048ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002048ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002048ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000002048ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_ERR_DET(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000002040ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000002040ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000002040ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_ERR_DET (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000002040ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_ERR_DET(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000002040ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002040ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002040ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000002040ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_ERR_RATE(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000002068ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000002068ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000002068ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_ERR_RATE (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000002068ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_ERR_RATE(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000002068ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002068ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002068ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000002068ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_ERR_RATE_EN(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000002044ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000002044ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000002044ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_ERR_RATE_EN (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000002044ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_ERR_RATE_EN(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000002044ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002044ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002044ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000002044ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_ERR_RATE_THR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000000206Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000000206Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000206Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_ERR_RATE_THR (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000000206Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_ERR_RATE_THR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000000206Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000000206Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000000206Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000000206Cull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_HDR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000002000ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000002000ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000002000ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_HDR (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000002000ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_HDR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000002000ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002000ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002000ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000002000ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_LT_ADDR_CAPT_H(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000002010ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000002010ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000002010ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_LT_ADDR_CAPT_H (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000002010ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_LT_ADDR_CAPT_H(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000002010ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002010ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002010ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000002010ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_LT_ADDR_CAPT_L(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000002014ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000002014ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000002014ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_LT_ADDR_CAPT_L (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000002014ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_LT_ADDR_CAPT_L(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000002014ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002014ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002014ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000002014ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_LT_CTRL_CAPT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000000201Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000000201Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000201Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_LT_CTRL_CAPT (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000000201Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_LT_CTRL_CAPT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000000201Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000000201Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000000201Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000000201Cull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_LT_DEV_ID(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000002028ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000002028ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000002028ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_LT_DEV_ID (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000002028ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_LT_DEV_ID(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000002028ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002028ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002028ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000002028ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_LT_DEV_ID_CAPT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000002018ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000002018ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000002018ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_LT_DEV_ID_CAPT (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000002018ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_LT_DEV_ID_CAPT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000002018ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002018ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002018ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000002018ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_LT_ERR_DET(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000002008ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000002008ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000002008ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_LT_ERR_DET (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000002008ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_LT_ERR_DET(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000002008ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002008ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002008ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000002008ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_LT_ERR_EN(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000000200Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000000200Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000200Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_LT_ERR_EN (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000000200Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_LT_ERR_EN(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000000200Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000000200Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000000200Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000000200Cull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_PACK_CAPT_1(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000002050ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000002050ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000002050ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_PACK_CAPT_1 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000002050ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_PACK_CAPT_1(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000002050ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002050ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002050ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000002050ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_PACK_CAPT_2(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000002054ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000002054ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000002054ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_PACK_CAPT_2 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000002054ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_PACK_CAPT_2(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000002054ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002054ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002054ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000002054ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_PACK_CAPT_3(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000002058ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000002058ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000002058ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_PACK_CAPT_3 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000002058ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_PACK_CAPT_3(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000002058ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002058ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000002058ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000002058ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_ERB_PACK_SYM_CAPT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000000204Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000000204Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000204Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_ERB_PACK_SYM_CAPT (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000000204Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_ERB_PACK_SYM_CAPT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000000204Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000000204Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000000204Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000000204Cull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_HB_DEV_ID_LOCK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000068ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000068ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000068ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_HB_DEV_ID_LOCK (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000068ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_HB_DEV_ID_LOCK(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000068ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000068ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000068ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000068ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_IR_BUFFER_CONFIG(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000102000ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000102000ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000102000ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_IR_BUFFER_CONFIG (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000102000ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_IR_BUFFER_CONFIG(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000102000ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000102000ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000102000ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000102000ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_IR_BUFFER_CONFIG2(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000102004ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000102004ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000102004ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_IR_BUFFER_CONFIG2 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000102004ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_IR_BUFFER_CONFIG2(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000102004ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000102004ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000102004ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000102004ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_IR_PD_PHY_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000107028ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000107028ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000107028ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_IR_PD_PHY_CTRL (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000107028ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_IR_PD_PHY_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000107028ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107028ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107028ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000107028ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_IR_PD_PHY_STAT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000010702Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000010702Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000010702Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_IR_PD_PHY_STAT (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000010702Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_IR_PD_PHY_STAT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000010702Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000010702Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000010702Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000010702Cull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_IR_PI_PHY_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000107020ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000107020ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000107020ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_IR_PI_PHY_CTRL (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000107020ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_IR_PI_PHY_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000107020ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107020ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107020ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000107020ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_IR_PI_PHY_STAT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000107024ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000107024ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000107024ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_IR_PI_PHY_STAT (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000107024ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_IR_PI_PHY_STAT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000107024ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107024ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107024ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000107024ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_IR_SP_RX_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000010700Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000010700Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000010700Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_IR_SP_RX_CTRL (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000010700Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_IR_SP_RX_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000010700Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000010700Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000010700Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000010700Cull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_IR_SP_RX_DATA(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000107014ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000107014ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000107014ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_IR_SP_RX_DATA (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000107014ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_IR_SP_RX_DATA(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000107014ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107014ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107014ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000107014ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_IR_SP_RX_STAT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000107010ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000107010ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000107010ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_IR_SP_RX_STAT (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000107010ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_IR_SP_RX_STAT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000107010ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107010ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107010ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000107010ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_IR_SP_TX_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000107000ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000107000ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000107000ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_IR_SP_TX_CTRL (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000107000ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_IR_SP_TX_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000107000ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107000ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107000ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000107000ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_IR_SP_TX_DATA(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000107008ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000107008ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000107008ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_IR_SP_TX_DATA (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000107008ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_IR_SP_TX_DATA(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000107008ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107008ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107008ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000107008ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_IR_SP_TX_STAT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000107004ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000107004ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000107004ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_IR_SP_TX_STAT (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000107004ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_IR_SP_TX_STAT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000107004ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107004ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000107004ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000107004ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_LANE_X_STATUS_0(unsigned long offset, unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if (((offset <= 3)) && ((block_id <= 1)))
				return 0x0000010000001010ull + (((offset) & 3) + ((block_id) & 1) * 0x8000000ull) * 32;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if (((offset <= 3)) && ((block_id == 0) || (block_id == 2) || (block_id == 3)))
				return 0x0000000000001010ull + (((offset) & 3) + ((block_id) & 3) * 0x0ull) * 32;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if (((offset <= 3)) && ((block_id <= 1)))
				return 0x0000000000001010ull + (((offset) & 3) + ((block_id) & 1) * 0x8000000ull) * 32;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_LANE_X_STATUS_0 (%lu, %lu) not supported on this chip\n", offset, block_id);
	return 0x0000010000001010ull + (((offset) & 3) + ((block_id) & 1) * 0x8000000ull) * 32;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_LANE_X_STATUS_0(unsigned long offset, unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000001010ull + ((offset) + (block_id) * 0x8000000ull) * 32;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001010ull + ((offset) + (block_id) * 0x0ull) * 32;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001010ull + ((offset) + (block_id) * 0x8000000ull) * 32;
	}
	return 0x0000010000001010ull + ((offset) + (block_id) * 0x8000000ull) * 32;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_LCS_BA0(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000058ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000058ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000058ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_LCS_BA0 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000058ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_LCS_BA0(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000058ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000058ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000058ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000058ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_LCS_BA1(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000000005Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000000005Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000005Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_LCS_BA1 (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000000005Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_LCS_BA1(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000000005Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000000005Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000000005Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000000005Cull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_M2S_BAR0_START0(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000200000ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000200000ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000200000ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_M2S_BAR0_START0 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000200000ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_M2S_BAR0_START0(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000200000ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200000ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200000ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000200000ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_M2S_BAR0_START1(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000200004ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000200004ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000200004ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_M2S_BAR0_START1 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000200004ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_M2S_BAR0_START1(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000200004ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200004ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200004ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000200004ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_M2S_BAR1_START0(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000200008ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000200008ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000200008ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_M2S_BAR1_START0 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000200008ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_M2S_BAR1_START0(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000200008ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200008ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200008ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000200008ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_M2S_BAR1_START1(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000020000Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000020000Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000020000Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_M2S_BAR1_START1 (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000020000Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_M2S_BAR1_START1(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000020000Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000020000Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000020000Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000020000Cull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_M2S_BAR2_START(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000200050ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000200050ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000200050ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_M2S_BAR2_START (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000200050ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_M2S_BAR2_START(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000200050ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200050ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200050ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000200050ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_MAC_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000200068ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000200068ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000200068ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_MAC_CTRL (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000200068ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_MAC_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000200068ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200068ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200068ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000200068ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_PE_FEAT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000010ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000010ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000010ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_PE_FEAT (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000010ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_PE_FEAT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000010ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000010ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000010ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000010ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_PE_LLC(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000000004Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000000004Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000004Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_PE_LLC (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000000004Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_PE_LLC(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000000004Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000000004Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000000004Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000000004Cull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_PORT_0_CTL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000000015Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000000015Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000015Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_PORT_0_CTL (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000000015Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_PORT_0_CTL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000000015Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000000015Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000000015Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000000015Cull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_PORT_0_CTL2(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000154ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000154ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000154ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_PORT_0_CTL2 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000154ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_PORT_0_CTL2(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000154ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000154ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000154ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000154ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_PORT_0_ERR_STAT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000158ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000158ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000158ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_PORT_0_ERR_STAT (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000158ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_PORT_0_ERR_STAT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000158ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000158ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000158ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000158ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_PORT_0_LINK_REQ(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000140ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000140ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000140ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_PORT_0_LINK_REQ (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000140ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_PORT_0_LINK_REQ(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000140ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000140ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000140ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000140ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_PORT_0_LINK_RESP(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000144ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000144ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000144ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_PORT_0_LINK_RESP (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000144ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_PORT_0_LINK_RESP(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000144ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000144ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000144ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000144ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_PORT_0_LOCAL_ACKID(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000148ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000148ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000148ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_PORT_0_LOCAL_ACKID (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000148ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_PORT_0_LOCAL_ACKID(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000148ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000148ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000148ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000148ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_PORT_GEN_CTL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000000013Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000000013Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000013Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_PORT_GEN_CTL (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000000013Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_PORT_GEN_CTL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000000013Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000000013Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000000013Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000000013Cull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_PORT_LT_CTL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000120ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000120ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000120ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_PORT_LT_CTL (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000120ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_PORT_LT_CTL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000120ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000120ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000120ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000120ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_PORT_MBH0(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000100ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000100ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000100ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_PORT_MBH0 (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000100ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_PORT_MBH0(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000100ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000100ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000100ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000100ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_PORT_RT_CTL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000124ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000124ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000124ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_PORT_RT_CTL (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000124ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_PORT_RT_CTL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000124ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000124ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000124ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000124ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_PORT_TTL_CTL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000000012Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000000012Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000000012Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_PORT_TTL_CTL (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000000012Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_PORT_TTL_CTL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000000012Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000000012Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000000012Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000000012Cull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_PRI_DEV_ID(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000060ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000060ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000060ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_PRI_DEV_ID (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000060ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_PRI_DEV_ID(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000060ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000060ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000060ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000060ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_SEC_DEV_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000200064ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000200064ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000200064ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_SEC_DEV_CTRL (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000200064ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_SEC_DEV_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000200064ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200064ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200064ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000200064ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_SEC_DEV_ID(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000200060ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000200060ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000200060ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_SEC_DEV_ID (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000200060ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_SEC_DEV_ID(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000200060ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200060ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000200060ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000200060ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_SERIAL_LANE_HDR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000001000ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000001000ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000001000ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_SERIAL_LANE_HDR (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000001000ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_SERIAL_LANE_HDR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000001000ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001000ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000001000ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000001000ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_SRC_OPS(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000010000000018ull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x0000000000000018ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x0000000000000018ull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_SRC_OPS (offset = %lu) not supported on this chip\n", offset);
	return 0x0000010000000018ull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_SRC_OPS(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x0000010000000018ull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000018ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x0000000000000018ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000018ull + (offset) * 0x100000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOMAINTX_TX_DROP(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000001000020006Cull + ((offset) & 1) * 0x100000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return 0x000000000020006Cull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return 0x000000000020006Cull + ((offset) & 1) * 0x100000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOMAINTX_TX_DROP (offset = %lu) not supported on this chip\n", offset);
	return 0x000001000020006Cull + ((offset) & 1) * 0x100000000ull;
}
#else
static inline uint64_t CVMX_SRIOMAINTX_TX_DROP(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return 0x000001000020006Cull + (offset) * 0x100000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return 0x000000000020006Cull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return 0x000000000020006Cull + (offset) * 0x100000000ull;
	}
	return 0x000001000020006Cull + (offset) * 0x100000000ull;
}
#endif

/**
 * cvmx_sriomaint#_asmbly_id
 *
 * This register shows the assembly id and vendor specified in SRIO()_ASMBLY_ID.
 *
 */
union cvmx_sriomaintx_asmbly_id {
	uint32_t u32;
	struct cvmx_sriomaintx_asmbly_id_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t assy_id                      : 16; /**< Assembly identifer. */
	uint32_t assy_ven                     : 16; /**< Assembly vendor identifer. */
#else
	uint32_t assy_ven                     : 16;
	uint32_t assy_id                      : 16;
#endif
	} s;
	struct cvmx_sriomaintx_asmbly_id_s    cn63xx;
	struct cvmx_sriomaintx_asmbly_id_s    cn63xxp1;
	struct cvmx_sriomaintx_asmbly_id_s    cn66xx;
	struct cvmx_sriomaintx_asmbly_id_s    cnf75xx;
};
typedef union cvmx_sriomaintx_asmbly_id cvmx_sriomaintx_asmbly_id_t;

/**
 * cvmx_sriomaint#_asmbly_info
 *
 * This register shows the assembly revision specified in SRIO()_ASMBLY_INFO and
 * extended feature pointer.
 */
union cvmx_sriomaintx_asmbly_info {
	uint32_t u32;
	struct cvmx_sriomaintx_asmbly_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t assy_rev                     : 16; /**< Assembly revision. */
	uint32_t ext_fptr                     : 16; /**< Pointer to the first entry in the extended feature list. */
#else
	uint32_t ext_fptr                     : 16;
	uint32_t assy_rev                     : 16;
#endif
	} s;
	struct cvmx_sriomaintx_asmbly_info_s  cn63xx;
	struct cvmx_sriomaintx_asmbly_info_s  cn63xxp1;
	struct cvmx_sriomaintx_asmbly_info_s  cn66xx;
	struct cvmx_sriomaintx_asmbly_info_s  cnf75xx;
};
typedef union cvmx_sriomaintx_asmbly_info cvmx_sriomaintx_asmbly_info_t;

/**
 * cvmx_sriomaint#_bar1_idx#
 *
 * Contains address index and control bits for access to memory ranges of BAR1.
 * This register specifies the OCTEON address, endian swap and cache status associated with each
 * of the 16 BAR1 entries.  The local address bits used are based on the BARSIZE field located in
 * SRIOMAINT()_M2S_BAR1_START0.  This register is only writeable over SRIO if
 * SRIO()_ACC_CTRL[DENY_BAR1] is zero.
 */
union cvmx_sriomaintx_bar1_idxx {
	uint32_t u32;
	struct cvmx_sriomaintx_bar1_idxx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_6_31                : 26;
	uint32_t es                           : 2;  /**< Endian swap mode.
                                                         0x0 = No swap.
                                                         0x1 = 64-bit swap bytes [ABCD_EFGH] -> [HGFE_DCBA].
                                                         0x2 = 32-bit swap words [ABCD_EFGH] -> [DCBA_HGFE].
                                                         0x3 = 32-bit word exch  [ABCD_EFGH] -> [EFGH_ABCD]. */
	uint32_t nca                          : 1;  /**< Non-cacheable access mode.  When set, transfers
                                                         through this window are not cacheable. */
	uint32_t reserved_1_2                 : 2;
	uint32_t enable                       : 1;  /**< When set the selected index address is valid. */
#else
	uint32_t enable                       : 1;
	uint32_t reserved_1_2                 : 2;
	uint32_t nca                          : 1;
	uint32_t es                           : 2;
	uint32_t reserved_6_31                : 26;
#endif
	} s;
	struct cvmx_sriomaintx_bar1_idxx_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t la                           : 22; /**< L2/DRAM Address bits [37:16]
                                                         Not all LA[21:0] bits are used by SRIO hardware,
                                                         depending on SRIOMAINT(0..1)_M2S_BAR1_START1[BARSIZE].

                                                                                 Become
                                                                                 L2/DRAM
                                                                                 Address  Entry
                                                         BARSIZE   LA Bits Used   Bits    Size
                                                            0        LA[21:0]    [37:16]   64KB
                                                            1        LA[21:1]    [37:17]  128KB
                                                            2        LA[21:2]    [37:18]  256KB
                                                            3        LA[21:3]    [37:19]  512KB
                                                            4        LA[21:4]    [37:20]    1MB
                                                            5        LA[21:5]    [37:21]    2MB
                                                            6        LA[21:6]    [37:22]    4MB
                                                            7        LA[21:7]    [37:23]    8MB
                                                            8        LA[21:8]    [37:24]   16MB
                                                            9        LA[21:9]    [37:25]   32MB
                                                           10        LA[21:10]   [37:26]   64MB
                                                           11        LA[21:11]   [37:27]  128MB
                                                           12        LA[21:12]   [37:28]  256MB
                                                           13        LA[21:13]   [37:29]  512MB */
	uint32_t reserved_6_7                 : 2;
	uint32_t es                           : 2;  /**< Endian Swap Mode.
                                                         0 = No Swap
                                                         1 = 64-bit Swap Bytes [ABCD_EFGH] -> [HGFE_DCBA]
                                                         2 = 32-bit Swap Words [ABCD_EFGH] -> [DCBA_HGFE]
                                                         3 = 32-bit Word Exch  [ABCD_EFGH] -> [EFGH_ABCD] */
	uint32_t nca                          : 1;  /**< Non-Cacheable Access Mode.  When set, transfers
                                                         through this window are not cacheable. */
	uint32_t reserved_1_2                 : 2;
	uint32_t enable                       : 1;  /**< When set the selected index address is valid. */
#else
	uint32_t enable                       : 1;
	uint32_t reserved_1_2                 : 2;
	uint32_t nca                          : 1;
	uint32_t es                           : 2;
	uint32_t reserved_6_7                 : 2;
	uint32_t la                           : 22;
	uint32_t reserved_30_31               : 2;
#endif
	} cn63xx;
	struct cvmx_sriomaintx_bar1_idxx_cn63xx cn63xxp1;
	struct cvmx_sriomaintx_bar1_idxx_cn63xx cn66xx;
	struct cvmx_sriomaintx_bar1_idxx_cnf75xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t la                           : 26; /**< L2/DRAM Address bits [41:16].
                                                         Not all LA[25:0] bits are used by SRIO hardware,
                                                         depending on SRIOMAINT()_M2S_BAR1_START1[BARSIZE].
                                                         <pre>
                                                         Become
                                                         L2/DRAM
                                                         Address  Entry
                                                         BARSIZE  LA Bits Used   Bits     Size
                                                         0        LA[25:0]    [41:16]     64KB
                                                         1        LA[25:1]    [41:17]    128KB
                                                         2        LA[25:2]    [41:18]    256KB
                                                         3        LA[25:3]    [41:19]    512KB
                                                         4        LA[25:4]    [41:20]      1MB
                                                         5        LA[25:5]    [41:21]      2MB
                                                         6        LA[25:6]    [41:22]      4MB
                                                         7        LA[25:7]    [41:23]      8MB
                                                         8        LA[25:8]    [41:24]     16MB
                                                         9        LA[25:9]    [41:25]     32MB
                                                         10       LA[25:10]   [41:26]     64MB
                                                         11       LA[25:11]   [41:27]    128MB
                                                         12       LA[25:12]   [41:28]    256MB
                                                         13       LA[25:13]   [41:29]    512MB
                                                         </pre> */
	uint32_t es                           : 2;  /**< Endian swap mode.
                                                         0x0 = No swap.
                                                         0x1 = 64-bit swap bytes [ABCD_EFGH] -> [HGFE_DCBA].
                                                         0x2 = 32-bit swap words [ABCD_EFGH] -> [DCBA_HGFE].
                                                         0x3 = 32-bit word exch  [ABCD_EFGH] -> [EFGH_ABCD]. */
	uint32_t nca                          : 1;  /**< Non-cacheable access mode.  When set, transfers
                                                         through this window are not cacheable. */
	uint32_t reserved_1_2                 : 2;
	uint32_t enable                       : 1;  /**< When set the selected index address is valid. */
#else
	uint32_t enable                       : 1;
	uint32_t reserved_1_2                 : 2;
	uint32_t nca                          : 1;
	uint32_t es                           : 2;
	uint32_t la                           : 26;
#endif
	} cnf75xx;
};
typedef union cvmx_sriomaintx_bar1_idxx cvmx_sriomaintx_bar1_idxx_t;

/**
 * cvmx_sriomaint#_bell_status
 *
 * This register displays the status of the doorbells received. If FULL is set the SRIO
 * device will retry incoming transactions.
 */
union cvmx_sriomaintx_bell_status {
	uint32_t u32;
	struct cvmx_sriomaintx_bell_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_1_31                : 31;
	uint32_t full                         : 1;  /**< Not able to receive doorbell transactions. */
#else
	uint32_t full                         : 1;
	uint32_t reserved_1_31                : 31;
#endif
	} s;
	struct cvmx_sriomaintx_bell_status_s  cn63xx;
	struct cvmx_sriomaintx_bell_status_s  cn63xxp1;
	struct cvmx_sriomaintx_bell_status_s  cn66xx;
	struct cvmx_sriomaintx_bell_status_s  cnf75xx;
};
typedef union cvmx_sriomaintx_bell_status cvmx_sriomaintx_bell_status_t;

/**
 * cvmx_sriomaint#_comp_tag
 *
 * This register contains a component tag value for the processing element and the value can be
 * assigned by software when the device is initialized.
 */
union cvmx_sriomaintx_comp_tag {
	uint32_t u32;
	struct cvmx_sriomaintx_comp_tag_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t comp_tag                     : 32; /**< Component tag for firmware use. */
#else
	uint32_t comp_tag                     : 32;
#endif
	} s;
	struct cvmx_sriomaintx_comp_tag_s     cn63xx;
	struct cvmx_sriomaintx_comp_tag_s     cn63xxp1;
	struct cvmx_sriomaintx_comp_tag_s     cn66xx;
	struct cvmx_sriomaintx_comp_tag_s     cnf75xx;
};
typedef union cvmx_sriomaintx_comp_tag cvmx_sriomaintx_comp_tag_t;

/**
 * cvmx_sriomaint#_core_enables
 *
 * This register displays the reset state of the OCTEON core logic while the SRIO link is
 * running.
 * The bit should be set after the software has initialized the chip to allow memory operations.
 */
union cvmx_sriomaintx_core_enables {
	uint32_t u32;
	struct cvmx_sriomaintx_core_enables_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_5_31                : 27;
	uint32_t halt                         : 1;  /**< CNXXXX currently in reset.
                                                         0 = All CNXXXX resources are available.
                                                         1 = The CNXXXX is in reset. When this bit is set,
                                                         SRIO maintenance registers can be accessed,
                                                         but BAR0, BAR1, and BAR2 cannot be. */
	uint32_t imsg1                        : 1;  /**< Allow incoming message unit 1 operations.
                                                         Note: This bit is cleared when the CNXXXX is reset.
                                                         0 = SRIO incoming messages to unit 1 ignored and
                                                         return error response.
                                                         1 = SRIO incoming messages to unit 1 allowed. */
	uint32_t imsg0                        : 1;  /**< Allow incoming message unit 0 operations
                                                         Note: This bit is cleared when the CNXXX is reset.
                                                         0 = SRIO incoming messages to unit 0 ignored and
                                                         return error response.
                                                         1 = SRIO incoming messages to unit 0 allowed. */
	uint32_t doorbell                     : 1;  /**< Allow inbound doorbell operations
                                                         Note: This bit is cleared when the CNXXXX is reset.
                                                         0 = SRIO doorbell OPs ignored and return error
                                                         response.
                                                         1 = SRIO doorbell OPs allowed. */
	uint32_t memory                       : 1;  /**< Allow inbound/outbound memory operations.
                                                         Note: This bit is cleared when the CNXXXX is reset.
                                                         0 = SRIO incoming nwrites and swrites are
                                                         dropped.  Incoming nreads, atomics and
                                                         nwriters return responses with ERROR status.
                                                         SRIO incoming maintenance BAR memory accesses
                                                         are processed normally.
                                                         Outgoing store operations are dropped.
                                                         Outgoing load operations are not issued and
                                                         return all 1's with an ERROR status.
                                                         In flight operations started while the bit is
                                                         set in both directions will complete normally.
                                                         1 = SRIO memory read/write OPs allowed */
#else
	uint32_t memory                       : 1;
	uint32_t doorbell                     : 1;
	uint32_t imsg0                        : 1;
	uint32_t imsg1                        : 1;
	uint32_t halt                         : 1;
	uint32_t reserved_5_31                : 27;
#endif
	} s;
	struct cvmx_sriomaintx_core_enables_s cn63xx;
	struct cvmx_sriomaintx_core_enables_s cn63xxp1;
	struct cvmx_sriomaintx_core_enables_s cn66xx;
	struct cvmx_sriomaintx_core_enables_s cnf75xx;
};
typedef union cvmx_sriomaintx_core_enables cvmx_sriomaintx_core_enables_t;

/**
 * cvmx_sriomaint#_dev_id
 *
 * This register identifies the vendor that manufactured the device.
 *
 */
union cvmx_sriomaintx_dev_id {
	uint32_t u32;
	struct cvmx_sriomaintx_dev_id_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t device                       : 16; /**< Product identity. */
	uint32_t vendor                       : 16; /**< Cavium vendor identity. */
#else
	uint32_t vendor                       : 16;
	uint32_t device                       : 16;
#endif
	} s;
	struct cvmx_sriomaintx_dev_id_s       cn63xx;
	struct cvmx_sriomaintx_dev_id_s       cn63xxp1;
	struct cvmx_sriomaintx_dev_id_s       cn66xx;
	struct cvmx_sriomaintx_dev_id_s       cnf75xx;
};
typedef union cvmx_sriomaintx_dev_id cvmx_sriomaintx_dev_id_t;

/**
 * cvmx_sriomaint#_dev_rev
 *
 * The device revision register identifies the chip pass and revision.
 *
 */
union cvmx_sriomaintx_dev_rev {
	uint32_t u32;
	struct cvmx_sriomaintx_dev_rev_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_8_31                : 24;
	uint32_t revision                     : 8;  /**< Chip pass/revision. */
#else
	uint32_t revision                     : 8;
	uint32_t reserved_8_31                : 24;
#endif
	} s;
	struct cvmx_sriomaintx_dev_rev_s      cn63xx;
	struct cvmx_sriomaintx_dev_rev_s      cn63xxp1;
	struct cvmx_sriomaintx_dev_rev_s      cn66xx;
	struct cvmx_sriomaintx_dev_rev_s      cnf75xx;
};
typedef union cvmx_sriomaintx_dev_rev cvmx_sriomaintx_dev_rev_t;

/**
 * cvmx_sriomaint#_dst_ops
 *
 * The logical operations supported from external devices.   This register shows
 * the operations specified in SRIO()_IP_FEATURE[OPS].
 */
union cvmx_sriomaintx_dst_ops {
	uint32_t u32;
	struct cvmx_sriomaintx_dst_ops_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t gsm_read                     : 1;  /**< PE does not support read home operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<31>. */
	uint32_t i_read                       : 1;  /**< PE does not support instruction read.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<30>. */
	uint32_t rd_own                       : 1;  /**< PE does not support read for ownership.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<29>. */
	uint32_t d_invald                     : 1;  /**< PE does not support data cache invalidate.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<28>. */
	uint32_t castout                      : 1;  /**< PE does not support castout operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<27>. */
	uint32_t d_flush                      : 1;  /**< PE does not support data cache flush.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<26>. */
	uint32_t io_read                      : 1;  /**< PE does not support IO read.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<25>. */
	uint32_t i_invald                     : 1;  /**< PE does not support instruction cache invalidate.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<24>. */
	uint32_t tlb_inv                      : 1;  /**< PE does not support TLB entry invalidate.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<23>. */
	uint32_t tlb_invs                     : 1;  /**< PE does not support TLB entry invalidate sync.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<22>. */
	uint32_t reserved_16_21               : 6;
	uint32_t read                         : 1;  /**< PE can support Nread operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS<15>] */
	uint32_t write                        : 1;  /**< PE can support Nwrite operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<14>. */
	uint32_t swrite                       : 1;  /**< PE can support Swrite operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<13>. */
	uint32_t write_r                      : 1;  /**< PE can support write with response operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<12>. */
	uint32_t msg                          : 1;  /**< PE can support data message operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<11>. */
	uint32_t doorbell                     : 1;  /**< PE can support doorbell operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<10>. */
	uint32_t compswap                     : 1;  /**< PE does not support atomic compare and swap.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<9>. */
	uint32_t testswap                     : 1;  /**< PE does not support atomic test and swap.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<8>. */
	uint32_t atom_inc                     : 1;  /**< PE can support atomic increment operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<7>. */
	uint32_t atom_dec                     : 1;  /**< PE can support atomic decrement operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<6>. */
	uint32_t atom_set                     : 1;  /**< PE can support atomic set operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<5>. */
	uint32_t atom_clr                     : 1;  /**< PE can support atomic clear operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<4>. */
	uint32_t atom_swp                     : 1;  /**< PE does not support atomic swap.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<3>. */
	uint32_t port_wr                      : 1;  /**< PE can port write operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<2>. */
	uint32_t reserved_0_1                 : 2;
#else
	uint32_t reserved_0_1                 : 2;
	uint32_t port_wr                      : 1;
	uint32_t atom_swp                     : 1;
	uint32_t atom_clr                     : 1;
	uint32_t atom_set                     : 1;
	uint32_t atom_dec                     : 1;
	uint32_t atom_inc                     : 1;
	uint32_t testswap                     : 1;
	uint32_t compswap                     : 1;
	uint32_t doorbell                     : 1;
	uint32_t msg                          : 1;
	uint32_t write_r                      : 1;
	uint32_t swrite                       : 1;
	uint32_t write                        : 1;
	uint32_t read                         : 1;
	uint32_t reserved_16_21               : 6;
	uint32_t tlb_invs                     : 1;
	uint32_t tlb_inv                      : 1;
	uint32_t i_invald                     : 1;
	uint32_t io_read                      : 1;
	uint32_t d_flush                      : 1;
	uint32_t castout                      : 1;
	uint32_t d_invald                     : 1;
	uint32_t rd_own                       : 1;
	uint32_t i_read                       : 1;
	uint32_t gsm_read                     : 1;
#endif
	} s;
	struct cvmx_sriomaintx_dst_ops_s      cn63xx;
	struct cvmx_sriomaintx_dst_ops_s      cn63xxp1;
	struct cvmx_sriomaintx_dst_ops_s      cn66xx;
	struct cvmx_sriomaintx_dst_ops_s      cnf75xx;
};
typedef union cvmx_sriomaintx_dst_ops cvmx_sriomaintx_dst_ops_t;

/**
 * cvmx_sriomaint#_erb_attr_capt
 *
 * This register contains the information captured during the error.
 * The hardware will not update this register (i.e. this register is locked) while
 * VALID is set in this CSR.
 * The hardware sets SRIO_INT_REG[PHY_ERB] every time it sets VALID in this CSR.
 * To handle the interrupt, the following procedure may be best:
 *
 * (1) read this CSR, corresponding SRIOMAINT()_ERB_ERR_DET, SRIOMAINT()_ERB_PACK_SYM_CAPT,
 * SRIOMAINT()_ERB_PACK_CAPT_1, SRIOMAINT()_ERB_PACK_CAPT_2, and SRIOMAINT()_ERB_PACK_CAPT_3.
 * (2) Write VALID in this CSR to 0.
 * (3) clear SRIO_INT_REG[PHY_ERB].
 */
union cvmx_sriomaintx_erb_attr_capt {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_attr_capt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t inf_type                     : 3;  /**< Type of information logged.
                                                         0x0 = Packet.
                                                         0x2 = Short control symbol
                                                         (use only first capture register).
                                                         0x4 = Implementation specific error reporting.
                                                         else = Reserved */
	uint32_t err_type                     : 5;  /**< The encoded value of the 31 minus the bit in
                                                         SRIOMAINT()_ERB_ERR_DET that describes the error
                                                         captured in SRIOMAINT()_ERB_()CAPT Registers.
                                                         (For example a value of 5 indicates 31-5 = bit 26). */
	uint32_t err_info                     : 20; /**< Error info.
                                                         <pre>
                                                         ERR_TYPE Bits   Description
                                                         -------- ------ -----------
                                                         0        23     TX Protocol Error
                                                                  22     RX Protocol Error
                                                                  21     TX Link Response Timeout
                                                                  20     TX ACKID Timeout
                                                                  - 19:16  Reserved
                                                                  - 15:12  TX Protocol ID
                                                                           1 = Rcvd Unexpected Link Response
                                                                           2 = Rcvd Link Response before Req
                                                                           3 = Rcvd NACK servicing NACK
                                                                           4 = Rcvd NACK
                                                                           5 = Rcvd RETRY servicing RETRY
                                                                           6 = Rcvd RETRY servicing NACK
                                                                           7 = Rcvd ACK servicing RETRY
                                                                           8 = Rcvd ACK servicing NACK
                                                                           9 = Unexp ACKID on ACK or RETRY
                                                                           10 = Unexp ACK or RETRY
                                                                 - 11:8   Reserved
                                                                - 7:4   RX Protocol ID
                                                                           1 = Rcvd EOP w/o Prev SOP
                                                                           2 = Rcvd STOMP w/o Prev SOP
                                                                           3 = Unexp RESTART
                                                                           4 = Redundant Status from LinkReq
                                                         9-16   23:20  RX K Bits
                                                                - 19:0   Reserved
                                                         26     23:20  RX K Bits
                                                                - 19:0   Reserved
                                                         27     23:12  Type
                                                                        0x000 TX
                                                                        0x010 RX
                                                                - 11:8   RX or TX Protocol ID (see above)
                                                                - 7:4   Reserved
                                                         30     23:20  RX K Bits
                                                                - 19:0   Reserved
                                                         31     23:16  ACKID Timeout 0x2
                                                                - 15:14  Reserved
                                                                - 13:8   AckID
                                                                - 7:4   Reserved
                                                         </pre>
                                                         All others ERR_TYPEs are reserved. */
	uint32_t reserved_1_3                 : 3;
	uint32_t valid                        : 1;  /**< This bit is set by hardware to indicate that the
                                                         packet/control symbol capture registers contain
                                                         valid information. For control symbols, only
                                                         capture register 0 will contain meaningful
                                                         information.  This bit must be cleared by software
                                                         to allow capture of other errors. */
#else
	uint32_t valid                        : 1;
	uint32_t reserved_1_3                 : 3;
	uint32_t err_info                     : 20;
	uint32_t err_type                     : 5;
	uint32_t inf_type                     : 3;
#endif
	} s;
	struct cvmx_sriomaintx_erb_attr_capt_s cn63xx;
	struct cvmx_sriomaintx_erb_attr_capt_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t inf_type                     : 3;  /**< Type of Information Logged.
                                                         000 - Packet
                                                         010 - Short Control Symbol
                                                               (use only first capture register)
                                                         All Others Reserved */
	uint32_t err_type                     : 5;  /**< The encoded value of the 31 minus the bit in
                                                         SRIOMAINT(0..1)_ERB_ERR_DET that describes the error
                                                         captured in SRIOMAINT(0..1)_ERB_*CAPT Registers.
                                                         (For example a value of 5 indicates 31-5 = bit 26) */
	uint32_t reserved_1_23                : 23;
	uint32_t valid                        : 1;  /**< This bit is set by hardware to indicate that the
                                                         Packet/control symbol capture registers contain
                                                         valid information. For control symbols, only
                                                         capture register 0 will contain meaningful
                                                         information.  This bit must be cleared by software
                                                         to allow capture of other errors. */
#else
	uint32_t valid                        : 1;
	uint32_t reserved_1_23                : 23;
	uint32_t err_type                     : 5;
	uint32_t inf_type                     : 3;
#endif
	} cn63xxp1;
	struct cvmx_sriomaintx_erb_attr_capt_s cn66xx;
	struct cvmx_sriomaintx_erb_attr_capt_s cnf75xx;
};
typedef union cvmx_sriomaintx_erb_attr_capt cvmx_sriomaintx_erb_attr_capt_t;

/**
 * cvmx_sriomaint#_erb_err_det
 *
 * The Error Detect Register indicates physical layer transmission errors detected by the
 * hardware. The hardware will not update this register (i.e. this register is locked) while
 * SRIOMAINT()_ERB_ATTR_CAPT[VALID] is set.
 */
union cvmx_sriomaintx_erb_err_det {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_err_det_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t imp_err                      : 1;  /**< Implementation specific error. */
	uint32_t reserved_23_30               : 8;
	uint32_t ctl_crc                      : 1;  /**< Received a control symbol with a bad CRC value.
                                                         Complete symbol in SRIOMAINT()_ERB_PACK_SYM_CAPT. */
	uint32_t uns_id                       : 1;  /**< Received an acknowledge control symbol with an
                                                         unexpected ackID (packet-accepted or packet_retry).
                                                         Partial symbol in SRIOMAINT()_ERB_PACK_SYM_CAPT. */
	uint32_t nack                         : 1;  /**< Received packet-not-accepted acknowledge control
                                                         symbols.
                                                         Partial symbol in SRIOMAINT()_ERB_PACK_SYM_CAPT. */
	uint32_t out_ack                      : 1;  /**< Received packet with unexpected ackID value.
                                                         Header in SRIOMAINT()_ERB_PACK_SYM_CAPT. */
	uint32_t pkt_crc                      : 1;  /**< Received a packet with a bad CRC value.
                                                         Header in SRIOMAINT()_ERB_PACK_SYM_CAPT. */
	uint32_t size                         : 1;  /**< Received packet which exceeds the maximum allowed
                                                         size of 276 bytes.
                                                         Header in SRIOMAINT()_ERB_PACK_SYM_CAPT. */
	uint32_t inv_char                     : 1;  /**< Received illegal, 8B/10B error  or undefined
                                                         codegroup within a packet.
                                                         Header in SRIOMAINT()_ERB_PACK_SYM_CAPT. */
	uint32_t inv_data                     : 1;  /**< Received data codegroup or 8B/10B error within an
                                                         IDLE sequence.
                                                         Header in SRIOMAINT()_ERB_PACK_SYM_CAPT */
	uint32_t reserved_6_14                : 9;
	uint32_t bad_ack                      : 1;  /**< Link_response received with an ackID that is not
                                                         outstanding.
                                                         Partial symbol in SRIOMAINT()_ERB_PACK_SYM_CAPT. */
	uint32_t proterr                      : 1;  /**< An unexpected packet or control symbol was
                                                         received.
                                                         Partial symbol in SRIOMAINT()_ERB_PACK_SYM_CAPT. */
	uint32_t f_toggle                     : 1;  /**< Reserved. */
	uint32_t del_err                      : 1;  /**< Received illegal or undefined codegroup
                                                         (either INV_DATA or INV_CHAR).
                                                         Complete symbol in SRIOMAINT()_ERB_PACK_SYM_CAPT. */
	uint32_t uns_ack                      : 1;  /**< An unexpected acknowledge control symbol was received.
                                                         Partial symbol in SRIOMAINT()_ERB_PACK_SYM_CAPT. */
	uint32_t lnk_tout                     : 1;  /**< An acknowledge or link-response control symbol is
                                                         not received within the specified timeout interval
                                                         Partial header in SRIOMAINT()_ERB_PACK_SYM_CAPT. */
#else
	uint32_t lnk_tout                     : 1;
	uint32_t uns_ack                      : 1;
	uint32_t del_err                      : 1;
	uint32_t f_toggle                     : 1;
	uint32_t proterr                      : 1;
	uint32_t bad_ack                      : 1;
	uint32_t reserved_6_14                : 9;
	uint32_t inv_data                     : 1;
	uint32_t inv_char                     : 1;
	uint32_t size                         : 1;
	uint32_t pkt_crc                      : 1;
	uint32_t out_ack                      : 1;
	uint32_t nack                         : 1;
	uint32_t uns_id                       : 1;
	uint32_t ctl_crc                      : 1;
	uint32_t reserved_23_30               : 8;
	uint32_t imp_err                      : 1;
#endif
	} s;
	struct cvmx_sriomaintx_erb_err_det_s  cn63xx;
	struct cvmx_sriomaintx_erb_err_det_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_23_31               : 9;
	uint32_t ctl_crc                      : 1;  /**< Received a control symbol with a bad CRC value
                                                         Complete Symbol in SRIOMAINT(0..1)_ERB_PACK_SYM_CAPT */
	uint32_t uns_id                       : 1;  /**< Received an acknowledge control symbol with an
                                                         unexpected ackID (packet-accepted or packet_retry)
                                                         Partial Symbol in SRIOMAINT(0..1)_ERB_PACK_SYM_CAPT */
	uint32_t nack                         : 1;  /**< Received packet-not-accepted acknowledge control
                                                         symbols.
                                                         Partial Symbol in SRIOMAINT(0..1)_ERB_PACK_SYM_CAPT */
	uint32_t out_ack                      : 1;  /**< Received packet with unexpected ackID value
                                                         Header in SRIOMAINT(0..1)_ERB_PACK_SYM_CAPT */
	uint32_t pkt_crc                      : 1;  /**< Received a packet with a bad CRC value
                                                         Header in SRIOMAINT(0..1)_ERB_PACK_SYM_CAPT */
	uint32_t size                         : 1;  /**< Received packet which exceeds the maximum allowed
                                                         size of 276 bytes.
                                                         Header in SRIOMAINT(0..1)_ERB_PACK_SYM_CAPT */
	uint32_t reserved_6_16                : 11;
	uint32_t bad_ack                      : 1;  /**< Link_response received with an ackID that is not
                                                         outstanding.
                                                         Partial Symbol in SRIOMAINT(0..1)_ERB_PACK_SYM_CAPT */
	uint32_t proterr                      : 1;  /**< An unexpected packet or control symbol was
                                                         received.
                                                         Partial Symbol in SRIOMAINT(0..1)_ERB_PACK_SYM_CAPT */
	uint32_t f_toggle                     : 1;  /**< Reserved. */
	uint32_t del_err                      : 1;  /**< Received illegal or undefined codegroup.
                                                         (either INV_DATA or INV_CHAR) (Pass 2)
                                                         Complete Symbol in SRIOMAINT(0..1)_ERB_PACK_SYM_CAPT */
	uint32_t uns_ack                      : 1;  /**< An unexpected acknowledge control symbol was
                                                         received.
                                                         Partial Symbol in SRIOMAINT(0..1)_ERB_PACK_SYM_CAPT */
	uint32_t lnk_tout                     : 1;  /**< An acknowledge or link-response control symbol is
                                                         not received within the specified timeout interval
                                                         Partial Header in SRIOMAINT(0..1)_ERB_PACK_SYM_CAPT */
#else
	uint32_t lnk_tout                     : 1;
	uint32_t uns_ack                      : 1;
	uint32_t del_err                      : 1;
	uint32_t f_toggle                     : 1;
	uint32_t proterr                      : 1;
	uint32_t bad_ack                      : 1;
	uint32_t reserved_6_16                : 11;
	uint32_t size                         : 1;
	uint32_t pkt_crc                      : 1;
	uint32_t out_ack                      : 1;
	uint32_t nack                         : 1;
	uint32_t uns_id                       : 1;
	uint32_t ctl_crc                      : 1;
	uint32_t reserved_23_31               : 9;
#endif
	} cn63xxp1;
	struct cvmx_sriomaintx_erb_err_det_s  cn66xx;
	struct cvmx_sriomaintx_erb_err_det_s  cnf75xx;
};
typedef union cvmx_sriomaintx_erb_err_det cvmx_sriomaintx_erb_err_det_t;

/**
 * cvmx_sriomaint#_erb_err_rate
 *
 * This register is used with the error rate threshold register to monitor and control
 * the reporting of transmission errors.
 */
union cvmx_sriomaintx_erb_err_rate {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_err_rate_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t err_bias                     : 8;  /**< These bits provide the error rate bias value.
                                                         0x00 = do not decrement the error rate counter.
                                                         0x01 = decrement every 1ms (+/-34%).
                                                         0x02 = decrement every 10ms (+/-34%).
                                                         0x04 = decrement every 100ms (+/-34%).
                                                         0x08 = decrement every 1s (+/-34%).
                                                         0x10 = decrement every 10s (+/-34%).
                                                         0x20 = decrement every 100s (+/-34%).
                                                         0x40 = decrement every 1000s (+/-34%).
                                                         0x80 = decrement every 10000s (+/-34%).
                                                         _ All other values are reserved */
	uint32_t reserved_18_23               : 6;
	uint32_t rate_lim                     : 2;  /**< These bits limit the incrementing of the error
                                                         rate counter above the failed threshold trigger.
                                                         0x0 = only count 2 errors above.
                                                         0x1 = only count 4 errors above.
                                                         0x2 = only count 16 error above.
                                                         0x3 = do not limit incrementing the error rate count. */
	uint32_t pk_rate                      : 8;  /**< Peak value attainted by the error rate counter */
	uint32_t rate_cnt                     : 8;  /**< These bits maintain a count of the number of transmission errors that have been
                                                         detected by the port, decremented by the error rate bias mechanism, to create an
                                                         indication of the link error rate. */
#else
	uint32_t rate_cnt                     : 8;
	uint32_t pk_rate                      : 8;
	uint32_t rate_lim                     : 2;
	uint32_t reserved_18_23               : 6;
	uint32_t err_bias                     : 8;
#endif
	} s;
	struct cvmx_sriomaintx_erb_err_rate_s cn63xx;
	struct cvmx_sriomaintx_erb_err_rate_s cn63xxp1;
	struct cvmx_sriomaintx_erb_err_rate_s cn66xx;
	struct cvmx_sriomaintx_erb_err_rate_s cnf75xx;
};
typedef union cvmx_sriomaintx_erb_err_rate cvmx_sriomaintx_erb_err_rate_t;

/**
 * cvmx_sriomaint#_erb_err_rate_en
 *
 * This register contains the bits that control when an error condition is allowed to increment
 * the error rate counter in the error rate threshold register and lock the error capture
 * registers.
 */
union cvmx_sriomaintx_erb_err_rate_en {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_err_rate_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t imp_err                      : 1;  /**< Enable implementation specific error. */
	uint32_t reserved_23_30               : 8;
	uint32_t ctl_crc                      : 1;  /**< Enable error rate counting of control symbols with
                                                         bad CRC values. */
	uint32_t uns_id                       : 1;  /**< Enable error rate counting of acknowledge control
                                                         symbol with unexpected ackIDs
                                                         (packet-accepted or packet_retry). */
	uint32_t nack                         : 1;  /**< Enable error rate counting of packet-not-accepted
                                                         acknowledge control symbols. */
	uint32_t out_ack                      : 1;  /**< Enable error rate counting of received packet with
                                                         unexpected ackID value. */
	uint32_t pkt_crc                      : 1;  /**< Enable error rate counting of received a packet
                                                         with a bad CRC value. */
	uint32_t size                         : 1;  /**< Enable error rate counting of received packet
                                                         which exceeds the maximum size of 276 bytes. */
	uint32_t inv_char                     : 1;  /**< Enable error rate counting of received illegal
                                                         8B/10B error or undefined codegroup within a
                                                         packet. */
	uint32_t inv_data                     : 1;  /**< Enable error rate counting of received data
                                                         codegroup or 8B/10B error within IDLE sequence. */
	uint32_t reserved_6_14                : 9;
	uint32_t bad_ack                      : 1;  /**< Enable error rate counting of link_responses with
                                                         an ackID that is not outstanding. */
	uint32_t proterr                      : 1;  /**< Enable error rate counting of unexpected packet or
                                                         control symbols received. */
	uint32_t f_toggle                     : 1;  /**< Reserved. */
	uint32_t del_err                      : 1;  /**< Enable error rate counting of illegal or undefined
                                                         codegroups (either INV_DATA or INV_CHAR). */
	uint32_t uns_ack                      : 1;  /**< Enable error rate counting of unexpected
                                                         acknowledge control symbols received. */
	uint32_t lnk_tout                     : 1;  /**< Enable error rate counting of acknowledge or
                                                         link-response control symbols not received within
                                                         the specified timeout interval. */
#else
	uint32_t lnk_tout                     : 1;
	uint32_t uns_ack                      : 1;
	uint32_t del_err                      : 1;
	uint32_t f_toggle                     : 1;
	uint32_t proterr                      : 1;
	uint32_t bad_ack                      : 1;
	uint32_t reserved_6_14                : 9;
	uint32_t inv_data                     : 1;
	uint32_t inv_char                     : 1;
	uint32_t size                         : 1;
	uint32_t pkt_crc                      : 1;
	uint32_t out_ack                      : 1;
	uint32_t nack                         : 1;
	uint32_t uns_id                       : 1;
	uint32_t ctl_crc                      : 1;
	uint32_t reserved_23_30               : 8;
	uint32_t imp_err                      : 1;
#endif
	} s;
	struct cvmx_sriomaintx_erb_err_rate_en_s cn63xx;
	struct cvmx_sriomaintx_erb_err_rate_en_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_23_31               : 9;
	uint32_t ctl_crc                      : 1;  /**< Enable error rate counting of control symbols with
                                                         bad CRC values */
	uint32_t uns_id                       : 1;  /**< Enable error rate counting of acknowledge control
                                                         symbol with unexpected ackIDs
                                                         (packet-accepted or packet_retry) */
	uint32_t nack                         : 1;  /**< Enable error rate counting of packet-not-accepted
                                                         acknowledge control symbols. */
	uint32_t out_ack                      : 1;  /**< Enable error rate counting of received packet with
                                                         unexpected ackID value */
	uint32_t pkt_crc                      : 1;  /**< Enable error rate counting of received a packet
                                                         with a bad CRC value */
	uint32_t size                         : 1;  /**< Enable error rate counting of received packet
                                                         which exceeds the maximum size of 276 bytes. */
	uint32_t reserved_6_16                : 11;
	uint32_t bad_ack                      : 1;  /**< Enable error rate counting of link_responses with
                                                         an ackID that is not outstanding. */
	uint32_t proterr                      : 1;  /**< Enable error rate counting of unexpected packet or
                                                         control symbols received. */
	uint32_t f_toggle                     : 1;  /**< Reserved. */
	uint32_t del_err                      : 1;  /**< Enable error rate counting of illegal or undefined
                                                         codegroups (either INV_DATA or INV_CHAR). (Pass 2) */
	uint32_t uns_ack                      : 1;  /**< Enable error rate counting of unexpected
                                                         acknowledge control symbols received. */
	uint32_t lnk_tout                     : 1;  /**< Enable error rate counting of acknowledge or
                                                         link-response control symbols not received within
                                                         the specified timeout interval */
#else
	uint32_t lnk_tout                     : 1;
	uint32_t uns_ack                      : 1;
	uint32_t del_err                      : 1;
	uint32_t f_toggle                     : 1;
	uint32_t proterr                      : 1;
	uint32_t bad_ack                      : 1;
	uint32_t reserved_6_16                : 11;
	uint32_t size                         : 1;
	uint32_t pkt_crc                      : 1;
	uint32_t out_ack                      : 1;
	uint32_t nack                         : 1;
	uint32_t uns_id                       : 1;
	uint32_t ctl_crc                      : 1;
	uint32_t reserved_23_31               : 9;
#endif
	} cn63xxp1;
	struct cvmx_sriomaintx_erb_err_rate_en_s cn66xx;
	struct cvmx_sriomaintx_erb_err_rate_en_s cnf75xx;
};
typedef union cvmx_sriomaintx_erb_err_rate_en cvmx_sriomaintx_erb_err_rate_en_t;

/**
 * cvmx_sriomaint#_erb_err_rate_thr
 *
 * This register is used to control the reporting of errors to the link
 * status. Typically the degraded threshold is less than the fail threshold.
 */
union cvmx_sriomaintx_erb_err_rate_thr {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_err_rate_thr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t fail_th                      : 8;  /**< These bits provide the threshold value for
                                                         reporting an error condition due to a possibly
                                                         broken link.
                                                         0x00 = Disable the error rate failed threshold trigger.
                                                         0x01 = Set the error reporting threshold to 1.
                                                         0x02 = Set the error reporting threshold to 2.
                                                         - ...
                                                         0xFF = Set the error reporting threshold to 255. */
	uint32_t dgrad_th                     : 8;  /**< These bits provide the threshold value for
                                                         reporting an error condition due to a possibly
                                                         degrading link.
                                                         0x00 = Disable the degrade rate failed threshold trigger.
                                                         0x01 - Set the error reporting threshold to 1.
                                                         0x02 - Set the error reporting threshold to 2.
                                                         - ...
                                                         0xFF - Set the error reporting threshold to 255. */
	uint32_t reserved_0_15                : 16;
#else
	uint32_t reserved_0_15                : 16;
	uint32_t dgrad_th                     : 8;
	uint32_t fail_th                      : 8;
#endif
	} s;
	struct cvmx_sriomaintx_erb_err_rate_thr_s cn63xx;
	struct cvmx_sriomaintx_erb_err_rate_thr_s cn63xxp1;
	struct cvmx_sriomaintx_erb_err_rate_thr_s cn66xx;
	struct cvmx_sriomaintx_erb_err_rate_thr_s cnf75xx;
};
typedef union cvmx_sriomaintx_erb_err_rate_thr cvmx_sriomaintx_erb_err_rate_thr_t;

/**
 * cvmx_sriomaint#_erb_hdr
 *
 * The error management extensions block header register contains the EF_PTR to the next EF_BLK
 * and the EF_ID that identifies this as the error management extensions block header. In this
 * implementation this is the last block and therefore the EF_PTR is a NULL pointer.
 */
union cvmx_sriomaintx_erb_hdr {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_hdr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ef_ptr                       : 16; /**< Pointer to the next block in the extended features
                                                         data structure. */
	uint32_t ef_id                        : 16; /**< Single port ID. */
#else
	uint32_t ef_id                        : 16;
	uint32_t ef_ptr                       : 16;
#endif
	} s;
	struct cvmx_sriomaintx_erb_hdr_s      cn63xx;
	struct cvmx_sriomaintx_erb_hdr_s      cn63xxp1;
	struct cvmx_sriomaintx_erb_hdr_s      cn66xx;
	struct cvmx_sriomaintx_erb_hdr_s      cnf75xx;
};
typedef union cvmx_sriomaintx_erb_hdr cvmx_sriomaintx_erb_hdr_t;

/**
 * cvmx_sriomaint#_erb_lt_addr_capt_h
 *
 * This register contains error information. It is locked when a logical/transport error is
 * detected and unlocked when the SRIOMAINT()_ERB_LT_ERR_DET is written to zero. This
 * register should be written only when error detection is disabled.  This register is only
 * required for end point transactions of 50 or 66 bits.
 */
union cvmx_sriomaintx_erb_lt_addr_capt_h {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_lt_addr_capt_h_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t addr                         : 32; /**< Most significant 32 bits of the address associated
                                                         with the error. Information supplied for requests
                                                         and responses if available. */
#else
	uint32_t addr                         : 32;
#endif
	} s;
	struct cvmx_sriomaintx_erb_lt_addr_capt_h_s cn63xx;
	struct cvmx_sriomaintx_erb_lt_addr_capt_h_s cn63xxp1;
	struct cvmx_sriomaintx_erb_lt_addr_capt_h_s cn66xx;
	struct cvmx_sriomaintx_erb_lt_addr_capt_h_s cnf75xx;
};
typedef union cvmx_sriomaintx_erb_lt_addr_capt_h cvmx_sriomaintx_erb_lt_addr_capt_h_t;

/**
 * cvmx_sriomaint#_erb_lt_addr_capt_l
 *
 * This register contains error information. It is locked when a logical/transport error is
 * detected and unlocked when the SRIOMAINT()_ERB_LT_ERR_DET is written to zero.
 * This register should be written only when error detection is disabled.
 */
union cvmx_sriomaintx_erb_lt_addr_capt_l {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_lt_addr_capt_l_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t addr                         : 29; /**< Least significant 29 bits of the address
                                                         associated with the error.  Bits 31:24 specify the
                                                         request HOP count for maintenance operations.
                                                         Information supplied for requests and responses if
                                                         available. */
	uint32_t reserved_2_2                 : 1;
	uint32_t xaddr                        : 2;  /**< Extended address bits of the address associated
                                                         with the error.  Information supplied for requests
                                                         and responses if available. */
#else
	uint32_t xaddr                        : 2;
	uint32_t reserved_2_2                 : 1;
	uint32_t addr                         : 29;
#endif
	} s;
	struct cvmx_sriomaintx_erb_lt_addr_capt_l_s cn63xx;
	struct cvmx_sriomaintx_erb_lt_addr_capt_l_s cn63xxp1;
	struct cvmx_sriomaintx_erb_lt_addr_capt_l_s cn66xx;
	struct cvmx_sriomaintx_erb_lt_addr_capt_l_s cnf75xx;
};
typedef union cvmx_sriomaintx_erb_lt_addr_capt_l cvmx_sriomaintx_erb_lt_addr_capt_l_t;

/**
 * cvmx_sriomaint#_erb_lt_ctrl_capt
 *
 * This register contains error information. It is locked when a logical/transport error is
 * detected and unlocked when the SRIOMAINT()_ERB_LT_ERR_DET is written to zero.
 * This register should be written only when error detection is disabled.
 */
union cvmx_sriomaintx_erb_lt_ctrl_capt {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_lt_ctrl_capt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ftype                        : 4;  /**< Format type associated with the error. */
	uint32_t ttype                        : 4;  /**< Transaction type associated with the error
                                                         (for messages). */
	uint32_t extra                        : 8;  /**< Additional information (for messages):
                                                         <23:22> = Letter.
                                                         <21:20> = Mbox.
                                                         <19:16> = Msgseg/xmbox.
                                                         Information for the last message request sent
                                                         for the mailbox that had an error (for responses):
                                                         <23:20> = Response request FTYPE.
                                                         <19:16> = Response request TTYPE.
                                                         For all other types:
                                                         _ Reserved. */
	uint32_t status                       : 4;  /**< Response status.
                                                         (For all other requests)
                                                         Reserved. */
	uint32_t size                         : 4;  /**< Size associated with the transaction. */
	uint32_t tt                           : 1;  /**< Transfer Type 0=ID8, 1=ID16. */
	uint32_t wdptr                        : 1;  /**< Word pointer associated with the error. */
	uint32_t reserved_5_5                 : 1;
	uint32_t capt_idx                     : 5;  /**< Capture index. 31 - Bit set in SRIOMAINT()_ERB_LT_ERR_DET. */
#else
	uint32_t capt_idx                     : 5;
	uint32_t reserved_5_5                 : 1;
	uint32_t wdptr                        : 1;
	uint32_t tt                           : 1;
	uint32_t size                         : 4;
	uint32_t status                       : 4;
	uint32_t extra                        : 8;
	uint32_t ttype                        : 4;
	uint32_t ftype                        : 4;
#endif
	} s;
	struct cvmx_sriomaintx_erb_lt_ctrl_capt_s cn63xx;
	struct cvmx_sriomaintx_erb_lt_ctrl_capt_s cn63xxp1;
	struct cvmx_sriomaintx_erb_lt_ctrl_capt_s cn66xx;
	struct cvmx_sriomaintx_erb_lt_ctrl_capt_s cnf75xx;
};
typedef union cvmx_sriomaintx_erb_lt_ctrl_capt cvmx_sriomaintx_erb_lt_ctrl_capt_t;

/**
 * cvmx_sriomaint#_erb_lt_dev_id
 *
 * This SRIO interface does not support generating port-writes based on ERB errors.  This
 * register is currently unused and should be treated as reserved.
 */
union cvmx_sriomaintx_erb_lt_dev_id {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_lt_dev_id_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t id16                         : 8;  /**< This is the most significant byte of the port-write destination deviceID (large
                                                         transport systems only) destination ID used for port write errors. */
	uint32_t id8                          : 8;  /**< This is the port-write destination deviceID. */
	uint32_t tt                           : 1;  /**< Transport type used for port write.
                                                         0 = Small transport, ID8 only.
                                                         1 = Large transport, ID16 and ID8. */
	uint32_t reserved_0_14                : 15;
#else
	uint32_t reserved_0_14                : 15;
	uint32_t tt                           : 1;
	uint32_t id8                          : 8;
	uint32_t id16                         : 8;
#endif
	} s;
	struct cvmx_sriomaintx_erb_lt_dev_id_s cn63xx;
	struct cvmx_sriomaintx_erb_lt_dev_id_s cn63xxp1;
	struct cvmx_sriomaintx_erb_lt_dev_id_s cn66xx;
	struct cvmx_sriomaintx_erb_lt_dev_id_s cnf75xx;
};
typedef union cvmx_sriomaintx_erb_lt_dev_id cvmx_sriomaintx_erb_lt_dev_id_t;

/**
 * cvmx_sriomaint#_erb_lt_dev_id_capt
 *
 * This register contains error information. It is locked when a logical/transport error is
 * detected and unlocked when the SRIOMAINT()_ERB_LT_ERR_DET is written to zero.  This
 * register should be written only when error detection is disabled.
 */
union cvmx_sriomaintx_erb_lt_dev_id_capt {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_lt_dev_id_capt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t dst_id16                     : 8;  /**< Most significant byte of the large transport
                                                         destination ID associated with the error. */
	uint32_t dst_id8                      : 8;  /**< Least significant byte of the large transport
                                                         destination ID or the 8-bit small transport
                                                         destination ID associated with the error. */
	uint32_t src_id16                     : 8;  /**< Most significant byte of the large transport
                                                         source ID associated with the error. */
	uint32_t src_id8                      : 8;  /**< Least significant byte of the large transport
                                                         source ID or the 8-bit small transport source ID
                                                         associated with the error. */
#else
	uint32_t src_id8                      : 8;
	uint32_t src_id16                     : 8;
	uint32_t dst_id8                      : 8;
	uint32_t dst_id16                     : 8;
#endif
	} s;
	struct cvmx_sriomaintx_erb_lt_dev_id_capt_s cn63xx;
	struct cvmx_sriomaintx_erb_lt_dev_id_capt_s cn63xxp1;
	struct cvmx_sriomaintx_erb_lt_dev_id_capt_s cn66xx;
	struct cvmx_sriomaintx_erb_lt_dev_id_capt_s cnf75xx;
};
typedef union cvmx_sriomaintx_erb_lt_dev_id_capt cvmx_sriomaintx_erb_lt_dev_id_capt_t;

/**
 * cvmx_sriomaint#_erb_lt_err_det
 *
 * This register indicates the error that was detected by the logical or transport logic layer.
 * Once a bit is set in this CSR, hardware will lock the register until software writes a zero to
 * clear all
 * the fields.  The hardware sets SRIO_INT_REG[LOG_ERB] every time it sets one of the bits.
 * To handle the interrupt, the following procedure may be best:
 *
 * (1) read this CSR, corresponding SRIOMAINT()_ERB_LT_ADDR_CAPT_H,
 * SRIOMAINT()_ERB_LT_ADDR_CAPT_L,
 * SRIOMAINT()_ERB_LT_DEV_ID_CAPT, and SRIOMAINT()_ERB_LT_CTRL_CAPT
 * (2) Write this CSR to 0.
 * (3) clear SRIO_INT_REG[LOG_ERB],
 */
union cvmx_sriomaintx_erb_lt_err_det {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_lt_err_det_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t io_err                       : 1;  /**< Received a response of ERROR for an IO logical
                                                         layer request.  This includes all maintenance and
                                                         memory responses not destined for the RX soft
                                                         packet FIFO. When SRIO receives an ERROR response
                                                         for a read, the issuing core or DPI DMA engine
                                                         receives result bytes with all bits set. In the
                                                         case of writes with response, this bit is the only
                                                         indication of failure. */
	uint32_t msg_err                      : 1;  /**< Received a response of ERROR for an outgoing
                                                         message segment. This bit is the only direct
                                                         indication of a MSG_ERR. When a MSG_ERR occurs,
                                                         SRIO drops the message segment and will not set
                                                         SRIO()_INT_REG[OMSG*] after the message
                                                         "transfer". NOTE: SRIO can continue to send or
                                                         retry other segments from the same message after
                                                         a MSG_ERR. */
	uint32_t gsm_err                      : 1;  /**< Received a response of ERROR for an GSM Logical
                                                         Request.  SRIO hardware never sets this bit. GSM
                                                         operations are not supported (outside of the Soft
                                                         Packet FIFO). */
	uint32_t msg_fmt                      : 1;  /**< Received an incoming message segment with a
                                                         formatting error.  A MSG_FMT error occurs when SRIO
                                                         receives a message segment with a reserved SSIZE,
                                                         or a illegal data payload size, or a MSGSEG greater
                                                         than MSGLEN, or a MSGSEG that is the duplicate of
                                                         one already received by an inflight message.
                                                         When a non-duplicate MSG_FMT error occurs, SRIO
                                                         drops the segment and sends an ERROR response.
                                                         When a duplicate MSG_FMT error occurs, SRIO
                                                         (internally) terminates the currently-inflight
                                                         message with an error and processes the duplicate,
                                                         which may result in a new message being generated
                                                         internally for the duplicate. */
	uint32_t ill_tran                     : 1;  /**< Received illegal fields in the request/response
                                                         packet for a supported transaction or any packet
                                                         with a reserved transaction type. When an ILL_TRAN
                                                         error occurs, SRIO ignores the packet. ILL_TRAN
                                                         errors are 2nd priority after ILL_TGT and may mask
                                                         other problems. Packets with ILL_TRAN errors cannot
                                                         enter the RX Soft Packet FIFO.
                                                         There are two things that can set ILL_TRAN:
                                                         (1) SRIO received a packet with a tt value is not
                                                         0 or 1, or (2) SRIO received a response to an
                                                         outstanding message segment whose status was not
                                                         DONE, RETRY, or ERROR. */
	uint32_t ill_tgt                      : 1;  /**< Received a packet that contained a destination ID
                                                         other than SRIOMAINT()_PRI_DEV_ID or
                                                         SRIOMAINT()_SEC_DEV_ID. When an ILL_TGT error
                                                         occurs, SRIO drops the packet. ILL_TGT errors are
                                                         highest priority, so may mask other problems.
                                                         Packets with ILL_TGT errors cannot enter the RX
                                                         soft packet fifo. */
	uint32_t msg_tout                     : 1;  /**< An expected incoming message request has not been
                                                         received within the time-out interval specified in
                                                         SRIOMAINT()_PORT_RT_CTL. When a MSG_TOUT occurs,
                                                         SRIO (internally) terminates the inflight message
                                                         with an error. */
	uint32_t pkt_tout                     : 1;  /**< A required response has not been received to an
                                                         outgoing memory, maintenance or message request
                                                         before the time-out interval specified in
                                                         SRIOMAINT()_PORT_RT_CTL.  When an IO or maintenance
                                                         read request operation has a PKT_TOUT, the issuing
                                                         core load or DPI DMA engine receive all ones for
                                                         the result. When an IO NWRITE_R has a PKT_TOUT,
                                                         this bit is the only indication of failure. When a
                                                         message request operation has a PKT_TOUT, SRIO
                                                         discards the the outgoing message segment,  and
                                                         this bit is the only direct indication of failure.
                                                         NOTE: SRIO may continue to send or retry other
                                                         segments from the same message. When one or more of
                                                         the segments in an outgoing message have a
                                                         PKT_TOUT, SRIO will not set SRIO()_INT_REG[OMSG*]
                                                         after the message "transfer". */
	uint32_t uns_resp                     : 1;  /**< An unsolicited/unexpected memory, maintenance or
                                                         message response packet was received that was not
                                                         destined for the RX soft packet FIFO.  When this
                                                         condition is detected, the packet is dropped. */
	uint32_t uns_tran                     : 1;  /**< A transaction is received that is not supported.
                                                         SRIO hardware will never set this bit - SRIO routes all
                                                         unsupported transactions to the RX soft packet
                                                         FIFO. */
	uint32_t reserved_1_21                : 21;
	uint32_t resp_sz                      : 1;  /**< Received an incoming memory or maintenance
                                                         read response packet with a DONE status and less
                                                         data then expected.  This condition causes the
                                                         Read to be completed and an error response to be
                                                         returned with all the data bits set to the issuing
                                                         core or DMA engine. */
#else
	uint32_t resp_sz                      : 1;
	uint32_t reserved_1_21                : 21;
	uint32_t uns_tran                     : 1;
	uint32_t uns_resp                     : 1;
	uint32_t pkt_tout                     : 1;
	uint32_t msg_tout                     : 1;
	uint32_t ill_tgt                      : 1;
	uint32_t ill_tran                     : 1;
	uint32_t msg_fmt                      : 1;
	uint32_t gsm_err                      : 1;
	uint32_t msg_err                      : 1;
	uint32_t io_err                       : 1;
#endif
	} s;
	struct cvmx_sriomaintx_erb_lt_err_det_s cn63xx;
	struct cvmx_sriomaintx_erb_lt_err_det_s cn63xxp1;
	struct cvmx_sriomaintx_erb_lt_err_det_s cn66xx;
	struct cvmx_sriomaintx_erb_lt_err_det_s cnf75xx;
};
typedef union cvmx_sriomaintx_erb_lt_err_det cvmx_sriomaintx_erb_lt_err_det_t;

/**
 * cvmx_sriomaint#_erb_lt_err_en
 *
 * This register contains the bits that control if an error condition locks the
 * logical/transport layer error detect and capture registers and is reported to the
 * system host.
 */
union cvmx_sriomaintx_erb_lt_err_en {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_lt_err_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t io_err                       : 1;  /**< Enable reporting of an IO error response. Save and lock original request
                                                         transaction information in all logical/transport layer capture CSRs. */
	uint32_t msg_err                      : 1;  /**< Enable reporting of a message error response. Save and lock original request
                                                         transaction information in all logical/transport layer capture CSRs. */
	uint32_t gsm_err                      : 1;  /**< Enable reporting of a GSM error response. Save and lock original request
                                                         transaction capture information in all logical/transport layer capture CSRs. */
	uint32_t msg_fmt                      : 1;  /**< Enable reporting of a message format error. Save and lock transaction capture
                                                         information in logical/transport layer device ID and control capture CSRs. */
	uint32_t ill_tran                     : 1;  /**< Enable reporting of an illegal transaction decode error Save and lock
                                                         transaction capture information in logical/transport layer device ID and control
                                                         capture CSRs. */
	uint32_t ill_tgt                      : 1;  /**< Enable reporting of an illegal transaction target error. Save and lock
                                                         transaction capture information in logical/transport layer device ID and control
                                                         capture CSRs. */
	uint32_t msg_tout                     : 1;  /**< Enable reporting of a message request time-out error. Save and lock transaction
                                                         capture information in logical/transport layer device ID and control capture
                                                         CSRs for the last message request segment packet received. */
	uint32_t pkt_tout                     : 1;  /**< Enable reporting of a packet response time-out error. Save and lock original
                                                         request address in logical/transport layer address capture CSRs. Save and lock
                                                         original request destination ID in logical/transport layer device ID capture
                                                         CSR. */
	uint32_t uns_resp                     : 1;  /**< Enable reporting of an unsolicited response error. Save and lock transaction
                                                         capture information in logical/transport layer device ID and control capture
                                                         CSRs. */
	uint32_t uns_tran                     : 1;  /**< Enable reporting of an unsupported transaction error. Save and lock transaction
                                                         capture information in logical/transport layer device ID and control capture
                                                         CSRs. */
	uint32_t reserved_1_21                : 21;
	uint32_t resp_sz                      : 1;  /**< Enable reporting of an incoming response with unexpected data size. */
#else
	uint32_t resp_sz                      : 1;
	uint32_t reserved_1_21                : 21;
	uint32_t uns_tran                     : 1;
	uint32_t uns_resp                     : 1;
	uint32_t pkt_tout                     : 1;
	uint32_t msg_tout                     : 1;
	uint32_t ill_tgt                      : 1;
	uint32_t ill_tran                     : 1;
	uint32_t msg_fmt                      : 1;
	uint32_t gsm_err                      : 1;
	uint32_t msg_err                      : 1;
	uint32_t io_err                       : 1;
#endif
	} s;
	struct cvmx_sriomaintx_erb_lt_err_en_s cn63xx;
	struct cvmx_sriomaintx_erb_lt_err_en_s cn63xxp1;
	struct cvmx_sriomaintx_erb_lt_err_en_s cn66xx;
	struct cvmx_sriomaintx_erb_lt_err_en_s cnf75xx;
};
typedef union cvmx_sriomaintx_erb_lt_err_en cvmx_sriomaintx_erb_lt_err_en_t;

/**
 * cvmx_sriomaint#_erb_pack_capt_1
 *
 * This register contains either long symbol capture information or bytes 4
 * through 7 of the packet header. The hardware will not update this register (i.e. this
 * register is locked) while SRIOMAINT()_ERB_ATTR_CAPT[VALID] is set. This register
 * should only be read while this bit is set.
 */
union cvmx_sriomaintx_erb_pack_capt_1 {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_pack_capt_1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t capture                      : 32; /**< Bytes 4 thru 7 of the packet header. */
#else
	uint32_t capture                      : 32;
#endif
	} s;
	struct cvmx_sriomaintx_erb_pack_capt_1_s cn63xx;
	struct cvmx_sriomaintx_erb_pack_capt_1_s cn63xxp1;
	struct cvmx_sriomaintx_erb_pack_capt_1_s cn66xx;
	struct cvmx_sriomaintx_erb_pack_capt_1_s cnf75xx;
};
typedef union cvmx_sriomaintx_erb_pack_capt_1 cvmx_sriomaintx_erb_pack_capt_1_t;

/**
 * cvmx_sriomaint#_erb_pack_capt_2
 *
 * This register contains bytes 8 through 11 of the packet header. The hardware
 * will not update this register (i.e. this register is locked) while
 * SRIOMAINT()_ERB_ATTR_CAPT[VALID] is set. This register should only be read while
 * this bit is set.
 */
union cvmx_sriomaintx_erb_pack_capt_2 {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_pack_capt_2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t capture                      : 32; /**< Bytes 8 thru 11 of the packet header. */
#else
	uint32_t capture                      : 32;
#endif
	} s;
	struct cvmx_sriomaintx_erb_pack_capt_2_s cn63xx;
	struct cvmx_sriomaintx_erb_pack_capt_2_s cn63xxp1;
	struct cvmx_sriomaintx_erb_pack_capt_2_s cn66xx;
	struct cvmx_sriomaintx_erb_pack_capt_2_s cnf75xx;
};
typedef union cvmx_sriomaintx_erb_pack_capt_2 cvmx_sriomaintx_erb_pack_capt_2_t;

/**
 * cvmx_sriomaint#_erb_pack_capt_3
 *
 * This register contains bytes 12 through 15 of the packet header.
 * The hardware will not update this register (i.e. this register is locked) while
 * SRIOMAINT()_ERB_ATTR_CAPT[VALID] is set.  This register should only be read while this bit is
 * set.
 */
union cvmx_sriomaintx_erb_pack_capt_3 {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_pack_capt_3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t capture                      : 32; /**< Bytes 12 thru 15 of the packet header. */
#else
	uint32_t capture                      : 32;
#endif
	} s;
	struct cvmx_sriomaintx_erb_pack_capt_3_s cn63xx;
	struct cvmx_sriomaintx_erb_pack_capt_3_s cn63xxp1;
	struct cvmx_sriomaintx_erb_pack_capt_3_s cn66xx;
	struct cvmx_sriomaintx_erb_pack_capt_3_s cnf75xx;
};
typedef union cvmx_sriomaintx_erb_pack_capt_3 cvmx_sriomaintx_erb_pack_capt_3_t;

/**
 * cvmx_sriomaint#_erb_pack_sym_capt
 *
 * This register contains either captured control symbol information or the first 4 bytes of
 * captured packet information.  The errors that generate partial control symbols can be found in
 * SRIOMAINT()_ERB_ERR_DET.  The hardware will not update this register (i.e. this register is
 * locked)
 * while SRIOMAINT()_ERB_ATTR_CAPT[VALID] is set.  This register should only be read while this
 * bit is set.
 */
union cvmx_sriomaintx_erb_pack_sym_capt {
	uint32_t u32;
	struct cvmx_sriomaintx_erb_pack_sym_capt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t capture                      : 32; /**< Control character and control symbol or bytes 0 to
                                                         3 of packet header.
                                                         The control symbol consists of:
                                                         <31:24> = SC Character (0 in partial symbol).
                                                         <23:21> = Stype 0.
                                                         <20:16> = Parameter 0.
                                                         <15:11> = Parameter 1.
                                                         <10:8> = Stype 1 (0 in partial symbol).
                                                         <7:5> = Command (0 in partial symbol).
                                                         <4:0> = CRC5    (0 in partial symbol). */
#else
	uint32_t capture                      : 32;
#endif
	} s;
	struct cvmx_sriomaintx_erb_pack_sym_capt_s cn63xx;
	struct cvmx_sriomaintx_erb_pack_sym_capt_s cn63xxp1;
	struct cvmx_sriomaintx_erb_pack_sym_capt_s cn66xx;
	struct cvmx_sriomaintx_erb_pack_sym_capt_s cnf75xx;
};
typedef union cvmx_sriomaintx_erb_pack_sym_capt cvmx_sriomaintx_erb_pack_sym_capt_t;

/**
 * cvmx_sriomaint#_hb_dev_id_lock
 *
 * This register contains the device ID of the host responsible for initializing this SRIO
 * device. The register contains a special write once function that captures the first HOSTID
 * written to it after reset.  The function allows several potential hosts to write to this
 * register and then read it to see if they have responsibility for initialization.  The register
 * can be unlocked by rewriting the current host value.  This will reset the lock and restore the
 * value to 0xFFFF.
 */
union cvmx_sriomaintx_hb_dev_id_lock {
	uint32_t u32;
	struct cvmx_sriomaintx_hb_dev_id_lock_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t hostid                       : 16; /**< Primary 16-bit device ID. */
#else
	uint32_t hostid                       : 16;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_sriomaintx_hb_dev_id_lock_s cn63xx;
	struct cvmx_sriomaintx_hb_dev_id_lock_s cn63xxp1;
	struct cvmx_sriomaintx_hb_dev_id_lock_s cn66xx;
	struct cvmx_sriomaintx_hb_dev_id_lock_s cnf75xx;
};
typedef union cvmx_sriomaintx_hb_dev_id_lock cvmx_sriomaintx_hb_dev_id_lock_t;

/**
 * cvmx_sriomaint#_ir_buffer_config
 *
 * This register controls the operation of the SRIO core buffer mux logic.
 *
 */
union cvmx_sriomaintx_ir_buffer_config {
	uint32_t u32;
	struct cvmx_sriomaintx_ir_buffer_config_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t tx_wm0                       : 4;  /**< Reserved. See SRIOMAINT()_IR_BUFFER_CONFIG2. */
	uint32_t tx_wm1                       : 4;  /**< Reserved. See SRIOMAINT()_IR_BUFFER_CONFIG2. */
	uint32_t tx_wm2                       : 4;  /**< Reserved. See SRIOMAINT()_IR_BUFFER_CONFIG2. */
	uint32_t reserved_3_19                : 17;
	uint32_t tx_flow                      : 1;  /**< Controls whether transmitter flow control is permitted on this device.
                                                         0 = Disabled.
                                                         1 = Permitted.
                                                         The reset value of this field is SRIO()_IP_FEATURE[TX_FLOW]. */
	uint32_t tx_sync                      : 1;  /**< Reserved. */
	uint32_t rx_sync                      : 1;  /**< Reserved. */
#else
	uint32_t rx_sync                      : 1;
	uint32_t tx_sync                      : 1;
	uint32_t tx_flow                      : 1;
	uint32_t reserved_3_19                : 17;
	uint32_t tx_wm2                       : 4;
	uint32_t tx_wm1                       : 4;
	uint32_t tx_wm0                       : 4;
#endif
	} s;
	struct cvmx_sriomaintx_ir_buffer_config_s cn63xx;
	struct cvmx_sriomaintx_ir_buffer_config_s cn63xxp1;
	struct cvmx_sriomaintx_ir_buffer_config_s cn66xx;
	struct cvmx_sriomaintx_ir_buffer_config_cnf75xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t tx_wm0                       : 4;  /**< Reserved. See SRIOMAINT()_IR_BUFFER_CONFIG2. */
	uint32_t tx_wm1                       : 4;  /**< Reserved. See SRIOMAINT()_IR_BUFFER_CONFIG2. */
	uint32_t tx_wm2                       : 4;  /**< Reserved. See SRIOMAINT()_IR_BUFFER_CONFIG2. */
	uint32_t reserved_19_3                : 17;
	uint32_t tx_flow                      : 1;  /**< Controls whether transmitter flow control is permitted on this device.
                                                         0 = Disabled.
                                                         1 = Permitted.
                                                         The reset value of this field is SRIO()_IP_FEATURE[TX_FLOW]. */
	uint32_t tx_sync                      : 1;  /**< Reserved. */
	uint32_t rx_sync                      : 1;  /**< Reserved. */
#else
	uint32_t rx_sync                      : 1;
	uint32_t tx_sync                      : 1;
	uint32_t tx_flow                      : 1;
	uint32_t reserved_19_3                : 17;
	uint32_t tx_wm2                       : 4;
	uint32_t tx_wm1                       : 4;
	uint32_t tx_wm0                       : 4;
#endif
	} cnf75xx;
};
typedef union cvmx_sriomaintx_ir_buffer_config cvmx_sriomaintx_ir_buffer_config_t;

/**
 * cvmx_sriomaint#_ir_buffer_config2
 *
 * This register controls the RX and TX buffer availability by priority.  The typical values
 * are optimized for normal operation.  Care must be taken when changing these values to avoid
 * values which can result in deadlocks.  Disabling a priority is not recommended and can result
 * in system level failures.
 */
union cvmx_sriomaintx_ir_buffer_config2 {
	uint32_t u32;
	struct cvmx_sriomaintx_ir_buffer_config2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t tx_wm3                       : 4;  /**< Number of buffers free before a priority 3 packet
                                                         will be transmitted.  A value of 9 will disable
                                                         this priority. */
	uint32_t tx_wm2                       : 4;  /**< Number of buffers free before a priority 2 packet
                                                         will be transmitted.  A value of 9 will disable
                                                         this priority. */
	uint32_t tx_wm1                       : 4;  /**< Number of buffers free before a priority 1 packet
                                                         will be transmitted.  A value of 9 will disable
                                                         this priority. */
	uint32_t tx_wm0                       : 4;  /**< Number of buffers free before a priority 0 packet
                                                         will be transmitted.  A value of 9 will disable
                                                         this priority. */
	uint32_t rx_wm3                       : 4;  /**< Number of buffers free before a priority 3 packet
                                                         will be accepted.  A value of 9 will disable this
                                                         priority and always cause a physical layer RETRY. */
	uint32_t rx_wm2                       : 4;  /**< Number of buffers free before a priority 2 packet
                                                         will be accepted.  A value of 9 will disable this
                                                         priority and always cause a physical layer RETRY. */
	uint32_t rx_wm1                       : 4;  /**< Number of buffers free before a priority 1 packet
                                                         will be accepted.  A value of 9 will disable this
                                                         priority and always cause a physical layer RETRY. */
	uint32_t rx_wm0                       : 4;  /**< Number of buffers free before a priority 0 packet
                                                         will be accepted.  A value of 9 will disable this
                                                         priority and always cause a physical layer RETRY. */
#else
	uint32_t rx_wm0                       : 4;
	uint32_t rx_wm1                       : 4;
	uint32_t rx_wm2                       : 4;
	uint32_t rx_wm3                       : 4;
	uint32_t tx_wm0                       : 4;
	uint32_t tx_wm1                       : 4;
	uint32_t tx_wm2                       : 4;
	uint32_t tx_wm3                       : 4;
#endif
	} s;
	struct cvmx_sriomaintx_ir_buffer_config2_s cn63xx;
	struct cvmx_sriomaintx_ir_buffer_config2_s cn66xx;
	struct cvmx_sriomaintx_ir_buffer_config2_s cnf75xx;
};
typedef union cvmx_sriomaintx_ir_buffer_config2 cvmx_sriomaintx_ir_buffer_config2_t;

/**
 * cvmx_sriomaint#_ir_pd_phy_ctrl
 *
 * This register can be used for testing.  The register is otherwise unused by the hardware.
 *
 */
union cvmx_sriomaintx_ir_pd_phy_ctrl {
	uint32_t u32;
	struct cvmx_sriomaintx_ir_pd_phy_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pd_ctrl                      : 32; /**< Unused register available for testing. */
#else
	uint32_t pd_ctrl                      : 32;
#endif
	} s;
	struct cvmx_sriomaintx_ir_pd_phy_ctrl_s cn63xx;
	struct cvmx_sriomaintx_ir_pd_phy_ctrl_s cn63xxp1;
	struct cvmx_sriomaintx_ir_pd_phy_ctrl_s cn66xx;
	struct cvmx_sriomaintx_ir_pd_phy_ctrl_s cnf75xx;
};
typedef union cvmx_sriomaintx_ir_pd_phy_ctrl cvmx_sriomaintx_ir_pd_phy_ctrl_t;

/**
 * cvmx_sriomaint#_ir_pd_phy_stat
 *
 * This register is used to monitor PHY status on each lane.  They are documented here to assist
 * in debugging only.  The lane numbers take into account the lane swap pin.
 */
union cvmx_sriomaintx_ir_pd_phy_stat {
	uint32_t u32;
	struct cvmx_sriomaintx_ir_pd_phy_stat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_16_31               : 16;
	uint32_t ln3_rx                       : 3;  /**< Phy lane 3 RX status.
                                                         0XX = Normal operation.
                                                         100 = 8B/10B error.
                                                         101 = Elastic buffer overflow (data lost).
                                                         110 = Elastic buffer underflow (data corrupted).
                                                         111 = Disparity error. */
	uint32_t ln3_dis                      : 1;  /**< Lane 3 phy clock disabled.
                                                         0 = Phy clock valid.
                                                         1 = Phy clock invalid. */
	uint32_t ln2_rx                       : 3;  /**< Phy lane 2 RX status.
                                                         0XX = Normal operation
                                                         100 = 8B/10B error.
                                                         101 = Elastic buffer overflow (data lost).
                                                         110 = Elastic buffer underflow (data corrupted).
                                                         111 = Disparity error. */
	uint32_t ln2_dis                      : 1;  /**< Lane 2 phy clock disabled.
                                                         0 = Phy clock valid.
                                                         1 = Phy clock invalid. */
	uint32_t ln1_rx                       : 3;  /**< Phy lane 1 RX status.
                                                         0XX = Normal operation.
                                                         100 = 8B/10B error.
                                                         101 = Elastic buffer overflow (data lost).
                                                         110 = Elastic buffer underflow (data corrupted).
                                                         111 = Disparity error. */
	uint32_t ln1_dis                      : 1;  /**< Lane 1 phy clock disabled.
                                                         0 = Phy clock valid.
                                                         1 = Phy clock invalid. */
	uint32_t ln0_rx                       : 3;  /**< Phy Lane 0 RX status.
                                                         0XX = Normal operation.
                                                         100 = 8B/10B error.
                                                         101 = Elastic buffer overflow (data lost).
                                                         110 = Elastic buffer underflow (data corrupted).
                                                         111 = Disparity error. */
	uint32_t ln0_dis                      : 1;  /**< Lane 0 phy clock disabled.
                                                         0 = Phy clock valid.
                                                         1 = Phy clock invalid. */
#else
	uint32_t ln0_dis                      : 1;
	uint32_t ln0_rx                       : 3;
	uint32_t ln1_dis                      : 1;
	uint32_t ln1_rx                       : 3;
	uint32_t ln2_dis                      : 1;
	uint32_t ln2_rx                       : 3;
	uint32_t ln3_dis                      : 1;
	uint32_t ln3_rx                       : 3;
	uint32_t reserved_16_31               : 16;
#endif
	} s;
	struct cvmx_sriomaintx_ir_pd_phy_stat_s cn63xx;
	struct cvmx_sriomaintx_ir_pd_phy_stat_s cn63xxp1;
	struct cvmx_sriomaintx_ir_pd_phy_stat_s cn66xx;
	struct cvmx_sriomaintx_ir_pd_phy_stat_s cnf75xx;
};
typedef union cvmx_sriomaintx_ir_pd_phy_stat cvmx_sriomaintx_ir_pd_phy_stat_t;

/**
 * cvmx_sriomaint#_ir_pi_phy_ctrl
 *
 * This register is used to control platform independent operating modes of the transceivers.
 * These control bits are uniform across all platforms.
 */
union cvmx_sriomaintx_ir_pi_phy_ctrl {
	uint32_t u32;
	struct cvmx_sriomaintx_ir_pi_phy_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t tx_reset                     : 1;  /**< Outgoing PHY Logic Reset.  0=Reset, 1=Normal operation. */
	uint32_t rx_reset                     : 1;  /**< Incoming PHY Logic Reset.  0=Reset, 1=Normal operation. */
	uint32_t reserved_29_29               : 1;
	uint32_t loopback                     : 2;  /**< These bits control the state of the loopback
                                                         control vector on the transceiver interface.  The
                                                         loopback modes are enumerated as follows:
                                                         0x0 - No loopback.
                                                         0x1 = Reserved.
                                                         0x2 = Far end PCS loopback.
                                                         0x3 = Reserved. */
	uint32_t reserved_0_26                : 27;
#else
	uint32_t reserved_0_26                : 27;
	uint32_t loopback                     : 2;
	uint32_t reserved_29_29               : 1;
	uint32_t rx_reset                     : 1;
	uint32_t tx_reset                     : 1;
#endif
	} s;
	struct cvmx_sriomaintx_ir_pi_phy_ctrl_s cn63xx;
	struct cvmx_sriomaintx_ir_pi_phy_ctrl_s cn63xxp1;
	struct cvmx_sriomaintx_ir_pi_phy_ctrl_s cn66xx;
	struct cvmx_sriomaintx_ir_pi_phy_ctrl_s cnf75xx;
};
typedef union cvmx_sriomaintx_ir_pi_phy_ctrl cvmx_sriomaintx_ir_pi_phy_ctrl_t;

/**
 * cvmx_sriomaint#_ir_pi_phy_stat
 *
 * This register displays the status of the link initialization state machine.  Changes to this
 * state cause the SRIO()_INT_REG[LINK_UP] or SRIO()_INT_REG[LINK_DOWN] interrupts.
 */
union cvmx_sriomaintx_ir_pi_phy_stat {
	uint32_t u32;
	struct cvmx_sriomaintx_ir_pi_phy_stat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_12_31               : 20;
	uint32_t tx_rdy                       : 1;  /**< Minimum number of status transmitted. */
	uint32_t rx_rdy                       : 1;  /**< Minimum number of good status received. */
	uint32_t init_sm                      : 10; /**< Initialization State Machine
                                                         0x001 = Silent.
                                                         0x002 = Seek.
                                                         0x004 = Discovery.
                                                         0x008 = 1x_Mode_Lane0.
                                                         0x010 = 1x_Mode_Lane1.
                                                         0x020 = 1x_Mode_Lane2.
                                                         0x040 = 1x_Recovery.
                                                         0x080 = 2x_Mode.
                                                         0x100 = 2x_Recovery.
                                                         0x200 = 4x_Mode.
                                                         _ All others are reserved */
#else
	uint32_t init_sm                      : 10;
	uint32_t rx_rdy                       : 1;
	uint32_t tx_rdy                       : 1;
	uint32_t reserved_12_31               : 20;
#endif
	} s;
	struct cvmx_sriomaintx_ir_pi_phy_stat_s cn63xx;
	struct cvmx_sriomaintx_ir_pi_phy_stat_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_10_31               : 22;
	uint32_t init_sm                      : 10; /**< Initialization State Machine
                                                         001 - Silent
                                                         002 - Seek
                                                         004 - Discovery
                                                         008 - 1x_Mode_Lane0
                                                         010 - 1x_Mode_Lane1
                                                         020 - 1x_Mode_Lane2
                                                         040 - 1x_Recovery
                                                         080 - 2x_Mode
                                                         100 - 2x_Recovery
                                                         200 - 4x_Mode
                                                         All others are reserved */
#else
	uint32_t init_sm                      : 10;
	uint32_t reserved_10_31               : 22;
#endif
	} cn63xxp1;
	struct cvmx_sriomaintx_ir_pi_phy_stat_s cn66xx;
	struct cvmx_sriomaintx_ir_pi_phy_stat_s cnf75xx;
};
typedef union cvmx_sriomaintx_ir_pi_phy_stat cvmx_sriomaintx_ir_pi_phy_stat_t;

/**
 * cvmx_sriomaint#_ir_sp_rx_ctrl
 *
 * This register is used to configure events generated by the reception of packets using the
 * soft packet FIFO.
 */
union cvmx_sriomaintx_ir_sp_rx_ctrl {
	uint32_t u32;
	struct cvmx_sriomaintx_ir_sp_rx_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_1_31                : 31;
	uint32_t overwrt                      : 1;  /**< When clear, SRIO drops received packets that should
                                                         enter the soft packet FIFO when the FIFO is full.
                                                         In this case, SRIO also increments
                                                         SRIOMAINT()_IR_SP_RX_STAT[DROP_CNT]. When set, SRIO
                                                         stalls received packets that should enter the soft
                                                         packet FIFO when the FIFO is full. SRIO may stop
                                                         receiving any packets in this stall case if
                                                         software does not drain the receive soft packet
                                                         FIFO. */
#else
	uint32_t overwrt                      : 1;
	uint32_t reserved_1_31                : 31;
#endif
	} s;
	struct cvmx_sriomaintx_ir_sp_rx_ctrl_s cn63xx;
	struct cvmx_sriomaintx_ir_sp_rx_ctrl_s cn63xxp1;
	struct cvmx_sriomaintx_ir_sp_rx_ctrl_s cn66xx;
	struct cvmx_sriomaintx_ir_sp_rx_ctrl_s cnf75xx;
};
typedef union cvmx_sriomaintx_ir_sp_rx_ctrl cvmx_sriomaintx_ir_sp_rx_ctrl_t;

/**
 * cvmx_sriomaint#_ir_sp_rx_data
 *
 * This register is used to read data from the soft packet FIFO.  The soft packet FIFO contains
 * the majority of the packet data received from the SRIO link.  The packet does not include the
 * control symbols or the initial byte containing AckId, 2 reserved bits and the CRF.  In the
 * case of packets with less than 80 bytes (including AckId byte) both the trailing CRC and pad
 * (if present) are included in the FIFO and octet count.  In the case of a packet with exactly
 * 80 bytes (including the AckId byte) the CRC is removed and the pad is maintained so the octet
 * count will read 81 bytes instead of the expected 83.  In cases over 80 bytes the CRC at 80
 * bytes
 * is removed but the trailing CRC and Pad (if necessary) are present.
 */
union cvmx_sriomaintx_ir_sp_rx_data {
	uint32_t u32;
	struct cvmx_sriomaintx_ir_sp_rx_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pkt_data                     : 32; /**< This register is used to read packet data from the RX FIFO. */
#else
	uint32_t pkt_data                     : 32;
#endif
	} s;
	struct cvmx_sriomaintx_ir_sp_rx_data_s cn63xx;
	struct cvmx_sriomaintx_ir_sp_rx_data_s cn63xxp1;
	struct cvmx_sriomaintx_ir_sp_rx_data_s cn66xx;
	struct cvmx_sriomaintx_ir_sp_rx_data_s cnf75xx;
};
typedef union cvmx_sriomaintx_ir_sp_rx_data cvmx_sriomaintx_ir_sp_rx_data_t;

/**
 * cvmx_sriomaint#_ir_sp_rx_stat
 *
 * This register is used to monitor the reception of packets using the soft packet FIFO.
 * The hardware sets SRIO_INT_REG[SOFT_RX] every time a packet arrives in the soft packet FIFO.
 * To read
 * out (one or more) packets, the following procedure may be best:
 *
 * (1) clear SRIO_INT_REG[SOFT_RX],
 *
 * (2) read this CSR to determine how many packets there are,
 *
 * (3) read the packets out (via SRIOMAINT()_IR_SP_RX_DATA).
 *
 * This procedure could lead to situations where SOFT_RX will be set even though there are
 * currently no packets the software interrupt handler would need to properly handle this case
 */
union cvmx_sriomaintx_ir_sp_rx_stat {
	uint32_t u32;
	struct cvmx_sriomaintx_ir_sp_rx_stat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t octets                       : 16; /**< This field shows how many octets are remaining
                                                         in the current packet in the RX FIFO. */
	uint32_t buffers                      : 4;  /**< This field indicates how many complete packets are
                                                         stored in the Rx FIFO. */
	uint32_t drop_cnt                     : 7;  /**< Number of Packets Received when the RX FIFO was
                                                         full and then discarded. */
	uint32_t full                         : 1;  /**< This bit is set when the value of Buffers Filled
                                                         equals the number of available reception buffers. */
	uint32_t fifo_st                      : 4;  /**< These bits display the state of the state machine
                                                         that controls loading of packet data into the RX
                                                         FIFO. The enumeration of states are as follows:
                                                         0x0 = Idle.
                                                         0x1 = Armed.
                                                         0x2 = Active.
                                                         _ All other states are reserved. */
#else
	uint32_t fifo_st                      : 4;
	uint32_t full                         : 1;
	uint32_t drop_cnt                     : 7;
	uint32_t buffers                      : 4;
	uint32_t octets                       : 16;
#endif
	} s;
	struct cvmx_sriomaintx_ir_sp_rx_stat_s cn63xx;
	struct cvmx_sriomaintx_ir_sp_rx_stat_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t octets                       : 16; /**< This field shows how many octets are remaining
                                                         in the current packet in the RX FIFO. */
	uint32_t buffers                      : 4;  /**< This field indicates how many complete packets are
                                                         stored in the Rx FIFO. */
	uint32_t reserved_5_11                : 7;
	uint32_t full                         : 1;  /**< This bit is set when the value of Buffers Filled
                                                         equals the number of available reception buffers.
                                                         This bit always reads zero in Pass 1 */
	uint32_t fifo_st                      : 4;  /**< These bits display the state of the state machine
                                                         that controls loading of packet data into the RX
                                                         FIFO. The enumeration of states are as follows:
                                                           0000 - Idle
                                                           0001 - Armed
                                                           0010 - Active
                                                           All other states are reserved. */
#else
	uint32_t fifo_st                      : 4;
	uint32_t full                         : 1;
	uint32_t reserved_5_11                : 7;
	uint32_t buffers                      : 4;
	uint32_t octets                       : 16;
#endif
	} cn63xxp1;
	struct cvmx_sriomaintx_ir_sp_rx_stat_s cn66xx;
	struct cvmx_sriomaintx_ir_sp_rx_stat_s cnf75xx;
};
typedef union cvmx_sriomaintx_ir_sp_rx_stat cvmx_sriomaintx_ir_sp_rx_stat_t;

/**
 * cvmx_sriomaint#_ir_sp_tx_ctrl
 *
 * This register is used to configure and control the transmission of packets using the soft
 * packet FIFO.
 */
union cvmx_sriomaintx_ir_sp_tx_ctrl {
	uint32_t u32;
	struct cvmx_sriomaintx_ir_sp_tx_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t octets                       : 16; /**< Writing a nonzero value (N) to this field arms
                                                         the packet FIFO for packet transmission. The FIFO
                                                         control logic will transmit the next N bytes
                                                         written 4-bytes at a time to
                                                         SRIOMAINT()_IR_SP_TX_DATA and create a
                                                         single RapidIO packet. */
	uint32_t reserved_0_15                : 16;
#else
	uint32_t reserved_0_15                : 16;
	uint32_t octets                       : 16;
#endif
	} s;
	struct cvmx_sriomaintx_ir_sp_tx_ctrl_s cn63xx;
	struct cvmx_sriomaintx_ir_sp_tx_ctrl_s cn63xxp1;
	struct cvmx_sriomaintx_ir_sp_tx_ctrl_s cn66xx;
	struct cvmx_sriomaintx_ir_sp_tx_ctrl_s cnf75xx;
};
typedef union cvmx_sriomaintx_ir_sp_tx_ctrl cvmx_sriomaintx_ir_sp_tx_ctrl_t;

/**
 * cvmx_sriomaint#_ir_sp_tx_data
 *
 * This register is used to write data to the soft packet FIFO.  The format of the packet
 * follows the internal packet format (add link here).  Care must be taken on creating TIDs
 * for the packets which generate a response.  Bits [7:6] of the 8 bit TID must be set for
 * all Soft Packet FIFO generated packets.  TID values of 0x00 0xBF are reserved for hardware
 * generated Tags.  The remainer of the TID[5:0] must be unique for each packet in flight and
 * cannot be reused until a response is received in the SRIOMAINT()_IR_SP_RX_DATA register.
 */
union cvmx_sriomaintx_ir_sp_tx_data {
	uint32_t u32;
	struct cvmx_sriomaintx_ir_sp_tx_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pkt_data                     : 32; /**< This register is used to write packet data to the
                                                         TX FIFO. Reads of this register will return zero. */
#else
	uint32_t pkt_data                     : 32;
#endif
	} s;
	struct cvmx_sriomaintx_ir_sp_tx_data_s cn63xx;
	struct cvmx_sriomaintx_ir_sp_tx_data_s cn63xxp1;
	struct cvmx_sriomaintx_ir_sp_tx_data_s cn66xx;
	struct cvmx_sriomaintx_ir_sp_tx_data_s cnf75xx;
};
typedef union cvmx_sriomaintx_ir_sp_tx_data cvmx_sriomaintx_ir_sp_tx_data_t;

/**
 * cvmx_sriomaint#_ir_sp_tx_stat
 *
 * This register is used to monitor the transmission of packets using the soft packet FIFO.
 *
 */
union cvmx_sriomaintx_ir_sp_tx_stat {
	uint32_t u32;
	struct cvmx_sriomaintx_ir_sp_tx_stat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t octets                       : 16; /**< This field shows how many octets are still to be
                                                         loaded in the current packet. */
	uint32_t buffers                      : 4;  /**< This field indicates how many complete packets are
                                                         stored in the TX FIFO.  The field always reads
                                                         zero in the current hardware. */
	uint32_t reserved_5_11                : 7;
	uint32_t full                         : 1;  /**< This bit is set when the value of buffers filled
                                                         equals the number of available transmission
                                                         buffers. */
	uint32_t fifo_st                      : 4;  /**< These bits display the state of the state machine
                                                         that controls loading of packet data into the TX
                                                         FIFO. The enumeration of states are as follows:
                                                         0x0 = Idle.
                                                         0x1 = Armed.
                                                         0x2 = Active.
                                                         _ All other states are reserved. */
#else
	uint32_t fifo_st                      : 4;
	uint32_t full                         : 1;
	uint32_t reserved_5_11                : 7;
	uint32_t buffers                      : 4;
	uint32_t octets                       : 16;
#endif
	} s;
	struct cvmx_sriomaintx_ir_sp_tx_stat_s cn63xx;
	struct cvmx_sriomaintx_ir_sp_tx_stat_s cn63xxp1;
	struct cvmx_sriomaintx_ir_sp_tx_stat_s cn66xx;
	struct cvmx_sriomaintx_ir_sp_tx_stat_s cnf75xx;
};
typedef union cvmx_sriomaintx_ir_sp_tx_stat cvmx_sriomaintx_ir_sp_tx_stat_t;

/**
 * cvmx_sriomaint#_lane_#_status_0
 *
 * This register contains status information about the local lane transceiver.
 *
 */
union cvmx_sriomaintx_lane_x_status_0 {
	uint32_t u32;
	struct cvmx_sriomaintx_lane_x_status_0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t port                         : 8;  /**< The number of the port within the device to which
                                                         the lane is assigned. */
	uint32_t lane                         : 4;  /**< Lane number within the port. */
	uint32_t tx_type                      : 1;  /**< Transmitter type:
                                                         0 = Short run.
                                                         1 = Long run. */
	uint32_t tx_mode                      : 1;  /**< Transmitter operating mode:
                                                         0 = Short run.
                                                         1 = Long run. */
	uint32_t rx_type                      : 2;  /**< Receiver type:
                                                         0x0 = Short run.
                                                         0x1 = Medium run.
                                                         0x2 = Long run.
                                                         0x3 = Reserved. */
	uint32_t rx_inv                       : 1;  /**< Receiver input inverted:
                                                         0 = No inversion.
                                                         1 = Input inverted. */
	uint32_t rx_adapt                     : 1;  /**< Receiver trained:
                                                         0 = One or more adaptive equalizers are
                                                         controlled by the lane receiver and at least
                                                         one is not trained.
                                                         1 = The lane receiver controls no adaptive
                                                         equalizers or all the equalizers are trained. */
	uint32_t rx_sync                      : 1;  /**< Receiver lane synced. */
	uint32_t rx_train                     : 1;  /**< Receiver lane trained. */
	uint32_t dec_err                      : 4;  /**< 8Bit/10Bit decoding errors.
                                                         0    = No errors since last read.
                                                         1-14 = Number of errors since last read.
                                                         15   = Fifteen or more errors since last read. */
	uint32_t xsync                        : 1;  /**< Receiver lane sync change.
                                                         0 = Lane sync has not changed since last read.
                                                         1 = Lane sync has changed since last read. */
	uint32_t xtrain                       : 1;  /**< Receiver training change.
                                                         0 = Training has not changed since last read.
                                                         1 = Training has changed since last read. */
	uint32_t reserved_4_5                 : 2;
	uint32_t status1                      : 1;  /**< Status 1 CSR implemented. */
	uint32_t statusn                      : 3;  /**< Status 2-7 not implemented. */
#else
	uint32_t statusn                      : 3;
	uint32_t status1                      : 1;
	uint32_t reserved_4_5                 : 2;
	uint32_t xtrain                       : 1;
	uint32_t xsync                        : 1;
	uint32_t dec_err                      : 4;
	uint32_t rx_train                     : 1;
	uint32_t rx_sync                      : 1;
	uint32_t rx_adapt                     : 1;
	uint32_t rx_inv                       : 1;
	uint32_t rx_type                      : 2;
	uint32_t tx_mode                      : 1;
	uint32_t tx_type                      : 1;
	uint32_t lane                         : 4;
	uint32_t port                         : 8;
#endif
	} s;
	struct cvmx_sriomaintx_lane_x_status_0_s cn63xx;
	struct cvmx_sriomaintx_lane_x_status_0_s cn63xxp1;
	struct cvmx_sriomaintx_lane_x_status_0_s cn66xx;
	struct cvmx_sriomaintx_lane_x_status_0_s cnf75xx;
};
typedef union cvmx_sriomaintx_lane_x_status_0 cvmx_sriomaintx_lane_x_status_0_t;

/**
 * cvmx_sriomaint#_lcs_ba0
 *
 * MSBs of SRIO address space mapped to maintenance BAR.
 * The double word aligned SRIO address window mapped to the SRIO maintenance BAR.  This window
 * has the highest priority and eclipses matches to the BAR0, BAR1 and BAR2 windows.
 * Note:  Address bits not supplied in the transfer are considered zero.  For example, SRIO
 * address 65:35 must be set to zero to match in a 34-bit access.  SRIO address 65:50 must be
 * set to zero to match in a 50-bit access.  This coding allows the maintenance bar window to
 * appear in specific address spaces. The remaining bits are located in SRIOMAINT()_LCS_BA1.
 * This SRIO maintenance BAR is effectively disabled when LCSBA[30] is set with 34 or 50-bit
 * addressing.
 */
union cvmx_sriomaintx_lcs_ba0 {
	uint32_t u32;
	struct cvmx_sriomaintx_lcs_ba0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_31_31               : 1;
	uint32_t lcsba                        : 31; /**< SRIO address 65:35. */
#else
	uint32_t lcsba                        : 31;
	uint32_t reserved_31_31               : 1;
#endif
	} s;
	struct cvmx_sriomaintx_lcs_ba0_s      cn63xx;
	struct cvmx_sriomaintx_lcs_ba0_s      cn63xxp1;
	struct cvmx_sriomaintx_lcs_ba0_s      cn66xx;
	struct cvmx_sriomaintx_lcs_ba0_s      cnf75xx;
};
typedef union cvmx_sriomaintx_lcs_ba0 cvmx_sriomaintx_lcs_ba0_t;

/**
 * cvmx_sriomaint#_lcs_ba1
 *
 * LSBs of SRIO address space mapped to maintenance BAR.
 * The double word aligned SRIO address window mapped to the SRIO maintenance BAR.  This window
 * has the highest priority and eclipses matches to the BAR0, BAR1 and BAR2 windows. Address
 * bits not supplied in the transfer are considered zero.  For example, SRIO address 65:35 must
 * be set to zero to match in a 34-bit access and SRIO address 65:50 must be set to zero to match
 * in a 50-bit access. This coding allows the maintenance bar window to appear in specific
 * address
 * spaces. Accesses through this BAR are limited to single word (32-bit) aligned transfers of one
 * to four bytes. Accesses which violate this rule will return an error response if possible and
 * be otherwise ignored.  The remaining bits are located in SRIOMAINT()_LCS_BA0.
 */
union cvmx_sriomaintx_lcs_ba1 {
	uint32_t u32;
	struct cvmx_sriomaintx_lcs_ba1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lcsba                        : 11; /**< SRIO address 34:24. */
	uint32_t reserved_0_20                : 21;
#else
	uint32_t reserved_0_20                : 21;
	uint32_t lcsba                        : 11;
#endif
	} s;
	struct cvmx_sriomaintx_lcs_ba1_s      cn63xx;
	struct cvmx_sriomaintx_lcs_ba1_s      cn63xxp1;
	struct cvmx_sriomaintx_lcs_ba1_s      cn66xx;
	struct cvmx_sriomaintx_lcs_ba1_cnf75xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t lcsba                        : 11; /**< SRIO address 34:24. */
	uint32_t reserved_20_0                : 21;
#else
	uint32_t reserved_20_0                : 21;
	uint32_t lcsba                        : 11;
#endif
	} cnf75xx;
};
typedef union cvmx_sriomaintx_lcs_ba1 cvmx_sriomaintx_lcs_ba1_t;

/**
 * cvmx_sriomaint#_m2s_bar0_start0
 *
 * The starting SRIO address to forwarded to the NPEI configuration space.
 * This register specifies the 50-bit and 66-bit SRIO address mapped to the BAR0 space.  See
 * SRIOMAINT()_M2S_BAR0_START1 for more details. This register is only writeable over SRIO if
 * the SRIO()_ACC_CTRL[DENY_BAR0] bit is zero.
 */
union cvmx_sriomaintx_m2s_bar0_start0 {
	uint32_t u32;
	struct cvmx_sriomaintx_m2s_bar0_start0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t addr64                       : 16; /**< SRIO address 63:48. */
	uint32_t addr48                       : 16; /**< SRIO address 47:32. */
#else
	uint32_t addr48                       : 16;
	uint32_t addr64                       : 16;
#endif
	} s;
	struct cvmx_sriomaintx_m2s_bar0_start0_s cn63xx;
	struct cvmx_sriomaintx_m2s_bar0_start0_s cn63xxp1;
	struct cvmx_sriomaintx_m2s_bar0_start0_s cn66xx;
	struct cvmx_sriomaintx_m2s_bar0_start0_s cnf75xx;
};
typedef union cvmx_sriomaintx_m2s_bar0_start0 cvmx_sriomaintx_m2s_bar0_start0_t;

/**
 * cvmx_sriomaint#_m2s_bar0_start1
 *
 * The starting SRIO address to forwarded to the NPEI configuration space.
 * This register specifies the SRIO address mapped to the BAR0 RSL space.  If the transaction
 * has not already been mapped to SRIO maintenance space through the SRIOMAINT_LCS_BA[1:0]
 * registers, if ENABLE is set and the address bits match then the SRIO memory transactions
 * will map to OCTEON SLI registers.  34-bit address transactions require a match in SRIO
 * address 33:24 and require all the other bits in ADDR48, ADDR64 and ADDR66 fields to be zero.
 * 50-bit address transactions a match of SRIO Address 49:24 and require all the other bits of
 * ADDR64 and ADDR66 to be zero.  66-bit address transactions require matches of all valid
 * address
 * field bits.  Reads and writes through Bar0 have a size limit of 8 bytes and cannot cross
 * a 64-bit boundary.  All accesses with sizes greater than this limit will be ignored and return
 * an error on any SRIO responses.  Note: ADDR48 and ADDR64 fields are located in
 * SRIOMAINT()_M2S_BAR0_START0.  The ADDR32/66 fields of this register
 * are writeable over SRIO if the SRIO()_ACC_CTRL[DENY_ADR0] bit is zero.  The ENABLE field is
 * writeable over SRIO if the SRIO()_ACC_CTRL[DENY_BAR0] bit is zero.
 */
union cvmx_sriomaintx_m2s_bar0_start1 {
	uint32_t u32;
	struct cvmx_sriomaintx_m2s_bar0_start1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_3_31                : 29;
	uint32_t addr66                       : 2;  /**< SRIO address 65:64. */
	uint32_t enable                       : 1;  /**< Enable BAR0 access. */
#else
	uint32_t enable                       : 1;
	uint32_t addr66                       : 2;
	uint32_t reserved_3_31                : 29;
#endif
	} s;
	struct cvmx_sriomaintx_m2s_bar0_start1_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t addr32                       : 18; /**< SRIO Address 31:14 */
	uint32_t reserved_3_13                : 11;
	uint32_t addr66                       : 2;  /**< SRIO Address 65:64 */
	uint32_t enable                       : 1;  /**< Enable BAR0 Access */
#else
	uint32_t enable                       : 1;
	uint32_t addr66                       : 2;
	uint32_t reserved_3_13                : 11;
	uint32_t addr32                       : 18;
#endif
	} cn63xx;
	struct cvmx_sriomaintx_m2s_bar0_start1_cn63xx cn63xxp1;
	struct cvmx_sriomaintx_m2s_bar0_start1_cn63xx cn66xx;
	struct cvmx_sriomaintx_m2s_bar0_start1_cnf75xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t addr32                       : 8;  /**< SRIO address 31:24. */
	uint32_t reserved_3_23                : 21;
	uint32_t addr66                       : 2;  /**< SRIO address 65:64. */
	uint32_t enable                       : 1;  /**< Enable BAR0 access. */
#else
	uint32_t enable                       : 1;
	uint32_t addr66                       : 2;
	uint32_t reserved_3_23                : 21;
	uint32_t addr32                       : 8;
#endif
	} cnf75xx;
};
typedef union cvmx_sriomaintx_m2s_bar0_start1 cvmx_sriomaintx_m2s_bar0_start1_t;

/**
 * cvmx_sriomaint#_m2s_bar1_start0
 *
 * The starting SRIO address to forwarded to the BAR1 memory space.
 * This register specifies the 50-bit and 66-bit SRIO address mapped to the BAR1 space.  See
 * SRIOMAINT()_M2S_BAR1_START1 for more details.  This register is only writeable over SRIO
 * if the SRIO()_ACC_CTRL[DENY_ADR1] bit is zero.
 */
union cvmx_sriomaintx_m2s_bar1_start0 {
	uint32_t u32;
	struct cvmx_sriomaintx_m2s_bar1_start0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t addr64                       : 16; /**< SRIO address 63:48. */
	uint32_t addr48                       : 16; /**< SRIO address 47:32.
                                                         The SRIO hardware does not use the low order
                                                         one or two bits of this field when BARSIZE is 12
                                                         or 13, respectively.
                                                         (BARSIZE is SRIOMAINT()_M2S_BAR1_START1[BARSIZE].) */
#else
	uint32_t addr48                       : 16;
	uint32_t addr64                       : 16;
#endif
	} s;
	struct cvmx_sriomaintx_m2s_bar1_start0_s cn63xx;
	struct cvmx_sriomaintx_m2s_bar1_start0_s cn63xxp1;
	struct cvmx_sriomaintx_m2s_bar1_start0_s cn66xx;
	struct cvmx_sriomaintx_m2s_bar1_start0_s cnf75xx;
};
typedef union cvmx_sriomaintx_m2s_bar1_start0 cvmx_sriomaintx_m2s_bar1_start0_t;

/**
 * cvmx_sriomaint#_m2s_bar1_start1
 *
 * The starting SRIO address to forwarded to the BAR1 memory space.
 * This register specifies the SRIO address mapped to the BAR1 space.  If the
 * transaction has not already been mapped to SRIO maintenance space through the
 * SRIOMAINT_LCS_BA[1:0] registers and the address bits do not match enabled BAR0
 * addresses and if ENABLE is set and the addresses match the BAR1 addresses then
 * SRIO memory transactions will map to OCTEON memory space specified by
 * SRIOMAINT()_BAR1_IDX[31:0] registers.  The BARSIZE field determines the size of
 * BAR1, the entry select bits, and the size of each entry.
 *
 * * A 34-bit address matches BAR1 when it matches SRIO_Address[33:20+BARSIZE] while
 * all the other bits in ADDR48, ADDR64 and ADDR66 are zero.
 *
 * * A 50-bit address matches BAR1 when it matches SRIO_Address[49:20+BARSIZE] while
 * all the other bits of ADDR64 and ADDR66 are zero.
 *
 * * A 66-bit address matches BAR1 when all of SRIO_Address[65:20+BARSIZE] match all
 * corresponding address CSR field bits.
 *
 * Note: ADDR48 and
 * ADDR64 fields are located in SRIOMAINT()_M2S_BAR1_START0. The ADDR32/66 fields of
 * this register are writeable over SRIO if SRIO()_ACC_CTRL[DENY_ADR1] is zero.
 * The remaining fields are writeable over SRIO if SRIO()_ACC_CTRL[DENY_BAR1] is zero.
 */
union cvmx_sriomaintx_m2s_bar1_start1 {
	uint32_t u32;
	struct cvmx_sriomaintx_m2s_bar1_start1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t addr32                       : 12; /**< SRIO address 31:20.
                                                         This field is not used by the SRIO hardware for
                                                         BARSIZE values 12 or 13.
                                                         With BARSIZE < 12, the upper 12-BARSIZE
                                                         bits of this field are used, and the lower BARSIZE
                                                         bits of this field are unused by the SRIO hardware. */
	uint32_t reserved_7_19                : 13;
	uint32_t barsize                      : 4;  /**< Bar size.
                                                         <pre>
                                                                           SRIO_Address*
                                                                            ---------------------
                                                                           /                     \
                                                         BARSIZE         BAR     Entry   Entry    Entry
                                                         Value   BAR    compare  Select  Offset   Size
                                                         Size    bits    bits    bits
                                                         0       1MB    65:20   19:16   15:0     64KB
                                                         1       2MB    65:21   20:17   16:0    128KB
                                                         2       4MB    65:22   21:18   17:0    256KB
                                                         3       8MB    65:23   22:19   18:0    512KB
                                                         4      16MB    65:24   23:20   19:0      1MB
                                                         5      32MB    65:25   24:21   20:0      2MB
                                                         6      64MB    65:26   25:22   21:0      4MB
                                                         7     128MB    65:27   26:23   22:0      8MB
                                                         8     256MB    65:28   27:24   23:0     16MB
                                                         9     512MB    65:29   28:25   24:0     32MB
                                                         10    1024MB   65:30   29:26   25:0     64MB
                                                         11    2048MB   65:31   30:27   26:0    128MB
                                                         12    4096MB   65:32   31:28   27:0    256MB
                                                         13    8192MB   65:33   32:29   28:0    512MB
                                                         </pre> */
	uint32_t addr66                       : 2;  /**< SRIO address 65:64. */
	uint32_t enable                       : 1;  /**< Enable BAR1 access. */
#else
	uint32_t enable                       : 1;
	uint32_t addr66                       : 2;
	uint32_t barsize                      : 4;
	uint32_t reserved_7_19                : 13;
	uint32_t addr32                       : 12;
#endif
	} s;
	struct cvmx_sriomaintx_m2s_bar1_start1_s cn63xx;
	struct cvmx_sriomaintx_m2s_bar1_start1_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t addr32                       : 12; /**< SRIO Address 31:20
                                                         With BARSIZE < 12, the upper 12-BARSIZE
                                                         bits of this field are used, and the lower BARSIZE
                                                         bits of this field are unused by the SRIO hardware. */
	uint32_t reserved_6_19                : 14;
	uint32_t barsize                      : 3;  /**< Bar Size.
                                                                              SRIO_Address*
                                                                         ---------------------
                                                                        /                     \
                                                         BARSIZE         BAR     Entry   Entry    Entry
                                                         Value   BAR    compare  Select  Offset   Size
                                                                 Size    bits    bits    bits
                                                          0       1MB    65:20   19:16   15:0     64KB
                                                          1       2MB    65:21   20:17   16:0    128KB
                                                          2       4MB    65:22   21:18   17:0    256KB
                                                          3       8MB    65:23   22:19   18:0    512KB
                                                          4      16MB    65:24   23:20   19:0      1MB
                                                          5      32MB    65:25   24:21   20:0      2MB
                                                          6      64MB    65:26   25:22   21:0      4MB
                                                          7     128MB    65:27   26:23   22:0      8MB
                                                          8     256MB  ** not in pass 1
                                                          9     512MB  ** not in pass 1
                                                         10       1GB  ** not in pass 1
                                                         11       2GB  ** not in pass 1
                                                         12       4GB  ** not in pass 1
                                                         13       8GB  ** not in pass 1

                                                         *The SRIO Transaction Address
                                                         The entry select bits is the X that  select an
                                                         SRIOMAINT(0..1)_BAR1_IDXX entry.

                                                         In O63 pass 2, BARSIZE is 4 bits (6:3 in this
                                                         CSR), and BARSIZE values 8-13 are implemented,
                                                         providing a total possible BAR1 size range from
                                                         1MB up to 8GB. */
	uint32_t addr66                       : 2;  /**< SRIO Address 65:64 */
	uint32_t enable                       : 1;  /**< Enable BAR1 Access */
#else
	uint32_t enable                       : 1;
	uint32_t addr66                       : 2;
	uint32_t barsize                      : 3;
	uint32_t reserved_6_19                : 14;
	uint32_t addr32                       : 12;
#endif
	} cn63xxp1;
	struct cvmx_sriomaintx_m2s_bar1_start1_s cn66xx;
	struct cvmx_sriomaintx_m2s_bar1_start1_s cnf75xx;
};
typedef union cvmx_sriomaintx_m2s_bar1_start1 cvmx_sriomaintx_m2s_bar1_start1_t;

/**
 * cvmx_sriomaint#_m2s_bar2_start
 *
 * The starting SRIO address to forwarded to the BAR2 memory space.
 * This register specifies the SRIO address mapped to the BAR2 space.  If ENABLE is set
 * and the address bits do not match the other enabled BAR address and match the BAR2
 * addresses then the SRIO memory transactions will map to OCTEON BAR2 memory space.
 * 34-bit address transactions require ADDR66, ADDR64 and ADDR48 fields set to zero
 * and supply zeros for unused local addresses 41:34.  50-bit address transactions a
 * match of SRIO address 49:42 and require all the other bits of ADDR64 and ADDR66 to
 * be zero. 66-bit address transactions require matches of all valid address field
 * bits. The ADDR32/48/64/66 fields of this register are writeable over SRIO if
 * SRIO()_ACC_CTRL[DENY_ADR2] is zero.  The remaining fields are writeable over SRIO
 * if SRIO()_ACC_CTRL[DENY_BAR2] is zero.
 */
union cvmx_sriomaintx_m2s_bar2_start {
	uint32_t u32;
	struct cvmx_sriomaintx_m2s_bar2_start_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t addr64                       : 16; /**< SRIO address 63:48. */
	uint32_t reserved_6_15                : 10;
	uint32_t esx                          : 2;  /**< Endian swap mode used for SRIO 34-bit access.
                                                         For 50/66-bit assesses endian swap is determine
                                                         by ESX XOR'd with SRIO address 43:42.
                                                         0x0 = No swap.
                                                         0x1 = 64-bit swap bytes [ABCD_EFGH] -> [HGFE_DCBA].
                                                         0x2 = 32-bit swap words [ABCD_EFGH] -> [DCBA_HGFE].
                                                         0x3 = 32-bit word exch  [ABCD_EFGH] -> [EFGH_ABCD]. */
	uint32_t cax                          : 1;  /**< Cacheable access mode.  When set transfer is
                                                         cached.  This bit is used for SRIO 34-bit access.
                                                         For 50/66-bit accesses NCA is determine by CAX
                                                         XOR'd with SRIO address 44. */
	uint32_t addr66                       : 2;  /**< SRIO address 65:64. */
	uint32_t enable                       : 1;  /**< Enable BAR2 access. */
#else
	uint32_t enable                       : 1;
	uint32_t addr66                       : 2;
	uint32_t cax                          : 1;
	uint32_t esx                          : 2;
	uint32_t reserved_6_15                : 10;
	uint32_t addr64                       : 16;
#endif
	} s;
	struct cvmx_sriomaintx_m2s_bar2_start_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t addr64                       : 16; /**< SRIO Address 63:48 */
	uint32_t addr48                       : 7;  /**< SRIO Address 47:41 */
	uint32_t reserved_6_8                 : 3;
	uint32_t esx                          : 2;  /**< Endian Swap Mode used for SRIO 34-bit access.
                                                         For 50/66-bit assesses Endian Swap is determine
                                                         by ESX XOR'd with SRIO Addr 39:38.
                                                         0 = No Swap
                                                         1 = 64-bit Swap Bytes [ABCD_EFGH] -> [HGFE_DCBA]
                                                         2 = 32-bit Swap Words [ABCD_EFGH] -> [DCBA_HGFE]
                                                         3 = 32-bit Word Exch  [ABCD_EFGH] -> [EFGH_ABCD] */
	uint32_t cax                          : 1;  /**< Cacheable Access Mode.  When set transfer is
                                                         cached.  This bit is used for SRIO 34-bit access.
                                                         For 50/66-bit accessas NCA is determine by CAX
                                                         XOR'd with SRIO Addr 40. */
	uint32_t addr66                       : 2;  /**< SRIO Address 65:64 */
	uint32_t enable                       : 1;  /**< Enable BAR2 Access */
#else
	uint32_t enable                       : 1;
	uint32_t addr66                       : 2;
	uint32_t cax                          : 1;
	uint32_t esx                          : 2;
	uint32_t reserved_6_8                 : 3;
	uint32_t addr48                       : 7;
	uint32_t addr64                       : 16;
#endif
	} cn63xx;
	struct cvmx_sriomaintx_m2s_bar2_start_cn63xx cn63xxp1;
	struct cvmx_sriomaintx_m2s_bar2_start_cn63xx cn66xx;
	struct cvmx_sriomaintx_m2s_bar2_start_cnf75xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t addr64                       : 16; /**< SRIO address 63:48. */
	uint32_t addr48                       : 3;  /**< SRIO address 47:45. */
	uint32_t reserved_6_12                : 7;
	uint32_t esx                          : 2;  /**< Endian swap mode used for SRIO 34-bit access.
                                                         For 50/66-bit assesses endian swap is determine
                                                         by ESX XOR'd with SRIO address 43:42.
                                                         0x0 = No swap.
                                                         0x1 = 64-bit swap bytes [ABCD_EFGH] -> [HGFE_DCBA].
                                                         0x2 = 32-bit swap words [ABCD_EFGH] -> [DCBA_HGFE].
                                                         0x3 = 32-bit word exch  [ABCD_EFGH] -> [EFGH_ABCD]. */
	uint32_t cax                          : 1;  /**< Cacheable access mode.  When set transfer is
                                                         cached.  This bit is used for SRIO 34-bit access.
                                                         For 50/66-bit accesses NCA is determine by CAX
                                                         XOR'd with SRIO address 44. */
	uint32_t addr66                       : 2;  /**< SRIO address 65:64. */
	uint32_t enable                       : 1;  /**< Enable BAR2 access. */
#else
	uint32_t enable                       : 1;
	uint32_t addr66                       : 2;
	uint32_t cax                          : 1;
	uint32_t esx                          : 2;
	uint32_t reserved_6_12                : 7;
	uint32_t addr48                       : 3;
	uint32_t addr64                       : 16;
#endif
	} cnf75xx;
};
typedef union cvmx_sriomaintx_m2s_bar2_start cvmx_sriomaintx_m2s_bar2_start_t;

/**
 * cvmx_sriomaint#_mac_ctrl
 *
 * This register enables MAC optimizations that may not be supported by all SRIO devices.  The
 * default values should be supported.  This register can be changed at any time while the MAC is
 * out of reset.
 */
union cvmx_sriomaintx_mac_ctrl {
	uint32_t u32;
	struct cvmx_sriomaintx_mac_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_21_31               : 11;
	uint32_t sec_spf                      : 1;  /**< Send all incoming packets matching secondary ID to
                                                         RX soft packet FIFO.  This bit is ignored if
                                                         RX_SPF is set. */
	uint32_t ack_zero                     : 1;  /**< Generate ACKs for all incoming zero byte packets.
                                                         Default behavior is to issue a NACK.  Regardless
                                                         of this setting the SRIO()_INT_REG[ZERO_PKT]
                                                         interrupt is generated in SRIO()_INT_REG. */
	uint32_t rx_spf                       : 1;  /**< Route all received packets to RX soft packet FIFO.
                                                         No logical layer ERB errors will be reported.
                                                         For diagnostic use only. */
	uint32_t eop_mrg                      : 1;  /**< Transmitted packets can eliminate EOP symbol on
                                                         back to back packets. */
	uint32_t type_mrg                     : 1;  /**< Allow STYPE merging on transmit. */
	uint32_t lnk_rtry                     : 16; /**< Number of times MAC will reissue link request
                                                         after timeout.  If retry count is exceeded a fatal
                                                         port error will occur (see SRIO()_INT_REG[F_ERROR]). */
#else
	uint32_t lnk_rtry                     : 16;
	uint32_t type_mrg                     : 1;
	uint32_t eop_mrg                      : 1;
	uint32_t rx_spf                       : 1;
	uint32_t ack_zero                     : 1;
	uint32_t sec_spf                      : 1;
	uint32_t reserved_21_31               : 11;
#endif
	} s;
	struct cvmx_sriomaintx_mac_ctrl_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_20_31               : 12;
	uint32_t ack_zero                     : 1;  /**< Generate ACKs for all incoming Zero Byte packets.
                                                         Default behavior is to issue a NACK.  Regardless
                                                         of this setting the SRIO(0..1)_INT_REG.ZERO_PKT
                                                         interrupt is generated.
                                                         SRIO(0..1)_INT_REG. */
	uint32_t rx_spf                       : 1;  /**< Route all received packets to RX Soft Packet FIFO.
                                                         No logical layer ERB Errors will be reported.
                                                         Used for Diagnostics Only. */
	uint32_t eop_mrg                      : 1;  /**< Transmitted Packets can eliminate EOP Symbol on
                                                         back to back packets. */
	uint32_t type_mrg                     : 1;  /**< Allow STYPE Merging on Transmit. */
	uint32_t lnk_rtry                     : 16; /**< Number of times MAC will reissue Link Request
                                                         after timeout.  If retry count is exceeded Fatal
                                                         Port Error will occur (see SRIO(0..1)_INT_REG.F_ERROR) */
#else
	uint32_t lnk_rtry                     : 16;
	uint32_t type_mrg                     : 1;
	uint32_t eop_mrg                      : 1;
	uint32_t rx_spf                       : 1;
	uint32_t ack_zero                     : 1;
	uint32_t reserved_20_31               : 12;
#endif
	} cn63xx;
	struct cvmx_sriomaintx_mac_ctrl_s     cn66xx;
	struct cvmx_sriomaintx_mac_ctrl_s     cnf75xx;
};
typedef union cvmx_sriomaintx_mac_ctrl cvmx_sriomaintx_mac_ctrl_t;

/**
 * cvmx_sriomaint#_pe_feat
 *
 * This register describes the major functionality provided by the SRIO device.
 *
 */
union cvmx_sriomaintx_pe_feat {
	uint32_t u32;
	struct cvmx_sriomaintx_pe_feat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t bridge                       : 1;  /**< Bridge functions not supported. */
	uint32_t memory                       : 1;  /**< PE contains addressable memory. */
	uint32_t proc                         : 1;  /**< PE contains a local processor. */
	uint32_t switchf                      : 1;  /**< Switch functions not supported. */
	uint32_t mult_prt                     : 1;  /**< Multiport functions not supported. */
	uint32_t reserved_7_26                : 20;
	uint32_t suppress                     : 1;  /**< Error recovery suppression not supported. */
	uint32_t crf                          : 1;  /**< Critical request flow not supported. */
	uint32_t lg_tran                      : 1;  /**< Large transport (16-bit device IDs) supported. */
	uint32_t ex_feat                      : 1;  /**< Extended feature pointer is valid. */
	uint32_t ex_addr                      : 3;  /**< PE supports 66, 50 and 34-bit addresses.
                                                         [2:1] are a RO copy of SRIO()_IP_FEATURE[A66,A50]. */
#else
	uint32_t ex_addr                      : 3;
	uint32_t ex_feat                      : 1;
	uint32_t lg_tran                      : 1;
	uint32_t crf                          : 1;
	uint32_t suppress                     : 1;
	uint32_t reserved_7_26                : 20;
	uint32_t mult_prt                     : 1;
	uint32_t switchf                      : 1;
	uint32_t proc                         : 1;
	uint32_t memory                       : 1;
	uint32_t bridge                       : 1;
#endif
	} s;
	struct cvmx_sriomaintx_pe_feat_s      cn63xx;
	struct cvmx_sriomaintx_pe_feat_s      cn63xxp1;
	struct cvmx_sriomaintx_pe_feat_s      cn66xx;
	struct cvmx_sriomaintx_pe_feat_s      cnf75xx;
};
typedef union cvmx_sriomaintx_pe_feat cvmx_sriomaintx_pe_feat_t;

/**
 * cvmx_sriomaint#_pe_llc
 *
 * The register is used for general configuration for the logical
 * interface.
 */
union cvmx_sriomaintx_pe_llc {
	uint32_t u32;
	struct cvmx_sriomaintx_pe_llc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_3_31                : 29;
	uint32_t ex_addr                      : 3;  /**< Controls the number of address bits generated by
                                                         PE as a source and processed by the PE as a
                                                         target of an operation.
                                                         0x1 = 34-bit Addresses.
                                                         0x2 = 50-bit Addresses.
                                                         0x4 = 66-bit Addresses.
                                                         All other encodings are reserved. */
#else
	uint32_t ex_addr                      : 3;
	uint32_t reserved_3_31                : 29;
#endif
	} s;
	struct cvmx_sriomaintx_pe_llc_s       cn63xx;
	struct cvmx_sriomaintx_pe_llc_s       cn63xxp1;
	struct cvmx_sriomaintx_pe_llc_s       cn66xx;
	struct cvmx_sriomaintx_pe_llc_s       cnf75xx;
};
typedef union cvmx_sriomaintx_pe_llc cvmx_sriomaintx_pe_llc_t;

/**
 * cvmx_sriomaint#_port_0_ctl
 *
 * This register contains assorted control bits.
 *
 */
union cvmx_sriomaintx_port_0_ctl {
	uint32_t u32;
	struct cvmx_sriomaintx_port_0_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pt_width                     : 2;  /**< Hardware port width:
                                                         0x0 = One Lane supported.
                                                         0x1 = One/Four Lanes supported.
                                                         0x2 = One/Two Lanes supported.
                                                         0x3 = One/Two/Four Lanes supported.
                                                         This value is a copy of SRIO()_IP_FEATURE[PT_WIDTH]
                                                         limited by the number of lanes the MAC has. */
	uint32_t it_width                     : 3;  /**< Initialized port width:
                                                         0x0 = Single-lane, lane 0.
                                                         0x1 = Single-lane, lane 1 or 2.
                                                         0x2 = Four-lane.
                                                         0x3 = Two-lane.
                                                         0x7 = Link uninitialized.
                                                         Others = Reserved. */
	uint32_t ov_width                     : 3;  /**< Override Port Width.  Writing this register causes
                                                         the port to reinitialize.
                                                         0x0 = No override all lanes possible.
                                                         0x1 = Reserved.
                                                         0x2 = Force single-lane, lane 0.
                                                         If Ln 0 is unavailable try lane 2 then lane 1.
                                                         0x3 = Force single-lane, lane 2.
                                                         If Ln 2 is unavailable try lane 1 then lane 0.
                                                         0x4 = Reserved.
                                                         0x5 = Enable two-lane, disable four-lane.
                                                         0x6 = Enable four-lane, disable two-lane.
                                                         0x7 = All lanes sizes enabled. */
	uint32_t reserved_23_23               : 1;
	uint32_t o_enable                     : 1;  /**< Port output enable. When cleared, port will generate control symbols and respond
                                                         to maintenance transactions only. When set, all transactions are allowed. */
	uint32_t i_enable                     : 1;  /**< Port input enable. When cleared, port will generate control symbols and respond
                                                         to maintenance packets only. All other packets will not be accepted. */
	uint32_t dis_err                      : 1;  /**< Disable error checking.  For diagnostic use only. */
	uint32_t mcast                        : 1;  /**< Reserved. */
	uint32_t reserved_18_18               : 1;
	uint32_t enumb                        : 1;  /**< Enumeration boundry. Software can use this bit to determine port enumeration. */
	uint32_t reserved_16_16               : 1;
	uint32_t ex_width                     : 2;  /**< Extended port width not supported. */
	uint32_t ex_stat                      : 2;  /**< Extended port width status. 0x0 = not supported. */
	uint32_t suppress                     : 8;  /**< Retransmit suppression mask.  CRF not Supported. */
	uint32_t stp_port                     : 1;  /**< Stop on failed port. This bit is used with the DROP_PKT bit to force certain
                                                         behavior when the error rate failed threshold has been met or exceeded. */
	uint32_t drop_pkt                     : 1;  /**< Drop on failed port. This bit is used with the STP_PORT bit to force certain
                                                         behavior when the error rate failed threshold has been met or exceeded. */
	uint32_t prt_lock                     : 1;  /**< When this bit is cleared, the packets that may be
                                                         received and issued are controlled by the state of
                                                         the O_ENABLE and I_ENABLE bits.  When this bit is
                                                         set, this port is stopped and is not enabled to
                                                         receive any packets; the input port can still
                                                         follow the training procedure and can still send
                                                         respond to link-requests; all received packets
                                                         return packet-not-accepted control symbols to
                                                         force an error condition to be signaled by the
                                                         sending device.  The O_ENABLE bit should also be
                                                         cleared to disable packet output. */
	uint32_t pt_type                      : 1;  /**< Port type.  1 = Serial port. */
#else
	uint32_t pt_type                      : 1;
	uint32_t prt_lock                     : 1;
	uint32_t drop_pkt                     : 1;
	uint32_t stp_port                     : 1;
	uint32_t suppress                     : 8;
	uint32_t ex_stat                      : 2;
	uint32_t ex_width                     : 2;
	uint32_t reserved_16_16               : 1;
	uint32_t enumb                        : 1;
	uint32_t reserved_18_18               : 1;
	uint32_t mcast                        : 1;
	uint32_t dis_err                      : 1;
	uint32_t i_enable                     : 1;
	uint32_t o_enable                     : 1;
	uint32_t reserved_23_23               : 1;
	uint32_t ov_width                     : 3;
	uint32_t it_width                     : 3;
	uint32_t pt_width                     : 2;
#endif
	} s;
	struct cvmx_sriomaintx_port_0_ctl_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pt_width                     : 2;  /**< Hardware Port Width.
                                                         00 = One Lane supported.
                                                         01 = One/Four Lanes supported.
                                                         10 = One/Two Lanes supported.
                                                         11 = One/Two/Four Lanes supported.
                                                         Pass 1 initialized to 0x2 but x1/x4 supported
                                                         This is a RO copy of SRIO*_IP_FEATURE[PT_WIDTH]. */
	uint32_t it_width                     : 3;  /**< Initialized Port Width
                                                         000 = Single-lane, Lane 0
                                                         001 = Single-lane, Lane 1 or 2
                                                         010 = Four-lane
                                                         011 = Two-lane
                                                         111 = Link Uninitialized
                                                         Others = Reserved */
	uint32_t ov_width                     : 3;  /**< Override Port Width.  Writing this register causes
                                                         the port to reinitialize.
                                                         000 = No Override all lanes possible
                                                         001 = Reserved
                                                         010 = Force Single-lane, Lane 0
                                                         011 = Force Single-lane, Lane 2
                                                               (Lane 1 if only lanes 0,1 are connected)
                                                         100 = Reserved
                                                         101 = Force Two-lane, Disable Four-Lane
                                                         110 = Force Four-lane, Disable Two-Lane
                                                         111 = All lanes sizes enabled */
	uint32_t disable                      : 1;  /**< Port Disable.  Setting this bit should disable
                                                         both SERDES drivers and receivers.
                                                         On Passes 1.x, 2.0 and 2.1 this bit disables only
                                                         the receiver side.  On Pass 2.2, this bit is
                                                         ignored by the hardware. */
	uint32_t o_enable                     : 1;  /**< Port Output Enable.  When cleared, port will
                                                         generate control symbols and respond to
                                                         maintenance transactions only.  When set, all
                                                         transactions are allowed. */
	uint32_t i_enable                     : 1;  /**< Port Input Enable.  When cleared, port will
                                                         generate control symbols and respond to
                                                         maintenance packets only.  All other packets will
                                                         not be accepted. */
	uint32_t dis_err                      : 1;  /**< Disable Error Checking.  Diagnostic Only. */
	uint32_t mcast                        : 1;  /**< Reserved. */
	uint32_t reserved_18_18               : 1;
	uint32_t enumb                        : 1;  /**< Enumeration Boundry. SW can use this bit to
                                                         determine port enumeration. */
	uint32_t reserved_16_16               : 1;
	uint32_t ex_width                     : 2;  /**< Extended Port Width not supported. */
	uint32_t ex_stat                      : 2;  /**< Extended Port Width Status. 00 = not supported */
	uint32_t suppress                     : 8;  /**< Retransmit Suppression Mask.  CRF not Supported. */
	uint32_t stp_port                     : 1;  /**< Stop on Failed Port.  This bit is used with the
                                                         DROP_PKT bit to force certain behavior when the
                                                         Error Rate Failed Threshold has been met or
                                                         exceeded. */
	uint32_t drop_pkt                     : 1;  /**< Drop on Failed Port.  This bit is used with the
                                                         STP_PORT bit to force certain behavior when the
                                                         Error Rate Failed Threshold has been met or
                                                         exceeded. */
	uint32_t prt_lock                     : 1;  /**< When this bit is cleared, the packets that may be
                                                         received and issued are controlled by the state of
                                                         the O_ENABLE and I_ENABLE bits.  When this bit is
                                                         set, this port is stopped and is not enabled to
                                                         receive any packets; the input port can still
                                                         follow the training procedure and can still send
                                                         respond to link-requests; all received packets
                                                         return packet-not-accepted control symbols to
                                                         force an error condition to be signaled by the
                                                         sending device.  The O_ENABLE bit should also be
                                                         cleared to disable packet output. */
	uint32_t pt_type                      : 1;  /**< Port Type.  1 = Serial port. */
#else
	uint32_t pt_type                      : 1;
	uint32_t prt_lock                     : 1;
	uint32_t drop_pkt                     : 1;
	uint32_t stp_port                     : 1;
	uint32_t suppress                     : 8;
	uint32_t ex_stat                      : 2;
	uint32_t ex_width                     : 2;
	uint32_t reserved_16_16               : 1;
	uint32_t enumb                        : 1;
	uint32_t reserved_18_18               : 1;
	uint32_t mcast                        : 1;
	uint32_t dis_err                      : 1;
	uint32_t i_enable                     : 1;
	uint32_t o_enable                     : 1;
	uint32_t disable                      : 1;
	uint32_t ov_width                     : 3;
	uint32_t it_width                     : 3;
	uint32_t pt_width                     : 2;
#endif
	} cn63xx;
	struct cvmx_sriomaintx_port_0_ctl_cn63xx cn63xxp1;
	struct cvmx_sriomaintx_port_0_ctl_cn63xx cn66xx;
	struct cvmx_sriomaintx_port_0_ctl_cnf75xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t pt_width                     : 2;  /**< Hardware port width:
                                                         0x0 = One Lane supported.
                                                         0x1 = One/Four Lanes supported.
                                                         0x2 = One/Two Lanes supported.
                                                         0x3 = One/Two/Four Lanes supported.
                                                         This value is a copy of SRIO()_IP_FEATURE[PT_WIDTH]
                                                         limited by the number of lanes the MAC has. */
	uint32_t it_width                     : 3;  /**< Initialized port width:
                                                         0x0 = Single-lane, lane 0.
                                                         0x1 = Single-lane, lane 1 or 2.
                                                         0x2 = Four-lane.
                                                         0x3 = Two-lane.
                                                         0x7 = Link uninitialized.
                                                         Others = Reserved. */
	uint32_t ov_width                     : 3;  /**< Override Port Width.  Writing this register causes
                                                         the port to reinitialize.
                                                         0x0 = No override all lanes possible.
                                                         0x1 = Reserved.
                                                         0x2 = Force single-lane, lane 0.
                                                         If Ln 0 is unavailable try lane 2 then lane 1.
                                                         0x3 = Force single-lane, lane 2.
                                                         If Ln 2 is unavailable try lane 1 then lane 0.
                                                         0x4 = Reserved.
                                                         0x5 = Enable two-lane, disable four-lane.
                                                         0x6 = Enable four-lane, disable two-lane.
                                                         0x7 = All lanes sizes enabled. */
	uint32_t port_disable                 : 1;  /**< Port disable. Setting this bit should disable SerDes drivers and receivers. On
                                                         this chip it disables SerDes receivers only. */
	uint32_t o_enable                     : 1;  /**< Port output enable. When cleared, port will generate control symbols and respond
                                                         to maintenance transactions only. When set, all transactions are allowed. */
	uint32_t i_enable                     : 1;  /**< Port input enable. When cleared, port will generate control symbols and respond
                                                         to maintenance packets only. All other packets will not be accepted. */
	uint32_t dis_err                      : 1;  /**< Disable error checking.  For diagnostic use only. */
	uint32_t mcast                        : 1;  /**< Reserved. */
	uint32_t reserved_18_18               : 1;
	uint32_t enumb                        : 1;  /**< Enumeration boundry. Software can use this bit to determine port enumeration. */
	uint32_t reserved_16_16               : 1;
	uint32_t ex_width                     : 2;  /**< Extended port width not supported. */
	uint32_t ex_stat                      : 2;  /**< Extended port width status. 0x0 = not supported. */
	uint32_t suppress                     : 8;  /**< Retransmit suppression mask.  CRF not Supported. */
	uint32_t stp_port                     : 1;  /**< Stop on failed port. This bit is used with the DROP_PKT bit to force certain
                                                         behavior when the error rate failed threshold has been met or exceeded. */
	uint32_t drop_pkt                     : 1;  /**< Drop on failed port. This bit is used with the STP_PORT bit to force certain
                                                         behavior when the error rate failed threshold has been met or exceeded. */
	uint32_t prt_lock                     : 1;  /**< When this bit is cleared, the packets that may be
                                                         received and issued are controlled by the state of
                                                         the O_ENABLE and I_ENABLE bits.  When this bit is
                                                         set, this port is stopped and is not enabled to
                                                         receive any packets; the input port can still
                                                         follow the training procedure and can still send
                                                         respond to link-requests; all received packets
                                                         return packet-not-accepted control symbols to
                                                         force an error condition to be signaled by the
                                                         sending device.  The O_ENABLE bit should also be
                                                         cleared to disable packet output. */
	uint32_t pt_type                      : 1;  /**< Port type.  1 = Serial port. */
#else
	uint32_t pt_type                      : 1;
	uint32_t prt_lock                     : 1;
	uint32_t drop_pkt                     : 1;
	uint32_t stp_port                     : 1;
	uint32_t suppress                     : 8;
	uint32_t ex_stat                      : 2;
	uint32_t ex_width                     : 2;
	uint32_t reserved_16_16               : 1;
	uint32_t enumb                        : 1;
	uint32_t reserved_18_18               : 1;
	uint32_t mcast                        : 1;
	uint32_t dis_err                      : 1;
	uint32_t i_enable                     : 1;
	uint32_t o_enable                     : 1;
	uint32_t port_disable                 : 1;
	uint32_t ov_width                     : 3;
	uint32_t it_width                     : 3;
	uint32_t pt_width                     : 2;
#endif
	} cnf75xx;
};
typedef union cvmx_sriomaintx_port_0_ctl cvmx_sriomaintx_port_0_ctl_t;

/**
 * cvmx_sriomaint#_port_0_ctl2
 *
 * These registers are accessed when a local processor or an external
 * device wishes to examine the port baudrate information.  The automatic
 * baud rate feature is not available on this device. The SUP_* and ENB_*
 * fields are set directly by the SRIO()_STATUS_REG[SPD] bits as a
 * reference but otherwise have no effect.
 *
 * WARNING!!  Writes to this register will reinitialize the SRIO link.
 */
union cvmx_sriomaintx_port_0_ctl2 {
	uint32_t u32;
	struct cvmx_sriomaintx_port_0_ctl2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t sel_baud                     : 4;  /**< Indicates the speed of the interface SerDes lanes.
                                                         Link baud rate selected:
                                                         0x0 = No rate selected.
                                                         0x1 = 1.25 GBaud.
                                                         0x2 = 2.5 GBaud.
                                                         0x3 = 3.125 GBaud.
                                                         0x4 = 5.0 GBaud.
                                                         0x5 = 6.25 GBaud (reserved).
                                                         0x6-0xF = Reserved. */
	uint32_t baud_sup                     : 1;  /**< Automatic baud rate discovery not supported. */
	uint32_t baud_enb                     : 1;  /**< Auto baud rate discovery enable. */
	uint32_t sup_125g                     : 1;  /**< 1.25 GB rate operation supported.
                                                         Set when the interface SerDes lanes are operating
                                                         at 1.25 Gbaud (as selected by SRIO()_STATUS_REG[SPD]). */
	uint32_t enb_125g                     : 1;  /**< 1.25 GB rate operation enable.
                                                         Set when the interface SerDes lanes are operating
                                                         at 1.25 Gbaud (as selected by SRIO()_STATUS_REG[SPD]). */
	uint32_t sup_250g                     : 1;  /**< 2.50 GB rate operation supported.
                                                         Set when the interface SerDes lanes are operating
                                                         at 2.5 Gbaud (as selected by SRIO()_STATUS_REG[SPD]). */
	uint32_t enb_250g                     : 1;  /**< 2.50 GB rate operation enable.
                                                         Set when the interface SerDes lanes are operating
                                                         at 2.5 Gbaud (as selected by SRIO()_STATUS_REG[SPD]). */
	uint32_t sup_312g                     : 1;  /**< 3.125 GB rate operation supported.
                                                         Set when the interface SerDes lanes are operating
                                                         at 3.125 Gbaud (as selected by SRIO()_STATUS_REG[SPD]). */
	uint32_t enb_312g                     : 1;  /**< 3.125 GB rate operation enable.
                                                         Set when the interface SerDes lanes are operating
                                                         at 3.125 Gbaud (as selected by SRIO()_STATUS_REG[SPD]). */
	uint32_t sub_500g                     : 1;  /**< 5.0 GB rate operation supported.
                                                         Set when the interface SerDes lanes are operating
                                                         at 5.0 Gbaud (as selected by SRIO()_STATUS_REG[SPD]). */
	uint32_t enb_500g                     : 1;  /**< 5.0 GB rate operation enable.
                                                         Set when the interface SerDes lanes are operating
                                                         at 5.0 Gbaud (as selected by SRIO()_STATUS_REG[SPD]). */
	uint32_t sup_625g                     : 1;  /**< 6.25 GB rate operation (not supported). */
	uint32_t enb_625g                     : 1;  /**< 6.25 GB rate operation enable. */
	uint32_t reserved_2_15                : 14;
	uint32_t tx_emph                      : 1;  /**< Indicates whether is port is able to transmit
                                                         commands to control the transmit emphasis in the
                                                         connected port. */
	uint32_t emph_en                      : 1;  /**< Controls whether a port may adjust the
                                                         transmit emphasis in the connected port.  This bit
                                                         should be cleared for normal operation. */
#else
	uint32_t emph_en                      : 1;
	uint32_t tx_emph                      : 1;
	uint32_t reserved_2_15                : 14;
	uint32_t enb_625g                     : 1;
	uint32_t sup_625g                     : 1;
	uint32_t enb_500g                     : 1;
	uint32_t sub_500g                     : 1;
	uint32_t enb_312g                     : 1;
	uint32_t sup_312g                     : 1;
	uint32_t enb_250g                     : 1;
	uint32_t sup_250g                     : 1;
	uint32_t enb_125g                     : 1;
	uint32_t sup_125g                     : 1;
	uint32_t baud_enb                     : 1;
	uint32_t baud_sup                     : 1;
	uint32_t sel_baud                     : 4;
#endif
	} s;
	struct cvmx_sriomaintx_port_0_ctl2_s  cn63xx;
	struct cvmx_sriomaintx_port_0_ctl2_s  cn63xxp1;
	struct cvmx_sriomaintx_port_0_ctl2_s  cn66xx;
	struct cvmx_sriomaintx_port_0_ctl2_s  cnf75xx;
};
typedef union cvmx_sriomaintx_port_0_ctl2 cvmx_sriomaintx_port_0_ctl2_t;

/**
 * cvmx_sriomaint#_port_0_err_stat
 *
 * This register displays port error and status information.  Several port error conditions are
 * captured here and must be cleared by writing 1's to the individual bits.
 */
union cvmx_sriomaintx_port_0_err_stat {
	uint32_t u32;
	struct cvmx_sriomaintx_port_0_err_stat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_27_31               : 5;
	uint32_t pkt_drop                     : 1;  /**< Output packet dropped. */
	uint32_t o_fail                       : 1;  /**< Output port has encountered a failure condition,
                                                         meaning the port's failed error threshold has
                                                         reached SRIOMAINT()_ERB_ERR_RATE_THR[ER_FAIL] value. */
	uint32_t o_dgrad                      : 1;  /**< Output port has encountered a degraded condition,
                                                         meaning the port's degraded threshold has
                                                         reached SRIOMAINT()_ERB_ERR_RATE_THR[ER_DGRAD]
                                                         value. */
	uint32_t reserved_21_23               : 3;
	uint32_t o_retry                      : 1;  /**< Output retry encountered.  This bit is set when
                                                         bit 18 is set. */
	uint32_t o_rtried                     : 1;  /**< Output port has received a packet-retry condition
                                                         and cannot make forward progress.  This bit is set
                                                         when  bit 18 is set and is cleared when a packet-
                                                         accepted or a packet-not-accepted control symbol
                                                         is received. */
	uint32_t o_sm_ret                     : 1;  /**< Output port state machine has received a
                                                         packet-retry control symbol and is retrying the
                                                         packet. */
	uint32_t o_error                      : 1;  /**< Output error encountered and possibly recovered
                                                         from.  This sticky bit is set with bit 16. */
	uint32_t o_sm_err                     : 1;  /**< Output port state machine has encountered an
                                                         error. */
	uint32_t reserved_11_15               : 5;
	uint32_t i_sm_ret                     : 1;  /**< Input port state machine has received a
                                                         packet-retry control symbol and is retrying the
                                                         packet. */
	uint32_t i_error                      : 1;  /**< Input error encountered and possibly recovered
                                                         from.  This sticky bit is set with bit 8. */
	uint32_t i_sm_err                     : 1;  /**< Input port state machine has encountered an
                                                         error. */
	uint32_t reserved_5_7                 : 3;
	uint32_t pt_write                     : 1;  /**< Port has encountered a condition which required it
                                                         initiate a maintenance port-write operation.
                                                         Never set by hardware. */
	uint32_t reserved_3_3                 : 1;
	uint32_t pt_error                     : 1;  /**< Input or output port has encountered an
                                                         unrecoverable error condition. */
	uint32_t pt_ok                        : 1;  /**< Input or output port are intitialized and the port
                                                         is exchanging error free control symbols with
                                                         attached device. */
	uint32_t pt_uinit                     : 1;  /**< Port is uninitialized.  This bit and bit 1 are
                                                         mutually exclusive. */
#else
	uint32_t pt_uinit                     : 1;
	uint32_t pt_ok                        : 1;
	uint32_t pt_error                     : 1;
	uint32_t reserved_3_3                 : 1;
	uint32_t pt_write                     : 1;
	uint32_t reserved_5_7                 : 3;
	uint32_t i_sm_err                     : 1;
	uint32_t i_error                      : 1;
	uint32_t i_sm_ret                     : 1;
	uint32_t reserved_11_15               : 5;
	uint32_t o_sm_err                     : 1;
	uint32_t o_error                      : 1;
	uint32_t o_sm_ret                     : 1;
	uint32_t o_rtried                     : 1;
	uint32_t o_retry                      : 1;
	uint32_t reserved_21_23               : 3;
	uint32_t o_dgrad                      : 1;
	uint32_t o_fail                       : 1;
	uint32_t pkt_drop                     : 1;
	uint32_t reserved_27_31               : 5;
#endif
	} s;
	struct cvmx_sriomaintx_port_0_err_stat_s cn63xx;
	struct cvmx_sriomaintx_port_0_err_stat_s cn63xxp1;
	struct cvmx_sriomaintx_port_0_err_stat_s cn66xx;
	struct cvmx_sriomaintx_port_0_err_stat_s cnf75xx;
};
typedef union cvmx_sriomaintx_port_0_err_stat cvmx_sriomaintx_port_0_err_stat_t;

/**
 * cvmx_sriomaint#_port_0_link_req
 *
 * Writing this register generates the link request symbol or eight device reset
 * symbols. The progress of the request can be determined by reading
 * SRIOMAINT()_PORT_0_LINK_RESP. Only a single request should be generated at a time.
 */
union cvmx_sriomaintx_port_0_link_req {
	uint32_t u32;
	struct cvmx_sriomaintx_port_0_link_req_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_3_31                : 29;
	uint32_t cmd                          : 3;  /**< Link request command.
                                                         0x3 = Reset device.
                                                         0x4 = Link request.
                                                         _ All other values reserved. */
#else
	uint32_t cmd                          : 3;
	uint32_t reserved_3_31                : 29;
#endif
	} s;
	struct cvmx_sriomaintx_port_0_link_req_s cn63xx;
	struct cvmx_sriomaintx_port_0_link_req_s cn66xx;
	struct cvmx_sriomaintx_port_0_link_req_s cnf75xx;
};
typedef union cvmx_sriomaintx_port_0_link_req cvmx_sriomaintx_port_0_link_req_t;

/**
 * cvmx_sriomaint#_port_0_link_resp
 *
 * This register only returns responses generated by writes to SRIOMAINT()_PORT_0_LINK_REQ.
 *
 */
union cvmx_sriomaintx_port_0_link_resp {
	uint32_t u32;
	struct cvmx_sriomaintx_port_0_link_resp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t valid                        : 1;  /**< Link response valid.
                                                         0 = No response received.
                                                         1 = Link Response Received or Reset Device
                                                         Symbols Transmitted.  Value cleared on read. */
	uint32_t reserved_11_30               : 20;
	uint32_t ackid                        : 6;  /**< AckID received from link response.
                                                         Reset Device symbol response is always zero.
                                                         Bit 10 is used for IDLE2 and always reads zero. */
	uint32_t status                       : 5;  /**< Link response status.
                                                         Status supplied by link response.
                                                         Reset Device symbol response is always zero. */
#else
	uint32_t status                       : 5;
	uint32_t ackid                        : 6;
	uint32_t reserved_11_30               : 20;
	uint32_t valid                        : 1;
#endif
	} s;
	struct cvmx_sriomaintx_port_0_link_resp_s cn63xx;
	struct cvmx_sriomaintx_port_0_link_resp_s cn66xx;
	struct cvmx_sriomaintx_port_0_link_resp_s cnf75xx;
};
typedef union cvmx_sriomaintx_port_0_link_resp cvmx_sriomaintx_port_0_link_resp_t;

/**
 * cvmx_sriomaint#_port_0_local_ackid
 *
 * This register is typically only written when recovering from a failed link. It may
 * be read at any time the MAC is out of reset. Writes to the O_ACKID field will be
 * used for both the O_ACKID and E_ACKID. Care must be taken to ensure that no packets
 * are pending at the time of a write. The number of pending packets can be read in the
 * TX_INUSE field of SRIO()_MAC_BUFFERS.
 */
union cvmx_sriomaintx_port_0_local_ackid {
	uint32_t u32;
	struct cvmx_sriomaintx_port_0_local_ackid_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_30_31               : 2;
	uint32_t i_ackid                      : 6;  /**< Next expected inbound AckID.
                                                         Bit 29 is used for IDLE2 and should be zero. */
	uint32_t reserved_14_23               : 10;
	uint32_t e_ackid                      : 6;  /**< Next expected unacknowledged AckID.
                                                         Bit 13 is used for IDLE2 and should be zero. */
	uint32_t reserved_6_7                 : 2;
	uint32_t o_ackid                      : 6;  /**< Next outgoing packet AckID.
                                                         Bit 5 is used for IDLE2 and should be zero. */
#else
	uint32_t o_ackid                      : 6;
	uint32_t reserved_6_7                 : 2;
	uint32_t e_ackid                      : 6;
	uint32_t reserved_14_23               : 10;
	uint32_t i_ackid                      : 6;
	uint32_t reserved_30_31               : 2;
#endif
	} s;
	struct cvmx_sriomaintx_port_0_local_ackid_s cn63xx;
	struct cvmx_sriomaintx_port_0_local_ackid_s cn66xx;
	struct cvmx_sriomaintx_port_0_local_ackid_s cnf75xx;
};
typedef union cvmx_sriomaintx_port_0_local_ackid cvmx_sriomaintx_port_0_local_ackid_t;

/**
 * cvmx_sriomaint#_port_gen_ctl
 *
 * Port General Control
 *
 */
union cvmx_sriomaintx_port_gen_ctl {
	uint32_t u32;
	struct cvmx_sriomaintx_port_gen_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t host                         : 1;  /**< Host device.
                                                         The HOST value is based on corresponding
                                                         SRIO()_STATUS_REG[HOST] bit but may be overwritten by software. */
	uint32_t menable                      : 1;  /**< Master enable.  Must be set for device to issue
                                                         read, write, doorbell, message requests. */
	uint32_t discover                     : 1;  /**< Discovered. The device has been discovered by the
                                                         host responsible for initialization. */
	uint32_t reserved_0_28                : 29;
#else
	uint32_t reserved_0_28                : 29;
	uint32_t discover                     : 1;
	uint32_t menable                      : 1;
	uint32_t host                         : 1;
#endif
	} s;
	struct cvmx_sriomaintx_port_gen_ctl_s cn63xx;
	struct cvmx_sriomaintx_port_gen_ctl_s cn63xxp1;
	struct cvmx_sriomaintx_port_gen_ctl_s cn66xx;
	struct cvmx_sriomaintx_port_gen_ctl_s cnf75xx;
};
typedef union cvmx_sriomaintx_port_gen_ctl cvmx_sriomaintx_port_gen_ctl_t;

/**
 * cvmx_sriomaint#_port_lt_ctl
 *
 * This register controls the timeout for link layer transactions.  It is used as the timeout
 * between sending a packet (of any type) or link request to receiving the corresponding link
 * acknowledge or link-response.  Each count represents 200 ns.  The minimum timeout period is
 * the TIMEOUT x200 ns and the maximum is twice that number.  A value less than 32 may not
 * guarantee that all timeout errors will be reported correctly.  When the timeout period expires
 * the packet or link request is dropped and the error is logged in the LNK_TOUT field of the
 * SRIOMAINT()_ERB_ERR_DET register. A value of 0 in this register will allow the packet or
 * link request to be issued but it will timeout immediately.  This value is not recommended for
 * normal operation.
 */
union cvmx_sriomaintx_port_lt_ctl {
	uint32_t u32;
	struct cvmx_sriomaintx_port_lt_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t timeout                      : 24; /**< Timeout value. */
	uint32_t reserved_0_7                 : 8;
#else
	uint32_t reserved_0_7                 : 8;
	uint32_t timeout                      : 24;
#endif
	} s;
	struct cvmx_sriomaintx_port_lt_ctl_s  cn63xx;
	struct cvmx_sriomaintx_port_lt_ctl_s  cn63xxp1;
	struct cvmx_sriomaintx_port_lt_ctl_s  cn66xx;
	struct cvmx_sriomaintx_port_lt_ctl_s  cnf75xx;
};
typedef union cvmx_sriomaintx_port_lt_ctl cvmx_sriomaintx_port_lt_ctl_t;

/**
 * cvmx_sriomaint#_port_mbh0
 *
 * SRIOMAINT_PORT_MBH0 = SRIO Port Maintenance Block Header 0
 *
 * Port Maintenance Block Header 0
 *
 * Notes:
 * Clk_Rst:        SRIOMAINT(0,2..3)_PORT_MBH0     hclk    hrst_n
 *
 */
union cvmx_sriomaintx_port_mbh0 {
	uint32_t u32;
	struct cvmx_sriomaintx_port_mbh0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ef_ptr                       : 16; /**< Pointer to error management block. */
	uint32_t ef_id                        : 16; /**< Extended feature ID (generic endpoint device). */
#else
	uint32_t ef_id                        : 16;
	uint32_t ef_ptr                       : 16;
#endif
	} s;
	struct cvmx_sriomaintx_port_mbh0_s    cn63xx;
	struct cvmx_sriomaintx_port_mbh0_s    cn63xxp1;
	struct cvmx_sriomaintx_port_mbh0_s    cn66xx;
	struct cvmx_sriomaintx_port_mbh0_s    cnf75xx;
};
typedef union cvmx_sriomaintx_port_mbh0 cvmx_sriomaintx_port_mbh0_t;

/**
 * cvmx_sriomaint#_port_rt_ctl
 *
 * This register controls the timeout for logical layer transactions. It is used under
 * two conditions. First, it is used as the timeout period between sending a packet
 * requiring a packet response being sent to receiving the corresponding response. This
 * is used for all outgoing packet types including memory, maintenance, doorbells and
 * message operations. When the timeout period expires the packet is disgarded and the
 * error is logged in the PKT_TOUT field of the SRIOMAINT()_ERB_LT_ERR_DET
 * register. The second use of this register is as a timeout period between incoming
 * message segments of the same message. If a message segment is received then the
 * MSG_TOUT field of the SRIOMAINT()_ERB_LT_ERR_DET register is set if the next segment
 * has not been received before the time expires. In both cases, each count represents
 * 200 ns. The minimum timeout period is the TIMEOUT x 200 ns and the maximum is twice
 * that number. A value less than 32 may not guarantee that all timeout errors will be
 * reported correctly. A value of 0 disables the logical layer timeouts and is not
 * recommended for normal operation.
 */
union cvmx_sriomaintx_port_rt_ctl {
	uint32_t u32;
	struct cvmx_sriomaintx_port_rt_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t timeout                      : 24; /**< Timeout value. */
	uint32_t reserved_0_7                 : 8;
#else
	uint32_t reserved_0_7                 : 8;
	uint32_t timeout                      : 24;
#endif
	} s;
	struct cvmx_sriomaintx_port_rt_ctl_s  cn63xx;
	struct cvmx_sriomaintx_port_rt_ctl_s  cn63xxp1;
	struct cvmx_sriomaintx_port_rt_ctl_s  cn66xx;
	struct cvmx_sriomaintx_port_rt_ctl_s  cnf75xx;
};
typedef union cvmx_sriomaintx_port_rt_ctl cvmx_sriomaintx_port_rt_ctl_t;

/**
 * cvmx_sriomaint#_port_ttl_ctl
 *
 * This register controls the timeout for outgoing packets. It is primilarly
 * used to make sure packets are being transmitted and acknowledged within a
 * reasonable period of time. The timeout value corresponds to TIMEOUT x 200 ns
 * and a value of 0 disables the timer. The actual value should be greater
 * than the physical layer timeout specified in SRIOMAINT()_PORT_LT_CTL and
 * is typically a less than the response timeout specified in
 * SRIOMAINT()_PORT_RT_CTL.
 * A second application of this timer is to remove all the packets waiting
 * to be transmitted including those already in flight. This may by necessary
 * for the case of a link going down (see SRIO()_INT_REG[LINK_DWN]).  Packet
 * removal can accomplished by setting the TIMEOUT value to small number so
 * that all TX packets can be dropped.
 *
 * In both cases, when the timeout expires the TTL interrupt is asserted, any
 * packets currently being transmitted are dropped, the SRIOMAINT()_TX_DROP[DROP]
 * bit is set (causing any scheduled packets to be dropped), the
 * SRIOMAINT()_TX_DROP[DROP_CNT] is incremented for each packet and the SRIO
 * output state is set to IDLE (all errors are cleared). Software must clear
 * the SRIOMAINT()_TX_DROP[DROP] bit to resume transmitting packets.
 */
union cvmx_sriomaintx_port_ttl_ctl {
	uint32_t u32;
	struct cvmx_sriomaintx_port_ttl_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t timeout                      : 24; /**< Timeout value. */
	uint32_t reserved_0_7                 : 8;
#else
	uint32_t reserved_0_7                 : 8;
	uint32_t timeout                      : 24;
#endif
	} s;
	struct cvmx_sriomaintx_port_ttl_ctl_s cn63xx;
	struct cvmx_sriomaintx_port_ttl_ctl_s cn66xx;
	struct cvmx_sriomaintx_port_ttl_ctl_s cnf75xx;
};
typedef union cvmx_sriomaintx_port_ttl_ctl cvmx_sriomaintx_port_ttl_ctl_t;

/**
 * cvmx_sriomaint#_pri_dev_id
 *
 * Primary 8 and 16 bit Device IDs.
 * This register defines the primary 8 and 16 bit device IDs used for large and small transport.
 * An optional secondary set of device IDs are located in SRIOMAINT()_SEC_DEV_ID.
 */
union cvmx_sriomaintx_pri_dev_id {
	uint32_t u32;
	struct cvmx_sriomaintx_pri_dev_id_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t id8                          : 8;  /**< Primary 8-bit device ID. */
	uint32_t id16                         : 16; /**< Primary 16-bit device ID. */
#else
	uint32_t id16                         : 16;
	uint32_t id8                          : 8;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_sriomaintx_pri_dev_id_s   cn63xx;
	struct cvmx_sriomaintx_pri_dev_id_s   cn63xxp1;
	struct cvmx_sriomaintx_pri_dev_id_s   cn66xx;
	struct cvmx_sriomaintx_pri_dev_id_s   cnf75xx;
};
typedef union cvmx_sriomaintx_pri_dev_id cvmx_sriomaintx_pri_dev_id_t;

/**
 * cvmx_sriomaint#_sec_dev_ctrl
 *
 * This register enables the secondary 8 and 16 bit device IDs used for large and small
 * transport.
 * The corresponding secondary ID must be written before the ID is enabled.  The secondary IDs
 * should not be enabled if the values of the primary and secondary IDs are identical.
 */
union cvmx_sriomaintx_sec_dev_ctrl {
	uint32_t u32;
	struct cvmx_sriomaintx_sec_dev_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_2_31                : 30;
	uint32_t enable8                      : 1;  /**< Enable matches to secondary 8-bit device ID. */
	uint32_t enable16                     : 1;  /**< Enable matches to secondary 16-bit device ID. */
#else
	uint32_t enable16                     : 1;
	uint32_t enable8                      : 1;
	uint32_t reserved_2_31                : 30;
#endif
	} s;
	struct cvmx_sriomaintx_sec_dev_ctrl_s cn63xx;
	struct cvmx_sriomaintx_sec_dev_ctrl_s cn63xxp1;
	struct cvmx_sriomaintx_sec_dev_ctrl_s cn66xx;
	struct cvmx_sriomaintx_sec_dev_ctrl_s cnf75xx;
};
typedef union cvmx_sriomaintx_sec_dev_ctrl cvmx_sriomaintx_sec_dev_ctrl_t;

/**
 * cvmx_sriomaint#_sec_dev_id
 *
 * Secondary 8 and 16 bit device IDs.
 * This register defines the secondary 8 and 16 bit device IDs used for large and small
 * transport.
 * The corresponding secondary ID must be written before the ID is enabled in the
 * SRIOMAINT()_SEC_DEV_CTRL register.  The primary set of device IDs are located in
 * SRIOMAINT()_PRI_DEV_ID register.  The secondary IDs should not be written to the same
 * values as the corresponding primary IDs.
 */
union cvmx_sriomaintx_sec_dev_id {
	uint32_t u32;
	struct cvmx_sriomaintx_sec_dev_id_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_24_31               : 8;
	uint32_t id8                          : 8;  /**< Secondary 8-bit device ID. */
	uint32_t id16                         : 16; /**< Secondary 16-bit device ID. */
#else
	uint32_t id16                         : 16;
	uint32_t id8                          : 8;
	uint32_t reserved_24_31               : 8;
#endif
	} s;
	struct cvmx_sriomaintx_sec_dev_id_s   cn63xx;
	struct cvmx_sriomaintx_sec_dev_id_s   cn63xxp1;
	struct cvmx_sriomaintx_sec_dev_id_s   cn66xx;
	struct cvmx_sriomaintx_sec_dev_id_s   cnf75xx;
};
typedef union cvmx_sriomaintx_sec_dev_id cvmx_sriomaintx_sec_dev_id_t;

/**
 * cvmx_sriomaint#_serial_lane_hdr
 *
 * The error management extensions block header register contains the EF_PTR to the next EF_BLK
 * and the EF_ID that identifies this as the serial lane status block.
 */
union cvmx_sriomaintx_serial_lane_hdr {
	uint32_t u32;
	struct cvmx_sriomaintx_serial_lane_hdr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t ef_ptr                       : 16; /**< Pointer to the next block in the extended features
                                                         data structure. */
	uint32_t ef_id                        : 16; /**< ID. */
#else
	uint32_t ef_id                        : 16;
	uint32_t ef_ptr                       : 16;
#endif
	} s;
	struct cvmx_sriomaintx_serial_lane_hdr_s cn63xx;
	struct cvmx_sriomaintx_serial_lane_hdr_s cn63xxp1;
	struct cvmx_sriomaintx_serial_lane_hdr_s cn66xx;
	struct cvmx_sriomaintx_serial_lane_hdr_s cnf75xx;
};
typedef union cvmx_sriomaintx_serial_lane_hdr cvmx_sriomaintx_serial_lane_hdr_t;

/**
 * cvmx_sriomaint#_src_ops
 *
 * This register shows the operations specified in the SRIO()_IP_FEATURE registers.
 *
 */
union cvmx_sriomaintx_src_ops {
	uint32_t u32;
	struct cvmx_sriomaintx_src_ops_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t gsm_read                     : 1;  /**< PE does not support read home operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<31>. */
	uint32_t i_read                       : 1;  /**< PE does not support instruction read.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<30>. */
	uint32_t rd_own                       : 1;  /**< PE does not support read for ownership.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<29>. */
	uint32_t d_invald                     : 1;  /**< PE does not support data cache invalidate.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<28>. */
	uint32_t castout                      : 1;  /**< PE does not support castout operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<27>. */
	uint32_t d_flush                      : 1;  /**< PE does not support data cache flush.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<26>. */
	uint32_t io_read                      : 1;  /**< PE does not support IO read.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<25>. */
	uint32_t i_invald                     : 1;  /**< PE does not support instruction cache invalidate.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<24>. */
	uint32_t tlb_inv                      : 1;  /**< PE does not support TLB entry invalidate.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<23>. */
	uint32_t tlb_invs                     : 1;  /**< PE does not support TLB entry invalidate sync.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<22>. */
	uint32_t reserved_16_21               : 6;
	uint32_t read                         : 1;  /**< PE can support Nread operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<15>. */
	uint32_t write                        : 1;  /**< PE can support Nwrite operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<14>. */
	uint32_t swrite                       : 1;  /**< PE can support Swrite operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<13>. */
	uint32_t write_r                      : 1;  /**< PE can support write with response operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<12>. */
	uint32_t msg                          : 1;  /**< PE can support data message operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<11>. */
	uint32_t doorbell                     : 1;  /**< PE can support doorbell operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<10>. */
	uint32_t compswap                     : 1;  /**< PE does not support atomic compare and swap.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<9>. */
	uint32_t testswap                     : 1;  /**< PE does not support atomic test and swap.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<8>. */
	uint32_t atom_inc                     : 1;  /**< PE can support atomic increment operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<7>. */
	uint32_t atom_dec                     : 1;  /**< PE can support atomic decrement operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<6>. */
	uint32_t atom_set                     : 1;  /**< PE can support atomic set operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<5>. */
	uint32_t atom_clr                     : 1;  /**< PE can support atomic clear operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<4>. */
	uint32_t atom_swp                     : 1;  /**< PE does not support atomic swap.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<3>. */
	uint32_t port_wr                      : 1;  /**< PE can port write operations.
                                                         This is a RO copy of SRIO()_IP_FEATURE[OPS]<2>. */
	uint32_t reserved_0_1                 : 2;
#else
	uint32_t reserved_0_1                 : 2;
	uint32_t port_wr                      : 1;
	uint32_t atom_swp                     : 1;
	uint32_t atom_clr                     : 1;
	uint32_t atom_set                     : 1;
	uint32_t atom_dec                     : 1;
	uint32_t atom_inc                     : 1;
	uint32_t testswap                     : 1;
	uint32_t compswap                     : 1;
	uint32_t doorbell                     : 1;
	uint32_t msg                          : 1;
	uint32_t write_r                      : 1;
	uint32_t swrite                       : 1;
	uint32_t write                        : 1;
	uint32_t read                         : 1;
	uint32_t reserved_16_21               : 6;
	uint32_t tlb_invs                     : 1;
	uint32_t tlb_inv                      : 1;
	uint32_t i_invald                     : 1;
	uint32_t io_read                      : 1;
	uint32_t d_flush                      : 1;
	uint32_t castout                      : 1;
	uint32_t d_invald                     : 1;
	uint32_t rd_own                       : 1;
	uint32_t i_read                       : 1;
	uint32_t gsm_read                     : 1;
#endif
	} s;
	struct cvmx_sriomaintx_src_ops_s      cn63xx;
	struct cvmx_sriomaintx_src_ops_s      cn63xxp1;
	struct cvmx_sriomaintx_src_ops_s      cn66xx;
	struct cvmx_sriomaintx_src_ops_s      cnf75xx;
};
typedef union cvmx_sriomaintx_src_ops cvmx_sriomaintx_src_ops_t;

/**
 * cvmx_sriomaint#_tx_drop
 *
 * This register controls and provides status for dropping outgoing SRIO packets.  The DROP bit
 * should only be cleared when no packets are currently being dropped.  This can be guaranteed by
 * clearing SRIOMAINT()_PORT_0_CTL[O_ENABLE] before changing the DROP bit and
 * restoring the O_ENABLE afterwards.
 */
union cvmx_sriomaintx_tx_drop {
	uint32_t u32;
	struct cvmx_sriomaintx_tx_drop_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint32_t reserved_17_31               : 15;
	uint32_t drop                         : 1;  /**< All outgoing packets are dropped.  Any packets
                                                         requiring a response will return 1's after the
                                                         SRIOMAINT()_PORT_RT_CTL timeout expires.  This bit
                                                         is set automatically when the TTL Timeout occurs
                                                         or can be set by software and must always be
                                                         cleared by software. */
	uint32_t drop_cnt                     : 16; /**< Number of packets dropped by transmit logic.
                                                         Packets are dropped whenever a packet is ready to
                                                         be transmitted and a TTL Timeouts occur, the  DROP
                                                         bit is set or the SRIOMAINT()_ERB_ERR_RATE_THR
                                                         FAIL_TH has been reached and the DROP_PKT bit is
                                                         set in SRIOMAINT()_PORT_0_CTL.  This counter wraps
                                                         on overflow and is cleared only on reset. */
#else
	uint32_t drop_cnt                     : 16;
	uint32_t drop                         : 1;
	uint32_t reserved_17_31               : 15;
#endif
	} s;
	struct cvmx_sriomaintx_tx_drop_s      cn63xx;
	struct cvmx_sriomaintx_tx_drop_s      cn66xx;
	struct cvmx_sriomaintx_tx_drop_s      cnf75xx;
};
typedef union cvmx_sriomaintx_tx_drop cvmx_sriomaintx_tx_drop_t;

#endif
