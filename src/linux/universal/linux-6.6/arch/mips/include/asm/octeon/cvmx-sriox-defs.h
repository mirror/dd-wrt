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
 * cvmx-sriox-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon sriox.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_SRIOX_DEFS_H__
#define __CVMX_SRIOX_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_ACC_CTRL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_ACC_CTRL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000148ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_ACC_CTRL(offset) (CVMX_ADD_IO_SEG(0x00011800C8000148ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_ASMBLY_ID(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_ASMBLY_ID(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000200ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_ASMBLY_ID(offset) (CVMX_ADD_IO_SEG(0x00011800C8000200ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_ASMBLY_INFO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_ASMBLY_INFO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000208ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_ASMBLY_INFO(offset) (CVMX_ADD_IO_SEG(0x00011800C8000208ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_BELL_LOOKUPX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 15)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_SRIOX_BELL_LOOKUPX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C8000500ull) + (((offset) & 15) + ((block_id) & 1) * 0x200000ull) * 8;
}
#else
#define CVMX_SRIOX_BELL_LOOKUPX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800C8000500ull) + (((offset) & 15) + ((block_id) & 1) * 0x200000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_BELL_RESP_CTRL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_BELL_RESP_CTRL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000310ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_BELL_RESP_CTRL(offset) (CVMX_ADD_IO_SEG(0x00011800C8000310ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_BELL_SELECT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_BELL_SELECT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000320ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_BELL_SELECT(offset) (CVMX_ADD_IO_SEG(0x00011800C8000320ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_BIST_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_BIST_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000108ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_BIST_STATUS(offset) (CVMX_ADD_IO_SEG(0x00011800C8000108ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_ECC_CTRL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_ECC_CTRL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000238ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_ECC_CTRL(offset) (CVMX_ADD_IO_SEG(0x00011800C8000238ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_ECC_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_ECC_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000230ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_ECC_STATUS(offset) (CVMX_ADD_IO_SEG(0x00011800C8000230ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_ECO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_ECO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C80005F8ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_ECO(offset) (CVMX_ADD_IO_SEG(0x00011800C80005F8ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_IMSG_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800C8000608ull) + ((offset) & 1) * 0x1000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return CVMX_ADD_IO_SEG(0x00011800C8000508ull) + ((offset) & 3) * 0x1000000ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800C8000508ull) + ((offset) & 1) * 0x1000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOX_IMSG_CTRL (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000608ull) + ((offset) & 1) * 0x1000000ull;
}
#else
static inline uint64_t CVMX_SRIOX_IMSG_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C8000608ull) + (offset) * 0x1000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C8000508ull) + (offset) * 0x1000000ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C8000508ull) + (offset) * 0x1000000ull;
	}
	return CVMX_ADD_IO_SEG(0x00011800C8000608ull) + (offset) * 0x1000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_IMSG_INST_HDRX(unsigned long offset, unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if (((offset <= 255)) && ((block_id <= 1)))
				return CVMX_ADD_IO_SEG(0x00011800C8000800ull) + (((offset) & 255) + ((block_id) & 1) * 0x200000ull) * 8;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if (((offset <= 1)) && ((block_id == 0) || (block_id == 2) || (block_id == 3)))
				return CVMX_ADD_IO_SEG(0x00011800C8000510ull) + (((offset) & 1) + ((block_id) & 3) * 0x200000ull) * 8;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if (((offset <= 1)) && ((block_id <= 1)))
				return CVMX_ADD_IO_SEG(0x00011800C8000510ull) + (((offset) & 1) + ((block_id) & 1) * 0x200000ull) * 8;
			break;
	}
	cvmx_warn("CVMX_SRIOX_IMSG_INST_HDRX (%lu, %lu) not supported on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C8000800ull) + (((offset) & 255) + ((block_id) & 1) * 0x200000ull) * 8;
}
#else
static inline uint64_t CVMX_SRIOX_IMSG_INST_HDRX(unsigned long offset, unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C8000800ull) + ((offset) + (block_id) * 0x200000ull) * 8;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C8000510ull) + ((offset) + (block_id) * 0x200000ull) * 8;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C8000510ull) + ((offset) + (block_id) * 0x200000ull) * 8;
	}
	return CVMX_ADD_IO_SEG(0x00011800C8000800ull) + ((offset) + (block_id) * 0x200000ull) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_IMSG_PKINDX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_SRIOX_IMSG_PKINDX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C8000620ull) + (((offset) & 1) + ((block_id) & 1) * 0x200000ull) * 8;
}
#else
#define CVMX_SRIOX_IMSG_PKINDX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800C8000620ull) + (((offset) & 1) + ((block_id) & 1) * 0x200000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_IMSG_PRT1_HDR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_IMSG_PRT1_HDR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000638ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_IMSG_PRT1_HDR(offset) (CVMX_ADD_IO_SEG(0x00011800C8000638ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_IMSG_QOS_GRPX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset <= 31)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset <= 31)) && ((block_id == 0) || (block_id == 2) || (block_id == 3))))))
		cvmx_warn("CVMX_SRIOX_IMSG_QOS_GRPX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C8000600ull) + (((offset) & 31) + ((block_id) & 3) * 0x200000ull) * 8;
}
#else
#define CVMX_SRIOX_IMSG_QOS_GRPX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800C8000600ull) + (((offset) & 31) + ((block_id) & 3) * 0x200000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_IMSG_STATUSX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset <= 23)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset <= 23)) && ((block_id == 0) || (block_id == 2) || (block_id == 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 23)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_SRIOX_IMSG_STATUSX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C8000700ull) + (((offset) & 31) + ((block_id) & 3) * 0x200000ull) * 8;
}
#else
#define CVMX_SRIOX_IMSG_STATUSX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800C8000700ull) + (((offset) & 31) + ((block_id) & 3) * 0x200000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_IMSG_VPORT_THR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800C8000600ull) + ((offset) & 1) * 0x1000000ull;
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			if ((offset == 0) || ((offset >= 2) && (offset <= 3)))
				return CVMX_ADD_IO_SEG(0x00011800C8000500ull) + ((offset) & 3) * 0x1000000ull;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x00011800C8000500ull) + ((offset) & 1) * 0x1000000ull;
			break;
	}
	cvmx_warn("CVMX_SRIOX_IMSG_VPORT_THR (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000600ull) + ((offset) & 1) * 0x1000000ull;
}
#else
static inline uint64_t CVMX_SRIOX_IMSG_VPORT_THR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C8000600ull) + (offset) * 0x1000000ull;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C8000500ull) + (offset) * 0x1000000ull;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C8000500ull) + (offset) * 0x1000000ull;
	}
	return CVMX_ADD_IO_SEG(0x00011800C8000600ull) + (offset) * 0x1000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_IMSG_VPORT_THR2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3))))))
		cvmx_warn("CVMX_SRIOX_IMSG_VPORT_THR2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000528ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_IMSG_VPORT_THR2(offset) (CVMX_ADD_IO_SEG(0x00011800C8000528ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_INT2_ENABLE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3))))))
		cvmx_warn("CVMX_SRIOX_INT2_ENABLE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C80003E0ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_INT2_ENABLE(offset) (CVMX_ADD_IO_SEG(0x00011800C80003E0ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_INT2_REG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3))))))
		cvmx_warn("CVMX_SRIOX_INT2_REG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C80003E8ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_INT2_REG(offset) (CVMX_ADD_IO_SEG(0x00011800C80003E8ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_INT_ENABLE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3))))))
		cvmx_warn("CVMX_SRIOX_INT_ENABLE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000110ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_INT_ENABLE(offset) (CVMX_ADD_IO_SEG(0x00011800C8000110ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_INT_INFO0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_INT_INFO0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000120ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_INT_INFO0(offset) (CVMX_ADD_IO_SEG(0x00011800C8000120ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_INT_INFO1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_INT_INFO1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000128ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_INT_INFO1(offset) (CVMX_ADD_IO_SEG(0x00011800C8000128ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_INT_INFO2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_INT_INFO2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000130ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_INT_INFO2(offset) (CVMX_ADD_IO_SEG(0x00011800C8000130ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_INT_INFO3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_INT_INFO3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000138ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_INT_INFO3(offset) (CVMX_ADD_IO_SEG(0x00011800C8000138ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_INT_REG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_INT_REG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000118ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_INT_REG(offset) (CVMX_ADD_IO_SEG(0x00011800C8000118ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_INT_W1S(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_INT_W1S(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000110ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_INT_W1S(offset) (CVMX_ADD_IO_SEG(0x00011800C8000110ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_IP_FEATURE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_IP_FEATURE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C80003F8ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_IP_FEATURE(offset) (CVMX_ADD_IO_SEG(0x00011800C80003F8ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_MAC_BUFFERS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_MAC_BUFFERS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000390ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_MAC_BUFFERS(offset) (CVMX_ADD_IO_SEG(0x00011800C8000390ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_MAINT_OP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_MAINT_OP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000158ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_MAINT_OP(offset) (CVMX_ADD_IO_SEG(0x00011800C8000158ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_MAINT_RD_DATA(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_MAINT_RD_DATA(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000160ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_MAINT_RD_DATA(offset) (CVMX_ADD_IO_SEG(0x00011800C8000160ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_MCE_TX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_MCE_TX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000240ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_MCE_TX_CTL(offset) (CVMX_ADD_IO_SEG(0x00011800C8000240ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_MEM_OP_CTRL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_MEM_OP_CTRL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000168ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_MEM_OP_CTRL(offset) (CVMX_ADD_IO_SEG(0x00011800C8000168ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_OMSG_CTRLX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset <= 1)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset <= 1)) && ((block_id == 0) || (block_id == 2) || (block_id == 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_SRIOX_OMSG_CTRLX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C8000488ull) + (((offset) & 1) + ((block_id) & 3) * 0x40000ull) * 64;
}
#else
#define CVMX_SRIOX_OMSG_CTRLX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800C8000488ull) + (((offset) & 1) + ((block_id) & 3) * 0x40000ull) * 64)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_OMSG_DONE_COUNTSX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset <= 1)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset <= 1)) && ((block_id == 0) || (block_id == 2) || (block_id == 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_SRIOX_OMSG_DONE_COUNTSX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C80004B0ull) + (((offset) & 1) + ((block_id) & 3) * 0x40000ull) * 64;
}
#else
#define CVMX_SRIOX_OMSG_DONE_COUNTSX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800C80004B0ull) + (((offset) & 1) + ((block_id) & 3) * 0x40000ull) * 64)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_OMSG_FMP_MRX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset <= 1)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset <= 1)) && ((block_id == 0) || (block_id == 2) || (block_id == 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_SRIOX_OMSG_FMP_MRX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C8000498ull) + (((offset) & 1) + ((block_id) & 3) * 0x40000ull) * 64;
}
#else
#define CVMX_SRIOX_OMSG_FMP_MRX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800C8000498ull) + (((offset) & 1) + ((block_id) & 3) * 0x40000ull) * 64)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_OMSG_NMP_MRX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset <= 1)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset <= 1)) && ((block_id == 0) || (block_id == 2) || (block_id == 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_SRIOX_OMSG_NMP_MRX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C80004A0ull) + (((offset) & 1) + ((block_id) & 3) * 0x40000ull) * 64;
}
#else
#define CVMX_SRIOX_OMSG_NMP_MRX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800C80004A0ull) + (((offset) & 1) + ((block_id) & 3) * 0x40000ull) * 64)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_OMSG_PORTX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset <= 1)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset <= 1)) && ((block_id == 0) || (block_id == 2) || (block_id == 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_SRIOX_OMSG_PORTX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C8000480ull) + (((offset) & 1) + ((block_id) & 3) * 0x40000ull) * 64;
}
#else
#define CVMX_SRIOX_OMSG_PORTX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800C8000480ull) + (((offset) & 1) + ((block_id) & 3) * 0x40000ull) * 64)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_OMSG_SILO_THR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_OMSG_SILO_THR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C80004F8ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_OMSG_SILO_THR(offset) (CVMX_ADD_IO_SEG(0x00011800C80004F8ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_OMSG_SP_MRX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset <= 1)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset <= 1)) && ((block_id == 0) || (block_id == 2) || (block_id == 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 1)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_SRIOX_OMSG_SP_MRX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C8000490ull) + (((offset) & 1) + ((block_id) & 3) * 0x40000ull) * 64;
}
#else
#define CVMX_SRIOX_OMSG_SP_MRX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800C8000490ull) + (((offset) & 1) + ((block_id) & 3) * 0x40000ull) * 64)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_PRIOX_IN_USE(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset <= 3)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset <= 3)) && ((block_id == 0) || (block_id == 2) || (block_id == 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 3)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_SRIOX_PRIOX_IN_USE(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C80003C0ull) + (((offset) & 3) + ((block_id) & 3) * 0x200000ull) * 8;
}
#else
#define CVMX_SRIOX_PRIOX_IN_USE(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800C80003C0ull) + (((offset) & 3) + ((block_id) & 3) * 0x200000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_RX_BELL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_RX_BELL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000308ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_RX_BELL(offset) (CVMX_ADD_IO_SEG(0x00011800C8000308ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_RX_BELL_CTRL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_RX_BELL_CTRL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000318ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_RX_BELL_CTRL(offset) (CVMX_ADD_IO_SEG(0x00011800C8000318ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_RX_BELL_SEQ(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_RX_BELL_SEQ(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000300ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_RX_BELL_SEQ(offset) (CVMX_ADD_IO_SEG(0x00011800C8000300ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_RX_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_RX_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000380ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_RX_STATUS(offset) (CVMX_ADD_IO_SEG(0x00011800C8000380ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_S2M_TYPEX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset <= 15)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset <= 15)) && ((block_id == 0) || (block_id == 2) || (block_id == 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 15)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_SRIOX_S2M_TYPEX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C8000180ull) + (((offset) & 15) + ((block_id) & 3) * 0x200000ull) * 8;
}
#else
#define CVMX_SRIOX_S2M_TYPEX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800C8000180ull) + (((offset) & 15) + ((block_id) & 3) * 0x200000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_SEQ(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_SEQ(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000278ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_SEQ(offset) (CVMX_ADD_IO_SEG(0x00011800C8000278ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_STATUS_REG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_STATUS_REG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000100ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_STATUS_REG(offset) (CVMX_ADD_IO_SEG(0x00011800C8000100ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_TAG_CTRL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_TAG_CTRL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000178ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_TAG_CTRL(offset) (CVMX_ADD_IO_SEG(0x00011800C8000178ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_TLP_CREDITS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_TLP_CREDITS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000150ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_TLP_CREDITS(offset) (CVMX_ADD_IO_SEG(0x00011800C8000150ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_TX_BELL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_TX_BELL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000280ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_TX_BELL(offset) (CVMX_ADD_IO_SEG(0x00011800C8000280ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_TX_BELL_INFO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_TX_BELL_INFO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000288ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_TX_BELL_INFO(offset) (CVMX_ADD_IO_SEG(0x00011800C8000288ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_TX_CTRL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_TX_CTRL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000170ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_TX_CTRL(offset) (CVMX_ADD_IO_SEG(0x00011800C8000170ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_TX_EMPHASIS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3))))))
		cvmx_warn("CVMX_SRIOX_TX_EMPHASIS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C80003F0ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_TX_EMPHASIS(offset) (CVMX_ADD_IO_SEG(0x00011800C80003F0ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_TX_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_TX_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000388ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_TX_STATUS(offset) (CVMX_ADD_IO_SEG(0x00011800C8000388ull) + ((offset) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SRIOX_WR_DONE_COUNTS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0) || ((offset >= 2) && (offset <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SRIOX_WR_DONE_COUNTS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800C8000340ull) + ((offset) & 3) * 0x1000000ull;
}
#else
#define CVMX_SRIOX_WR_DONE_COUNTS(offset) (CVMX_ADD_IO_SEG(0x00011800C8000340ull) + ((offset) & 3) * 0x1000000ull)
#endif

/**
 * cvmx_srio#_acc_ctrl
 *
 * This register controls write access to the BAR registers via SRIO maintenance operations.
 * At powerup the BAR registers can be accessed via RSL and maintenance operations.  If the
 * DENY_BAR* bits or DENY_ADR* bits are set then maintenance writes to the corresponding BAR
 * fields are ignored.  This register does not effect read operations.  Reset values for
 * DENY_BAR[2:0] are typically clear but they are set if the chip is operating in Authentik
 * mode.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_acc_ctrl {
	uint64_t u64;
	struct cvmx_sriox_acc_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t deny_adr2                    : 1;  /**< Deny SRIO write access to SRIO address fields in SRIOMAINT()_BAR2* registers. */
	uint64_t deny_adr1                    : 1;  /**< Deny SRIO write access to SRIO address fields in SRIOMAINT()_BAR1* registers. */
	uint64_t deny_adr0                    : 1;  /**< Deny SRIO write access to SRIO address fields in SRIOMAINT()_BAR0* registers. */
	uint64_t reserved_3_3                 : 1;
	uint64_t deny_bar2                    : 1;  /**< Deny SRIO write access to non-SRIO address fields in the SRIOMAINT_BAR2 registers.
                                                         This field is set during mac reset while in authentik mode and typically
                                                         cleared once link partner has been validated. */
	uint64_t deny_bar1                    : 1;  /**< Deny SRIO write access to non-SRIO address fields in the SRIOMAINT_BAR1 registers.
                                                         This field is set during mac reset while in authentik mode and typically
                                                         cleared once link partner has been validated. */
	uint64_t deny_bar0                    : 1;  /**< Deny SRIO write access to non-SRIO address fields in the SRIOMAINT_BAR0 registers.
                                                         This field is set during mac reset while in authentik mode and typically
                                                         cleared once link partner has been validated. */
#else
	uint64_t deny_bar0                    : 1;
	uint64_t deny_bar1                    : 1;
	uint64_t deny_bar2                    : 1;
	uint64_t reserved_3_3                 : 1;
	uint64_t deny_adr0                    : 1;
	uint64_t deny_adr1                    : 1;
	uint64_t deny_adr2                    : 1;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_sriox_acc_ctrl_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t deny_bar2                    : 1;  /**< Deny SRIO Write Access to BAR2 Registers */
	uint64_t deny_bar1                    : 1;  /**< Deny SRIO Write Access to BAR1 Registers */
	uint64_t deny_bar0                    : 1;  /**< Deny SRIO Write Access to BAR0 Registers */
#else
	uint64_t deny_bar0                    : 1;
	uint64_t deny_bar1                    : 1;
	uint64_t deny_bar2                    : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} cn63xx;
	struct cvmx_sriox_acc_ctrl_cn63xx     cn63xxp1;
	struct cvmx_sriox_acc_ctrl_s          cn66xx;
	struct cvmx_sriox_acc_ctrl_s          cnf75xx;
};
typedef union cvmx_sriox_acc_ctrl cvmx_sriox_acc_ctrl_t;

/**
 * cvmx_srio#_asmbly_id
 *
 * This register specifies the assembly ID and vendor visible in the
 * SRIOMAINT()_ASMBLY_ID register.  The assembly vendor ID is typically
 * supplied by the RapidIO Trade Association.  This register is only
 * reset during COLD boot and may only be modified while SRIO()_STATUS_REG[ACCESS]
 * is zero.
 *
 * This register is reset by the coprocessor-clock cold reset.
 */
union cvmx_sriox_asmbly_id {
	uint64_t u64;
	struct cvmx_sriox_asmbly_id_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t assy_id                      : 16; /**< Assembly identifer. */
	uint64_t assy_ven                     : 16; /**< Assembly vendor identifer. */
#else
	uint64_t assy_ven                     : 16;
	uint64_t assy_id                      : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sriox_asmbly_id_s         cn63xx;
	struct cvmx_sriox_asmbly_id_s         cn63xxp1;
	struct cvmx_sriox_asmbly_id_s         cn66xx;
	struct cvmx_sriox_asmbly_id_s         cnf75xx;
};
typedef union cvmx_sriox_asmbly_id cvmx_sriox_asmbly_id_t;

/**
 * cvmx_srio#_asmbly_info
 *
 * The Assembly Info register controls the assembly revision visible in the
 * ASSY_REV field of the SRIOMAINT()_ASMBLY_INFO register.  This register is
 * only reset during COLD boot and may only be modified while SRIO()_STATUS_REG[ACCESS]
 * is zero.
 *
 * This register is reset by the coprocessor-clock cold reset.
 */
union cvmx_sriox_asmbly_info {
	uint64_t u64;
	struct cvmx_sriox_asmbly_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t assy_rev                     : 16; /**< Assembly revision. */
	uint64_t reserved_0_15                : 16;
#else
	uint64_t reserved_0_15                : 16;
	uint64_t assy_rev                     : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sriox_asmbly_info_s       cn63xx;
	struct cvmx_sriox_asmbly_info_s       cn63xxp1;
	struct cvmx_sriox_asmbly_info_s       cn66xx;
	struct cvmx_sriox_asmbly_info_s       cnf75xx;
};
typedef union cvmx_sriox_asmbly_info cvmx_sriox_asmbly_info_t;

/**
 * cvmx_srio#_bell_lookup#
 *
 * The QOS/GRP table contains 16 entries with 16 QOS/GRP pairs
 * per entry 256 pairs total.  HW selects the FIFO by using the
 * fields determined by the SRIO_BELL_SELECT register to parse
 * the doorbell packet.  The actual FIFO number selected is limited
 * by the SRIO()_RX_BELL_CTRL[NUM_FIFO].
 * For example, if NUM_FIFO has a value of 011 (4 FIFOs) then only
 * the two LSBs of the FIFO number stored in the SRIO_BELL_LOOKUP are
 * used.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_bell_lookupx {
	uint64_t u64;
	struct cvmx_sriox_bell_lookupx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t entry15                      : 4;  /**< FIFO number addressed by IDX=15. */
	uint64_t entry14                      : 4;  /**< FIFO number addressed by IDX=14. */
	uint64_t entry13                      : 4;  /**< FIFO number addressed by IDX=13. */
	uint64_t entry12                      : 4;  /**< FIFO number addressed by IDX=12. */
	uint64_t entry11                      : 4;  /**< FIFO number addressed by IDX=11. */
	uint64_t entry10                      : 4;  /**< FIFO number addressed by IDX=10. */
	uint64_t entry9                       : 4;  /**< FIFO number addressed by IDX=9. */
	uint64_t entry8                       : 4;  /**< FIFO number addressed by IDX=8. */
	uint64_t entry7                       : 4;  /**< FIFO number addressed by IDX=7. */
	uint64_t entry6                       : 4;  /**< FIFO number addressed by IDX=6. */
	uint64_t entry5                       : 4;  /**< FIFO number addressed by IDX=5. */
	uint64_t entry4                       : 4;  /**< FIFO number addressed by IDX=4. */
	uint64_t entry3                       : 4;  /**< FIFO number addressed by IDX=3. */
	uint64_t entry2                       : 4;  /**< FIFO number addressed by IDX=2. */
	uint64_t entry1                       : 4;  /**< FIFO number addressed by IDX=1. */
	uint64_t entry0                       : 4;  /**< FIFO number addressed by IDX=0. */
#else
	uint64_t entry0                       : 4;
	uint64_t entry1                       : 4;
	uint64_t entry2                       : 4;
	uint64_t entry3                       : 4;
	uint64_t entry4                       : 4;
	uint64_t entry5                       : 4;
	uint64_t entry6                       : 4;
	uint64_t entry7                       : 4;
	uint64_t entry8                       : 4;
	uint64_t entry9                       : 4;
	uint64_t entry10                      : 4;
	uint64_t entry11                      : 4;
	uint64_t entry12                      : 4;
	uint64_t entry13                      : 4;
	uint64_t entry14                      : 4;
	uint64_t entry15                      : 4;
#endif
	} s;
	struct cvmx_sriox_bell_lookupx_s      cnf75xx;
};
typedef union cvmx_sriox_bell_lookupx cvmx_sriox_bell_lookupx_t;

/**
 * cvmx_srio#_bell_resp_ctrl
 *
 * This register is used to override the response priority of the outgoing
 * doorbell responses.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_bell_resp_ctrl {
	uint64_t u64;
	struct cvmx_sriox_bell_resp_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t rp1_sid                      : 1;  /**< Sets response priority for incomimg doorbells
                                                         of priority 1 on the secondary ID (0=2, 1=3). */
	uint64_t rp0_sid                      : 2;  /**< Sets response priority for incomimg doorbells
                                                         of priority 0 on the secondary ID (0,1=1 2=2, 3=3). */
	uint64_t rp1_pid                      : 1;  /**< Sets response priority for incomimg doorbells
                                                         of priority 1 on the primary ID (0=2, 1=3). */
	uint64_t rp0_pid                      : 2;  /**< Sets response priority for incomimg doorbells
                                                         of priority 0 on the primary ID (0,1=1 2=2, 3=3). */
#else
	uint64_t rp0_pid                      : 2;
	uint64_t rp1_pid                      : 1;
	uint64_t rp0_sid                      : 2;
	uint64_t rp1_sid                      : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_sriox_bell_resp_ctrl_s    cn63xx;
	struct cvmx_sriox_bell_resp_ctrl_s    cn63xxp1;
	struct cvmx_sriox_bell_resp_ctrl_s    cn66xx;
	struct cvmx_sriox_bell_resp_ctrl_s    cnf75xx;
};
typedef union cvmx_sriox_bell_resp_ctrl cvmx_sriox_bell_resp_ctrl_t;

/**
 * cvmx_srio#_bell_select
 *
 * This register is used select which bits in the doorbell packet
 * are used to provide the 4 bit address and 4 bit index into the
 * lookup table specified in SRIO()_BELL_LOOKUP[0:15].  Each address
 * and index has a 6-bit selector to pick a bit out of the incoming
 * doorbell using the following:
 *
 *   35-34  Doorbell Priority [1:0].
 *   33       Secondary ID (1=Packet Matched Enabled Secondary ID8 or ID16).
 *   32       ID8/ID16 bit (1=Packet used 16-bit Device IDs).
 *   31-16  Source ID [15:0].
 *   15-0   Doorbell Payload [15:0].
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_bell_select {
	uint64_t u64;
	struct cvmx_sriox_bell_select_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t addr3                        : 6;  /**< Selector for address 3. */
	uint64_t reserved_54_55               : 2;
	uint64_t addr2                        : 6;  /**< Selector for address 2. */
	uint64_t reserved_46_47               : 2;
	uint64_t addr1                        : 6;  /**< Selector for address 1. */
	uint64_t reserved_38_39               : 2;
	uint64_t addr0                        : 6;  /**< Selector for address 0. */
	uint64_t reserved_30_31               : 2;
	uint64_t idx3                         : 6;  /**< Selector for index 3. */
	uint64_t reserved_22_23               : 2;
	uint64_t idx2                         : 6;  /**< Selector for index 2. */
	uint64_t reserved_14_15               : 2;
	uint64_t idx1                         : 6;  /**< Selector for index 1. */
	uint64_t reserved_6_7                 : 2;
	uint64_t idx0                         : 6;  /**< Selector for index 0. */
#else
	uint64_t idx0                         : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t idx1                         : 6;
	uint64_t reserved_14_15               : 2;
	uint64_t idx2                         : 6;
	uint64_t reserved_22_23               : 2;
	uint64_t idx3                         : 6;
	uint64_t reserved_30_31               : 2;
	uint64_t addr0                        : 6;
	uint64_t reserved_38_39               : 2;
	uint64_t addr1                        : 6;
	uint64_t reserved_46_47               : 2;
	uint64_t addr2                        : 6;
	uint64_t reserved_54_55               : 2;
	uint64_t addr3                        : 6;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_sriox_bell_select_s       cnf75xx;
};
typedef union cvmx_sriox_bell_select cvmx_sriox_bell_select_t;

/**
 * cvmx_srio#_bist_status
 *
 * Results from BIST runs of SRIO's memories.  This register can be accesed at any time
 * but some memories will only indicate failures when SRIO(0..1)_STATUS_REG[ACCESS] is true.
 */
union cvmx_sriox_bist_status {
	uint64_t u64;
	struct cvmx_sriox_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_45_63               : 19;
	uint64_t lram                         : 1;  /**< Incoming Doorbell Lookup RAM. */
	uint64_t mram                         : 2;  /**< Incoming Message SLI FIFO. */
	uint64_t cram                         : 2;  /**< Incoming Rd/Wr/Response Command FIFO. */
	uint64_t bell                         : 2;  /**< Incoming Doorbell FIFO. */
	uint64_t otag                         : 2;  /**< Outgoing Tag Data. */
	uint64_t itag                         : 1;  /**< Incoming TAG Data. */
	uint64_t ofree                        : 1;  /**< Outgoing Free Pointer RAM (OFIFO) */
	uint64_t reserved_0_33                : 34;
#else
	uint64_t reserved_0_33                : 34;
	uint64_t ofree                        : 1;
	uint64_t itag                         : 1;
	uint64_t otag                         : 2;
	uint64_t bell                         : 2;
	uint64_t cram                         : 2;
	uint64_t mram                         : 2;
	uint64_t lram                         : 1;
	uint64_t reserved_45_63               : 19;
#endif
	} s;
	struct cvmx_sriox_bist_status_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t mram                         : 2;  /**< Incoming Message SLI FIFO. */
	uint64_t cram                         : 2;  /**< Incoming Rd/Wr/Response Command FIFO. */
	uint64_t bell                         : 2;  /**< Incoming Doorbell FIFO. */
	uint64_t otag                         : 2;  /**< Outgoing Tag Data. */
	uint64_t itag                         : 1;  /**< Incoming TAG Data. */
	uint64_t ofree                        : 1;  /**< Outgoing Free Pointer RAM (OFIFO) */
	uint64_t rtn                          : 2;  /**< Outgoing Response Return FIFO. */
	uint64_t obulk                        : 4;  /**< Outgoing Bulk Data RAMs (OFIFO) */
	uint64_t optrs                        : 4;  /**< Outgoing Priority Pointer RAMs (OFIFO) */
	uint64_t oarb2                        : 2;  /**< Additional Outgoing Priority RAMs (Pass 2). */
	uint64_t rxbuf2                       : 2;  /**< Additional Incoming SRIO MAC Buffers (Pass 2). */
	uint64_t oarb                         : 2;  /**< Outgoing Priority RAMs (OARB) */
	uint64_t ispf                         : 1;  /**< Incoming Soft Packet FIFO */
	uint64_t ospf                         : 1;  /**< Outgoing Soft Packet FIFO */
	uint64_t txbuf                        : 2;  /**< Outgoing SRIO MAC Buffer. */
	uint64_t rxbuf                        : 2;  /**< Incoming SRIO MAC Buffer. */
	uint64_t imsg                         : 5;  /**< Incoming Message RAMs.
                                                         IMSG<0> (i.e. <7>) unused in Pass 2 */
	uint64_t omsg                         : 7;  /**< Outgoing Message RAMs. */
#else
	uint64_t omsg                         : 7;
	uint64_t imsg                         : 5;
	uint64_t rxbuf                        : 2;
	uint64_t txbuf                        : 2;
	uint64_t ospf                         : 1;
	uint64_t ispf                         : 1;
	uint64_t oarb                         : 2;
	uint64_t rxbuf2                       : 2;
	uint64_t oarb2                        : 2;
	uint64_t optrs                        : 4;
	uint64_t obulk                        : 4;
	uint64_t rtn                          : 2;
	uint64_t ofree                        : 1;
	uint64_t itag                         : 1;
	uint64_t otag                         : 2;
	uint64_t bell                         : 2;
	uint64_t cram                         : 2;
	uint64_t mram                         : 2;
	uint64_t reserved_44_63               : 20;
#endif
	} cn63xx;
	struct cvmx_sriox_bist_status_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t mram                         : 2;  /**< Incoming Message SLI FIFO. */
	uint64_t cram                         : 2;  /**< Incoming Rd/Wr/Response Command FIFO. */
	uint64_t bell                         : 2;  /**< Incoming Doorbell FIFO. */
	uint64_t otag                         : 2;  /**< Outgoing Tag Data. */
	uint64_t itag                         : 1;  /**< Incoming TAG Data. */
	uint64_t ofree                        : 1;  /**< Outgoing Free Pointer RAM (OFIFO) */
	uint64_t rtn                          : 2;  /**< Outgoing Response Return FIFO. */
	uint64_t obulk                        : 4;  /**< Outgoing Bulk Data RAMs (OFIFO) */
	uint64_t optrs                        : 4;  /**< Outgoing Priority Pointer RAMs (OFIFO) */
	uint64_t reserved_20_23               : 4;
	uint64_t oarb                         : 2;  /**< Outgoing Priority RAMs (OARB) */
	uint64_t ispf                         : 1;  /**< Incoming Soft Packet FIFO */
	uint64_t ospf                         : 1;  /**< Outgoing Soft Packet FIFO */
	uint64_t txbuf                        : 2;  /**< Outgoing SRIO MAC Buffer. */
	uint64_t rxbuf                        : 2;  /**< Incoming SRIO MAC Buffer. */
	uint64_t imsg                         : 5;  /**< Incoming Message RAMs. */
	uint64_t omsg                         : 7;  /**< Outgoing Message RAMs. */
#else
	uint64_t omsg                         : 7;
	uint64_t imsg                         : 5;
	uint64_t rxbuf                        : 2;
	uint64_t txbuf                        : 2;
	uint64_t ospf                         : 1;
	uint64_t ispf                         : 1;
	uint64_t oarb                         : 2;
	uint64_t reserved_20_23               : 4;
	uint64_t optrs                        : 4;
	uint64_t obulk                        : 4;
	uint64_t rtn                          : 2;
	uint64_t ofree                        : 1;
	uint64_t itag                         : 1;
	uint64_t otag                         : 2;
	uint64_t bell                         : 2;
	uint64_t cram                         : 2;
	uint64_t mram                         : 2;
	uint64_t reserved_44_63               : 20;
#endif
	} cn63xxp1;
	struct cvmx_sriox_bist_status_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_45_63               : 19;
	uint64_t lram                         : 1;  /**< Incoming Doorbell Lookup RAM. */
	uint64_t mram                         : 2;  /**< Incoming Message SLI FIFO. */
	uint64_t cram                         : 2;  /**< Incoming Rd/Wr/Response Command FIFO. */
	uint64_t bell                         : 2;  /**< Incoming Doorbell FIFO. */
	uint64_t otag                         : 2;  /**< Outgoing Tag Data. */
	uint64_t itag                         : 1;  /**< Incoming TAG Data. */
	uint64_t ofree                        : 1;  /**< Outgoing Free Pointer RAM (OFIFO) */
	uint64_t rtn                          : 2;  /**< Outgoing Response Return FIFO. */
	uint64_t obulk                        : 4;  /**< Outgoing Bulk Data RAMs (OFIFO) */
	uint64_t optrs                        : 4;  /**< Outgoing Priority Pointer RAMs (OFIFO) */
	uint64_t oarb2                        : 2;  /**< Additional Outgoing Priority RAMs. */
	uint64_t rxbuf2                       : 2;  /**< Additional Incoming SRIO MAC Buffers. */
	uint64_t oarb                         : 2;  /**< Outgoing Priority RAMs (OARB) */
	uint64_t ispf                         : 1;  /**< Incoming Soft Packet FIFO */
	uint64_t ospf                         : 1;  /**< Outgoing Soft Packet FIFO */
	uint64_t txbuf                        : 2;  /**< Outgoing SRIO MAC Buffer. */
	uint64_t rxbuf                        : 2;  /**< Incoming SRIO MAC Buffer. */
	uint64_t imsg                         : 5;  /**< Incoming Message RAMs. */
	uint64_t omsg                         : 7;  /**< Outgoing Message RAMs. */
#else
	uint64_t omsg                         : 7;
	uint64_t imsg                         : 5;
	uint64_t rxbuf                        : 2;
	uint64_t txbuf                        : 2;
	uint64_t ospf                         : 1;
	uint64_t ispf                         : 1;
	uint64_t oarb                         : 2;
	uint64_t rxbuf2                       : 2;
	uint64_t oarb2                        : 2;
	uint64_t optrs                        : 4;
	uint64_t obulk                        : 4;
	uint64_t rtn                          : 2;
	uint64_t ofree                        : 1;
	uint64_t itag                         : 1;
	uint64_t otag                         : 2;
	uint64_t bell                         : 2;
	uint64_t cram                         : 2;
	uint64_t mram                         : 2;
	uint64_t lram                         : 1;
	uint64_t reserved_45_63               : 19;
#endif
	} cn66xx;
	struct cvmx_sriox_bist_status_cnf75xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_34_63               : 30;
	uint64_t status                       : 34; /**< Bist status set bit indicates failure. */
#else
	uint64_t status                       : 34;
	uint64_t reserved_34_63               : 30;
#endif
	} cnf75xx;
};
typedef union cvmx_sriox_bist_status cvmx_sriox_bist_status_t;

/**
 * cvmx_srio#_ecc_ctrl
 *
 * ECC Diagnostic Control Register
 *
 */
union cvmx_sriox_ecc_ctrl {
	uint64_t u64;
	struct cvmx_sriox_ecc_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ecc_dis                      : 1;  /**< Disable ECC checking on all SRIO internal memories.
                                                         Typically set to zero. */
	uint64_t reserved_44_62               : 19;
	uint64_t flip                         : 44; /**< Error insertion bits for ECC syndromes.  Two per RAM.  Typically set to zero. */
#else
	uint64_t flip                         : 44;
	uint64_t reserved_44_62               : 19;
	uint64_t ecc_dis                      : 1;
#endif
	} s;
	struct cvmx_sriox_ecc_ctrl_s          cnf75xx;
};
typedef union cvmx_sriox_ecc_ctrl cvmx_sriox_ecc_ctrl_t;

/**
 * cvmx_srio#_ecc_status
 *
 * List of detected ECC failures.
 *
 */
union cvmx_sriox_ecc_status {
	uint64_t u64;
	struct cvmx_sriox_ecc_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t dbe                          : 22; /**< Double bit error detected.  Set bit indicates failure. */
	uint64_t reserved_22_31               : 10;
	uint64_t sbe                          : 22; /**< Single bit error detected.  Set bit indicates failure. */
#else
	uint64_t sbe                          : 22;
	uint64_t reserved_22_31               : 10;
	uint64_t dbe                          : 22;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_sriox_ecc_status_s        cnf75xx;
};
typedef union cvmx_sriox_ecc_status cvmx_sriox_ecc_status_t;

/**
 * cvmx_srio#_eco
 *
 * Reserved.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_eco {
	uint64_t u64;
	struct cvmx_sriox_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t eco_rw                       : 32; /**< ECO flops. */
#else
	uint64_t eco_rw                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sriox_eco_s               cnf75xx;
};
typedef union cvmx_sriox_eco cvmx_sriox_eco_t;

/**
 * cvmx_srio#_imsg_ctrl
 *
 * This register is reset by the h-clock reset.
 *
 */
union cvmx_sriox_imsg_ctrl {
	uint64_t u64;
	struct cvmx_sriox_imsg_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t to_mode                      : 1;  /**< MP message timeout mode:
                                                         0 = The timeout counter gets reset whenever the
                                                         next sequential segment is received, regardless
                                                         of whether it is accepted.
                                                         1 = The timeout counter gets reset only when the
                                                         next sequential segment is received and
                                                         accepted. */
	uint64_t reserved_30_30               : 1;
	uint64_t rsp_thr                      : 6;  /**< Reserved. */
	uint64_t reserved_22_23               : 2;
	uint64_t rp1_sid                      : 1;  /**< Sets msg response priority for incomimg messages
                                                         of priority 1 on the secondary ID (0=2, 1=3). */
	uint64_t rp0_sid                      : 2;  /**< Sets msg response priority for incomimg messages
                                                         of priority 0 on the secondary ID (0,1=1 2=2, 3=3). */
	uint64_t rp1_pid                      : 1;  /**< Sets msg response priority for incomimg messages
                                                         of priority 1 on the primary ID (0=2, 1=3). */
	uint64_t rp0_pid                      : 2;  /**< Sets msg response priority for incomimg messages
                                                         of priority 0 on the primary ID (0,1=1 2=2, 3=3). */
	uint64_t reserved_15_15               : 1;
	uint64_t prt_sel                      : 3;  /**< Port/controller selection method:
                                                         0x0 = Table lookup based on mailbox (See MBOX).
                                                         0x1 = Table lookup based on priority (See PRIO).
                                                         0x2 = Table lookup based on letter (See LTTR).
                                                         0x3 = Size-based (SP to port 0, MP to port 1).
                                                         0x4 = ID-based (pri ID to port 0, sec ID to port 1). */
	uint64_t lttr                         : 4;  /**< Port/controller selection letter table.
                                                         Type 11 traffic supports 4 letters (A-D).
                                                         0x0 = All letters to port 0.
                                                         0x1 = Letter A to port 1, others to port 0.
                                                         - ....
                                                         0x5 = Letter A,C to port 1, others to port 0.
                                                         - ....
                                                         0xF = All letters to port 1. */
	uint64_t prio                         : 4;  /**< Port/controller selection priority table.
                                                         SRIO supports 4 priorities (0-3).
                                                         0x0 = All priorities to port 0.
                                                         0x1 = Priority 0 to port 1, others to port 0.
                                                         - ....
                                                         0x9 = Priority 0,3 to port 1, others to port 0.
                                                         - ....
                                                         0xF = All priorities to port 1. */
	uint64_t mbox                         : 4;  /**< Port/controller selection mailbox table.
                                                         Type 11 traffic supports 4 mailboxes (0-3).
                                                         0x0 = All mailboxes to port 0.
                                                         0x1 = Mailbox 0 to port 1, others to port 0.
                                                         - ....
                                                         0x6 = Mailboxes 1,2 to port 1, others to port 0.
                                                         - ....
                                                         0xF = All mailboxes to port 1. */
#else
	uint64_t mbox                         : 4;
	uint64_t prio                         : 4;
	uint64_t lttr                         : 4;
	uint64_t prt_sel                      : 3;
	uint64_t reserved_15_15               : 1;
	uint64_t rp0_pid                      : 2;
	uint64_t rp1_pid                      : 1;
	uint64_t rp0_sid                      : 2;
	uint64_t rp1_sid                      : 1;
	uint64_t reserved_22_23               : 2;
	uint64_t rsp_thr                      : 6;
	uint64_t reserved_30_30               : 1;
	uint64_t to_mode                      : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sriox_imsg_ctrl_s         cn63xx;
	struct cvmx_sriox_imsg_ctrl_s         cn63xxp1;
	struct cvmx_sriox_imsg_ctrl_s         cn66xx;
	struct cvmx_sriox_imsg_ctrl_s         cnf75xx;
};
typedef union cvmx_sriox_imsg_ctrl cvmx_sriox_imsg_ctrl_t;

/**
 * cvmx_srio#_imsg_inst_hdr#
 *
 * SRIO HW generates the SRIO_WORD1 fields from this table.
 * SRIO_WORD1 is the 2nd of two header words that SRIO inserts in
 * front of all received messages. SRIO_WORD1 is commonly used
 * as a PKI_INST_HDR. The actual header word used is indexed by
 * the concatenation of SRIO_WORD0_S[TT,LETTER,PRIO,DIS,MBOX].
 * If port 1 has been selected by SRIO()_IMSG_CTRL[PRT_SEL]
 * then the SRIO()_IMSG_PRT1_HDR is XOR'ed with the resulting
 * header.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_imsg_inst_hdrx {
	uint64_t u64;
	struct cvmx_sriox_imsg_inst_hdrx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_sriox_imsg_inst_hdrx_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t r                            : 1;  /**< Port/Controller X R */
	uint64_t reserved_58_62               : 5;
	uint64_t pm                           : 2;  /**< Port/Controller X PM */
	uint64_t reserved_55_55               : 1;
	uint64_t sl                           : 7;  /**< Port/Controller X SL */
	uint64_t reserved_46_47               : 2;
	uint64_t nqos                         : 1;  /**< Port/Controller X NQOS */
	uint64_t ngrp                         : 1;  /**< Port/Controller X NGRP */
	uint64_t ntt                          : 1;  /**< Port/Controller X NTT */
	uint64_t ntag                         : 1;  /**< Port/Controller X NTAG */
	uint64_t reserved_35_41               : 7;
	uint64_t rs                           : 1;  /**< Port/Controller X RS */
	uint64_t tt                           : 2;  /**< Port/Controller X TT */
	uint64_t tag                          : 32; /**< Port/Controller X TAG */
#else
	uint64_t tag                          : 32;
	uint64_t tt                           : 2;
	uint64_t rs                           : 1;
	uint64_t reserved_35_41               : 7;
	uint64_t ntag                         : 1;
	uint64_t ntt                          : 1;
	uint64_t ngrp                         : 1;
	uint64_t nqos                         : 1;
	uint64_t reserved_46_47               : 2;
	uint64_t sl                           : 7;
	uint64_t reserved_55_55               : 1;
	uint64_t pm                           : 2;
	uint64_t reserved_58_62               : 5;
	uint64_t r                            : 1;
#endif
	} cn63xx;
	struct cvmx_sriox_imsg_inst_hdrx_cn63xx cn63xxp1;
	struct cvmx_sriox_imsg_inst_hdrx_cn63xx cn66xx;
	struct cvmx_sriox_imsg_inst_hdrx_cnf75xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t hdr                          : 64; /**< PKI instruction header word. */
#else
	uint64_t hdr                          : 64;
#endif
	} cnf75xx;
};
typedef union cvmx_sriox_imsg_inst_hdrx cvmx_sriox_imsg_inst_hdrx_t;

/**
 * cvmx_srio#_imsg_pkind#
 *
 * This register is reset by the h-clock reset.
 *
 */
union cvmx_sriox_imsg_pkindx {
	uint64_t u64;
	struct cvmx_sriox_imsg_pkindx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t pknd                         : 6;  /**< PKI port kind for this controller. */
#else
	uint64_t pknd                         : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_sriox_imsg_pkindx_s       cnf75xx;
};
typedef union cvmx_sriox_imsg_pkindx cvmx_sriox_imsg_pkindx_t;

/**
 * cvmx_srio#_imsg_prt1_hdr
 *
 * This register allows extra control in generating SRIO_WORD1 header
 * information for message port 1.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_imsg_prt1_hdr {
	uint64_t u64;
	struct cvmx_sriox_imsg_prt1_hdr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t xhdr                         : 64; /**< The field provided is XOR'd with the value provided by SRIO()_IMSG_INST_HDR.
                                                         The field is typically zero and is probably used only for cases when message port
                                                         is selected based on size (see SRIO()_IMSG_CTRL[PRT_SEL] = 3). */
#else
	uint64_t xhdr                         : 64;
#endif
	} s;
	struct cvmx_sriox_imsg_prt1_hdr_s     cnf75xx;
};
typedef union cvmx_sriox_imsg_prt1_hdr cvmx_sriox_imsg_prt1_hdr_t;

/**
 * cvmx_srio#_imsg_qos_grp#
 *
 * SRIO_IMSG_QOS_GRPX = SRIO Incoming Message QOS/GRP Table
 *
 * The SRIO Incoming Message QOS/GRP Table Entry X
 *
 * Notes:
 * The QOS/GRP table contains 32 entries with 8 QOS/GRP pairs per entry - 256 pairs total.  HW
 *  selects the table entry by the concatenation of SRIO_WORD0[PRIO,DIS,MBOX], thus entry 0 is used
 *  for messages with PRIO=0,DIS=0,MBOX=0, entry 1 is for PRIO=0,DIS=0,MBOX=1, etc.  HW selects the
 *  QOS/GRP pair from the table entry by the concatenation of SRIO_WORD0[ID,LETTER] as shown above. HW
 *  then inserts the QOS/GRP pair into SRIO_WORD1[QOS,GRP], which may commonly be used for the PIP/IPD
 *  PKT_INST_HDR[QOS,GRP] fields.
 *
 * Clk_Rst:        SRIO(0,2..3)_IMSG_QOS_GRP[0:1]  hclk    hrst_n
 */
union cvmx_sriox_imsg_qos_grpx {
	uint64_t u64;
	struct cvmx_sriox_imsg_qos_grpx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_63_63               : 1;
	uint64_t qos7                         : 3;  /**< Entry X:7 QOS (ID=1, LETTER=3) */
	uint64_t grp7                         : 4;  /**< Entry X:7 GRP (ID=1, LETTER=3) */
	uint64_t reserved_55_55               : 1;
	uint64_t qos6                         : 3;  /**< Entry X:6 QOS (ID=1, LETTER=2) */
	uint64_t grp6                         : 4;  /**< Entry X:6 GRP (ID=1, LETTER=2) */
	uint64_t reserved_47_47               : 1;
	uint64_t qos5                         : 3;  /**< Entry X:5 QOS (ID=1, LETTER=1) */
	uint64_t grp5                         : 4;  /**< Entry X:5 GRP (ID=1, LETTER=1) */
	uint64_t reserved_39_39               : 1;
	uint64_t qos4                         : 3;  /**< Entry X:4 QOS (ID=1, LETTER=0) */
	uint64_t grp4                         : 4;  /**< Entry X:4 GRP (ID=1, LETTER=0) */
	uint64_t reserved_31_31               : 1;
	uint64_t qos3                         : 3;  /**< Entry X:3 QOS (ID=0, LETTER=3) */
	uint64_t grp3                         : 4;  /**< Entry X:3 GRP (ID=0, LETTER=3) */
	uint64_t reserved_23_23               : 1;
	uint64_t qos2                         : 3;  /**< Entry X:2 QOS (ID=0, LETTER=2) */
	uint64_t grp2                         : 4;  /**< Entry X:2 GRP (ID=0, LETTER=2) */
	uint64_t reserved_15_15               : 1;
	uint64_t qos1                         : 3;  /**< Entry X:1 QOS (ID=0, LETTER=1) */
	uint64_t grp1                         : 4;  /**< Entry X:1 GRP (ID=0, LETTER=1) */
	uint64_t reserved_7_7                 : 1;
	uint64_t qos0                         : 3;  /**< Entry X:0 QOS (ID=0, LETTER=0) */
	uint64_t grp0                         : 4;  /**< Entry X:0 GRP (ID=0, LETTER=0) */
#else
	uint64_t grp0                         : 4;
	uint64_t qos0                         : 3;
	uint64_t reserved_7_7                 : 1;
	uint64_t grp1                         : 4;
	uint64_t qos1                         : 3;
	uint64_t reserved_15_15               : 1;
	uint64_t grp2                         : 4;
	uint64_t qos2                         : 3;
	uint64_t reserved_23_23               : 1;
	uint64_t grp3                         : 4;
	uint64_t qos3                         : 3;
	uint64_t reserved_31_31               : 1;
	uint64_t grp4                         : 4;
	uint64_t qos4                         : 3;
	uint64_t reserved_39_39               : 1;
	uint64_t grp5                         : 4;
	uint64_t qos5                         : 3;
	uint64_t reserved_47_47               : 1;
	uint64_t grp6                         : 4;
	uint64_t qos6                         : 3;
	uint64_t reserved_55_55               : 1;
	uint64_t grp7                         : 4;
	uint64_t qos7                         : 3;
	uint64_t reserved_63_63               : 1;
#endif
	} s;
	struct cvmx_sriox_imsg_qos_grpx_s     cn63xx;
	struct cvmx_sriox_imsg_qos_grpx_s     cn63xxp1;
	struct cvmx_sriox_imsg_qos_grpx_s     cn66xx;
};
typedef union cvmx_sriox_imsg_qos_grpx cvmx_sriox_imsg_qos_grpx_t;

/**
 * cvmx_srio#_imsg_status#
 *
 * This register is reset by the h-clock reset.
 *
 */
union cvmx_sriox_imsg_statusx {
	uint64_t u64;
	struct cvmx_sriox_imsg_statusx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t val1                         : 1;  /**< Entry 1 valid. */
	uint64_t err1                         : 1;  /**< Entry 1 error. */
	uint64_t toe1                         : 1;  /**< Entry 1 timeout error. */
	uint64_t toc1                         : 1;  /**< Entry 1 timeout count. */
	uint64_t prt1                         : 1;  /**< Entry 1 port. */
	uint64_t reserved_58_58               : 1;
	uint64_t tt1                          : 1;  /**< Entry 1 TT ID. */
	uint64_t dis1                         : 1;  /**< Entry 1 Dest ID. */
	uint64_t seg1                         : 4;  /**< Entry 1 next segment. */
	uint64_t mbox1                        : 2;  /**< Entry 1 mailbox. */
	uint64_t lttr1                        : 2;  /**< Entry 1 letter. */
	uint64_t sid1                         : 16; /**< Entry 1 source ID. */
	uint64_t val0                         : 1;  /**< Entry 0 valid. */
	uint64_t err0                         : 1;  /**< Entry 0 error. */
	uint64_t toe0                         : 1;  /**< Entry 0 timeout error. */
	uint64_t toc0                         : 1;  /**< Entry 0 timeout count. */
	uint64_t prt0                         : 1;  /**< Entry 0 port. */
	uint64_t reserved_26_26               : 1;
	uint64_t tt0                          : 1;  /**< Entry 0 TT ID. */
	uint64_t dis0                         : 1;  /**< Entry 0 dest ID. */
	uint64_t seg0                         : 4;  /**< Entry 0 next segment. */
	uint64_t mbox0                        : 2;  /**< Entry 0 mailbox. */
	uint64_t lttr0                        : 2;  /**< Entry 0 letter. */
	uint64_t sid0                         : 16; /**< Entry 0 source ID. */
#else
	uint64_t sid0                         : 16;
	uint64_t lttr0                        : 2;
	uint64_t mbox0                        : 2;
	uint64_t seg0                         : 4;
	uint64_t dis0                         : 1;
	uint64_t tt0                          : 1;
	uint64_t reserved_26_26               : 1;
	uint64_t prt0                         : 1;
	uint64_t toc0                         : 1;
	uint64_t toe0                         : 1;
	uint64_t err0                         : 1;
	uint64_t val0                         : 1;
	uint64_t sid1                         : 16;
	uint64_t lttr1                        : 2;
	uint64_t mbox1                        : 2;
	uint64_t seg1                         : 4;
	uint64_t dis1                         : 1;
	uint64_t tt1                          : 1;
	uint64_t reserved_58_58               : 1;
	uint64_t prt1                         : 1;
	uint64_t toc1                         : 1;
	uint64_t toe1                         : 1;
	uint64_t err1                         : 1;
	uint64_t val1                         : 1;
#endif
	} s;
	struct cvmx_sriox_imsg_statusx_s      cn63xx;
	struct cvmx_sriox_imsg_statusx_s      cn63xxp1;
	struct cvmx_sriox_imsg_statusx_s      cn66xx;
	struct cvmx_sriox_imsg_statusx_s      cnf75xx;
};
typedef union cvmx_sriox_imsg_statusx cvmx_sriox_imsg_statusx_t;

/**
 * cvmx_srio#_imsg_vport_thr
 *
 * This register allocates the virtual ports (vports) between the two
 * inbound message ports used by each SRIO MAC.  These channels are also
 * known as reassembly IDs (RIDs) on some of the other devices.  Care must
 * be taken to avoid using the same vport on more than one device.  The
 * 75xx default settings create a conflict that forces reprogramming this
 * register before message traffic can be enabled.  The typical 75xx vport
 * IDs allocations allow the SRIO MACS to use vports 6 thru 95.  The default
 * csr values allocate the following:
 *
 * SRIO0  BASE =  8, MAX_TOT = 44, SP_EN = 1
 *     This uses vports  8 thru 50 for MP Messages and vport 52 for SP Messages
 * SRIO1  BASE = 52, MAX_TOT = 44, SP_EN = 1
 *     This uses vports 52 thru 94 for MP Messages and vport 96 for SP Messages
 *
 * Unfortunately vport 52 conflicts between SRIOs and vport 96 conflicts
 * with another device.  Multiple configurations can be used to resolve the
 * issue depending on SRIO usage.  Many more options are possible.
 *
 *   1.  Changing the value of MAX_TOT to 43 on both devices.
 *   2.  Switching the SRIO0 BASE to 6 and the SRIO1 BASE to 51 allows
 *       vports 6 thru 95 to be allocated.
 *   3.  If a single SRIO handles inbound message traffic then setting
 *       SRIOn BASE = 8, MAX_TOT = 47, SP_EN = 1.
 *   4.  If Single Packet message are uncommon then setting SP_EN = 0
 *       on both SRIOs.
 *
 * This register can be accessed regardless of the value in
 * SRIO()_STATUS_REG[ACCESS] and is not effected by MAC reset.  The
 * maximum number of VPORTs allocated to a MAC is limited to 47.
 *
 * This register is reset by the coprocessor-clock reset.
 */
union cvmx_sriox_imsg_vport_thr {
	uint64_t u64;
	struct cvmx_sriox_imsg_vport_thr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_63_63               : 1;
	uint64_t base                         : 7;  /**< Vport starting offset.  The Vports used between SRIO0 and SRIO1 must not overlap
                                                         with each other or other devices.  The first 8 vports are initially for BGX, Loopback,
                                                         etc. See PKI_REASM_E. */
	uint64_t reserved_54_55               : 2;
	uint64_t max_tot                      : 6;  /**< Sets max number of vports available to this SRIO MAC.  Maximum value supported by
                                                         hardware is 47 with SP_VPORT set or 46 with SP_VPORT clear.  The total number of
                                                         vports available to SRIO MACs by the 75xx is 94 but the number available to SRIO
                                                         depends on the configuration of other blocks.  The default is 44 vports each.
                                                         MAX_TOT value must be greater than or equal to the value in BUF_THR to
                                                         effectively limit the number of vports used. */
	uint64_t reserved_46_47               : 2;
	uint64_t max_s1                       : 6;  /**< Diagnostic use only.  Must be written to 0x30 for normal operation. */
	uint64_t reserved_38_39               : 2;
	uint64_t max_s0                       : 6;  /**< Diagnostic use only.  Must be written to 0x30 for normal operation. */
	uint64_t sp_vport                     : 1;  /**< Single-segment vport pre-allocation.
                                                         When set, single-segment messages use pre-allocated
                                                         vport slots and a single port is removed from the
                                                         MAX_TOT.  This port does not count towards the MAX_S0
                                                         and MAX_S1 thresholds.
                                                         When clear, single-segment messages must allocate
                                                         vport slots just like multi-segment messages do. */
	uint64_t reserved_20_30               : 11;
	uint64_t buf_thr                      : 4;  /**< Sets number of vports to be buffered by this
                                                         interface. BUF_THR must not be zero when receiving
                                                         messages. The max and recommended BUF_THR value is 8.
                                                         Lack of a buffered vport can force a retry for a received
                                                         first segment, so, particularly if SP_VPORT=0
                                                         (which is not recommended) or the segment size is
                                                         small, larger BUF_THR values may improve
                                                         performance. */
	uint64_t reserved_14_15               : 2;
	uint64_t max_p1                       : 6;  /**< Maximum number of open vports in port 1.  Setting the value
                                                         less than MAX_TOT (MAX_TOT-1 if SP_VPORT is set) effectively
                                                         further restricts the number of vports (partial messages)
                                                         received by the port at a time.
                                                         The values used in the MAX_P1 and MAX_P0 fields can be used
                                                         to "reserve" vports for each message port. */
	uint64_t reserved_6_7                 : 2;
	uint64_t max_p0                       : 6;  /**< Maximum number of open vports in port 0.  Setting the value
                                                         less than MAX_TOT (MAX_TOT-1 if SP_VPORT is set) effectively
                                                         further restricts the number of vports (partial messages)
                                                         received by the port at a time.
                                                         The values used in the MAX_P1 and MAX_P0 fields can be used
                                                         to "reserve" vports for each message port. */
#else
	uint64_t max_p0                       : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t max_p1                       : 6;
	uint64_t reserved_14_15               : 2;
	uint64_t buf_thr                      : 4;
	uint64_t reserved_20_30               : 11;
	uint64_t sp_vport                     : 1;
	uint64_t max_s0                       : 6;
	uint64_t reserved_38_39               : 2;
	uint64_t max_s1                       : 6;
	uint64_t reserved_46_47               : 2;
	uint64_t max_tot                      : 6;
	uint64_t reserved_54_55               : 2;
	uint64_t base                         : 7;
	uint64_t reserved_63_63               : 1;
#endif
	} s;
	struct cvmx_sriox_imsg_vport_thr_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t max_tot                      : 6;  /**< Sets max number of vports available to SRIO0+SRIO1
                                                         This field is only used in SRIO0.
                                                         SRIO1 never uses SRIO1_IMSG_VPORT_THR[MAX_TOT]. */
	uint64_t reserved_46_47               : 2;
	uint64_t max_s1                       : 6;  /**< Sets max number of vports available to SRIO1
                                                         This field is only used in SRIO0.
                                                         SRIO1 never uses SRIO1_IMSG_VPORT_THR[MAX_S1]. */
	uint64_t reserved_38_39               : 2;
	uint64_t max_s0                       : 6;  /**< Sets max number of vports available to SRIO0
                                                         This field is only used in SRIO0.
                                                         SRIO1 never uses SRIO1_IMSG_VPORT_THR[MAX_S0]. */
	uint64_t sp_vport                     : 1;  /**< Single-segment vport pre-allocation.
                                                         When set, single-segment messages use pre-allocated
                                                         vport slots (that do not count toward thresholds).
                                                         When clear, single-segment messages must allocate
                                                         vport slots just like multi-segment messages do. */
	uint64_t reserved_20_30               : 11;
	uint64_t buf_thr                      : 4;  /**< Sets number of vports to be buffered by this
                                                         interface. BUF_THR must not be zero when receiving
                                                         messages. The max BUF_THR value is 8.
                                                         Recommend BUF_THR values 1-4. If the 46 available
                                                         vports are not statically-allocated across the two
                                                         SRIO's, smaller BUF_THR values may leave more
                                                         vports available for the other SRIO. Lack of a
                                                         buffered vport can force a retry for a received
                                                         first segment, so, particularly if SP_VPORT=0
                                                         (which is not recommended) or the segment size is
                                                         small, larger BUF_THR values may improve
                                                         performance. */
	uint64_t reserved_14_15               : 2;
	uint64_t max_p1                       : 6;  /**< Sets max number of open vports in port 1
                                                         A setting of 0x3F disables vport limit
                                                         checking for this message port.  This port should
                                                         be disabled if timeouts or other message format
                                                         errors are detected. */
	uint64_t reserved_6_7                 : 2;
	uint64_t max_p0                       : 6;  /**< Sets max number of open vports in port 0
                                                         A setting of 0x3F disables vport limit
                                                         checking for this message port.  This port should
                                                         be disabled if timeouts or other message format
                                                         errors are detected. */
#else
	uint64_t max_p0                       : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t max_p1                       : 6;
	uint64_t reserved_14_15               : 2;
	uint64_t buf_thr                      : 4;
	uint64_t reserved_20_30               : 11;
	uint64_t sp_vport                     : 1;
	uint64_t max_s0                       : 6;
	uint64_t reserved_38_39               : 2;
	uint64_t max_s1                       : 6;
	uint64_t reserved_46_47               : 2;
	uint64_t max_tot                      : 6;
	uint64_t reserved_54_63               : 10;
#endif
	} cn63xx;
	struct cvmx_sriox_imsg_vport_thr_cn63xx cn63xxp1;
	struct cvmx_sriox_imsg_vport_thr_cn63xx cn66xx;
	struct cvmx_sriox_imsg_vport_thr_s    cnf75xx;
};
typedef union cvmx_sriox_imsg_vport_thr cvmx_sriox_imsg_vport_thr_t;

/**
 * cvmx_srio#_imsg_vport_thr2
 *
 * SRIO_IMSG_VPORT_THR2 = SRIO Incoming Message Virtual Port Additional Threshold
 *
 * The SRIO Incoming Message Virtual Port Additional Threshold Register
 *
 * Notes:
 * Additional vport thresholds for SRIO MACs 2 and 3.  This register is only used in SRIO0 and is only
 * used when the QLM0 is configured as x1 lanes or x2 lanes.  In the x1 case the maximum number of
 * VPORTs is limited to 44.  In the x2 case the maximum number of VPORTs is limited to 46.  These
 * values are ignored in the x4 configuration.  This register can be accessed regardless of the value
 * in SRIO(0,2..3)_STATUS_REG.ACCESS and is not effected by MAC reset.
 *
 * Clk_Rst:        SRIO(0,2..3)_IMSG_VPORT_THR     sclk    srst_n
 */
union cvmx_sriox_imsg_vport_thr2 {
	uint64_t u64;
	struct cvmx_sriox_imsg_vport_thr2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_46_63               : 18;
	uint64_t max_s3                       : 6;  /**< Sets max number of vports available to SRIO3
                                                         This field is only used in SRIO0. */
	uint64_t reserved_38_39               : 2;
	uint64_t max_s2                       : 6;  /**< Sets max number of vports available to SRIO2
                                                         This field is only used in SRIO0. */
	uint64_t reserved_0_31                : 32;
#else
	uint64_t reserved_0_31                : 32;
	uint64_t max_s2                       : 6;
	uint64_t reserved_38_39               : 2;
	uint64_t max_s3                       : 6;
	uint64_t reserved_46_63               : 18;
#endif
	} s;
	struct cvmx_sriox_imsg_vport_thr2_s   cn66xx;
};
typedef union cvmx_sriox_imsg_vport_thr2 cvmx_sriox_imsg_vport_thr2_t;

/**
 * cvmx_srio#_int2_enable
 *
 * SRIO_INT2_ENABLE = SRIO Interrupt 2 Enable
 *
 * Allows SRIO to generate additional interrupts when corresponding enable bit is set.
 *
 * Notes:
 * This register enables interrupts in SRIO(0,2..3)_INT2_REG that can be asserted while the MAC is in reset.
 *  The register can be accessed/modified regardless of the value of SRIO(0,2..3)_STATUS_REG.ACCESS.
 *
 * Clk_Rst:        SRIO(0,2..3)_INT2_ENABLE        sclk    srst_n
 */
union cvmx_sriox_int2_enable {
	uint64_t u64;
	struct cvmx_sriox_int2_enable_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t pko_rst                      : 1;  /**< PKO Reset Error Enable */
#else
	uint64_t pko_rst                      : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_sriox_int2_enable_s       cn63xx;
	struct cvmx_sriox_int2_enable_s       cn66xx;
};
typedef union cvmx_sriox_int2_enable cvmx_sriox_int2_enable_t;

/**
 * cvmx_srio#_int2_reg
 *
 * SRIO_INT2_REG = SRIO Interrupt 2 Register
 *
 * Displays and clears which enabled interrupts have occured
 *
 * Notes:
 * This register provides interrupt status. Unlike SRIO*_INT_REG, SRIO*_INT2_REG can be accessed
 *  whenever the SRIO is present, regardless of whether the corresponding SRIO is in reset or not.
 *  INT_SUM shows the status of the interrupts in SRIO(0,2..3)_INT_REG.  Any set bits written to this
 *  register clear the corresponding interrupt.  The register can be accessed/modified regardless of
 *  the value of SRIO(0,2..3)_STATUS_REG.ACCESS and probably should be the first register read when an SRIO
 *  interrupt occurs.
 *
 * Clk_Rst:        SRIO(0,2..3)_INT2_REG   sclk    srst_n
 */
union cvmx_sriox_int2_reg {
	uint64_t u64;
	struct cvmx_sriox_int2_reg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t int_sum                      : 1;  /**< Interrupt Set and Enabled in SRIO(0,2..3)_INT_REG */
	uint64_t reserved_1_30                : 30;
	uint64_t pko_rst                      : 1;  /**< PKO Reset Error - Message Received from PKO while
                                                         MAC in reset. */
#else
	uint64_t pko_rst                      : 1;
	uint64_t reserved_1_30                : 30;
	uint64_t int_sum                      : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sriox_int2_reg_s          cn63xx;
	struct cvmx_sriox_int2_reg_s          cn66xx;
};
typedef union cvmx_sriox_int2_reg cvmx_sriox_int2_reg_t;

/**
 * cvmx_srio#_int_enable
 *
 * SRIO_INT_ENABLE = SRIO Interrupt Enable
 *
 * Allows SRIO to generate interrupts when corresponding enable bit is set.
 *
 * Notes:
 * This register enables interrupts.
 *
 * Clk_Rst:        SRIO(0,2..3)_INT_ENABLE hclk    hrst_n
 */
union cvmx_sriox_int_enable {
	uint64_t u64;
	struct cvmx_sriox_int_enable_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t zero_pkt                     : 1;  /**< Received Incoming SRIO Zero byte packet */
	uint64_t ttl_tout                     : 1;  /**< Outgoing Packet Time to Live Timeout */
	uint64_t fail                         : 1;  /**< ERB Error Rate reached Fail Count */
	uint64_t degrade                      : 1;  /**< ERB Error Rate reached Degrade Count */
	uint64_t mac_buf                      : 1;  /**< SRIO MAC Buffer CRC Error */
	uint64_t f_error                      : 1;  /**< SRIO Fatal Port Error (MAC reset required) */
	uint64_t rtry_err                     : 1;  /**< Outbound Message Retry Threshold Exceeded */
	uint64_t pko_err                      : 1;  /**< Outbound Message Received PKO Error */
	uint64_t omsg_err                     : 1;  /**< Outbound Message Invalid Descriptor Error */
	uint64_t omsg1                        : 1;  /**< Controller 1 Outbound Message Complete */
	uint64_t omsg0                        : 1;  /**< Controller 0 Outbound Message Complete */
	uint64_t link_up                      : 1;  /**< Serial Link going from Inactive to Active */
	uint64_t link_dwn                     : 1;  /**< Serial Link going from Active to Inactive */
	uint64_t phy_erb                      : 1;  /**< Physical Layer Error detected in ERB */
	uint64_t log_erb                      : 1;  /**< Logical/Transport Layer Error detected in ERB */
	uint64_t soft_rx                      : 1;  /**< Incoming Packet received by Soft Packet FIFO */
	uint64_t soft_tx                      : 1;  /**< Outgoing Packet sent by Soft Packet FIFO */
	uint64_t mce_rx                       : 1;  /**< Incoming Multicast Event Symbol */
	uint64_t mce_tx                       : 1;  /**< Outgoing Multicast Event Transmit Complete */
	uint64_t wr_done                      : 1;  /**< Outgoing Last Nwrite_R DONE Response Received. */
	uint64_t sli_err                      : 1;  /**< Unsupported S2M Transaction Received. */
	uint64_t deny_wr                      : 1;  /**< Incoming Maint_Wr Access to Denied Bar Registers. */
	uint64_t bar_err                      : 1;  /**< Incoming Access Crossing/Missing BAR Address */
	uint64_t maint_op                     : 1;  /**< Internal Maintenance Operation Complete. */
	uint64_t rxbell                       : 1;  /**< One or more Incoming Doorbells Received. */
	uint64_t bell_err                     : 1;  /**< Outgoing Doorbell Timeout, Retry or Error. */
	uint64_t txbell                       : 1;  /**< Outgoing Doorbell Complete. */
#else
	uint64_t txbell                       : 1;
	uint64_t bell_err                     : 1;
	uint64_t rxbell                       : 1;
	uint64_t maint_op                     : 1;
	uint64_t bar_err                      : 1;
	uint64_t deny_wr                      : 1;
	uint64_t sli_err                      : 1;
	uint64_t wr_done                      : 1;
	uint64_t mce_tx                       : 1;
	uint64_t mce_rx                       : 1;
	uint64_t soft_tx                      : 1;
	uint64_t soft_rx                      : 1;
	uint64_t log_erb                      : 1;
	uint64_t phy_erb                      : 1;
	uint64_t link_dwn                     : 1;
	uint64_t link_up                      : 1;
	uint64_t omsg0                        : 1;
	uint64_t omsg1                        : 1;
	uint64_t omsg_err                     : 1;
	uint64_t pko_err                      : 1;
	uint64_t rtry_err                     : 1;
	uint64_t f_error                      : 1;
	uint64_t mac_buf                      : 1;
	uint64_t degrade                      : 1;
	uint64_t fail                         : 1;
	uint64_t ttl_tout                     : 1;
	uint64_t zero_pkt                     : 1;
	uint64_t reserved_27_63               : 37;
#endif
	} s;
	struct cvmx_sriox_int_enable_s        cn63xx;
	struct cvmx_sriox_int_enable_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_22_63               : 42;
	uint64_t f_error                      : 1;  /**< SRIO Fatal Port Error (MAC reset required) */
	uint64_t rtry_err                     : 1;  /**< Outbound Message Retry Threshold Exceeded */
	uint64_t pko_err                      : 1;  /**< Outbound Message Received PKO Error */
	uint64_t omsg_err                     : 1;  /**< Outbound Message Invalid Descriptor Error */
	uint64_t omsg1                        : 1;  /**< Controller 1 Outbound Message Complete */
	uint64_t omsg0                        : 1;  /**< Controller 0 Outbound Message Complete */
	uint64_t link_up                      : 1;  /**< Serial Link going from Inactive to Active */
	uint64_t link_dwn                     : 1;  /**< Serial Link going from Active to Inactive */
	uint64_t phy_erb                      : 1;  /**< Physical Layer Error detected in ERB */
	uint64_t log_erb                      : 1;  /**< Logical/Transport Layer Error detected in ERB */
	uint64_t soft_rx                      : 1;  /**< Incoming Packet received by Soft Packet FIFO */
	uint64_t soft_tx                      : 1;  /**< Outgoing Packet sent by Soft Packet FIFO */
	uint64_t mce_rx                       : 1;  /**< Incoming Multicast Event Symbol */
	uint64_t mce_tx                       : 1;  /**< Outgoing Multicast Event Transmit Complete */
	uint64_t wr_done                      : 1;  /**< Outgoing Last Nwrite_R DONE Response Received. */
	uint64_t sli_err                      : 1;  /**< Unsupported S2M Transaction Received. */
	uint64_t deny_wr                      : 1;  /**< Incoming Maint_Wr Access to Denied Bar Registers. */
	uint64_t bar_err                      : 1;  /**< Incoming Access Crossing/Missing BAR Address */
	uint64_t maint_op                     : 1;  /**< Internal Maintenance Operation Complete. */
	uint64_t rxbell                       : 1;  /**< One or more Incoming Doorbells Received. */
	uint64_t bell_err                     : 1;  /**< Outgoing Doorbell Timeout, Retry or Error. */
	uint64_t txbell                       : 1;  /**< Outgoing Doorbell Complete. */
#else
	uint64_t txbell                       : 1;
	uint64_t bell_err                     : 1;
	uint64_t rxbell                       : 1;
	uint64_t maint_op                     : 1;
	uint64_t bar_err                      : 1;
	uint64_t deny_wr                      : 1;
	uint64_t sli_err                      : 1;
	uint64_t wr_done                      : 1;
	uint64_t mce_tx                       : 1;
	uint64_t mce_rx                       : 1;
	uint64_t soft_tx                      : 1;
	uint64_t soft_rx                      : 1;
	uint64_t log_erb                      : 1;
	uint64_t phy_erb                      : 1;
	uint64_t link_dwn                     : 1;
	uint64_t link_up                      : 1;
	uint64_t omsg0                        : 1;
	uint64_t omsg1                        : 1;
	uint64_t omsg_err                     : 1;
	uint64_t pko_err                      : 1;
	uint64_t rtry_err                     : 1;
	uint64_t f_error                      : 1;
	uint64_t reserved_22_63               : 42;
#endif
	} cn63xxp1;
	struct cvmx_sriox_int_enable_s        cn66xx;
};
typedef union cvmx_sriox_int_enable cvmx_sriox_int_enable_t;

/**
 * cvmx_srio#_int_info0
 *
 * This register contains the first header word of the illegal s2m transaction
 * associated with the SLI_ERR interrupt.  The remaining information is located
 * in SRIO()_INT_INFO1.   This register is only updated when the SLI_ERR is initially
 * detected.  Once the interrupt is cleared then additional information can be captured.
 * Common errors include:
 *
 *   1.  Load/stores with length over 32.
 *
 *   2.  Load/stores that translate to maintenance ops with a length over 8.
 *
 *   3.  Load ops that translate to atomic ops with other than 1, 2 and 4 byte accesses.
 *
 *   4.  Load/store ops with a length 0.
 *
 *   5.  Unexpected responses.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_int_info0 {
	uint64_t u64;
	struct cvmx_sriox_int_info0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cmd                          : 4;  /**< Command.
                                                         0x0 = Load, outgoing read request.
                                                         0x4 = Store, outgoing write request.
                                                         0x8 = Response, outgoing read response.
                                                         _ All others are reserved and generate errors. */
	uint64_t reserved_56_59               : 4;
	uint64_t tag                          : 8;  /**< Internal transaction number */
	uint64_t reserved_42_47               : 6;
	uint64_t length                       : 10; /**< Data length in 64-bit words (load/store only). */
	uint64_t status                       : 3;  /**< Response status.
                                                         0x0 = Success.
                                                         0x1 = Error.
                                                         _ All others reserved. */
	uint64_t reserved_16_28               : 13;
	uint64_t be0                          : 8;  /**< First 64-bit word byte enables (load/store only). */
	uint64_t be1                          : 8;  /**< Last 64-bit word byte enables (load/store only). */
#else
	uint64_t be1                          : 8;
	uint64_t be0                          : 8;
	uint64_t reserved_16_28               : 13;
	uint64_t status                       : 3;
	uint64_t length                       : 10;
	uint64_t reserved_42_47               : 6;
	uint64_t tag                          : 8;
	uint64_t reserved_56_59               : 4;
	uint64_t cmd                          : 4;
#endif
	} s;
	struct cvmx_sriox_int_info0_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cmd                          : 4;  /**< Command
                                                         0 = Load, Outgoing Read Request
                                                         4 = Store, Outgoing Write Request
                                                         8 = Response, Outgoing Read Response
                                                         All Others are reserved and generate errors */
	uint64_t type                         : 4;  /**< Command Type
                                                         Load/Store SRIO_S2M_TYPE used
                                                         Response (Reserved) */
	uint64_t tag                          : 8;  /**< Internal Transaction Number */
	uint64_t reserved_42_47               : 6;
	uint64_t length                       : 10; /**< Data Length in 64-bit Words (Load/Store Only) */
	uint64_t status                       : 3;  /**< Response Status
                                                         0 = Success
                                                         1 = Error
                                                         All others reserved */
	uint64_t reserved_16_28               : 13;
	uint64_t be0                          : 8;  /**< First 64-bit Word Byte Enables (Load/Store Only) */
	uint64_t be1                          : 8;  /**< Last 64-bit Word Byte Enables (Load/Store Only) */
#else
	uint64_t be1                          : 8;
	uint64_t be0                          : 8;
	uint64_t reserved_16_28               : 13;
	uint64_t status                       : 3;
	uint64_t length                       : 10;
	uint64_t reserved_42_47               : 6;
	uint64_t tag                          : 8;
	uint64_t type                         : 4;
	uint64_t cmd                          : 4;
#endif
	} cn63xx;
	struct cvmx_sriox_int_info0_cn63xx    cn63xxp1;
	struct cvmx_sriox_int_info0_cn63xx    cn66xx;
	struct cvmx_sriox_int_info0_cnf75xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cmd                          : 4;  /**< Command.
                                                         0x0 = Load, outgoing read request.
                                                         0x4 = Store, outgoing write request.
                                                         0x8 = Response, outgoing read response.
                                                         _ All others are reserved and generate errors. */
	uint64_t typ                          : 4;  /**< Command type.
                                                         Load/store SRIO_S2M_TYPE used.
                                                         Response (Reserved). */
	uint64_t tag                          : 8;  /**< Internal transaction number */
	uint64_t reserved_47_42               : 6;
	uint64_t length                       : 10; /**< Data length in 64-bit words (load/store only). */
	uint64_t status                       : 3;  /**< Response status.
                                                         0x0 = Success.
                                                         0x1 = Error.
                                                         _ All others reserved. */
	uint64_t reserved_28_16               : 13;
	uint64_t be0                          : 8;  /**< First 64-bit word byte enables (load/store only). */
	uint64_t be1                          : 8;  /**< Last 64-bit word byte enables (load/store only). */
#else
	uint64_t be1                          : 8;
	uint64_t be0                          : 8;
	uint64_t reserved_28_16               : 13;
	uint64_t status                       : 3;
	uint64_t length                       : 10;
	uint64_t reserved_47_42               : 6;
	uint64_t tag                          : 8;
	uint64_t typ                          : 4;
	uint64_t cmd                          : 4;
#endif
	} cnf75xx;
};
typedef union cvmx_sriox_int_info0 cvmx_sriox_int_info0_t;

/**
 * cvmx_srio#_int_info1
 *
 * This register contains the second header word of the illegal s2m transaction
 * associated with the SLI_ERR interrupt.  The remaining information is located
 * in SRIO()_INT_INFO0.   This register is only updated when the SLI_ERR is initially
 * detected.  Once the interrupt is cleared then additional information can be captured.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_int_info1 {
	uint64_t u64;
	struct cvmx_sriox_int_info1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t info1                        : 64; /**< Address (load/store) or first 64-bit word of
                                                         response data associated with interrupt. */
#else
	uint64_t info1                        : 64;
#endif
	} s;
	struct cvmx_sriox_int_info1_s         cn63xx;
	struct cvmx_sriox_int_info1_s         cn63xxp1;
	struct cvmx_sriox_int_info1_s         cn66xx;
	struct cvmx_sriox_int_info1_s         cnf75xx;
};
typedef union cvmx_sriox_int_info1 cvmx_sriox_int_info1_t;

/**
 * cvmx_srio#_int_info2
 *
 * This register contains the invalid SRIO_OMSG_HDR_S associated
 * with the OMSG_ERR interrupt.  This register is only updated when the OMSG_ERR
 * is initially detected.  Once the interrupt is cleared then additional
 * information can be captured.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_int_info2 {
	uint64_t u64;
	struct cvmx_sriox_int_info2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t prio                         : 2;  /**< SRIO_OMSG_HDR_S[PRIO] associated with the OMSG_ERR interrupt. */
	uint64_t tt                           : 1;  /**< SRIO_OMSG_HDR_S[TT] associated with the OMSG_ERR interrupt. */
	uint64_t sis                          : 1;  /**< SRIO_OMSG_HDR_S[SIS] associated with the OMSG_ERR interrupt. */
	uint64_t ssize                        : 4;  /**< SRIO_OMSG_HDR_S[SSIZE] associated with the OMSG_ERR interrupt. */
	uint64_t did                          : 16; /**< SRIO_OMSG_HDR_S[DID] associated with the OMSG_ERR interrupt. */
	uint64_t xmbox                        : 4;  /**< SRIO_OMSG_HDR_S[XMBOX] associated with the OMSG_ERR interrupt. */
	uint64_t mbox                         : 2;  /**< SRIO_OMSG_HDR_S[MBOX] associated with the OMSG_ERR interrupt. */
	uint64_t letter                       : 2;  /**< SRIO_OMSG_HDR_S[LETTER] associated with the OMSG_ERR interrupt. */
	uint64_t rsrvd                        : 30; /**< SRIO_OMSG_HDR_S[RSRVD] associated with the OMSG_ERR interrupt. */
	uint64_t lns                          : 1;  /**< SRIO_OMSG_HDR_S[LNS] associated with the OMSG_ERR interrupt. */
	uint64_t intr                         : 1;  /**< SRIO_OMSG_HDR_S[INTR] associated with the OMSG_ERR interrupt. */
#else
	uint64_t intr                         : 1;
	uint64_t lns                          : 1;
	uint64_t rsrvd                        : 30;
	uint64_t letter                       : 2;
	uint64_t mbox                         : 2;
	uint64_t xmbox                        : 4;
	uint64_t did                          : 16;
	uint64_t ssize                        : 4;
	uint64_t sis                          : 1;
	uint64_t tt                           : 1;
	uint64_t prio                         : 2;
#endif
	} s;
	struct cvmx_sriox_int_info2_s         cn63xx;
	struct cvmx_sriox_int_info2_s         cn63xxp1;
	struct cvmx_sriox_int_info2_s         cn66xx;
	struct cvmx_sriox_int_info2_s         cnf75xx;
};
typedef union cvmx_sriox_int_info2 cvmx_sriox_int_info2_t;

/**
 * cvmx_srio#_int_info3
 *
 * This register contains the retry response associated with the RTRY_ERR interrupt.
 * The register is only updated when the RTRY_ERR is initially detected.  Once the
 * interrupt is cleared then additional information can be captured.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_int_info3 {
	uint64_t u64;
	struct cvmx_sriox_int_info3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t prio                         : 2;  /**< Priority of received retry response message. */
	uint64_t tt                           : 2;  /**< TT of received retry response message. */
	uint64_t reserved_56_59               : 4;
	uint64_t other                        : 48; /**< Other fields of received retry response message
                                                         If [TT]==0 (8-bit ID's):
                                                         _ <47:40> => destination ID.
                                                         _ <39:32> => source ID.
                                                         _ <31:28> => transaction (should be 1 - msg).
                                                         _ <27:24> => status (should be 3 - retry).
                                                         _ <23:22> => letter.
                                                         _ <21:20> => mbox.
                                                         _ <19:16> => msgseg.
                                                         _ <15:0>  => unused.
                                                         If [TT]==1 (16-bit ID's):
                                                         _ <47:32> => destination ID.
                                                         _ <31:16> => source ID.
                                                         _ <15:12> => transaction (should be 1 - msg).
                                                         _ <11:8>  => status (should be 3 - retry).
                                                         _ <7:6>   => letter.
                                                         _ <5:4>   => mbox.
                                                         _ <3:0>   => msgseg. */
	uint64_t reserved_0_7                 : 8;
#else
	uint64_t reserved_0_7                 : 8;
	uint64_t other                        : 48;
	uint64_t reserved_56_59               : 4;
	uint64_t tt                           : 2;
	uint64_t prio                         : 2;
#endif
	} s;
	struct cvmx_sriox_int_info3_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t prio                         : 2;  /**< Priority of received retry response message */
	uint64_t tt                           : 2;  /**< TT of received retry response message */
	uint64_t type                         : 4;  /**< Type of received retry response message
                                                         (should be 13) */
	uint64_t other                        : 48; /**< Other fields of received retry response message
                                                         If TT==0 (8-bit ID's)
                                                          OTHER<47:40> => destination ID
                                                          OTHER<39:32> => source ID
                                                          OTHER<31:28> => transaction (should be 1 - msg)
                                                          OTHER<27:24> => status (should be 3 - retry)
                                                          OTHER<23:22> => letter
                                                          OTHER<21:20> => mbox
                                                          OTHER<19:16> => msgseg
                                                          OTHER<15:0>  => unused
                                                         If TT==1 (16-bit ID's)
                                                          OTHER<47:32> => destination ID
                                                          OTHER<31:16> => source ID
                                                          OTHER<15:12> => transaction (should be 1 - msg)
                                                          OTHER<11:8>  => status (should be 3 - retry)
                                                          OTHER<7:6>   => letter
                                                          OTHER<5:4>   => mbox
                                                          OTHER<3:0>   => msgseg */
	uint64_t reserved_0_7                 : 8;
#else
	uint64_t reserved_0_7                 : 8;
	uint64_t other                        : 48;
	uint64_t type                         : 4;
	uint64_t tt                           : 2;
	uint64_t prio                         : 2;
#endif
	} cn63xx;
	struct cvmx_sriox_int_info3_cn63xx    cn63xxp1;
	struct cvmx_sriox_int_info3_cn63xx    cn66xx;
	struct cvmx_sriox_int_info3_cnf75xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t prio                         : 2;  /**< Priority of received retry response message. */
	uint64_t tt                           : 2;  /**< TT of received retry response message. */
	uint64_t typ                          : 4;  /**< Type of received retry response message (should be 13). */
	uint64_t other                        : 48; /**< Other fields of received retry response message
                                                         If [TT]==0 (8-bit ID's):
                                                         _ <47:40> => destination ID.
                                                         _ <39:32> => source ID.
                                                         _ <31:28> => transaction (should be 1 - msg).
                                                         _ <27:24> => status (should be 3 - retry).
                                                         _ <23:22> => letter.
                                                         _ <21:20> => mbox.
                                                         _ <19:16> => msgseg.
                                                         _ <15:0>  => unused.
                                                         If [TT]==1 (16-bit ID's):
                                                         _ <47:32> => destination ID.
                                                         _ <31:16> => source ID.
                                                         _ <15:12> => transaction (should be 1 - msg).
                                                         _ <11:8>  => status (should be 3 - retry).
                                                         _ <7:6>   => letter.
                                                         _ <5:4>   => mbox.
                                                         _ <3:0>   => msgseg. */
	uint64_t reserved_0_7                 : 8;
#else
	uint64_t reserved_0_7                 : 8;
	uint64_t other                        : 48;
	uint64_t typ                          : 4;
	uint64_t tt                           : 2;
	uint64_t prio                         : 2;
#endif
	} cnf75xx;
};
typedef union cvmx_sriox_int_info3 cvmx_sriox_int_info3_t;

/**
 * cvmx_srio#_int_reg
 *
 * This register provides interrupt status.  Unlike most of other SRIO registers,
 * this register can be accessed even when SRIO is in reset.
 * Any set bits written to this register clear the
 * corresponding interrupt.  The RXBELL interrupt is cleared by reading all the entries in the
 * incoming doorbell FIFO.  OMSG_ERR is set when
 * an invalid SRIO_OMSG_HDR_S is received.  The SRIO_OMSG_HDR_S is deemed to be invalid if
 * the SSIZE field is set to a reserved value, the SSIZE field combined with the packet length
 * would result in more than 16 message segments, or the packet only contains a SRIO_OMSG_HDR_S
 * (no data).
 *
 * This register is reset by the coprocessor clock reset.
 */
union cvmx_sriox_int_reg {
	uint64_t u64;
	struct cvmx_sriox_int_reg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t door_bell                    : 16; /**< One or more incoming doorbells received for SRIO doorbell 0-15.
                                                         These interrupts are valid when SRIO()_RX_BELL_CTRL[NUM_FIFO] = 1-5; */
	uint64_t int2_sum                     : 1;  /**< Interrupt Set and Enabled in SRIO(0,2..3)_INT2_REG */
	uint64_t reserved_29_30               : 2;
	uint64_t dbe                          : 1;  /**< Double bit internal memory error detected.
                                                         See SRIO()_ECC_STATUS[DBE]. */
	uint64_t sbe                          : 1;  /**< Single bit internal memory error detected.
                                                         See SRIO()_ECC_STATUS[SBE]. */
	uint64_t zero_pkt                     : 1;  /**< Received incoming SRIO zero byte packet. */
	uint64_t ttl_tout                     : 1;  /**< Outgoing packet time to live timeout.
                                                         See SRIOMAINT()_PORT_TTL_CTL.
                                                         See SRIOMAINT()_TX_DROP. */
	uint64_t fail                         : 1;  /**< ERB error rate reached fail count.
                                                         See SRIOMAINT()_ERB_ERR_RATE. */
	uint64_t degrad                       : 1;  /**< ERB error rate reached degrade count.
                                                         See SRIOMAINT()_ERB_ERR_RATE. */
	uint64_t mac_buf                      : 1;  /**< SRIO MAC buffer CRC error.  See SRIO()_MAC_BUFFERS. */
	uint64_t f_error                      : 1;  /**< SRIO fatal port error (MAC reset required). */
	uint64_t rtry_err                     : 1;  /**< Outbound message retry threshold exceeded.
                                                         See SRIO()_INT_INFO3.
                                                         When one or more of the segments in an outgoing
                                                         message have a RTRY_ERR, SRIO will not set
                                                         OMSG* after the message "transfer". */
	uint64_t reserved_19_19               : 1;
	uint64_t omsg_err                     : 1;  /**< Outbound message invalid SRIO_OMSG_HDR_S error.
                                                         See SRIO()_INT_INFO2. */
	uint64_t omsg1                        : 1;  /**< Controller 1 outbound message complete.
                                                         See SRIO()_OMSG_DONE_COUNTS1. */
	uint64_t omsg0                        : 1;  /**< Controller 0 outbound message complete.
                                                         See SRIO()_OMSG_DONE_COUNTS0. */
	uint64_t link_up                      : 1;  /**< Serial link going from inactive to active. */
	uint64_t link_dwn                     : 1;  /**< Serial link going from active to inactive. */
	uint64_t phy_erb                      : 1;  /**< Physical layer error detected in erb.
                                                         This is a summary interrupt of all SRIOMAINT physical layer interrupts,
                                                         See SRIOMAINT()_ERB_ATTR_CAPT. */
	uint64_t log_erb                      : 1;  /**< Logical/transport layer error detected in ERB.
                                                         This is a summary interrupt of all SRIOMAINT logical layer interrupts,
                                                         See SRIOMAINT()_ERB_LT_ERR_DET. */
	uint64_t soft_rx                      : 1;  /**< Incoming packet received by soft packet FIFO. */
	uint64_t soft_tx                      : 1;  /**< Outgoing packet sent by soft packet FIFO. */
	uint64_t mce_rx                       : 1;  /**< Incoming multicast event symbol. */
	uint64_t mce_tx                       : 1;  /**< Outgoing multicast event transmit complete. */
	uint64_t wr_done                      : 1;  /**< Outgoing last nwrite_r DONE response received.
                                                         See SRIO()_WR_DONE_COUNTS. */
	uint64_t sli_err                      : 1;  /**< Unsupported S2M transaction received.
                                                         See SRIO()_INT_INFO0, SRIO()_INT_INFO1. */
	uint64_t deny_wr                      : 1;  /**< Incoming maint_wr access to denied bar registers. */
	uint64_t bar_err                      : 1;  /**< Incoming access crossing/missing BAR address. */
	uint64_t maint_op                     : 1;  /**< Internal maintenance operation complete.
                                                         See SRIO()_MAINT_OP and SRIO()_MAINT_RD_DATA. */
	uint64_t rxbell                       : 1;  /**< One or more incoming doorbells received.
                                                         This interrupt is only valid in back-compatible mode,
                                                         when SRIO()_RX_BELL_CTRL[NUM_FIFO] = 0;
                                                         Read SRIO()_RX_BELL to empty FIFO. */
	uint64_t bell_err                     : 1;  /**< Outgoing doorbell timeout, retry or error.
                                                         See SRIO()_TX_BELL_INFO. */
	uint64_t txbell                       : 1;  /**< Outgoing doorbell complete.
                                                         TXBELL will not be asserted if a timeout, retry or
                                                         error occurs. */
#else
	uint64_t txbell                       : 1;
	uint64_t bell_err                     : 1;
	uint64_t rxbell                       : 1;
	uint64_t maint_op                     : 1;
	uint64_t bar_err                      : 1;
	uint64_t deny_wr                      : 1;
	uint64_t sli_err                      : 1;
	uint64_t wr_done                      : 1;
	uint64_t mce_tx                       : 1;
	uint64_t mce_rx                       : 1;
	uint64_t soft_tx                      : 1;
	uint64_t soft_rx                      : 1;
	uint64_t log_erb                      : 1;
	uint64_t phy_erb                      : 1;
	uint64_t link_dwn                     : 1;
	uint64_t link_up                      : 1;
	uint64_t omsg0                        : 1;
	uint64_t omsg1                        : 1;
	uint64_t omsg_err                     : 1;
	uint64_t reserved_19_19               : 1;
	uint64_t rtry_err                     : 1;
	uint64_t f_error                      : 1;
	uint64_t mac_buf                      : 1;
	uint64_t degrad                       : 1;
	uint64_t fail                         : 1;
	uint64_t ttl_tout                     : 1;
	uint64_t zero_pkt                     : 1;
	uint64_t sbe                          : 1;
	uint64_t dbe                          : 1;
	uint64_t reserved_29_30               : 2;
	uint64_t int2_sum                     : 1;
	uint64_t door_bell                    : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_sriox_int_reg_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t int2_sum                     : 1;  /**< Interrupt Set and Enabled in SRIO(0..1)_INT2_REG
                                                         (Pass 2) */
	uint64_t reserved_27_30               : 4;
	uint64_t zero_pkt                     : 1;  /**< Received Incoming SRIO Zero byte packet (Pass 2) */
	uint64_t ttl_tout                     : 1;  /**< Outgoing Packet Time to Live Timeout (Pass 2)
                                                         See SRIOMAINT(0..1)_PORT_TTL_CTL
                                                         See SRIOMAINT(0..1)_TX_DROP */
	uint64_t fail                         : 1;  /**< ERB Error Rate reached Fail Count (Pass 2)
                                                         See SRIOMAINT(0..1)_ERB_ERR_RATE */
	uint64_t degrad                       : 1;  /**< ERB Error Rate reached Degrade Count (Pass 2)
                                                         See SRIOMAINT(0..1)_ERB_ERR_RATE */
	uint64_t mac_buf                      : 1;  /**< SRIO MAC Buffer CRC Error (Pass 2)
                                                         See SRIO(0..1)_MAC_BUFFERS */
	uint64_t f_error                      : 1;  /**< SRIO Fatal Port Error (MAC reset required) */
	uint64_t rtry_err                     : 1;  /**< Outbound Message Retry Threshold Exceeded
                                                         See SRIO(0..1)_INT_INFO3
                                                         When one or more of the segments in an outgoing
                                                         message have a RTRY_ERR, SRIO will not set
                                                         OMSG* after the message "transfer". */
	uint64_t pko_err                      : 1;  /**< Outbound Message Received PKO Error */
	uint64_t omsg_err                     : 1;  /**< Outbound Message Invalid Descriptor Error
                                                         See SRIO(0..1)_INT_INFO2 */
	uint64_t omsg1                        : 1;  /**< Controller 1 Outbound Message Complete
                                                         See SRIO(0..1)_OMSG_DONE_COUNTS1 */
	uint64_t omsg0                        : 1;  /**< Controller 0 Outbound Message Complete
                                                         See SRIO(0..1)_OMSG_DONE_COUNTS0 */
	uint64_t link_up                      : 1;  /**< Serial Link going from Inactive to Active */
	uint64_t link_dwn                     : 1;  /**< Serial Link going from Active to Inactive */
	uint64_t phy_erb                      : 1;  /**< Physical Layer Error detected in ERB
                                                         See SRIOMAINT*_ERB_ATTR_CAPT */
	uint64_t log_erb                      : 1;  /**< Logical/Transport Layer Error detected in ERB
                                                         See SRIOMAINT(0..1)_ERB_LT_ERR_DET */
	uint64_t soft_rx                      : 1;  /**< Incoming Packet received by Soft Packet FIFO */
	uint64_t soft_tx                      : 1;  /**< Outgoing Packet sent by Soft Packet FIFO */
	uint64_t mce_rx                       : 1;  /**< Incoming Multicast Event Symbol */
	uint64_t mce_tx                       : 1;  /**< Outgoing Multicast Event Transmit Complete */
	uint64_t wr_done                      : 1;  /**< Outgoing Last Nwrite_R DONE Response Received.
                                                         See SRIO(0..1)_WR_DONE_COUNTS */
	uint64_t sli_err                      : 1;  /**< Unsupported S2M Transaction Received.
                                                         See SRIO(0..1)_INT_INFO[1:0] */
	uint64_t deny_wr                      : 1;  /**< Incoming Maint_Wr Access to Denied Bar Registers. */
	uint64_t bar_err                      : 1;  /**< Incoming Access Crossing/Missing BAR Address */
	uint64_t maint_op                     : 1;  /**< Internal Maintenance Operation Complete.
                                                         See SRIO(0..1)_MAINT_OP and SRIO(0..1)_MAINT_RD_DATA */
	uint64_t rxbell                       : 1;  /**< One or more Incoming Doorbells Received.
                                                         Read SRIO(0..1)_RX_BELL to empty FIFO */
	uint64_t bell_err                     : 1;  /**< Outgoing Doorbell Timeout, Retry or Error.
                                                         See SRIO(0..1)_TX_BELL_INFO */
	uint64_t txbell                       : 1;  /**< Outgoing Doorbell Complete.
                                                         TXBELL will not be asserted if a Timeout, Retry or
                                                         Error occurs. */
#else
	uint64_t txbell                       : 1;
	uint64_t bell_err                     : 1;
	uint64_t rxbell                       : 1;
	uint64_t maint_op                     : 1;
	uint64_t bar_err                      : 1;
	uint64_t deny_wr                      : 1;
	uint64_t sli_err                      : 1;
	uint64_t wr_done                      : 1;
	uint64_t mce_tx                       : 1;
	uint64_t mce_rx                       : 1;
	uint64_t soft_tx                      : 1;
	uint64_t soft_rx                      : 1;
	uint64_t log_erb                      : 1;
	uint64_t phy_erb                      : 1;
	uint64_t link_dwn                     : 1;
	uint64_t link_up                      : 1;
	uint64_t omsg0                        : 1;
	uint64_t omsg1                        : 1;
	uint64_t omsg_err                     : 1;
	uint64_t pko_err                      : 1;
	uint64_t rtry_err                     : 1;
	uint64_t f_error                      : 1;
	uint64_t mac_buf                      : 1;
	uint64_t degrad                       : 1;
	uint64_t fail                         : 1;
	uint64_t ttl_tout                     : 1;
	uint64_t zero_pkt                     : 1;
	uint64_t reserved_27_30               : 4;
	uint64_t int2_sum                     : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} cn63xx;
	struct cvmx_sriox_int_reg_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_22_63               : 42;
	uint64_t f_error                      : 1;  /**< SRIO Fatal Port Error (MAC reset required) */
	uint64_t rtry_err                     : 1;  /**< Outbound Message Retry Threshold Exceeded
                                                         See SRIO(0..1)_INT_INFO3
                                                         When one or more of the segments in an outgoing
                                                         message have a RTRY_ERR, SRIO will not set
                                                         OMSG* after the message "transfer". */
	uint64_t pko_err                      : 1;  /**< Outbound Message Received PKO Error */
	uint64_t omsg_err                     : 1;  /**< Outbound Message Invalid Descriptor Error
                                                         See SRIO(0..1)_INT_INFO2 */
	uint64_t omsg1                        : 1;  /**< Controller 1 Outbound Message Complete */
	uint64_t omsg0                        : 1;  /**< Controller 0 Outbound Message Complete */
	uint64_t link_up                      : 1;  /**< Serial Link going from Inactive to Active */
	uint64_t link_dwn                     : 1;  /**< Serial Link going from Active to Inactive */
	uint64_t phy_erb                      : 1;  /**< Physical Layer Error detected in ERB
                                                         See SRIOMAINT*_ERB_ATTR_CAPT */
	uint64_t log_erb                      : 1;  /**< Logical/Transport Layer Error detected in ERB
                                                         See SRIOMAINT(0..1)_ERB_LT_ERR_DET */
	uint64_t soft_rx                      : 1;  /**< Incoming Packet received by Soft Packet FIFO */
	uint64_t soft_tx                      : 1;  /**< Outgoing Packet sent by Soft Packet FIFO */
	uint64_t mce_rx                       : 1;  /**< Incoming Multicast Event Symbol */
	uint64_t mce_tx                       : 1;  /**< Outgoing Multicast Event Transmit Complete */
	uint64_t wr_done                      : 1;  /**< Outgoing Last Nwrite_R DONE Response Received. */
	uint64_t sli_err                      : 1;  /**< Unsupported S2M Transaction Received.
                                                         See SRIO(0..1)_INT_INFO[1:0] */
	uint64_t deny_wr                      : 1;  /**< Incoming Maint_Wr Access to Denied Bar Registers. */
	uint64_t bar_err                      : 1;  /**< Incoming Access Crossing/Missing BAR Address */
	uint64_t maint_op                     : 1;  /**< Internal Maintenance Operation Complete.
                                                         See SRIO(0..1)_MAINT_OP and SRIO(0..1)_MAINT_RD_DATA */
	uint64_t rxbell                       : 1;  /**< One or more Incoming Doorbells Received.
                                                         Read SRIO(0..1)_RX_BELL to empty FIFO */
	uint64_t bell_err                     : 1;  /**< Outgoing Doorbell Timeout, Retry or Error.
                                                         See SRIO(0..1)_TX_BELL_INFO */
	uint64_t txbell                       : 1;  /**< Outgoing Doorbell Complete.
                                                         TXBELL will not be asserted if a Timeout, Retry or
                                                         Error occurs. */
#else
	uint64_t txbell                       : 1;
	uint64_t bell_err                     : 1;
	uint64_t rxbell                       : 1;
	uint64_t maint_op                     : 1;
	uint64_t bar_err                      : 1;
	uint64_t deny_wr                      : 1;
	uint64_t sli_err                      : 1;
	uint64_t wr_done                      : 1;
	uint64_t mce_tx                       : 1;
	uint64_t mce_rx                       : 1;
	uint64_t soft_tx                      : 1;
	uint64_t soft_rx                      : 1;
	uint64_t log_erb                      : 1;
	uint64_t phy_erb                      : 1;
	uint64_t link_dwn                     : 1;
	uint64_t link_up                      : 1;
	uint64_t omsg0                        : 1;
	uint64_t omsg1                        : 1;
	uint64_t omsg_err                     : 1;
	uint64_t pko_err                      : 1;
	uint64_t rtry_err                     : 1;
	uint64_t f_error                      : 1;
	uint64_t reserved_22_63               : 42;
#endif
	} cn63xxp1;
	struct cvmx_sriox_int_reg_cn63xx      cn66xx;
	struct cvmx_sriox_int_reg_cnf75xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t door_bell                    : 16; /**< One or more incoming doorbells received for SRIO doorbell 0-15.
                                                         These interrupts are valid when SRIO()_RX_BELL_CTRL[NUM_FIFO] = 1-5; */
	uint64_t reserved_29_31               : 3;
	uint64_t dbe                          : 1;  /**< Double bit internal memory error detected.
                                                         See SRIO()_ECC_STATUS[DBE]. */
	uint64_t sbe                          : 1;  /**< Single bit internal memory error detected.
                                                         See SRIO()_ECC_STATUS[SBE]. */
	uint64_t zero_pkt                     : 1;  /**< Received incoming SRIO zero byte packet. */
	uint64_t ttl_tout                     : 1;  /**< Outgoing packet time to live timeout.
                                                         See SRIOMAINT()_PORT_TTL_CTL.
                                                         See SRIOMAINT()_TX_DROP. */
	uint64_t fail                         : 1;  /**< ERB error rate reached fail count.
                                                         See SRIOMAINT()_ERB_ERR_RATE. */
	uint64_t degrad                       : 1;  /**< ERB error rate reached degrade count.
                                                         See SRIOMAINT()_ERB_ERR_RATE. */
	uint64_t mac_buf                      : 1;  /**< SRIO MAC buffer CRC error.  See SRIO()_MAC_BUFFERS. */
	uint64_t f_error                      : 1;  /**< SRIO fatal port error (MAC reset required). */
	uint64_t rtry_err                     : 1;  /**< Outbound message retry threshold exceeded.
                                                         See SRIO()_INT_INFO3.
                                                         When one or more of the segments in an outgoing
                                                         message have a RTRY_ERR, SRIO will not set
                                                         OMSG* after the message "transfer". */
	uint64_t pko_rst_err                  : 1;  /**< PKO reset error - message received from PKO while MAC in reset. */
	uint64_t omsg_err                     : 1;  /**< Outbound message invalid SRIO_OMSG_HDR_S error.
                                                         See SRIO()_INT_INFO2. */
	uint64_t omsg1                        : 1;  /**< Controller 1 outbound message complete.
                                                         See SRIO()_OMSG_DONE_COUNTS1. */
	uint64_t omsg0                        : 1;  /**< Controller 0 outbound message complete.
                                                         See SRIO()_OMSG_DONE_COUNTS0. */
	uint64_t link_up                      : 1;  /**< Serial link going from inactive to active. */
	uint64_t link_dwn                     : 1;  /**< Serial link going from active to inactive. */
	uint64_t phy_erb                      : 1;  /**< Physical layer error detected in erb.
                                                         This is a summary interrupt of all SRIOMAINT physical layer interrupts,
                                                         See SRIOMAINT()_ERB_ATTR_CAPT. */
	uint64_t log_erb                      : 1;  /**< Logical/transport layer error detected in ERB.
                                                         This is a summary interrupt of all SRIOMAINT logical layer interrupts,
                                                         See SRIOMAINT()_ERB_LT_ERR_DET. */
	uint64_t soft_rx                      : 1;  /**< Incoming packet received by soft packet FIFO. */
	uint64_t soft_tx                      : 1;  /**< Outgoing packet sent by soft packet FIFO. */
	uint64_t mce_rx                       : 1;  /**< Incoming multicast event symbol. */
	uint64_t mce_tx                       : 1;  /**< Outgoing multicast event transmit complete. */
	uint64_t wr_done                      : 1;  /**< Outgoing last nwrite_r DONE response received.
                                                         See SRIO()_WR_DONE_COUNTS. */
	uint64_t sli_err                      : 1;  /**< Unsupported S2M transaction received.
                                                         See SRIO()_INT_INFO0, SRIO()_INT_INFO1. */
	uint64_t deny_wr                      : 1;  /**< Incoming maint_wr access to denied bar registers. */
	uint64_t bar_err                      : 1;  /**< Incoming access crossing/missing BAR address. */
	uint64_t maint_op                     : 1;  /**< Internal maintenance operation complete.
                                                         See SRIO()_MAINT_OP and SRIO()_MAINT_RD_DATA. */
	uint64_t rxbell                       : 1;  /**< One or more incoming doorbells received.
                                                         This interrupt is only valid in back-compatible mode,
                                                         when SRIO()_RX_BELL_CTRL[NUM_FIFO] = 0;
                                                         Read SRIO()_RX_BELL to empty FIFO. */
	uint64_t bell_err                     : 1;  /**< Outgoing doorbell timeout, retry or error.
                                                         See SRIO()_TX_BELL_INFO. */
	uint64_t txbell                       : 1;  /**< Outgoing doorbell complete.
                                                         TXBELL will not be asserted if a timeout, retry or
                                                         error occurs. */
#else
	uint64_t txbell                       : 1;
	uint64_t bell_err                     : 1;
	uint64_t rxbell                       : 1;
	uint64_t maint_op                     : 1;
	uint64_t bar_err                      : 1;
	uint64_t deny_wr                      : 1;
	uint64_t sli_err                      : 1;
	uint64_t wr_done                      : 1;
	uint64_t mce_tx                       : 1;
	uint64_t mce_rx                       : 1;
	uint64_t soft_tx                      : 1;
	uint64_t soft_rx                      : 1;
	uint64_t log_erb                      : 1;
	uint64_t phy_erb                      : 1;
	uint64_t link_dwn                     : 1;
	uint64_t link_up                      : 1;
	uint64_t omsg0                        : 1;
	uint64_t omsg1                        : 1;
	uint64_t omsg_err                     : 1;
	uint64_t pko_rst_err                  : 1;
	uint64_t rtry_err                     : 1;
	uint64_t f_error                      : 1;
	uint64_t mac_buf                      : 1;
	uint64_t degrad                       : 1;
	uint64_t fail                         : 1;
	uint64_t ttl_tout                     : 1;
	uint64_t zero_pkt                     : 1;
	uint64_t sbe                          : 1;
	uint64_t dbe                          : 1;
	uint64_t reserved_29_31               : 3;
	uint64_t door_bell                    : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} cnf75xx;
};
typedef union cvmx_sriox_int_reg cvmx_sriox_int_reg_t;

/**
 * cvmx_srio#_int_w1s
 */
union cvmx_sriox_int_w1s {
	uint64_t u64;
	struct cvmx_sriox_int_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t door_bell                    : 16; /**< Reads SRIO()_INT_REG[DOOR_BELL]. */
	uint64_t reserved_29_31               : 3;
	uint64_t dbe                          : 1;  /**< Reads SRIO()_INT_REG[DBE]. */
	uint64_t sbe                          : 1;  /**< Reads SRIO()_INT_REG[SBE]. */
	uint64_t zero_pkt                     : 1;  /**< Reads or sets  SRIO()_INT_REG[ZERO_PKT]. */
	uint64_t ttl_tout                     : 1;  /**< Reads or sets  SRIO()_INT_REG[TTL_TOUT]. */
	uint64_t fail                         : 1;  /**< Reads or sets  SRIO()_INT_REG[FAIL]. */
	uint64_t degrade                      : 1;  /**< Reads or sets  SRIO()_INT_REG[DEGRADE]. */
	uint64_t mac_buf                      : 1;  /**< Reads or sets  SRIO()_INT_REG[MAC_BUF]. */
	uint64_t f_error                      : 1;  /**< Reads or sets  SRIO()_INT_REG[F_ERROR]. */
	uint64_t rtry_err                     : 1;  /**< Reads or sets  SRIO()_INT_REG[RTRY_ERR]. */
	uint64_t pko_rst_err                  : 1;  /**< Reads or sets  SRIO()_INT_REG[PKO_RST_ERR]. */
	uint64_t omsg_err                     : 1;  /**< Reads or sets  SRIO()_INT_REG[OMSG_ERR]. */
	uint64_t omsg1                        : 1;  /**< Reads or sets  SRIO()_INT_REG[OMSG1]. */
	uint64_t omsg0                        : 1;  /**< Reads or sets  SRIO()_INT_REG[OMSG0]. */
	uint64_t link_up                      : 1;  /**< Reads or sets  SRIO()_INT_REG[LINK_UP]. */
	uint64_t link_dwn                     : 1;  /**< Reads or sets  SRIO()_INT_REG[LINK_DWN]. */
	uint64_t phy_erb                      : 1;  /**< Reads or sets SRIO()_INT_REG[PHY_ERB]. */
	uint64_t log_erb                      : 1;  /**< Reads or sets  SRIO()_INT_REG[LOG_ERB]. */
	uint64_t soft_rx                      : 1;  /**< Reads or sets  SRIO()_INT_REG[SOFT_RX]. */
	uint64_t soft_tx                      : 1;  /**< Reads or sets  SRIO()_INT_REG[SOFT_TX]. */
	uint64_t mce_rx                       : 1;  /**< Reads or sets  SRIO()_INT_REG[MCE_RX]. */
	uint64_t mce_tx                       : 1;  /**< Reads or sets  SRIO()_INT_REG[MCE_TX]. */
	uint64_t wr_done                      : 1;  /**< Reads or sets  SRIO()_INT_REG[WR_DONE]. */
	uint64_t sli_err                      : 1;  /**< Reads or sets  SRIO()_INT_REG[SLI_ERR]. */
	uint64_t deny_wr                      : 1;  /**< Reads or sets  SRIO()_INT_REG[BAR_ERR]. */
	uint64_t bar_err                      : 1;  /**< Reads or sets  SRIO()_INT_REG[BAR_ERR]. */
	uint64_t maint_op                     : 1;  /**< Reads or sets  SRIO()_INT_REG[MAINT_OP]. */
	uint64_t rxbell                       : 1;  /**< Reads SRIO()_INT_REG[RX_BELL]. */
	uint64_t bell_err                     : 1;  /**< Reads or sets  SRIO()_INT_REG[BELL_ERR]. */
	uint64_t txbell                       : 1;  /**< Reads or sets  SRIO()_INT_REG[TXBELL]. */
#else
	uint64_t txbell                       : 1;
	uint64_t bell_err                     : 1;
	uint64_t rxbell                       : 1;
	uint64_t maint_op                     : 1;
	uint64_t bar_err                      : 1;
	uint64_t deny_wr                      : 1;
	uint64_t sli_err                      : 1;
	uint64_t wr_done                      : 1;
	uint64_t mce_tx                       : 1;
	uint64_t mce_rx                       : 1;
	uint64_t soft_tx                      : 1;
	uint64_t soft_rx                      : 1;
	uint64_t log_erb                      : 1;
	uint64_t phy_erb                      : 1;
	uint64_t link_dwn                     : 1;
	uint64_t link_up                      : 1;
	uint64_t omsg0                        : 1;
	uint64_t omsg1                        : 1;
	uint64_t omsg_err                     : 1;
	uint64_t pko_rst_err                  : 1;
	uint64_t rtry_err                     : 1;
	uint64_t f_error                      : 1;
	uint64_t mac_buf                      : 1;
	uint64_t degrade                      : 1;
	uint64_t fail                         : 1;
	uint64_t ttl_tout                     : 1;
	uint64_t zero_pkt                     : 1;
	uint64_t sbe                          : 1;
	uint64_t dbe                          : 1;
	uint64_t reserved_29_31               : 3;
	uint64_t door_bell                    : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_sriox_int_w1s_s           cnf75xx;
};
typedef union cvmx_sriox_int_w1s cvmx_sriox_int_w1s_t;

/**
 * cvmx_srio#_ip_feature
 *
 * This register is used to override powerup values used by the
 * SRIOMAINT registers and QLM configuration.  The register is
 * only reset during COLD boot.  It should only be modified only
 * while SRIO()_STATUS_REG[ACCESS] is zero.
 *
 * This register is reset by the coprocessor-clock cold reset.
 */
union cvmx_sriox_ip_feature {
	uint64_t u64;
	struct cvmx_sriox_ip_feature_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ops                          : 32; /**< Reset value for the OPs fields in both SRIOMAINT()_SRC_OPS and
                                                         SRIOMAINT()_DST_OPS. */
	uint64_t reserved_15_31               : 17;
	uint64_t no_vmin                      : 1;  /**< Lane sync valid minimum count disable.
                                                         0 = Wait for 2^12 valid codewords and at least
                                                         127 comma characters before starting
                                                         alignment.
                                                         1 = Wait only for 127 comma characters before
                                                         starting alignment. (SRIO V1.3 Compatable). */
	uint64_t a66                          : 1;  /**< 66-bit address support.  Value for bit 2 of the
                                                         EX_ADDR field in the SRIOMAINT()_PE_FEAT register. */
	uint64_t a50                          : 1;  /**< 50-bit address support.  Value for bit 1 of the
                                                         EX_ADDR field in the SRIOMAINT()_PE_FEAT register. */
	uint64_t reserved_11_11               : 1;
	uint64_t tx_flow                      : 1;  /**< Reset value for SRIOMAINT()_IR_BUFFER_CONFIG[TX_FLOW]. */
	uint64_t pt_width                     : 2;  /**< Reset value for SRIOMAINT()_PORT_0_CTL[PT_WIDTH]. */
	uint64_t tx_pol                       : 4;  /**< TX SerDes polarity lanes 3-0.
                                                         0 = Normal operation.
                                                         1 = Invert, Swap +/- Tx SERDES pins. */
	uint64_t rx_pol                       : 4;  /**< RX SerDes polarity lanes 3-0.
                                                         0 = Normal operation.
                                                         1 = Invert, Swap +/- Rx SERDES pins. */
#else
	uint64_t rx_pol                       : 4;
	uint64_t tx_pol                       : 4;
	uint64_t pt_width                     : 2;
	uint64_t tx_flow                      : 1;
	uint64_t reserved_11_11               : 1;
	uint64_t a50                          : 1;
	uint64_t a66                          : 1;
	uint64_t no_vmin                      : 1;
	uint64_t reserved_15_31               : 17;
	uint64_t ops                          : 32;
#endif
	} s;
	struct cvmx_sriox_ip_feature_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ops                          : 32; /**< Reset Value for the OPs fields in both the
                                                         SRIOMAINT(0..1)_SRC_OPS and SRIOMAINT(0..1)_DST_OPS
                                                         registers. */
	uint64_t reserved_14_31               : 18;
	uint64_t a66                          : 1;  /**< 66-bit Address Support.  Value for bit 2 of the
                                                         EX_ADDR field in the SRIOMAINT(0..1)_PE_FEAT register. */
	uint64_t a50                          : 1;  /**< 50-bit Address Support.  Value for bit 1 of the
                                                         EX_ADDR field in the SRIOMAINT(0..1)_PE_FEAT register. */
	uint64_t reserved_11_11               : 1;
	uint64_t tx_flow                      : 1;  /**< Reset Value for the TX_FLOW field in the
                                                         SRIOMAINT(0..1)_IR_BUFFER_CONFIG register.
                                                         Pass 2 will Reset to 1 when RTL ready.
                                                         (TX flow control not supported in pass 1) */
	uint64_t pt_width                     : 2;  /**< Value for the PT_WIDTH field in the
                                                         SRIOMAINT(0..1)_PORT_0_CTL register.
                                                         Reset to 0x2 rather than 0x3 in pass 1 (2 lane
                                                         interface supported in pass 1). */
	uint64_t tx_pol                       : 4;  /**< TX Serdes Polarity Lanes 3-0
                                                         0 = Normal Operation
                                                         1 = Invert, Swap +/- Tx SERDES Pins */
	uint64_t rx_pol                       : 4;  /**< RX Serdes Polarity Lanes 3-0
                                                         0 = Normal Operation
                                                         1 = Invert, Swap +/- Rx SERDES Pins */
#else
	uint64_t rx_pol                       : 4;
	uint64_t tx_pol                       : 4;
	uint64_t pt_width                     : 2;
	uint64_t tx_flow                      : 1;
	uint64_t reserved_11_11               : 1;
	uint64_t a50                          : 1;
	uint64_t a66                          : 1;
	uint64_t reserved_14_31               : 18;
	uint64_t ops                          : 32;
#endif
	} cn63xx;
	struct cvmx_sriox_ip_feature_cn63xx   cn63xxp1;
	struct cvmx_sriox_ip_feature_s        cn66xx;
	struct cvmx_sriox_ip_feature_s        cnf75xx;
};
typedef union cvmx_sriox_ip_feature cvmx_sriox_ip_feature_t;

/**
 * cvmx_srio#_mac_buffers
 *
 * Register displays errors status for each of the eight
 * RX and TX buffers and controls use of the buffer in
 * future operations.  It also displays the number of RX
 * and TX buffers currently used by the MAC.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_mac_buffers {
	uint64_t u64;
	struct cvmx_sriox_mac_buffers_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t tx_enb                       : 8;  /**< TX buffer enable.  Each bit enables a specific TX
                                                         buffer.  At least 2 of these bits must be set for
                                                         proper operation.  These bits must be cleared to
                                                         and then set again to reuse the buffer after an
                                                         error occurs. */
	uint64_t reserved_44_47               : 4;
	uint64_t tx_inuse                     : 4;  /**< Number of TX buffers containing packets waiting
                                                         to be transmitted or to be acknowledged. */
	uint64_t tx_stat                      : 8;  /**< Errors detected in main SRIO transmit buffers.
                                                         CRC error detected in buffer sets the corresponding bit
                                                         until the corresponding TX_ENB is disabled.  Each
                                                         bit set causes the SRIO()_INT_REG[MAC_BUF]
                                                         interrupt. */
	uint64_t reserved_24_31               : 8;
	uint64_t rx_enb                       : 8;  /**< RX buffer enable.  Each bit enables a specific RX
                                                         Buffer.  At least 2 of these bits must be set for
                                                         proper operation.  These bits must be cleared to
                                                         and then set again to reuse the buffer after an
                                                         error occurs. */
	uint64_t reserved_12_15               : 4;
	uint64_t rx_inuse                     : 4;  /**< Number of RX buffers containing valid packets
                                                         waiting to be processed by the logical layer. */
	uint64_t rx_stat                      : 8;  /**< Errors detected in main SRIO receive buffers.  CRC
                                                         error detected in buffer sets the corresponding bit
                                                         until the corresponding RX_ENB is disabled.  Each
                                                         bit set causes the SRIO()_INT_REG[MAC_BUF]
                                                         interrupt. */
#else
	uint64_t rx_stat                      : 8;
	uint64_t rx_inuse                     : 4;
	uint64_t reserved_12_15               : 4;
	uint64_t rx_enb                       : 8;
	uint64_t reserved_24_31               : 8;
	uint64_t tx_stat                      : 8;
	uint64_t tx_inuse                     : 4;
	uint64_t reserved_44_47               : 4;
	uint64_t tx_enb                       : 8;
	uint64_t reserved_56_63               : 8;
#endif
	} s;
	struct cvmx_sriox_mac_buffers_s       cn63xx;
	struct cvmx_sriox_mac_buffers_s       cn66xx;
	struct cvmx_sriox_mac_buffers_s       cnf75xx;
};
typedef union cvmx_sriox_mac_buffers cvmx_sriox_mac_buffers_t;

/**
 * cvmx_srio#_maint_op
 *
 * This register allows write access to the local SRIOMAINT registers.
 * A write to this register posts a read or write operation selected
 * by the OP bit to the local SRIOMAINT register selected by ADDR.
 * This write also sets the PENDING bit.  The PENDING bit is cleared by
 * hardware when the operation is complete.  The MAINT_OP Interrupt is
 * also set as the PENDING bit is cleared.  While this bit is set,
 * additional writes to this register stall the RSL.  The FAIL bit is
 * set with the clearing of the PENDING bit when an illegal address is
 * selected. WR_DATA is used only during write operations.  Only
 * 32-bit maintenance operations are supported.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_maint_op {
	uint64_t u64;
	struct cvmx_sriox_maint_op_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wr_data                      : 32; /**< Write data <31:0>. */
	uint64_t reserved_27_31               : 5;
	uint64_t fail                         : 1;  /**< Maintenance operation address error. */
	uint64_t pending                      : 1;  /**< Maintenance operation pending. */
	uint64_t op                           : 1;  /**< Operation. 0=Read, 1=Write. */
	uint64_t addr                         : 24; /**< Address. Addr[1:0] are ignored. */
#else
	uint64_t addr                         : 24;
	uint64_t op                           : 1;
	uint64_t pending                      : 1;
	uint64_t fail                         : 1;
	uint64_t reserved_27_31               : 5;
	uint64_t wr_data                      : 32;
#endif
	} s;
	struct cvmx_sriox_maint_op_s          cn63xx;
	struct cvmx_sriox_maint_op_s          cn63xxp1;
	struct cvmx_sriox_maint_op_s          cn66xx;
	struct cvmx_sriox_maint_op_s          cnf75xx;
};
typedef union cvmx_sriox_maint_op cvmx_sriox_maint_op_t;

/**
 * cvmx_srio#_maint_rd_data
 *
 * This register allows read access of the local SRIOMAINT registers.  A write to the
 * SRIO()_MAINT_OP register with the OP bit set to zero initiates a read request and
 * clears the VALID bit.  The resulting read is returned here and the VALID bit is set.
 * Access to the register will not stall the RSL but the VALID bit should be read.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_maint_rd_data {
	uint64_t u64;
	struct cvmx_sriox_maint_rd_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t valid                        : 1;  /**< Read data valid. */
	uint64_t rd_data                      : 32; /**< Read data[31:0]. */
#else
	uint64_t rd_data                      : 32;
	uint64_t valid                        : 1;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_sriox_maint_rd_data_s     cn63xx;
	struct cvmx_sriox_maint_rd_data_s     cn63xxp1;
	struct cvmx_sriox_maint_rd_data_s     cn66xx;
	struct cvmx_sriox_maint_rd_data_s     cnf75xx;
};
typedef union cvmx_sriox_maint_rd_data cvmx_sriox_maint_rd_data_t;

/**
 * cvmx_srio#_mce_tx_ctl
 *
 * Writes to this register cause the SRIO device to generate a multicast event.
 * Setting the MCE bit requests the logic to generate the multicast event symbol.
 * Reading the bit shows the status of the transmit event.  The hardware will
 * clear the bit when the event has been transmitted and set the MCE_TX Interrupt.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_mce_tx_ctl {
	uint64_t u64;
	struct cvmx_sriox_mce_tx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t mce                          : 1;  /**< Multicast event transmit. */
#else
	uint64_t mce                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_sriox_mce_tx_ctl_s        cn63xx;
	struct cvmx_sriox_mce_tx_ctl_s        cn63xxp1;
	struct cvmx_sriox_mce_tx_ctl_s        cn66xx;
	struct cvmx_sriox_mce_tx_ctl_s        cnf75xx;
};
typedef union cvmx_sriox_mce_tx_ctl cvmx_sriox_mce_tx_ctl_t;

/**
 * cvmx_srio#_mem_op_ctrl
 *
 * This register is used to control memory operations.  Bits are provided to override
 * the priority of the outgoing responses to memory operations.  The memory operations
 * with responses include NREAD, NWRITE_R, ATOMIC_INC, ATOMIC_DEC, ATOMIC_SET and ATOMIC_CLR.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_mem_op_ctrl {
	uint64_t u64;
	struct cvmx_sriox_mem_op_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t rr_ro                        : 1;  /**< Read response relaxed ordering.  Controls ordering
                                                         rules for incoming memory operations.
                                                         0 = Normal ordering.
                                                         1 = Relaxed ordering. */
	uint64_t w_ro                         : 1;  /**< Write relaxed ordering.  Controls ordering rules
                                                         for incoming memory operations.
                                                         0 = Normal ordering.
                                                         1 = Relaxed ordering. */
	uint64_t reserved_6_7                 : 2;
	uint64_t rp1_sid                      : 1;  /**< Sets response priority for incomimg memory ops
                                                         of priority 1 on the secondary ID (0=2, 1=3). */
	uint64_t rp0_sid                      : 2;  /**< Sets response priority for incomimg memory ops
                                                         of priority 0 on the secondary ID (0,1=1 2=2, 3=3). */
	uint64_t rp1_pid                      : 1;  /**< Sets response priority for incomimg memory ops
                                                         of priority 1 on the primary ID (0=2, 1=3). */
	uint64_t rp0_pid                      : 2;  /**< Sets response priority for incomimg memory ops
                                                         of priority 0 on the primary ID (0,1=1 2=2, 3=3). */
#else
	uint64_t rp0_pid                      : 2;
	uint64_t rp1_pid                      : 1;
	uint64_t rp0_sid                      : 2;
	uint64_t rp1_sid                      : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t w_ro                         : 1;
	uint64_t rr_ro                        : 1;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_sriox_mem_op_ctrl_s       cn63xx;
	struct cvmx_sriox_mem_op_ctrl_s       cn63xxp1;
	struct cvmx_sriox_mem_op_ctrl_s       cn66xx;
	struct cvmx_sriox_mem_op_ctrl_s       cnf75xx;
};
typedef union cvmx_sriox_mem_op_ctrl cvmx_sriox_mem_op_ctrl_t;

/**
 * cvmx_srio#_omsg_ctrl#
 *
 * 1) If IDM_TT, IDM_SIS, and IDM_DID are all clear, then the "ID match" will always be false.
 *
 * 2) LTTR_SP and LTTR_MP must be nonzero at all times, otherwise the message output queue can
 * get blocked.
 *
 * 3) TESTMODE has no function on controller 1.
 *
 * 4) When IDM_TT=0, it is possible for an ID match to match an 8-bit DID with a 16-bit DID SRIO
 * zero-extends all 8-bit DID's, and the DID comparisons are always 16-bits.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_omsg_ctrlx {
	uint64_t u64;
	struct cvmx_sriox_omsg_ctrlx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t testmode                     : 1;  /**< Reserved. */
	uint64_t reserved_37_62               : 26;
	uint64_t silo_max                     : 5;  /**< Sets max number outgoing segments for this controller.
                                                         Valid range is 0x01 .. 0x10  Note that lower
                                                         values will reduce bandwidth. */
	uint64_t rtry_thr                     : 16; /**< Controller retry threshold. */
	uint64_t rtry_en                      : 1;  /**< Controller retry threshold enable. */
	uint64_t reserved_11_14               : 4;
	uint64_t idm_tt                       : 1;  /**< Controller ID match includes TT ID. */
	uint64_t idm_sis                      : 1;  /**< Controller ID match includes SIS. */
	uint64_t idm_did                      : 1;  /**< Controller ID match includes DID. */
	uint64_t lttr_sp                      : 4;  /**< Controller SP allowable letters in dynamic letter select mode (LNS). */
	uint64_t lttr_mp                      : 4;  /**< Controller MP allowable letters in dynamic letter select mode (LNS). */
#else
	uint64_t lttr_mp                      : 4;
	uint64_t lttr_sp                      : 4;
	uint64_t idm_did                      : 1;
	uint64_t idm_sis                      : 1;
	uint64_t idm_tt                       : 1;
	uint64_t reserved_11_14               : 4;
	uint64_t rtry_en                      : 1;
	uint64_t rtry_thr                     : 16;
	uint64_t silo_max                     : 5;
	uint64_t reserved_37_62               : 26;
	uint64_t testmode                     : 1;
#endif
	} s;
	struct cvmx_sriox_omsg_ctrlx_s        cn63xx;
	struct cvmx_sriox_omsg_ctrlx_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t testmode                     : 1;  /**< Controller X test mode (keep as RSVD in HRM) */
	uint64_t reserved_32_62               : 31;
	uint64_t rtry_thr                     : 16; /**< Controller X Retry threshold */
	uint64_t rtry_en                      : 1;  /**< Controller X Retry threshold enable */
	uint64_t reserved_11_14               : 4;
	uint64_t idm_tt                       : 1;  /**< Controller X ID match includes TT ID */
	uint64_t idm_sis                      : 1;  /**< Controller X ID match includes SIS */
	uint64_t idm_did                      : 1;  /**< Controller X ID match includes DID */
	uint64_t lttr_sp                      : 4;  /**< Controller X SP allowable letters in dynamic
                                                         letter select mode (LNS) */
	uint64_t lttr_mp                      : 4;  /**< Controller X MP allowable letters in dynamic
                                                         letter select mode (LNS) */
#else
	uint64_t lttr_mp                      : 4;
	uint64_t lttr_sp                      : 4;
	uint64_t idm_did                      : 1;
	uint64_t idm_sis                      : 1;
	uint64_t idm_tt                       : 1;
	uint64_t reserved_11_14               : 4;
	uint64_t rtry_en                      : 1;
	uint64_t rtry_thr                     : 16;
	uint64_t reserved_32_62               : 31;
	uint64_t testmode                     : 1;
#endif
	} cn63xxp1;
	struct cvmx_sriox_omsg_ctrlx_s        cn66xx;
	struct cvmx_sriox_omsg_ctrlx_s        cnf75xx;
};
typedef union cvmx_sriox_omsg_ctrlx cvmx_sriox_omsg_ctrlx_t;

/**
 * cvmx_srio#_omsg_done_counts#
 *
 * This register shows the number of successful and unsuccessful outgoing messages issued
 * through this controller.  The only messages considered are the ones with
 * SRIO_OMSG_HDR_S[INTR] set.  This register is typically not written while Outbound
 * SRIO Memory traffic is enabled.  The sum of the GOOD and BAD counts should equal the
 * number of messages sent unless the MAC has been reset.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_omsg_done_countsx {
	uint64_t u64;
	struct cvmx_sriox_omsg_done_countsx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t bad                          : 16; /**< Number of outbound messages with SRIO_OMSG_HDR_S[INTR] set that
                                                         did not increment GOOD. (One or more segment of the
                                                         message either timed out, reached the retry limit,
                                                         or received an ERROR response.) */
	uint64_t good                         : 16; /**< Number of outbound messages with SRIO_OMSG_HDR_S[INTR] set that
                                                         received a DONE response for every segment. */
#else
	uint64_t good                         : 16;
	uint64_t bad                          : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sriox_omsg_done_countsx_s cn63xx;
	struct cvmx_sriox_omsg_done_countsx_s cn66xx;
	struct cvmx_sriox_omsg_done_countsx_s cnf75xx;
};
typedef union cvmx_sriox_omsg_done_countsx cvmx_sriox_omsg_done_countsx_t;

/**
 * cvmx_srio#_omsg_fmp_mr#
 *
 * This CSR controls when FMP candidate message segments (from the two different controllers)
 * can enter the message segment silo to be sent out. A segment remains in the silo until after
 * is has been transmitted and either acknowledged or errored out.
 * Candidates and silo entries are one of 4 types:
 *
 * _ SP  a single-segment message.
 *
 * _ FMP the first segment of a multi-segment message.
 *
 * _ NMP the other segments in a multi-segment message.
 *
 * _ PSD the silo psuedo-entry that is valid only while a controller is in the middle of pushing
 *   a multi-segment message into the silo and can match against segments generated by
 *   the other controller
 *
 * When a candidate "matches" against a silo entry or pseudo entry, it cannot enter the
 * silo.  By default (i.e. zeroes in this CSR), the FMP candidate matches against all
 * entries in the silo. When fields in this CSR are set, FMP candidate segments will
 * match fewer silo entries and can enter the silo more freely, probably providing
 * better performance.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_omsg_fmp_mrx {
	uint64_t u64;
	struct cvmx_sriox_omsg_fmp_mrx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t ctlr_sp                      : 1;  /**< Controller FIRSTMP enable controller SP.
                                                         When set, the FMP candidate message segment can
                                                         only match siloed SP segments that were created
                                                         by the same controller. When clear, this FMP-SP
                                                         match can also occur when the segments were
                                                         created by the other controller.
                                                         Not used by the hardware when ALL_SP is set. */
	uint64_t ctlr_fmp                     : 1;  /**< Controller FIRSTMP enable controller FIRSTMP.
                                                         When set, the FMP candidate message segment can
                                                         only match siloed FMP segments that were created
                                                         by the same controller. When clear, this FMP-FMP
                                                         match can also occur when the segments were
                                                         created by the other controller.
                                                         Not used by the hardware when ALL_FMP is set. */
	uint64_t ctlr_nmp                     : 1;  /**< Controller FIRSTMP enable controller NFIRSTMP.
                                                         When set, the FMP candidate message segment can
                                                         only match siloed NMP segments that were created
                                                         by the same controller. When clear, this FMP-NMP
                                                         match can also occur when the segments were
                                                         created by the other controller.
                                                         Not used by the hardware when ALL_NMP is set. */
	uint64_t id_sp                        : 1;  /**< Controller FIRSTMP enable ID SP.
                                                         When set, the FMP candidate message segment can
                                                         only match siloed SP segments that "ID match" the
                                                         candidate. When clear, this FMP-SP match can occur
                                                         with any ID values.
                                                         Not used by the hardware when ALL_SP is set. */
	uint64_t id_fmp                       : 1;  /**< Controller FIRSTMP enable ID FIRSTMP.
                                                         When set, the FMP candidate message segment can
                                                         only match siloed FMP segments that "ID match" the
                                                         candidate. When clear, this FMP-FMP match can occur
                                                         with any ID values.
                                                         Not used by the hardware when ALL_FMP is set. */
	uint64_t id_nmp                       : 1;  /**< Controller FIRSTMP enable ID NFIRSTMP.
                                                         When set, the FMP candidate message segment can
                                                         only match siloed NMP segments that "ID match" the
                                                         candidate. When clear, this FMP-NMP match can occur
                                                         with any ID values.
                                                         Not used by the hardware when ALL_NMP is set. */
	uint64_t id_psd                       : 1;  /**< Controller FIRSTMP enable ID PSEUDO.
                                                         When set, the FMP candidate message segment can
                                                         only match the silo pseudo (for the other
                                                         controller) when it is an "ID match". When clear,
                                                         this FMP-PSD match can occur with any ID values.
                                                         Not used by the hardware when ALL_PSD is set. */
	uint64_t mbox_sp                      : 1;  /**< Controller FIRSTMP enable MBOX SP.
                                                         When set, the FMP candidate message segment can
                                                         only match siloed SP segments with the same 2-bit
                                                         mbox value as the candidate. When clear, this
                                                         FMP-SP match can occur with any mbox values.
                                                         Not used by the hardware when ALL_SP is set. */
	uint64_t mbox_fmp                     : 1;  /**< Controller FIRSTMP enable MBOX FIRSTMP.
                                                         When set, the FMP candidate message segment can
                                                         only match siloed FMP segments with the same 2-bit
                                                         mbox value as the candidate. When clear, this
                                                         FMP-FMP match can occur with any mbox values.
                                                         Not used by the hardware when ALL_FMP is set. */
	uint64_t mbox_nmp                     : 1;  /**< Controller FIRSTMP enable MBOX NFIRSTMP.
                                                         When set, the FMP candidate message segment can
                                                         only match siloed NMP segments with the same 2-bit
                                                         mbox value as the candidate. When clear, this
                                                         FMP-NMP match can occur with any mbox values.
                                                         Not used by the hardware when ALL_NMP is set. */
	uint64_t mbox_psd                     : 1;  /**< Controller FIRSTMP enable MBOX PSEUDO.
                                                         When set, the FMP candidate message segment can
                                                         only match the silo pseudo (for the other
                                                         controller) if the pseudo has the same 2-bit mbox
                                                         value as the candidate. When clear, this FMP-PSD
                                                         match can occur with any mbox values.
                                                         Not used by the hardware when ALL_PSD is set. */
	uint64_t all_sp                       : 1;  /**< Controller FIRSTMP enable all SP.
                                                         When set, no FMP candidate message segments ever
                                                         match siloed SP segments and ID_SP
                                                         and MBOX_SP are not used. When clear, FMP-SP
                                                         matches can occur. */
	uint64_t all_fmp                      : 1;  /**< Controller FIRSTMP enable all FIRSTMP.
                                                         When set, no FMP candidate message segments ever
                                                         match siloed FMP segments and ID_FMP and MBOX_FMP
                                                         are not used. When clear, FMP-FMP matches can
                                                         occur. */
	uint64_t all_nmp                      : 1;  /**< Controller FIRSTMP enable all NFIRSTMP.
                                                         When set, no FMP candidate message segments ever
                                                         match siloed NMP segments and ID_NMP and MBOX_NMP
                                                         are not used. When clear, FMP-NMP matches can
                                                         occur. */
	uint64_t all_psd                      : 1;  /**< Controller FIRSTMP enable all PSEUDO.
                                                         When set, no FMP candidate message segments ever
                                                         match the silo pseudo (for the other controller)
                                                         and ID_PSD and MBOX_PSD are not used. When clear,
                                                         FMP-PSD matches can occur. */
#else
	uint64_t all_psd                      : 1;
	uint64_t all_nmp                      : 1;
	uint64_t all_fmp                      : 1;
	uint64_t all_sp                       : 1;
	uint64_t mbox_psd                     : 1;
	uint64_t mbox_nmp                     : 1;
	uint64_t mbox_fmp                     : 1;
	uint64_t mbox_sp                      : 1;
	uint64_t id_psd                       : 1;
	uint64_t id_nmp                       : 1;
	uint64_t id_fmp                       : 1;
	uint64_t id_sp                        : 1;
	uint64_t ctlr_nmp                     : 1;
	uint64_t ctlr_fmp                     : 1;
	uint64_t ctlr_sp                      : 1;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_sriox_omsg_fmp_mrx_s      cn63xx;
	struct cvmx_sriox_omsg_fmp_mrx_s      cn63xxp1;
	struct cvmx_sriox_omsg_fmp_mrx_s      cn66xx;
	struct cvmx_sriox_omsg_fmp_mrx_s      cnf75xx;
};
typedef union cvmx_sriox_omsg_fmp_mrx cvmx_sriox_omsg_fmp_mrx_t;

/**
 * cvmx_srio#_omsg_nmp_mr#
 *
 * This CSR controls when NMP candidate message segments (from the two different controllers)
 * can enter the message segment silo to be sent out. A segment remains in the silo until after
 * it has been transmitted and either acknowledged or errored out.
 * Candidates and silo entries are one of 4 types:
 *
 * _ SP  a single-segment message.
 *
 * _ FMP the first segment of a multi-segment message.
 *
 * _ NMP the other segments in a multi-segment message.
 *
 * _ PSD the silo psuedo-entry that is valid only while a controller is in the middle of pushing
 *   a multi-segment message into the silo and can match against segments generated by
 *   the other controller
 *
 * When a candidate "matches" against a silo entry or pseudo entry, it cannot enter the
 * silo.  By default (i.e. zeroes in this CSR), the NMP candidate matches against all
 * entries in the silo. When fields in this CSR are set, NMP candidate segments will
 * match fewer silo entries and can enter the silo more freely, probably providing
 * better performance.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_omsg_nmp_mrx {
	uint64_t u64;
	struct cvmx_sriox_omsg_nmp_mrx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t ctlr_sp                      : 1;  /**< Controller NFIRSTMP enable controller SP.
                                                         When set, the NMP candidate message segment can
                                                         only match siloed SP segments that were created
                                                         by the same controller. When clear, this NMP-SP
                                                         match can also occur when the segments were
                                                         created by the other controller.
                                                         Not used by the hardware when ALL_SP is set. */
	uint64_t ctlr_fmp                     : 1;  /**< Controller NFIRSTMP enable controller FIRSTMP.
                                                         When set, the NMP candidate message segment can
                                                         only match siloed FMP segments that were created
                                                         by the same controller. When clear, this NMP-FMP
                                                         match can also occur when the segments were
                                                         created by the other controller.
                                                         Not used by the hardware when ALL_FMP is set. */
	uint64_t ctlr_nmp                     : 1;  /**< Controller NFIRSTMP enable controller NFIRSTMP.
                                                         When set, the NMP candidate message segment can
                                                         only match siloed NMP segments that were created
                                                         by the same controller. When clear, this NMP-NMP
                                                         match can also occur when the segments were
                                                         created by the other controller.
                                                         Not used by the hardware when ALL_NMP is set. */
	uint64_t id_sp                        : 1;  /**< Controller NFIRSTMP enable ID SP.
                                                         When set, the NMP candidate message segment can
                                                         only match siloed SP segments that "ID match" the
                                                         candidate. When clear, this NMP-SP match can occur
                                                         with any ID values.
                                                         Not used by the hardware when ALL_SP is set. */
	uint64_t id_fmp                       : 1;  /**< Controller NFIRSTMP enable ID FIRSTMP.
                                                         When set, the NMP candidate message segment can
                                                         only match siloed FMP segments that "ID match" the
                                                         candidate. When clear, this NMP-FMP match can occur
                                                         with any ID values.
                                                         Not used by the hardware when ALL_FMP is set. */
	uint64_t id_nmp                       : 1;  /**< Controller NFIRSTMP enable ID NFIRSTMP.
                                                         When set, the NMP candidate message segment can
                                                         only match siloed NMP segments that "ID match" the
                                                         candidate. When clear, this NMP-NMP match can occur
                                                         with any ID values.
                                                         Not used by the hardware when ALL_NMP is set. */
	uint64_t reserved_8_8                 : 1;
	uint64_t mbox_sp                      : 1;  /**< Controller NFIRSTMP enable MBOX SP.
                                                         When set, the NMP candidate message segment can
                                                         only match siloed SP segments with the same 2-bit
                                                         mbox  value as the candidate. When clear, this
                                                         NMP-SP match can occur with any mbox values.
                                                         Not used by the hardware when ALL_SP is set. */
	uint64_t mbox_fmp                     : 1;  /**< Controller NFIRSTMP enable MBOX FIRSTMP.
                                                         When set, the NMP candidate message segment can
                                                         only match siloed FMP segments with the same 2-bit
                                                         mbox value as the candidate. When clear, this
                                                         NMP-FMP match can occur with any mbox values.
                                                         Not used by the hardware when ALL_FMP is set. */
	uint64_t mbox_nmp                     : 1;  /**< Controller NFIRSTMP enable MBOX NFIRSTMP.
                                                         When set, the NMP candidate message segment can
                                                         only match siloed NMP segments with the same 2-bit
                                                         mbox value as the candidate. When clear, this
                                                         NMP-NMP match can occur with any mbox values.
                                                         Not used by the hardware when ALL_NMP is set. */
	uint64_t reserved_4_4                 : 1;
	uint64_t all_sp                       : 1;  /**< Controller NFIRSTMP enable all SP.
                                                         When set, no NMP candidate message segments ever
                                                         match siloed SP segments and ID_SP
                                                         and MBOX_SP are not used. When clear, NMP-SP
                                                         matches can occur. */
	uint64_t all_fmp                      : 1;  /**< Controller NFIRSTMP enable all FIRSTMP.
                                                         When set, no NMP candidate message segments ever
                                                         match siloed FMP segments and ID_FMP and MBOX_FMP
                                                         are not used. When clear, NMP-FMP matches can
                                                         occur. */
	uint64_t all_nmp                      : 1;  /**< Controller NFIRSTMP enable all NFIRSTMP.
                                                         When set, no NMP candidate message segments ever
                                                         match siloed NMP segments and ID_NMP and MBOX_NMP
                                                         are not used. When clear, NMP-NMP matches can
                                                         occur. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t all_nmp                      : 1;
	uint64_t all_fmp                      : 1;
	uint64_t all_sp                       : 1;
	uint64_t reserved_4_4                 : 1;
	uint64_t mbox_nmp                     : 1;
	uint64_t mbox_fmp                     : 1;
	uint64_t mbox_sp                      : 1;
	uint64_t reserved_8_8                 : 1;
	uint64_t id_nmp                       : 1;
	uint64_t id_fmp                       : 1;
	uint64_t id_sp                        : 1;
	uint64_t ctlr_nmp                     : 1;
	uint64_t ctlr_fmp                     : 1;
	uint64_t ctlr_sp                      : 1;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_sriox_omsg_nmp_mrx_s      cn63xx;
	struct cvmx_sriox_omsg_nmp_mrx_s      cn63xxp1;
	struct cvmx_sriox_omsg_nmp_mrx_s      cn66xx;
	struct cvmx_sriox_omsg_nmp_mrx_s      cnf75xx;
};
typedef union cvmx_sriox_omsg_nmp_mrx cvmx_sriox_omsg_nmp_mrx_t;

/**
 * cvmx_srio#_omsg_port#
 *
 * Enable Outgoing message ports.
 *
 * This register is reset by the coprocessor-clock reset.
 */
union cvmx_sriox_omsg_portx {
	uint64_t u64;
	struct cvmx_sriox_omsg_portx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t enable                       : 1;  /**< Controller enable. */
	uint64_t reserved_3_30                : 28;
	uint64_t port                         : 3;  /**< Controller X PKO port */
#else
	uint64_t port                         : 3;
	uint64_t reserved_3_30                : 28;
	uint64_t enable                       : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sriox_omsg_portx_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t enable                       : 1;  /**< Controller X enable */
	uint64_t reserved_2_30                : 29;
	uint64_t port                         : 2;  /**< Controller X PKO port */
#else
	uint64_t port                         : 2;
	uint64_t reserved_2_30                : 29;
	uint64_t enable                       : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} cn63xx;
	struct cvmx_sriox_omsg_portx_cn63xx   cn63xxp1;
	struct cvmx_sriox_omsg_portx_s        cn66xx;
	struct cvmx_sriox_omsg_portx_cnf75xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t enable                       : 1;  /**< Controller enable. */
	uint64_t reserved_0_30                : 31;
#else
	uint64_t reserved_0_30                : 31;
	uint64_t enable                       : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} cnf75xx;
};
typedef union cvmx_sriox_omsg_portx cvmx_sriox_omsg_portx_t;

/**
 * cvmx_srio#_omsg_silo_thr
 *
 * This register limits the number of outgoing message segments in flight at a time.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_omsg_silo_thr {
	uint64_t u64;
	struct cvmx_sriox_omsg_silo_thr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t tot_silo                     : 5;  /**< Sets max number segments in flight for all
                                                         controllers.  Valid range is 0x01..0x10 but
                                                         lower values reduce bandwidth. */
#else
	uint64_t tot_silo                     : 5;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_sriox_omsg_silo_thr_s     cn63xx;
	struct cvmx_sriox_omsg_silo_thr_s     cn66xx;
	struct cvmx_sriox_omsg_silo_thr_s     cnf75xx;
};
typedef union cvmx_sriox_omsg_silo_thr cvmx_sriox_omsg_silo_thr_t;

/**
 * cvmx_srio#_omsg_sp_mr#
 *
 * This CSR controls when SP candidate message segments (from the two different controllers)
 * can enter the message segment silo to be sent out. A segment remains in the silo until
 * after is has been transmitted and either acknowledged or errored out.
 * Candidates and silo entries are one of 4 types:
 *
 * _ SP  a single-segment message
 *
 * _ FMP the first segment of a multi-segment message
 *
 * _ NMP the other segments in a multi-segment message
 *
 * _ PSD the silo psuedo-entry that is valid only while a controller is in the middle of pushing
 *   a multi-segment message into the silo and can match against segments generated by
 *   the other controller
 *
 * When a candidate "matches" against a silo entry or pseudo entry, it cannot enter the silo.
 * By default (i.e. zeroes in this CSR), the SP candidate matches against all entries in the
 * silo. When fields in this CSR are set, SP candidate segments will match fewer silo entries and
 * can enter the silo more freely, probably providing better performance.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_omsg_sp_mrx {
	uint64_t u64;
	struct cvmx_sriox_omsg_sp_mrx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t xmbox_sp                     : 1;  /**< Controller SP enable XMBOX SP.
                                                         When set, the SP candidate message can only
                                                         match siloed SP segments with the same 4-bit xmbox
                                                         value as the candidate. When clear, this SP-SP
                                                         match can occur with any xmbox values.
                                                         When XMBOX_SP is set, MBOX_SP will commonly be set.
                                                         Not used by the hardware when ALL_SP is set. */
	uint64_t ctlr_sp                      : 1;  /**< Controller SP enable controller SP.
                                                         When set, the SP candidate message can
                                                         only match siloed SP segments that were created
                                                         by the same controller. When clear, this SP-SP
                                                         match can also occur when the segments were
                                                         created by the other controller.
                                                         Not used by the hardware when ALL_SP is set. */
	uint64_t ctlr_fmp                     : 1;  /**< Controller SP enable controller FIRSTMP.
                                                         When set, the SP candidate message can
                                                         only match siloed FMP segments that were created
                                                         by the same controller. When clear, this SP-FMP
                                                         match can also occur when the segments were
                                                         created by the other controller.
                                                         Not used by the hardware when ALL_FMP is set. */
	uint64_t ctlr_nmp                     : 1;  /**< Controller SP enable controller NFIRSTMP.
                                                         When set, the SP candidate message can
                                                         only match siloed NMP segments that were created
                                                         by the same controller. When clear, this SP-NMP
                                                         match can also occur when the segments were
                                                         created by the other controller.
                                                         Not used by the hardware when ALL_NMP is set. */
	uint64_t id_sp                        : 1;  /**< Controller SP enable ID SP.
                                                         When set, the SP candidate message can
                                                         only match siloed SP segments that "ID match" the
                                                         candidate. When clear, this SP-SP match can occur
                                                         with any ID values.
                                                         Not used by the hardware when ALL_SP is set. */
	uint64_t id_fmp                       : 1;  /**< Controller SP enable ID FIRSTMP.
                                                         When set, the SP candidate message can
                                                         only match siloed FMP segments that "ID match" the
                                                         candidate. When clear, this SP-FMP match can occur
                                                         with any ID values.
                                                         Not used by the hardware when ALL_FMP is set. */
	uint64_t id_nmp                       : 1;  /**< Controller SP enable ID NFIRSTMP.
                                                         When set, the SP candidate message can
                                                         only match siloed NMP segments that "ID match" the
                                                         candidate. When clear, this SP-NMP match can occur
                                                         with any ID values.
                                                         Not used by the hardware when ALL_NMP is set. */
	uint64_t id_psd                       : 1;  /**< Controller SP enable ID PSEUDO.
                                                         When set, the SP candidate message can
                                                         only match the silo pseudo (for the other
                                                         controller) when it is an "ID match". When clear,
                                                         this SP-PSD match can occur with any ID values.
                                                         Not used by the hardware when ALL_PSD is set. */
	uint64_t mbox_sp                      : 1;  /**< Controller SP enable MBOX SP.
                                                         When set, the SP candidate message can only
                                                         match siloed SP segments with the same 2-bit mbox
                                                         value as the candidate. When clear, this SP-SP
                                                         match can occur with any mbox values.
                                                         Not used by the hardware when ALL_SP is set. */
	uint64_t mbox_fmp                     : 1;  /**< Controller SP enable MBOX FIRSTMP.
                                                         When set, the SP candidate message can only
                                                         match siloed FMP segments with the same 2-bit mbox
                                                         value as the candidate. When clear, this SP-FMP
                                                         match can occur with any mbox values.
                                                         Not used by the hardware when ALL_FMP is set. */
	uint64_t mbox_nmp                     : 1;  /**< Controller SP enable MBOX NFIRSTMP.
                                                         When set, the SP candidate message can only
                                                         match siloed NMP segments with the same 2-bit mbox
                                                         value as the candidate. When clear, this SP-NMP
                                                         match can occur with any mbox values.
                                                         Not used by the hardware when ALL_NMP is set. */
	uint64_t mbox_psd                     : 1;  /**< Controller SP enable MBOX PSEUDO.
                                                         When set, the SP candidate message can only
                                                         match the silo pseudo (for the other controller)
                                                         if the pseudo has the same 2-bit mbox value as the
                                                         candidate. When clear, this SP-PSD match can occur
                                                         with any mbox values.
                                                         Not used by the hardware when ALL_PSD is set. */
	uint64_t all_sp                       : 1;  /**< Controller SP enable all SP.
                                                         When set, no SP candidate messages ever
                                                         match siloed SP segments, and XMBOX_SP, ID_SP,
                                                         and MBOX_SP are not used. When clear, SP-SP
                                                         matches can occur. */
	uint64_t all_fmp                      : 1;  /**< Controller SP enable all FIRSTMP.
                                                         When set, no SP candidate messages ever
                                                         match siloed FMP segments and ID_FMP and MBOX_FMP
                                                         are not used. When clear, SP-FMP matches can
                                                         occur. */
	uint64_t all_nmp                      : 1;  /**< Controller SP enable all NFIRSTMP.
                                                         When set, no SP candidate messages ever
                                                         match siloed NMP segments and ID_NMP and MBOX_NMP
                                                         are not used. When clear, SP-NMP matches can
                                                         occur. */
	uint64_t all_psd                      : 1;  /**< Controller SP enable all PSEUDO.
                                                         When set, no SP candidate messages ever
                                                         match the silo pseudo (for the other controller)
                                                         and ID_PSD and MBOX_PSD are not used. When clear,
                                                         SP-PSD matches can occur. */
#else
	uint64_t all_psd                      : 1;
	uint64_t all_nmp                      : 1;
	uint64_t all_fmp                      : 1;
	uint64_t all_sp                       : 1;
	uint64_t mbox_psd                     : 1;
	uint64_t mbox_nmp                     : 1;
	uint64_t mbox_fmp                     : 1;
	uint64_t mbox_sp                      : 1;
	uint64_t id_psd                       : 1;
	uint64_t id_nmp                       : 1;
	uint64_t id_fmp                       : 1;
	uint64_t id_sp                        : 1;
	uint64_t ctlr_nmp                     : 1;
	uint64_t ctlr_fmp                     : 1;
	uint64_t ctlr_sp                      : 1;
	uint64_t xmbox_sp                     : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_sriox_omsg_sp_mrx_s       cn63xx;
	struct cvmx_sriox_omsg_sp_mrx_s       cn63xxp1;
	struct cvmx_sriox_omsg_sp_mrx_s       cn66xx;
	struct cvmx_sriox_omsg_sp_mrx_s       cnf75xx;
};
typedef union cvmx_sriox_omsg_sp_mrx cvmx_sriox_omsg_sp_mrx_t;

/**
 * cvmx_srio#_prio#_in_use
 *
 * These registers provide status information on the number of
 * read/write requests pending in the S2M priority FIFOs.
 * The information can be used to help determine when an S2M_TYPE
 * register can be reallocated.  For example, if an S2M_TYPE
 * is used N times in a DMA write operation and the DMA has
 * completed.  The register corresponding to the RD/WR_PRIOR
 * of the S2M_TYPE can be read to determine the START_CNT and
 * then can be polled to see if the END_CNT equals the START_CNT
 * or at least START_CNT+N.   These registers can be accessed
 * regardless of the value of SRIO()_STATUS_REG[ACCESS]
 * but are reset by either the MAC or Core being reset.
 *
 * This register is reset by the coprocessor-clock or h-clock reset.
 */
union cvmx_sriox_priox_in_use {
	uint64_t u64;
	struct cvmx_sriox_priox_in_use_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t end_cnt                      : 16; /**< Count of packets with S2M_TYPES completed for this priority FIFO. */
	uint64_t start_cnt                    : 16; /**< Count of packets with S2M_TYPES started for this priority FIFO. */
#else
	uint64_t start_cnt                    : 16;
	uint64_t end_cnt                      : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sriox_priox_in_use_s      cn63xx;
	struct cvmx_sriox_priox_in_use_s      cn66xx;
	struct cvmx_sriox_priox_in_use_s      cnf75xx;
};
typedef union cvmx_sriox_priox_in_use cvmx_sriox_priox_in_use_t;

/**
 * cvmx_srio#_rx_bell
 *
 * This register contains the SRIO information, device ID, transaction type
 * and priority of the incoming doorbell transaction as well as the number
 * of transactions waiting to be read.  Reading this register causes a
 * doorbell to be removed from the RX Bell FIFO and the COUNT to be decremented.
 * If the COUNT is zero then the FIFO is empty and the other fields should be
 * considered invalid.  When the FIFO is full an ERROR is automatically issued.
 * The RXBELL Interrupt can be used to detect posts to this FIFO.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_rx_bell {
	uint64_t u64;
	struct cvmx_sriox_rx_bell_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t data                         : 16; /**< Information field from received doorbell. */
	uint64_t src_id                       : 16; /**< Doorbell source device ID[15:0]. */
	uint64_t count                        : 8;  /**< RX bell FIFO count.
                                                         Count must be > 0x0 for entry to be valid. */
	uint64_t reserved_5_7                 : 3;
	uint64_t dest_id                      : 1;  /**< Destination device ID. 0=Primary, 1=Secondary. */
	uint64_t id16                         : 1;  /**< Transaction type. 0=use ID[7:0], 1=use ID[15:0]. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t id16                         : 1;
	uint64_t dest_id                      : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t count                        : 8;
	uint64_t src_id                       : 16;
	uint64_t data                         : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_sriox_rx_bell_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t data                         : 16; /**< Information field from received doorbell */
	uint64_t src_id                       : 16; /**< Doorbell Source Device ID[15:0] */
	uint64_t count                        : 8;  /**< RX Bell FIFO Count
                                                         Note:  Count must be > 0 for entry to be valid. */
	uint64_t reserved_5_7                 : 3;
	uint64_t dest_id                      : 1;  /**< Destination Device ID 0=Primary, 1=Secondary */
	uint64_t id16                         : 1;  /**< Transaction Type, 0=use ID[7:0], 1=use ID[15:0] */
	uint64_t reserved_2_2                 : 1;
	uint64_t priority                     : 2;  /**< Doorbell Priority */
#else
	uint64_t priority                     : 2;
	uint64_t reserved_2_2                 : 1;
	uint64_t id16                         : 1;
	uint64_t dest_id                      : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t count                        : 8;
	uint64_t src_id                       : 16;
	uint64_t data                         : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} cn63xx;
	struct cvmx_sriox_rx_bell_cn63xx      cn63xxp1;
	struct cvmx_sriox_rx_bell_cn63xx      cn66xx;
	struct cvmx_sriox_rx_bell_cnf75xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t data                         : 16; /**< Information field from received doorbell. */
	uint64_t src_id                       : 16; /**< Doorbell source device ID[15:0]. */
	uint64_t count                        : 8;  /**< RX bell FIFO count.
                                                         Count must be > 0x0 for entry to be valid. */
	uint64_t reserved_5_7                 : 3;
	uint64_t dest_id                      : 1;  /**< Destination device ID. 0=Primary, 1=Secondary. */
	uint64_t id16                         : 1;  /**< Transaction type. 0=use ID[7:0], 1=use ID[15:0]. */
	uint64_t reserved_2_2                 : 1;
	uint64_t prior                        : 2;  /**< Doorbell priority. */
#else
	uint64_t prior                        : 2;
	uint64_t reserved_2_2                 : 1;
	uint64_t id16                         : 1;
	uint64_t dest_id                      : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t count                        : 8;
	uint64_t src_id                       : 16;
	uint64_t data                         : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} cnf75xx;
};
typedef union cvmx_sriox_rx_bell cvmx_sriox_rx_bell_t;

/**
 * cvmx_srio#_rx_bell_ctrl
 *
 * This register is used to control the number and size of RX doorbell FIFOs.
 * The NUM_FIFO field should only be changed when SRIOMAINT()_CORE_ENABLES[DOORBELL]
 * is disabled and the FIFOs are empty or doorbells may be lost.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_rx_bell_ctrl {
	uint64_t u64;
	struct cvmx_sriox_rx_bell_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t num_fifo                     : 3;  /**< Number, size and access of RX Doorbell FIFOs.
                                                         0x0 = Single 128 Entry FIFO accessed by
                                                               SRIO()_RX_BELL and SRIO()_RX_BELL_SEQ
                                                         For all other values Doorbell FIFOs can be
                                                         accessed by reading DPI_SRIO_RX_BELL and
                                                         DPI_SRIO_RX_BELL_SEQ.
                                                         0x1 = Single 128 entry FIFO.
                                                         0x2 = Two 64 entry FIFOs.
                                                         0x3 = Four 32 entry FIFOs.
                                                         0x4 = Eight 16 entry FIFOs.
                                                         0x5 = Sixteen 8 entry FIFOs.
                                                         _ All values others reserved. */
#else
	uint64_t num_fifo                     : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_sriox_rx_bell_ctrl_s      cnf75xx;
};
typedef union cvmx_sriox_rx_bell_ctrl cvmx_sriox_rx_bell_ctrl_t;

/**
 * cvmx_srio#_rx_bell_seq
 *
 * This register contains the value of the sequence counter when the doorbell
 * was received and a shadow copy of the bell FIFO count that can be read without
 * emptying the FIFO.  This register must be read prior to SRIO()_RX_BELL to
 * guarantee that the information corresponds to the correct doorbell.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_rx_bell_seq {
	uint64_t u64;
	struct cvmx_sriox_rx_bell_seq_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t count                        : 8;  /**< RX bell fifo count.
                                                         Count must be > 0x0 for entry to be valid. */
	uint64_t seq                          : 32; /**< 32-bit sequence number associated with doorbell message. */
#else
	uint64_t seq                          : 32;
	uint64_t count                        : 8;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_sriox_rx_bell_seq_s       cn63xx;
	struct cvmx_sriox_rx_bell_seq_s       cn63xxp1;
	struct cvmx_sriox_rx_bell_seq_s       cn66xx;
	struct cvmx_sriox_rx_bell_seq_s       cnf75xx;
};
typedef union cvmx_sriox_rx_bell_seq cvmx_sriox_rx_bell_seq_t;

/**
 * cvmx_srio#_rx_status
 *
 * Debug register specifying the number of credits/responses
 * currently in use for inbound traffic.  The maximum value
 * for COMP, N_POST and POST is set in SRIO()_TLP_CREDITS.
 * When all inbound traffic has stopped the values should
 * eventually return to the maximum values.  The RTN_PR[3:1] entry
 * counts should eventually return to the reset values.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_rx_status {
	uint64_t u64;
	struct cvmx_sriox_rx_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rtn_pr3                      : 8;  /**< Number of pending priority 3 response entries. */
	uint64_t rtn_pr2                      : 8;  /**< Number of pending priority 2 response entries. */
	uint64_t rtn_pr1                      : 8;  /**< Number of pending priority 1 response entries. */
	uint64_t reserved_29_39               : 11;
	uint64_t mbox                         : 5;  /**< Credits for mailbox data used in X2P. */
	uint64_t comp                         : 8;  /**< Credits for read completions used in M2S. */
	uint64_t reserved_13_15               : 3;
	uint64_t n_post                       : 5;  /**< Credits for read requests used in M2S. */
	uint64_t post                         : 8;  /**< Credits for write request postings used in M2S. */
#else
	uint64_t post                         : 8;
	uint64_t n_post                       : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t comp                         : 8;
	uint64_t mbox                         : 5;
	uint64_t reserved_29_39               : 11;
	uint64_t rtn_pr1                      : 8;
	uint64_t rtn_pr2                      : 8;
	uint64_t rtn_pr3                      : 8;
#endif
	} s;
	struct cvmx_sriox_rx_status_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rtn_pr3                      : 8;  /**< Number of pending Priority 3 Response Entries. */
	uint64_t rtn_pr2                      : 8;  /**< Number of pending Priority 2 Response Entries. */
	uint64_t rtn_pr1                      : 8;  /**< Number of pending Priority 1 Response Entries. */
	uint64_t reserved_28_39               : 12;
	uint64_t mbox                         : 4;  /**< Credits for Mailbox Data used in M2S. */
	uint64_t comp                         : 8;  /**< Credits for Read Completions used in M2S. */
	uint64_t reserved_13_15               : 3;
	uint64_t n_post                       : 5;  /**< Credits for Read Requests used in M2S. */
	uint64_t post                         : 8;  /**< Credits for Write Request Postings used in M2S. */
#else
	uint64_t post                         : 8;
	uint64_t n_post                       : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t comp                         : 8;
	uint64_t mbox                         : 4;
	uint64_t reserved_28_39               : 12;
	uint64_t rtn_pr1                      : 8;
	uint64_t rtn_pr2                      : 8;
	uint64_t rtn_pr3                      : 8;
#endif
	} cn63xx;
	struct cvmx_sriox_rx_status_cn63xx    cn63xxp1;
	struct cvmx_sriox_rx_status_cn63xx    cn66xx;
	struct cvmx_sriox_rx_status_s         cnf75xx;
};
typedef union cvmx_sriox_rx_status cvmx_sriox_rx_status_t;

/**
 * cvmx_srio#_s2m_type#
 *
 * This CSR table specifies how to convert a SLI/DPI MAC read or write
 * into sRIO operations.  Each SLI/DPI read or write access supplies a
 * 64-bit address (MACADD[63:0]), 2-bit ADDRTYPE and 2-bit endian-swap.
 * This SRIO()_S2M_TYPE* CSR description specifies a table with 16 CSRs.
 * SRIO selects one of the table entries with TYPEIDX[3:0], which it
 * creates from the SLI/DPI MAC memory space read or write as follows:
 *   TYPEIDX[1:0] = ADDRTYPE[1:0] (ADDRTYPE[1] is no-snoop to the PCIe MAC,
 *   ADDRTYPE[0] is relaxed-ordering to the PCIe MAC)
 *   TYPEIDX[2] = MACADD[50]
 *   TYPEIDX[3] = MACADD[59]
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_s2m_typex {
	uint64_t u64;
	struct cvmx_sriox_s2m_typex_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t wr_op                        : 3;  /**< Write operation.  See SRIO_WR_OP_E for details. */
	uint64_t reserved_15_15               : 1;
	uint64_t rd_op                        : 3;  /**< Read operation.  see SRIO_RD_OP_E for details. */
	uint64_t wr_prior                     : 2;  /**< Transaction priority 0-3 used for writes. */
	uint64_t rd_prior                     : 2;  /**< Transaction priority 0-3 used for reads/ATOMICs */
	uint64_t reserved_6_7                 : 2;
	uint64_t src_id                       : 1;  /**< Source ID.
                                                         0 = Use primary ID as source ID
                                                         (SRIOMAINT()_PRI_DEV_ID[ID16 or ID8], depending
                                                         on SRIO TT ID (i.e. ID16 below)).
                                                         1 = Use secondary ID as source ID
                                                         (SRIOMAINT()_SEC_DEV_ID[ID16 or ID8], depending
                                                         on SRIO TT ID (i.e. ID16 below)). */
	uint64_t id16                         : 1;  /**< SRIO TT ID 0=8bit, 1=16-bit.
                                                         IAOW_SEL must not be 2 when ID16=1. */
	uint64_t reserved_2_3                 : 2;
	uint64_t iaow_sel                     : 2;  /**< Internal address offset width select.  See SRIO_IAOW_E for details. */
#else
	uint64_t iaow_sel                     : 2;
	uint64_t reserved_2_3                 : 2;
	uint64_t id16                         : 1;
	uint64_t src_id                       : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t rd_prior                     : 2;
	uint64_t wr_prior                     : 2;
	uint64_t rd_op                        : 3;
	uint64_t reserved_15_15               : 1;
	uint64_t wr_op                        : 3;
	uint64_t reserved_19_63               : 45;
#endif
	} s;
	struct cvmx_sriox_s2m_typex_s         cn63xx;
	struct cvmx_sriox_s2m_typex_s         cn63xxp1;
	struct cvmx_sriox_s2m_typex_s         cn66xx;
	struct cvmx_sriox_s2m_typex_s         cnf75xx;
};
typedef union cvmx_sriox_s2m_typex cvmx_sriox_s2m_typex_t;

/**
 * cvmx_srio#_seq
 *
 * This register contains the current value of the sequence counter.  This counter increments
 * every time a doorbell or the first segment of a message is accepted.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_seq {
	uint64_t u64;
	struct cvmx_sriox_seq_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t seq                          : 32; /**< 32-bit sequence number. */
#else
	uint64_t seq                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sriox_seq_s               cn63xx;
	struct cvmx_sriox_seq_s               cn63xxp1;
	struct cvmx_sriox_seq_s               cn66xx;
	struct cvmx_sriox_seq_s               cnf75xx;
};
typedef union cvmx_sriox_seq cvmx_sriox_seq_t;

/**
 * cvmx_srio#_status_reg
 *
 * The SRIO field displays if the port has been configured for SRIO operation.  This register
 * can be read regardless of whether the SRIO is selected or being reset.  Although some other
 * registers can be accessed while the ACCESS bit is zero (see individual registers for details),
 * the majority of SRIO registers and all the SRIOMAINT registers can be used only when the
 * ACCESS bit is asserted.
 *
 * This register is reset by the coprocessor-clock reset.
 */
union cvmx_sriox_status_reg {
	uint64_t u64;
	struct cvmx_sriox_status_reg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t host                         : 1;  /**< SRIO host setting.  This field is initialized on a cold reset based on the
                                                         value of the corresponding SRIOx_SPD pins. If the pins are set to 15 then
                                                         the port is disabled and HOST is set otherwise it is initialized as an endpoint
                                                         (HOST is cleared).  The values in this field are used to determine
                                                         the setting in the SRIOMAINT()_PORT_GEN_CTL[HOST] field.  The value is not
                                                         modified during a warm or soft reset and should be set before SRIO bit is
                                                         enabled.
                                                          0 = SRIO port is endpoint (EP).
                                                          1 = SRIO port is host. */
	uint64_t spd                          : 4;  /**< SRIO speed setting.  This field is initialized on a cold reset based on the
                                                         value of the corresponding SRIOx_SPD pins. The values in this field are
                                                         used to determine the setting in the SRIOMAINT()_PORT_0_CTL2 register and to
                                                         the QLM PLL setting.  The value is not modified during a warm or soft reset
                                                         and should be set before SRIO bit is enabled.
                                                         <pre>
                                                          0x0 =   5.0G  100 MHz reference.
                                                          0x1 =   2.5G  100 MHz reference.
                                                          0x2 =   2.5G  100 MHz reference.
                                                          0x3 =  1.25G  100 MHz reference.
                                                          0x4 =  1.25G  156.25 MHz reference.
                                                          0x5 =  6.25G  125 MHz reference. Reserved.
                                                          0x6 =   5.0G  125 MHz reference.
                                                          0x7 =   2.5G  156.25 MHz reference.
                                                          0x8 = 3.125G  125 MHz reference.
                                                          0x9 =   2.5G  125 MHz reference.
                                                          0xA =  1.25G  125 MHz reference.
                                                          0xB =   5.0G  156.25 MHz reference.
                                                          0xC =  6.25G  156.25 MHz reference. Reserved.
                                                          0xD =  3.75G  156.25 MHz reference. Reserved.
                                                          0xE = 3.125G  156.25 MHz reference.
                                                          0xF =         Interface disabled.
                                                         </pre> */
	uint64_t run_type                     : 2;  /**< SRIO run type.  This field is initialized on a cold reset based on the
                                                         value of the corresponding SRIOx_TYPE pin.  The values in this field are
                                                         used to determine tx/rx type settings in the SRIOMAINT()_LANE_()_STATUS_0
                                                         registers.  The value is not modified during a warm or soft reset and should
                                                         be set before SRIO bit is enabled.
                                                         0x0 = Short run  (SRIO()_TYPE is 0).
                                                         0x1 = Medium run.
                                                         0x2 = Long run   (SRIO()_TYPE is 1). */
	uint64_t access                       : 1;  /**< SRIO register access.
                                                         0 = Access disabled.
                                                         1 = Access enabled. */
	uint64_t srio                         : 1;  /**< SRIO port enabled.  This bit is initialized on a cold reset based on the
                                                         value of the SRIO()_SPD pins.  If the SPD is all 1's the interface is disabled.
                                                         The value is not modified during a warm or soft reset.
                                                         0 = All SRIO functions disabled.
                                                         1 = All SRIO operations permitted. */
#else
	uint64_t srio                         : 1;
	uint64_t access                       : 1;
	uint64_t run_type                     : 2;
	uint64_t spd                          : 4;
	uint64_t host                         : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_sriox_status_reg_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t access                       : 1;  /**< SRIO and SRIOMAINT Register Access.
                                                         0 - Register Access Disabled.
                                                         1 - Register Access Enabled. */
	uint64_t srio                         : 1;  /**< SRIO Port Enabled.
                                                         0 - All SRIO functions disabled.
                                                         1 - All SRIO Operations permitted. */
#else
	uint64_t srio                         : 1;
	uint64_t access                       : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} cn63xx;
	struct cvmx_sriox_status_reg_cn63xx   cn63xxp1;
	struct cvmx_sriox_status_reg_cn63xx   cn66xx;
	struct cvmx_sriox_status_reg_s        cnf75xx;
};
typedef union cvmx_sriox_status_reg cvmx_sriox_status_reg_t;

/**
 * cvmx_srio#_tag_ctrl
 *
 * This register is used to show the state of the internal transaction
 * tags and provides a manual reset of the outgoing tags.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_tag_ctrl {
	uint64_t u64;
	struct cvmx_sriox_tag_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t o_clr                        : 1;  /**< Manual OTAG clear.  This bit manually resets the
                                                         number of OTAGs back to 16 and loses track of any
                                                         outgoing packets.  This function is automatically
                                                         performed when the SRIO MAC is reset but it may be
                                                         necessary after a chip reset while the MAC is in
                                                         operation.  This bit must be set then cleared to
                                                         return to normal operation.  Typically, outgoing
                                                         SRIO packets must be halted 6 seconds prior to
                                                         this bit is set to avoid generating duplicate tags
                                                         and unexpected response errors. */
	uint64_t reserved_13_15               : 3;
	uint64_t otag                         : 5;  /**< Number of available outbound tags.  Tags are
                                                         required for all outgoing memory and maintenance
                                                         operations that require a response. (Max 16). */
	uint64_t reserved_5_7                 : 3;
	uint64_t itag                         : 5;  /**< Number of available inbound tags.  Tags are
                                                         required for all incoming memory operations that
                                                         require a response. (Max 16). */
#else
	uint64_t itag                         : 5;
	uint64_t reserved_5_7                 : 3;
	uint64_t otag                         : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t o_clr                        : 1;
	uint64_t reserved_17_63               : 47;
#endif
	} s;
	struct cvmx_sriox_tag_ctrl_s          cn63xx;
	struct cvmx_sriox_tag_ctrl_s          cn63xxp1;
	struct cvmx_sriox_tag_ctrl_s          cn66xx;
	struct cvmx_sriox_tag_ctrl_s          cnf75xx;
};
typedef union cvmx_sriox_tag_ctrl cvmx_sriox_tag_ctrl_t;

/**
 * cvmx_srio#_tlp_credits
 *
 * This register is for diagnostic use only.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_tlp_credits {
	uint64_t u64;
	struct cvmx_sriox_tlp_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t mbox                         : 5;  /**< Credits for mailbox data used in X2P.
                                                         Legal values are 0x1 to 0x10. */
	uint64_t comp                         : 8;  /**< Credits for read completions used in M2S.
                                                         Default is 64 (0x40) credits per SRIO MAC.  Can be increased to 128 (0x80)
                                                         if only one SRIO MAC is used.  Legal values are 0x22 to 0x80. */
	uint64_t reserved_13_15               : 3;
	uint64_t n_post                       : 5;  /**< Credits for read requests used in M2S.
                                                         Default is 8 credits per SRIO MAC.  Can be increased to 16 (0x10)
                                                         if only one SRIO MAC is used.  Legal values are 0x4 to 0x10. */
	uint64_t post                         : 8;  /**< Credits for write request postings used in M2S.
                                                         Default is 64 (0x40) credits per SRIO MAC.  Can be increased to 128 (0x80)
                                                         if only one SRIO MAC is used.  Legal values are 0x22 to 0x80. */
#else
	uint64_t post                         : 8;
	uint64_t n_post                       : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t comp                         : 8;
	uint64_t mbox                         : 5;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_sriox_tlp_credits_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t mbox                         : 4;  /**< Credits for Mailbox Data used in M2S.
                                                         Legal values are 0x2 to 0x8. */
	uint64_t comp                         : 8;  /**< Credits for Read Completions used in M2S.
                                                         Legal values are 0x22 to 0x80. */
	uint64_t reserved_13_15               : 3;
	uint64_t n_post                       : 5;  /**< Credits for Read Requests used in M2S.
                                                         Legal values are 0x4 to 0x10. */
	uint64_t post                         : 8;  /**< Credits for Write Request Postings used in M2S.
                                                         Legal values are 0x22 to 0x80. */
#else
	uint64_t post                         : 8;
	uint64_t n_post                       : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t comp                         : 8;
	uint64_t mbox                         : 4;
	uint64_t reserved_28_63               : 36;
#endif
	} cn63xx;
	struct cvmx_sriox_tlp_credits_cn63xx  cn63xxp1;
	struct cvmx_sriox_tlp_credits_cn63xx  cn66xx;
	struct cvmx_sriox_tlp_credits_s       cnf75xx;
};
typedef union cvmx_sriox_tlp_credits cvmx_sriox_tlp_credits_t;

/**
 * cvmx_srio#_tx_bell
 *
 * This register specifies SRIO information, device ID, transaction type and
 * priority of the outgoing doorbell transaction.  Writes to this register
 * cause the doorbell to be issued using these bits.  The write also causes the
 * PENDING bit to be set. The hardware automatically clears bit when the
 * doorbell operation has been acknowledged.  A write to this register while
 * the PENDING bit is set should be avoided as it will stall the RSL until
 * the first doorbell has completed.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_tx_bell {
	uint64_t u64;
	struct cvmx_sriox_tx_bell_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t data                         : 16; /**< Information field for next doorbell operation */
	uint64_t dest_id                      : 16; /**< Doorbell destination device ID[15:0]. */
	uint64_t reserved_9_15                : 7;
	uint64_t pending                      : 1;  /**< Doorbell transmit in progress. */
	uint64_t reserved_5_7                 : 3;
	uint64_t src_id                       : 1;  /**< Source device ID. 0=primary, 1=secondary. */
	uint64_t id16                         : 1;  /**< Transaction type: 0=use ID[7:0], 1=use ID[15:0]. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t id16                         : 1;
	uint64_t src_id                       : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t pending                      : 1;
	uint64_t reserved_9_15                : 7;
	uint64_t dest_id                      : 16;
	uint64_t data                         : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_sriox_tx_bell_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t data                         : 16; /**< Information field for next doorbell operation */
	uint64_t dest_id                      : 16; /**< Doorbell Destination Device ID[15:0] */
	uint64_t reserved_9_15                : 7;
	uint64_t pending                      : 1;  /**< Doorbell Transmit in Progress */
	uint64_t reserved_5_7                 : 3;
	uint64_t src_id                       : 1;  /**< Source Device ID 0=Primary, 1=Secondary */
	uint64_t id16                         : 1;  /**< Transaction Type, 0=use ID[7:0], 1=use ID[15:0] */
	uint64_t reserved_2_2                 : 1;
	uint64_t priority                     : 2;  /**< Doorbell Priority */
#else
	uint64_t priority                     : 2;
	uint64_t reserved_2_2                 : 1;
	uint64_t id16                         : 1;
	uint64_t src_id                       : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t pending                      : 1;
	uint64_t reserved_9_15                : 7;
	uint64_t dest_id                      : 16;
	uint64_t data                         : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} cn63xx;
	struct cvmx_sriox_tx_bell_cn63xx      cn63xxp1;
	struct cvmx_sriox_tx_bell_cn63xx      cn66xx;
	struct cvmx_sriox_tx_bell_cnf75xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t data                         : 16; /**< Information field for next doorbell operation */
	uint64_t dest_id                      : 16; /**< Doorbell destination device ID[15:0]. */
	uint64_t reserved_9_15                : 7;
	uint64_t pending                      : 1;  /**< Doorbell transmit in progress. */
	uint64_t reserved_5_7                 : 3;
	uint64_t src_id                       : 1;  /**< Source device ID. 0=primary, 1=secondary. */
	uint64_t id16                         : 1;  /**< Transaction type: 0=use ID[7:0], 1=use ID[15:0]. */
	uint64_t reserved_2_2                 : 1;
	uint64_t prior                        : 2;  /**< Doorbell priority. */
#else
	uint64_t prior                        : 2;
	uint64_t reserved_2_2                 : 1;
	uint64_t id16                         : 1;
	uint64_t src_id                       : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t pending                      : 1;
	uint64_t reserved_9_15                : 7;
	uint64_t dest_id                      : 16;
	uint64_t data                         : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} cnf75xx;
};
typedef union cvmx_sriox_tx_bell cvmx_sriox_tx_bell_t;

/**
 * cvmx_srio#_tx_bell_info
 *
 * This register is only updated if the BELL_ERR bit is clear in SRIO()_INT_REG.
 * This register displays SRIO information, device ID, transaction type and
 * priority of the doorbell transaction that generated the BELL_ERR interrupt.
 * The register includes either a RETRY, ERROR or TIMEOUT Status.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_tx_bell_info {
	uint64_t u64;
	struct cvmx_sriox_tx_bell_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t data                         : 16; /**< Information field from last doorbell operation. */
	uint64_t dest_id                      : 16; /**< Doorbell destination device ID[15:0]. */
	uint64_t reserved_8_15                : 8;
	uint64_t timeout                      : 1;  /**< Transmit doorbell failed with timeout. */
	uint64_t error                        : 1;  /**< Transmit doorbell destination returned error. */
	uint64_t retry                        : 1;  /**< Transmit doorbell requests a retransmission. */
	uint64_t src_id                       : 1;  /**< Source device ID. 0=primary, 1=secondary. */
	uint64_t id16                         : 1;  /**< Transaction type: 0=use ID[7:0], 1=use ID[15:0]. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t id16                         : 1;
	uint64_t src_id                       : 1;
	uint64_t retry                        : 1;
	uint64_t error                        : 1;
	uint64_t timeout                      : 1;
	uint64_t reserved_8_15                : 8;
	uint64_t dest_id                      : 16;
	uint64_t data                         : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_sriox_tx_bell_info_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t data                         : 16; /**< Information field from last doorbell operation */
	uint64_t dest_id                      : 16; /**< Doorbell Destination Device ID[15:0] */
	uint64_t reserved_8_15                : 8;
	uint64_t timeout                      : 1;  /**< Transmit Doorbell Failed with Timeout. */
	uint64_t error                        : 1;  /**< Transmit Doorbell Destination returned Error. */
	uint64_t retry                        : 1;  /**< Transmit Doorbell Requests a retransmission. */
	uint64_t src_id                       : 1;  /**< Source Device ID 0=Primary, 1=Secondary */
	uint64_t id16                         : 1;  /**< Transaction Type, 0=use ID[7:0], 1=use ID[15:0] */
	uint64_t reserved_2_2                 : 1;
	uint64_t priority                     : 2;  /**< Doorbell Priority */
#else
	uint64_t priority                     : 2;
	uint64_t reserved_2_2                 : 1;
	uint64_t id16                         : 1;
	uint64_t src_id                       : 1;
	uint64_t retry                        : 1;
	uint64_t error                        : 1;
	uint64_t timeout                      : 1;
	uint64_t reserved_8_15                : 8;
	uint64_t dest_id                      : 16;
	uint64_t data                         : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} cn63xx;
	struct cvmx_sriox_tx_bell_info_cn63xx cn63xxp1;
	struct cvmx_sriox_tx_bell_info_cn63xx cn66xx;
	struct cvmx_sriox_tx_bell_info_cnf75xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t data                         : 16; /**< Information field from last doorbell operation. */
	uint64_t dest_id                      : 16; /**< Doorbell destination device ID[15:0]. */
	uint64_t reserved_8_15                : 8;
	uint64_t timeout                      : 1;  /**< Transmit doorbell failed with timeout. */
	uint64_t error                        : 1;  /**< Transmit doorbell destination returned error. */
	uint64_t retry                        : 1;  /**< Transmit doorbell requests a retransmission. */
	uint64_t src_id                       : 1;  /**< Source device ID. 0=primary, 1=secondary. */
	uint64_t id16                         : 1;  /**< Transaction type: 0=use ID[7:0], 1=use ID[15:0]. */
	uint64_t reserved_2_2                 : 1;
	uint64_t prior                        : 2;  /**< Doorbell priority. */
#else
	uint64_t prior                        : 2;
	uint64_t reserved_2_2                 : 1;
	uint64_t id16                         : 1;
	uint64_t src_id                       : 1;
	uint64_t retry                        : 1;
	uint64_t error                        : 1;
	uint64_t timeout                      : 1;
	uint64_t reserved_8_15                : 8;
	uint64_t dest_id                      : 16;
	uint64_t data                         : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} cnf75xx;
};
typedef union cvmx_sriox_tx_bell_info cvmx_sriox_tx_bell_info_t;

/**
 * cvmx_srio#_tx_ctrl
 *
 * This register is used to control SRIO outgoing packet allocation.
 * TAG_TH[2:0] set the thresholds to allow priority traffic requiring
 * responses to be queued based on the number of outgoing tags (TIDs)
 * available.  16 Tags are available.  If a priority is blocked for
 * lack of tags then all lower priority packets are also blocked
 * regardless of whether they require tags.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_tx_ctrl {
	uint64_t u64;
	struct cvmx_sriox_tx_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_53_63               : 11;
	uint64_t tag_th2                      : 5;  /**< Sets threshold for minimum number of OTAGs
                                                         required before a packet of priority 2 requiring a
                                                         response will be queued for transmission. (Max 16)
                                                         There generally should be no priority 3 request
                                                         packets which require a response/tag, so a TAG_THR
                                                         value as low as 0 is allowed. */
	uint64_t reserved_45_47               : 3;
	uint64_t tag_th1                      : 5;  /**< Sets threshold for minimum number of OTAGs
                                                         required before a packet of priority 1 requiring a
                                                         response will be queued for transmission. (Max 16)
                                                         Generally, TAG_TH1 must be > TAG_TH2 to leave OTAGs
                                                         for outgoing priority 2 (or 3) requests. */
	uint64_t reserved_37_39               : 3;
	uint64_t tag_th0                      : 5;  /**< Sets threshold for minimum number of OTAGs
                                                         required before a packet of priority 0 requiring a
                                                         response will be queued for transmission. (Max 16)
                                                         Generally, TAG_TH0 must be > TAG_TH1 to leave OTAGs
                                                         for outgoing priority 1 or 2 (or 3) requests. */
	uint64_t reserved_20_31               : 12;
	uint64_t tx_th2                       : 4;  /**< Reserved. See SRIOMAINT()_IR_BUFFER_CONFIG2. */
	uint64_t reserved_12_15               : 4;
	uint64_t tx_th1                       : 4;  /**< Reserved. See SRIOMAINT()_IR_BUFFER_CONFIG2. */
	uint64_t reserved_4_7                 : 4;
	uint64_t tx_th0                       : 4;  /**< Reserved. See SRIOMAINT()_IR_BUFFER_CONFIG2. */
#else
	uint64_t tx_th0                       : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t tx_th1                       : 4;
	uint64_t reserved_12_15               : 4;
	uint64_t tx_th2                       : 4;
	uint64_t reserved_20_31               : 12;
	uint64_t tag_th0                      : 5;
	uint64_t reserved_37_39               : 3;
	uint64_t tag_th1                      : 5;
	uint64_t reserved_45_47               : 3;
	uint64_t tag_th2                      : 5;
	uint64_t reserved_53_63               : 11;
#endif
	} s;
	struct cvmx_sriox_tx_ctrl_s           cn63xx;
	struct cvmx_sriox_tx_ctrl_s           cn63xxp1;
	struct cvmx_sriox_tx_ctrl_s           cn66xx;
	struct cvmx_sriox_tx_ctrl_s           cnf75xx;
};
typedef union cvmx_sriox_tx_ctrl cvmx_sriox_tx_ctrl_t;

/**
 * cvmx_srio#_tx_emphasis
 *
 * SRIO_TX_EMPHASIS = SRIO TX Lane Emphasis
 *
 * Controls TX Emphasis used by the SRIO SERDES
 *
 * Notes:
 * This controls the emphasis value used by the SRIO SERDES.  This register is only reset during COLD
 *  boot and may be modified regardless of the value in SRIO(0,2..3)_STATUS_REG.ACCESS.  This register is not
 *  connected to the QLM and thus has no effect.  It should not be included in the documentation.
 *
 * Clk_Rst:        SRIO(0,2..3)_TX_EMPHASIS        sclk    srst_cold_n
 */
union cvmx_sriox_tx_emphasis {
	uint64_t u64;
	struct cvmx_sriox_tx_emphasis_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t emph                         : 4;  /**< Emphasis Value used for all lanes.  Default value
                                                         is 0x0 for 1.25G b/s and 0xA for all other rates. */
#else
	uint64_t emph                         : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_sriox_tx_emphasis_s       cn63xx;
	struct cvmx_sriox_tx_emphasis_s       cn66xx;
};
typedef union cvmx_sriox_tx_emphasis cvmx_sriox_tx_emphasis_t;

/**
 * cvmx_srio#_tx_status
 *
 * Debug register specifying the number of credits/ops currently
 * in use for Outbound Traffic.  When all outbound traffic has
 * stopped the values should eventually return to the reset values.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_tx_status {
	uint64_t u64;
	struct cvmx_sriox_tx_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t s2m_pr3                      : 8;  /**< Number of pending S2M priority 3 entries. */
	uint64_t s2m_pr2                      : 8;  /**< Number of pending S2M priority 2 entries. */
	uint64_t s2m_pr1                      : 8;  /**< Number of pending S2M priority 1 entries. */
	uint64_t s2m_pr0                      : 8;  /**< Number of pending S2M priority 0 entries. */
#else
	uint64_t s2m_pr0                      : 8;
	uint64_t s2m_pr1                      : 8;
	uint64_t s2m_pr2                      : 8;
	uint64_t s2m_pr3                      : 8;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sriox_tx_status_s         cn63xx;
	struct cvmx_sriox_tx_status_s         cn63xxp1;
	struct cvmx_sriox_tx_status_s         cn66xx;
	struct cvmx_sriox_tx_status_s         cnf75xx;
};
typedef union cvmx_sriox_tx_status cvmx_sriox_tx_status_t;

/**
 * cvmx_srio#_wr_done_counts
 *
 * This register shows the number of successful and unsuccessful
 * NwriteRs issued through this MAC.  These count only considers
 * the last NwriteR generated by each store instruction.  If any
 * NwriteR in the series receives an ERROR Status then it is reported
 * in SRIOMAINT()_ERB_LT_ERR_DET[IO_ERR].  If any NwriteR does not
 * receive a response within the timeout period then it is reported in
 * SRIOMAINT()_ERB_LT_ERR_DET[PKT_TOUT].  Only errors on the last NwriteR's
 * are counted as BAD.  This register is typically not written while
 * outbound SRIO Memory traffic is enabled.
 *
 * This register is reset by the h-clock reset.
 */
union cvmx_sriox_wr_done_counts {
	uint64_t u64;
	struct cvmx_sriox_wr_done_counts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t bad                          : 16; /**< Count of the final outbound NwriteR in the series
                                                         associated with a store operation that have timed
                                                         out or received a response with an ERROR status. */
	uint64_t good                         : 16; /**< Count of the final outbound NwriteR in the series
                                                         associated with a store operation that has
                                                         received a response with a DONE status. */
#else
	uint64_t good                         : 16;
	uint64_t bad                          : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sriox_wr_done_counts_s    cn63xx;
	struct cvmx_sriox_wr_done_counts_s    cn66xx;
	struct cvmx_sriox_wr_done_counts_s    cnf75xx;
};
typedef union cvmx_sriox_wr_done_counts cvmx_sriox_wr_done_counts_t;

#endif
